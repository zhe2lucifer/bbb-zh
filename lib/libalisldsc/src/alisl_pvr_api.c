#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/ali_pvr.h>
#include <alisldsc.h>
#include "dsc_internal.h"


#define ALI_PVR_DEV_PATH                "/dev/ali_pvr0"
#define C200A_TS_PACKAGE_SIZE           188 //bytes

/*
* Because of the defects of ali pvr driver.
* Need to use the same buff when encrypted es video and audio video playback at the same time
*/
static void* g_pvr_mmap_addr = NULL;

alisl_retcode alisldsc_config_pvr(int *pvr_fd, int encrypt_fd,
                                  int pid_count,unsigned short * pid,int block_count)
{
	struct ali_pvr_reencrypt    pvr_enc_param;
	int ali_pvr_fd = -1;
	int ret = -1;
	SL_DBG("*pvr_fd : %d\n",*pvr_fd);
	if(*pvr_fd <= 0) {
		ali_pvr_fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
		SL_DBG(" open  fd: %d\n", ali_pvr_fd);
		if(ali_pvr_fd < 0) {
			SL_ERR("Invalid C200A ali_pvr_fd  fd: %d\n", ali_pvr_fd);
			goto quit;
		}
	}else
		ali_pvr_fd = *pvr_fd;
	memset(&pvr_enc_param,0,sizeof(struct ali_pvr_reencrypt));
    int i;
    for(i = 0; i < pid_count; i++) {
        pvr_enc_param.pid_list[i] = pid[i];
        SL_DBG("pid :%d\n",pvr_enc_param.pid_list[i]);
    }
    printf("pid_count:%d\n",pid_count);
    pvr_enc_param.pid_num = pid_count;
    pvr_enc_param.source_mode =  (1<<24);//TS mode
    pvr_enc_param.dsc_fd = encrypt_fd;
    SL_DBG("PVR_IO_START_REENCRYPT  :%d,open_pvr fd: %d\n",PVR_IO_START_REENCRYPT,ali_pvr_fd);
    ret = ioctl(ali_pvr_fd,PVR_IO_SET_BLOCK_SIZE,block_count);
    if(ret < 0) {
        SL_DBG("Error: ioctl PVR_IO_SET_BLOCK_SIZE fail !! ret:%d \n", ret);         
		goto quit;
    }
    ret = ioctl(ali_pvr_fd,PVR_IO_START_REENCRYPT,&pvr_enc_param);
    if(ret < 0) {
        SL_DBG("Error: ioctl PVR_IO_START_BLOCK fail !! ret:%d \n", ret);         
        goto quit;
    }    
    SL_DBG("IO_SET_DEC_CONFIG  OK\n");
	*pvr_fd = ali_pvr_fd;
	SL_DBG("open pvr fd:%d\n",*pvr_fd);
	return 0;

quit:
	if(ali_pvr_fd > 0)
		close(ali_pvr_fd);
	return -1;
}

alisl_retcode alisldsc_pvr_start_block_mode(int *pvr_fd,
	unsigned long *enc_param,
	unsigned long block_count)
{
	int ali_pvr_fd = -1;
	int ret = -1;
	if ((!enc_param) || (block_count%188 != 0)){
		SL_DBG("invalid parameters!\n");
		return -1;
	}
	if(*pvr_fd <= 0){
	 	ali_pvr_fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
		SL_DBG(" open  fd: %d\n", ali_pvr_fd);
        if(ali_pvr_fd < 0) {
	        SL_DBG("Invalid C200A ali_pvr_fd  fd: %d\n", ali_pvr_fd);
	       	goto quit;
	    }  
	}else
		ali_pvr_fd = *pvr_fd;
		
    SL_DBG("PVR_IO_START_BLOCK  :%d,open_pvr fd: %d,block_size: %d\n",PVR_IO_START_BLOCK,*pvr_fd,block_count);
    ret = ioctl(ali_pvr_fd,PVR_IO_SET_BLOCK_SIZE,block_count);
    if(ret < 0) {
        SL_DBG("Error: ioctl PVR_IO_SET_BLOCK_SIZE fail !! ret:%d \n", ret);         
		goto quit;
    }
    ret = ioctl(ali_pvr_fd,PVR_IO_START_BLOCK_EVO,enc_param);
    if(ret < 0) {
        SL_DBG("Error: ioctl PVR_IO_START_BLOCK fail !! ret:%d \n", ret);         
        goto quit;
    }    
    SL_DBG("IO_SET_DEC_CONFIG  OK\n");
	*pvr_fd = ali_pvr_fd;
	SL_DBG("open pvr fd:%d\n",*pvr_fd);
	return 0;

quit:
	if(ali_pvr_fd > 0)
		close(ali_pvr_fd);
	return -1;
}

