#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <alipltflog.h>

#include "alisli2c.h"


/*
*  @Function name    alisli2c_get_idx

*  @brief         use the user input "idx" to look for the right name of the i2c bus device.     
*  @param      idx        the number of i2c bus device that user want to use.

*  @return       char*     the string pointer that point to the name of the i2c bus device.
*
*  @author	 Alfa Shang  <alfa.shang@alitech.com>
*  @date	        09/30/2016, Created

*  @note         the value range of the idx is 0-5
*/
static char *alisli2c_get_idx(unsigned int idx);

alisl_retcode alisli2c_open(alisli2c_attr *p_attr, alisl_handle *handle)
{
	if ((NULL == p_attr) || (NULL == handle)) {
		SL_ERR("fail to get p_attr or handle");
		return ALI_RTN_FAIL;
	}
	
	alisl_retcode ret = ALI_RTN_SUCCESS;
	alisli2c_attr *dev_attr =p_attr;
	alisli2c_handle *hdl;
	char *i2c_device_id = 0;
	
	hdl = malloc(sizeof(alisli2c_handle));	
	if (NULL == hdl) {
		SL_ERR("fail to malloc hdl!\n");
		return ALI_RTN_FAIL;
	}
	memset(hdl, 0, sizeof(alisli2c_handle));


	//look for the i2c device id for open()	
	i2c_device_id = alisli2c_get_idx(dev_attr->uc_dev_idx);                    	
    if (NULL == i2c_device_id){
		SL_ERR("fail to get the right i2c id");
		free(hdl);
       return ALI_RTN_FAIL;          
    } 
	
    //open i2c device
	hdl->fd = open(i2c_device_id,O_RDWR);
	SL_DBG("##fd:%d,i2c_device_id:%s\n",hdl->fd,i2c_device_id);
    if (hdl->fd < 0){
		SL_ERR("open i2c fail!\n");
		free(hdl);
		return ALI_RTN_FAIL;
	}

	//set the timeout, the unit is 10ms, the default is 1
	ret = ioctl(hdl->fd, I2C_TIMEOUT, I2C_DEFAULT_TIMEOUT);
	if (ret < 0){
	    SL_ERR("fail to set timeout!\n");
		free(hdl);
        return ALI_RTN_FAIL;
	}

	//set the number retry times, the default is 3
	ret = ioctl(hdl->fd, I2C_RETRIES, I2C_DEFAULT_RETRY);
	if (ret < 0){
		SL_ERR("fail to set retries!\n");
		free(hdl);
		return ALI_RTN_FAIL;
	 }

	//use the handle store the message of opened i2c device
	*handle = (void *)hdl;
	
	return ret;
}