alisl_retcode alisldsc_pvr_free_block_mode(int *pvr_fd,int encyrpt_fd)
{
	int pvr_temp_fd = *pvr_fd;
	int ret = 0;
	SL_DBG("close pvr fd:%d\n",*pvr_fd);
	if(pvr_temp_fd > 0){
		ret = ioctl(pvr_temp_fd,PVR_IO_STOP_REENCRYPT,encyrpt_fd);
		if(ret < 0)
			return -1;
		
	}else{
		SL_DBG("PVR invalid fd error!");
		return -1;
	}
	return 0;
}

alisl_retcode alisldsc_pvr_free_resource(int *pvr_fd)
{
	int pvr_temp_fd = *pvr_fd;
	int ret = 0;
	SL_DBG("close pvr fd:%d\n",*pvr_fd);
	if(pvr_temp_fd > 0){
		ret = ioctl(pvr_temp_fd,PVR_IO_FREE_BLOCK_EVO,NULL);
		if(ret < 0)
			return -1;
		
	}else{
		SL_DBG("PVR invalid fd error!");
		return -1;
	}
	return 0;
}

alisl_retcode alisldsc_pvr_ioctl(int pvr_fd,unsigned int cmd, unsigned long param)
{
	int ret = 0;
	if(pvr_fd <= 0){
		SL_DBG("PVR FD error!\n");
		return -1;
	}
	switch(cmd){
		case SL_PVR_IO_UPDATE_ENC_PARAMTOR:
			ret = ioctl(pvr_fd,PVR_IO_UPDATE_ENC_PARAMTOR_EVO,(INT32)param);
			if(ret != 0){
                SL_DBG("PVR SL_PVR_IO_UPDATE_ENC_PARAMTOR error!\n");
				return -1;
			}
			break;
		case SL_PVR_IO_DECRYPT:
			ret = ioctl(pvr_fd,PVR_IO_DECRYPT,(INT32)param);
			if(ret != 0){
				return -1;
			}
			break;
		case SL_PVR_IO_FREE_BLOCK:
			ret = ioctl(pvr_fd,PVR_IO_FREE_BLOCK_EVO,NULL);
			if(ret < 0)
				return -1;
			break;
		 case SL_PVR_IO_SET_BLOCK_SIZE:
		    ret = ioctl(pvr_fd,PVR_IO_SET_BLOCK_SIZE,(INT32)param);
		    if(ret < 0) {
		        SL_DBG("Error: ioctl PVR_IO_SET_BLOCK_SIZE fail !! ret:%d \n", ret);         
				return -1;
		    } 
			break;
		case SL_PVR_IO_CAPTURE_DECRYPT_RES:
			ret = ioctl(pvr_fd,PVR_IO_CAPTURE_DECRYPT_RES,param);
		    if(ret < 0) {
		        SL_DBG("Error: ioctl PVR_IO_CAPTURE_DECRYPT_RES fail !! ret:%d \n", ret);         
				return -1;
		    } 
			break;
		case SL_PVR_IO_SET_DECRYPT_RES:
			ret = ioctl(pvr_fd,PVR_IO_SET_DECRYPT_RES,param);
		    if(ret < 0) {
		        SL_DBG("Error: ioctl PVR_IO_SET_DECRYPT_RES fail !! ret:%d \n", ret);         
				return -1;
		    } 
			break;
		case SL_PVR_IO_RELEASE_DECRYPT_RES:
			ret = ioctl(pvr_fd,PVR_IO_RELEASE_DECRYPT_RES,param);
		    if(ret < 0) {
		        SL_DBG("Error: ioctl PVR_IO_RELEASE_DECRYPT_RES fail !! ret:%d \n", ret);         
				return -1;
		    } 
			break;
		case SL_PVR_IO_DECRYPT_EVO:
			ret = ioctl(pvr_fd,PVR_IO_DECRYPT_EVO,param);
		    if(ret < 0) {
		        SL_DBG("Error: ioctl PVR_IO_DECRYPT_EVO fail !! ret:%d \n", ret);         
				return ret;
		    } 
			break;
		case SL_PVR_IO_DECRYPT_ES_EVO:
			ret = ioctl(pvr_fd,PVR_IO_DECRYPT_ES_EVO,param);
		    if(ret < 0) {
		        SL_ERR("Error: ioctl PVR_IO_DECRYPT_EVO fail !! ret:%d \n", ret);         
				return -1;
		    } 
			break;
 		case SL_PVR_IO_DECRYPT_EVO_SUB:
			ret = ioctl(pvr_fd,PVR_IO_DECRYPT_EVO_SUB,param);
		    if(ret < 0) {
		        SL_ERR("Error: ioctl PVR_IO_DECRYPT_EVO_SUB fail !! ret:%d \n", ret);         
				return ret;
		    } 
			break;
		default:
			SL_DBG("cmd parameter error!\n");
			return -1;
	}
		
	return 0;
}