alisl_retcode alisli2c_write_data(alisl_handle handle, unsigned int chip_addr, unsigned int sub_addr, unsigned char *buf, unsigned int buf_len, unsigned int subaddr_flag)
{
	if ((NULL == handle) || (NULL == buf) || (MAX_DATA_LENGTH< buf_len)) {
       	SL_ERR("fail to get the right input!\n");
       	return ALI_RTN_FAIL;
	}

	unsigned int i;
	alisl_retcode ret = ALI_RTN_SUCCESS;
	alisli2c_handle *hdl = (alisli2c_handle *)handle;
	unsigned char *send_data = NULL;
	unsigned int data_len = 0;

	//if the slave device has no data register, that is subaddr_flag==1, copy data in @buf to @send_data
	//and the @data_len is @sizeof(data_len).
	//if the slave device has some data register, that is subaddr_flag==0, copy sub_addr to send_data[0],
	//and copy the data in the buf to send_data[i](i>0).
	if (subaddr_flag) {
		send_data = (unsigned char *)malloc((buf_len+1) * sizeof(unsigned char));
		if (NULL == send_data) {
            SL_ERR("fail to malloc the memory for send_data!\n");
			return ALI_RTN_FAIL;
	 	}
	  
		memset(send_data, 0, buf_len+1);
	 	send_data[0] = (unsigned char)sub_addr;
	 	for(i=1;i<buf_len+1;i++){
			send_data[i] = buf[i-1];
	 	}
	 
	 	data_len = buf_len +1;
	
    }else{
       	send_data = (unsigned char *)malloc((buf_len) * sizeof(unsigned char));
	 
	 	if (NULL == send_data) {
            SL_ERR("fail to malloc the memory for send_data!\n");
			return ALI_RTN_FAIL;
		}
	 
		memset(send_data, 0, buf_len);
	 	memcpy(send_data, buf, buf_len);
	 	data_len = buf_len;
    }

	//this struct contains the whole information that sent to the slave device.
	struct i2c_rdwr_ioctl_data i2c_data;

	//set the address of slave device, that is, the slave device is mounted on i2c bus device
	if (ioctl(hdl->fd, I2C_SLAVE, chip_addr) < 0){
       	SL_ERR("fail to set i2c pin slave addr!\n");
		free(send_data);
       	return ALI_RTN_FAIL;
    	} 
	
    	//set the number of messages. while executing the write operation, only one message should be send to
    	//the slave device by the host.
	i2c_data.nmsgs = 1;
	i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));	
	if (NULL == i2c_data.msgs){
		SL_ERR("fail to malloc the memory for i2c_data->msgs!\n");
		free(send_data);
       	return ALI_RTN_FAIL;
	}
	
	//configure the message
	i2c_data.msgs[0].len = data_len;                                    //the number of bytes of the send_data
	i2c_data.msgs[0].addr = chip_addr;                               //the slave device address
	i2c_data.msgs[0].flags = WRITE;                                      //write flag
	i2c_data.msgs[0].buf = (unsigned char *)malloc(i2c_data.msgs[0].len);    //malloc buffer to store the send data
	if (NULL == i2c_data.msgs[0].buf){
		SL_ERR("fail to malloc the memory for i2c_data.msgs.buf!\n");
		free(i2c_data.msgs);
		free(send_data);
       	return ALI_RTN_FAIL;
	}
	//i2c_data.msgs[0].buf[0] = sub_addr;                  
    for (i=0; i<i2c_data.msgs[0].len; i++) {                              //the first element store the sub_addr, and the rest store the sent data
	    i2c_data.msgs[0].buf[i] = send_data[i];
    }

	for(i=0;i<i2c_data.msgs[0].len;i++)
		SL_DBG("write buff[%d]=0x%x\n",i,i2c_data.msgs[0].buf[i]);

	//call the ioctl function to send message to the slave device
	SL_DBG("fd:%d, chip_addr:0x%x, sub_addr:0x%x, data_len:%d\n", hdl->fd, i2c_data.msgs[0].addr, i2c_data.msgs[0].buf[0], i2c_data.msgs[0].len );
	ret = ioctl(hdl->fd, I2C_RDWR, (unsigned long)&i2c_data);
	if (ret < 0){
        SL_ERR("\n I2C_RDWR error1!\n");
		free(i2c_data.msgs[0].buf);	
		free(i2c_data.msgs);
		free(send_data);
		return ALI_RTN_FAIL;
	}

	SL_DBG("\n TEST I2C WRITE OVER!\n");

	free(i2c_data.msgs[0].buf);	
	free(i2c_data.msgs);
	free(send_data);  
    return ret;
}

alisl_retcode alisli2c_read_data(alisl_handle handle, unsigned int chip_addr, unsigned int sub_addr, unsigned char *buf, unsigned int buf_len, unsigned int subaddr_flag)
{
   	if ((NULL == handle) || (NULL == buf) || (MAX_DATA_LENGTH < buf_len)) {
	  	 SL_ERR("fail to get the right input!\n");
         return ALI_RTN_FAIL;
	}
	int i;
	alisl_retcode ret = ALI_RTN_SUCCESS;
	alisli2c_handle *hdl = (alisli2c_handle *)handle;
	
	//this struct contains the whole information that sent to the slave device.
	struct i2c_rdwr_ioctl_data i2c_data;

	//set the address of slave device, that is, the slave device is mounted on i2c bus device
	if(ioctl(hdl->fd, I2C_SLAVE, chip_addr) < 0){
		SL_ERR("fail to set i2c pin slave addr!\n");
		return ALI_RTN_FAIL;
    } 

    if (subaddr_flag) {
	    //set the number of messages. While executing the write operation, twi messages should be send to
	    //the slave device by the host. The first one execute the write operation to write the address for reading. 
	    //The second execute the read operation to read the data.
	    i2c_data.nmsgs = 2;
	    if ((i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg))) == NULL) {
	    	SL_ERR("fail to malloc the memory for i2c_data->msgs!\n");
           	return ALI_RTN_FAIL;
	    }
	    
	    //configure the first message
	    i2c_data.msgs[0].addr = chip_addr;                       //write address of the slave device
	    i2c_data.msgs[0].flags = WRITE;                          //write flag
	    i2c_data.msgs[0].len = 1;                                //the length of the sent data
	    if ((i2c_data.msgs[0].buf = (unsigned char *)malloc(sizeof(unsigned char))) == NULL) {
	    	SL_ERR("fail to malloc the memeory for i2c_data->msgs.buf[0]\n");	
	    	free(i2c_data.msgs);
           	return ALI_RTN_FAIL;
	    }
	    i2c_data.msgs[0].buf[0] = sub_addr;                      //write address of the data to read
	    	
	    //configure the second message
	    i2c_data.msgs[1].addr = chip_addr;                       //the address of the slave device
	    i2c_data.msgs[1].flags = I2C_M_RD;                       //read flag
	    i2c_data.msgs[1].len = buf_len;                          //the length of the sent data
	    if ((i2c_data.msgs[1].buf = (unsigned char *)malloc(i2c_data.msgs[1].len)) == NULL) {
	    	SL_ERR("fail to malloc the memeory for i2c_data->msgs.buf[1]");	
	    	free(i2c_data.msgs[0].buf);	
	    	free(i2c_data.msgs);

	    	return ALI_RTN_FAIL;
	    }
	    memset(i2c_data.msgs[1].buf, 0, i2c_data.msgs[1].len);     //init the buffer for store the data to read

	    //call the ioctl function to send message to the slave device
	    ret = ioctl(hdl->fd, I2C_RDWR, (unsigned long)&i2c_data);
	    if (ret< 0){
	    	SL_ERR("\n I2C_RDWR error2 \n");
	    	free(i2c_data.msgs[1].buf);	
	    	free(i2c_data.msgs[0].buf);	
	    	free(i2c_data.msgs);
	    	return ALI_RTN_FAIL;
	    }
	    
	    for(i = 0 ;i < i2c_data.msgs[1].len; i++){
	    	SL_DBG("read buff[%d]=0x%x\n",i,i2c_data.msgs[1].buf[i]);
	    	buf[i] = i2c_data.msgs[1].buf[i];
	    	SL_DBG("buff[%d]=0x%x\n",i,buf[i]);
	    }

	    free(i2c_data.msgs[1].buf); 
	    free(i2c_data.msgs[0].buf); 
	    free(i2c_data.msgs);
    }else {
	    //set the number of messages. While executing the write operation, when the device has no sub address, 
        //one message should be sent to the slave device by the host. The first execute the read operation to read the data.
	    i2c_data.nmsgs = 1;
        struct i2c_msg i2c_msg_send;
	    //configure the first message sent to the driver
	    i2c_msg_send.addr = chip_addr;                       //the address of the slave device
	    i2c_msg_send.flags = I2C_M_RD;                       //read flag
	    i2c_msg_send.len = buf_len;                          //the length of the sent data
        i2c_msg_send.buf = buf;

	    //configure the first message sent to the driver
	    i2c_data.msgs = &i2c_msg_send;                       //the address of the slave device
	    
        //call the ioctl function to send message to the slave device
	    ret = ioctl(hdl->fd, I2C_RDWR, (unsigned long)&i2c_data);
	    if (ret< 0){
	    	SL_ERR("\n I2C_RDWR error2 \n");
	    	return ALI_RTN_FAIL;
	    }
	    
	    for(i = 0 ;i < i2c_data.msgs[0].len; i++){
	    	SL_DBG("read buff[%d]=0x%x\n",i,i2c_data.msgs[0].buf[i]);
	    	SL_DBG("buff[%d]=0x%x\n",i,buf[i]);
	    }
    
    }
	return ret;
}