typedef struct dsc_pvr_hdl{
	int fd;
}dsc_pvr_hdl;

alisl_retcode alisldsc_pvr_open(alisl_handle *handle)
{
    dsc_pvr_hdl *pvr_hdl= (dsc_pvr_hdl *)calloc(1, sizeof(dsc_pvr_hdl));
	if(pvr_hdl)
	{
		memset(pvr_hdl, 0, sizeof(dsc_pvr_hdl));
	}
	else
	{
		printf("%s @ %d fail: %d\n", __FUNCTION__, __LINE__, (int)pvr_hdl);
		return -1;	
	}
	
	pvr_hdl->fd = -1;
 	pvr_hdl->fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
    if(pvr_hdl->fd < 0) {
        printf("%s @ %d fail: %d\n", __FUNCTION__, __LINE__, pvr_hdl->fd);
		free(pvr_hdl);
		return -1;
    }
	*handle = (alisl_handle*)pvr_hdl;
	return 0;
}

alisl_retcode alisldsc_pvr_close(alisl_handle handle)
{
    int ret = 0;
	dsc_pvr_hdl *hdl = (struct dsc_pvr_hdl *)handle;

	if(!handle){
		printf("%s @ %d fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
    ret = close(hdl->fd);
    free(hdl);
    
	return ret;
}

alisl_retcode alisldsc_pvr_ioctl_ex(alisl_handle handle, unsigned int cmd, unsigned long param)
{
	int ret = -1;
	dsc_pvr_hdl *hdl = (dsc_pvr_hdl *)handle;

	if(!handle){
		printf("%s @ %d fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
	ret = alisldsc_pvr_ioctl(hdl->fd, cmd, param);
	return ret;
}

alisl_retcode alisldsc_pvr_mmap(alisl_handle handle, unsigned int *p_addr, unsigned int *buf_len, int need_map)
{
	dsc_pvr_hdl *hdl = (dsc_pvr_hdl *)handle;

	if((!handle) ||(!p_addr) || (!buf_len)){
		printf("%s @ %d fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if(need_map) {
		void*  tmp_addr = NULL;
		/*
		* call mmap start addr and end start will change in ali pvr driver
		* but there is file parivate one area in ali_pvr driver,
		* When using mmap twice, start addr and end start will be overwritten.
		* This problem should be changed in the ali pvr driver.
		*/
        if (g_pvr_mmap_addr) {
            tmp_addr = g_pvr_mmap_addr;
        } else {
            tmp_addr = mmap(NULL, MMAP_LEN, PROT_WRITE, MAP_SHARED, hdl->fd, 0);
            if(MAP_FAILED == (void *)tmp_addr) {
                printf("%s@%d mmap fail\n",__func__, __LINE__);
                return -1;
            }
            g_pvr_mmap_addr = tmp_addr;
        }
		*p_addr = (unsigned int)tmp_addr;
		*buf_len = MMAP_LEN;
		 //printf("%s -> mmap ok: addr: 0x%p\n", __func__, tmp_addr);
	} else {
	    if (g_pvr_mmap_addr) {
            if(munmap(p_addr, *buf_len)) {
                printf("%s@%d munmap fail\n",__func__, __LINE__);
                return -1;
            }
            g_pvr_mmap_addr = NULL;
        }
	}
	return 0;
}

alisl_retcode alisldsc_pvr_fd_get(alisl_handle handle, int *pvr_fd)
{
    //alisl_retcode ret;
    dsc_pvr_hdl *hdl = (dsc_pvr_hdl *)handle;

    *pvr_fd = hdl->fd;
   	printf("get pvr fd: %d\n",*pvr_fd);
    return 0;
}