alisl_retcode alisli2c_write_read(alisl_handle handle, unsigned int chip_addr, unsigned int sub_addr, 
	                                         unsigned char *write_buf, unsigned int write_buf_len, unsigned char *read_buf, 
	                                         unsigned int read_buf_len, unsigned int subaddr_flag)
{
	if ((NULL == handle) || (NULL == write_buf) || (MAX_DATA_LENGTH < write_buf_len)
	   || (NULL == read_buf) || (MAX_DATA_LENGTH < read_buf_len)) {
		SL_ERR("fail to get the right input!\n");
		return ALI_RTN_FAIL;
	}
	unsigned int i;
	alisl_retcode ret = ALI_RTN_SUCCESS;
	alisli2c_handle *hdl = (alisli2c_handle *)handle;
	unsigned char *send_data = NULL;
	unsigned int data_len = 0;
	 
	//if the slave device has no data register, that is sub_addr==0xFF, copy data in @buf to @send_data
	//and the @data_len is @sizeof(data_len).
	//if the slave device has some data register, that is sub_addr !=0xFF, copy sub_addr to send_data[0],
	//and copy the data in the buf to send_data[i](i>0).
	if (subaddr_flag) {
		send_data = (unsigned char *)malloc((write_buf_len+1) * sizeof(unsigned char));
		if (NULL == send_data) {
			SL_ERR("fail to malloc the memory for send_data!\n");
			return ALI_RTN_FAIL;
    		}
		memset(send_data, 0, write_buf_len+1);
		send_data[0] = (unsigned char)sub_addr;
		for(i=1;i<write_buf_len+1;i++){
			send_data[i] = write_buf[i-1];
    		}
		data_len = write_buf_len +1;
         
    	}else{
	    	send_data = (unsigned char *)malloc((write_buf_len) * sizeof(unsigned char));
	    	if (NULL == send_data) {
	    		SL_ERR("fail to malloc the memory for send_data!\n");
	    		return ALI_RTN_FAIL;
	     	}
            
            memset(send_data, 0, write_buf_len);
           	memcpy(send_data, write_buf, write_buf_len);
	     	data_len = write_buf_len;	
    	}

	//set the address of slave device, that is, the slave device is mounted on i2c bus device
	if (ioctl(hdl->fd, I2C_SLAVE, chip_addr) < 0){
		SL_ERR("fail to set i2c pin slave addr!\n");
		free(send_data);
		return ALI_RTN_FAIL;
    } 
	
	//this struct contains the whole information that sent to the slave device.
	struct i2c_rdwr_ioctl_data i2c_data;
		 
	//set the number of messages. while executing the write operation, only one message should be send to
	//the slave device by the host.
	i2c_data.nmsgs = 3;
	i2c_data.msgs = (struct i2c_msg *)malloc(i2c_data.nmsgs * sizeof(struct i2c_msg));  
	if (NULL == i2c_data.msgs){
		SL_DBG("fail to malloc the memory for i2c_data->msgs!\n");
		free(send_data);
		return ALI_RTN_FAIL;
    	}
		 
	//configure the message
	i2c_data.msgs[0].len = data_len;									 //the number of bytes of the send_data
	i2c_data.msgs[0].addr = chip_addr;								 //the slave device address
	i2c_data.msgs[0].flags = WRITE;									  //write flag
	i2c_data.msgs[0].buf = (unsigned char *)malloc(i2c_data.msgs[0].len);	  //malloc buffer to store the send data
	if (NULL == i2c_data.msgs[0].buf){
		SL_ERR("fail to malloc the memory for i2c_data.msgs.buf!\n");
        free(i2c_data.msgs);
		free(send_data);
		return ALI_RTN_FAIL;
    	}
	
	//i2c_data.msgs[0].buf[0] = sub_addr;					
	for (i=0; i<i2c_data.msgs[0].len; i++) {								//the first element store the sub_addr, and the rest store the sent data
		i2c_data.msgs[0].buf[i] = send_data[i];
	}
	
	for(i=0;i<i2c_data.msgs[0].len;i++) {
	SL_ERR("write buff[%d]=0x%x\n",i,i2c_data.msgs[0].buf[i]);
	}
	
	if (read_buf_len) {
		//configure the first message
		i2c_data.msgs[1].addr = chip_addr;                       //write address of the slave device
		i2c_data.msgs[1].flags = WRITE;                          //write flag
		i2c_data.msgs[1].len = subaddr_flag ? 1 : 0;                                //the length of the sent data
		if ((i2c_data.msgs[1].buf = (unsigned char *)malloc(sizeof(unsigned char))) == NULL) {
			SL_ERR("fail to malloc the memeory for i2c_data->msgs.buf[0]\n");
			free(i2c_data.msgs[0].buf);
			free(i2c_data.msgs);
			free(send_data);
			return ALI_RTN_FAIL;
		}
		i2c_data.msgs[1].buf[0] = sub_addr;                      //write address of the data to read
		
        //configure the second message
		i2c_data.msgs[2].addr = chip_addr;                       //the address of the slave device
		i2c_data.msgs[2].flags = I2C_M_RD;                       //read flag
		i2c_data.msgs[2].len = read_buf_len;                          //the length of the sent data
		if ((i2c_data.msgs[2].buf = (unsigned char *)malloc(i2c_data.msgs[2].len)) == NULL) {
			SL_ERR("fail to malloc the memeory for i2c_data->msgs.buf[1]");
			free(i2c_data.msgs[1].buf);
			free(i2c_data.msgs[0].buf);
			free(i2c_data.msgs);
			free(send_data);
			return ALI_RTN_FAIL;
		}
		memset(i2c_data.msgs[2].buf, 0, i2c_data.msgs[2].len);     //init the buffer for store the data to read

	}
	
	//call the ioctl function to send message to the slave device
	SL_DBG("fd:%d, chip_addr:0x%x, sub_addr:0x%x, data_len:%d\n", hdl->fd, i2c_data.msgs[0].addr, i2c_data.msgs[0].buf[0], i2c_data.msgs[0].len );
	ret = ioctl(hdl->fd, I2C_RDWR, (unsigned long)&i2c_data);
	SL_DBG("################ret:%d\n",ret);
	if (ret < 0){
		SL_ERR("\n I2C_RDWR error1!\n");
		free(i2c_data.msgs[2].buf);
		free(i2c_data.msgs[1].buf);
		free(i2c_data.msgs[0].buf);
		free(i2c_data.msgs);
		free(send_data);
		return ALI_RTN_FAIL;
    	}

	for(i = 0 ;i < i2c_data.msgs[2].len; i++){
		SL_DBG("read buff[%d]=0x%x\n",i,i2c_data.msgs[2].buf[i]);
		read_buf[i] = i2c_data.msgs[2].buf[i];
		SL_DBG("buff[%d]=0x%x\n",i,read_buf[i]);
	}
	 
      SL_ERR("\n TEST I2C WRITE READ OVER!\n");
	 
	free(i2c_data.msgs[2].buf);
	free(i2c_data.msgs[1].buf);
	free(i2c_data.msgs[0].buf);
	free(i2c_data.msgs);
	free(send_data);	
	return ret;
}    

alisl_retcode alisli2c_close(alisl_handle handle)
{
	int ret = ALI_RTN_SUCCESS;
	if (NULL == handle){
		SL_ERR("fail to get the right handle!\n");
		return ALI_RTN_FAIL;
	}
	
    alisli2c_handle *hdl =(alisli2c_handle *)handle;   

    close(hdl->fd);

	hdl->fd = ALI_RTN_FAIL;

	free(handle);                                            //corresponding to the malloc in alisli2c_open()
	return ret;
}



static char *alisli2c_get_idx(unsigned int idx)
{
    char *pin = NULL;
	
	switch(idx){
		case 0: pin = IDX_0;	
				break;
		case 1: pin = IDX_1;
				break;
		case 2: pin = IDX_2;
			    break;
		case 3: pin = IDX_3;
				break;
		case 4: pin = IDX_4;
				break;
		case 5: pin = IDX_5;
			    break;
		default: SL_ERR("fail to get the right idx!\n"); 
		}
	
	return pin;
}
