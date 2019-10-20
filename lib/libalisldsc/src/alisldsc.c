/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisl_dsc_api.c
 *  @brief              implement DSC function, provide relevant API
 *
 *  @version            1.0
 *  @date               06/27/2013 02:06:14 PM
 *  @revision           none
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>

#include "dsc_internal.h"
#include "alisldsc.h"
#include "ca_dsc.h"



/* indicate which part of userspace for descramble is used */
static struct dsc_mem_bitmap dsc_mem_bm = {{NULL}, 0};

static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
//static alisldsc_dev_t *m_dev = NULL;


alisl_retcode alisldsc_dsc_open(alisl_handle *handle)
{
    alisl_retcode ret = 0;
//    struct dsc_see_dev_hld see_dev_hld;
    //alisldsc_dev_t **dsc_dev = (alisldsc_dev_t **)handle;
    alisldsc_dev_t *dev;

    pthread_mutex_lock(&m_mutex);

    dev = (alisldsc_dev_t *)calloc(1, sizeof(alisldsc_dev_t));
    if (!dev) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_MALLOCFAIL, "in %s alloc memory fail.\n",
                       __func__);
        pthread_mutex_unlock(&m_mutex);
        return ALISLDSC_ERR_MALLOCFAIL;
    }
    memset((char*)dev,0,sizeof(alisldsc_dev_t));

    dev->type = ALISLDSC_DEV_TYPE_DSC;
    dev->name = ALISLDSC_DSC_DEV_NAME;
    dev->open_cnt = 0;
    dev->fd = 0;
	dev->dsc_user_baseaddr = (void *)MAP_FAILED;

    dev->fd = open(dev->name, O_RDWR);
	SL_DBG("open dsc,dev->name: %s, fd: %d\n", dev->name,dev->fd);
    if (dev->fd < 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_CANTOPENDEV, "in %s open DSC fail,"
                       "err: %s\n", __func__, strerror(errno));
        free(dev);
        pthread_mutex_unlock(&m_mutex);
        return ALISLDSC_ERR_CANTOPENDEV;
    }
    dev->dsc_user_baseaddr = mmap(NULL, DSC_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, 0);
    if (dev->dsc_user_baseaddr == MAP_FAILED)
        ALISLDSC_DEBUG(ALISLDSC_ERR_MMAPERROR, "in %s descramble mmap fail err: %s\n",
                       __func__, strerror(errno));
    dev->key_hdl = -1;
    *handle = (alisl_handle*)dev;
	dev->add_pid_flag = 0;
    pthread_mutex_unlock(&m_mutex);
    return ret;
}


alisl_retcode alisldsc_dsc_close(alisl_handle handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t *)handle;

    if(NULL == dsc_dev) {
        SL_ERR("Invalid parameters!\n");
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    pthread_mutex_lock(&m_mutex);
    /*
       if(0 <= dsc_dev->key_hdl) {
       ret = ioctl(dsc_dev->fd,CA_DELETE_KEY,dsc_dev->key_hdl);
       if(-1 == ret)
       ALISLDSC_DEBUG(ALISLDSC_ERR_CLOSEERROR, "in %s delet key handle fail,"
       " err: %s\n", __func__, strerror(errno));
       dsc_dev->key_hdl = -1;
       }
     */

    if(dsc_dev->dsc_user_baseaddr) {
        ret = munmap(dsc_dev->dsc_user_baseaddr,DSC_MEM_SIZE);
        if(ret == -1)
            ALISLDSC_DEBUG(ALISLDSC_ERR_MMAPERROR, "in %s close DSC fail,"
                    " err: %s\n", __func__, strerror(errno));
    }
	SL_DBG("close dsc,dsc_dev->fd=%d\n",dsc_dev->fd);
    ret = close(dsc_dev->fd);
    if(-1 == ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_CLOSEERROR, "in %s close DSC fail,"
                " err: %s\n", __func__, strerror(errno));

    free(dsc_dev);

    pthread_mutex_unlock(&m_mutex);

    return ret;
}

alisl_retcode alisldsc_dsc_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param)
{
    alisldsc_dev_t *dsc_dev = NULL;
	int ret = -1;

    if (NULL == handle) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "DSC handle is NULL\n");
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    dsc_dev = (alisldsc_dev_t *)handle;
	SL_DBG("dsc_dev->fd: %d, cmd: 0x%x, param: 0x%x\n",dsc_dev->fd,cmd,param);
	pthread_mutex_lock(&m_mutex);
	ret = ioctl(dsc_dev->fd, cmd, param);
	pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alisldsc_decrypt(alisl_handle handle,
                                    uint8_t *input, uint8_t *output,
                                    uint32_t total_len)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
//    int size = 0;

    struct ca_dio_write_read crypto;
    memset(&crypto, 0, sizeof(crypto));
    memset(output, 0, total_len);
	crypto.input = (char *)input;
    crypto.crypt_mode = CA_DECRYPT;
   	crypto.output = (char *)output;
    crypto.length = total_len;
	SL_DBG("call CA_DIO_WRITE_READ dsc_dev->fd:%d,crypto.length:%d,crypto.input:%p,crypto.output:%p,crypto.crypt_mode:%d,"\
				"crypto.mode:%d,crypto.eventfd:%d\n",
				dsc_dev->fd,crypto.length,crypto.input,crypto.output,crypto.crypt_mode,crypto.mode,crypto.eventfd);	
	pthread_mutex_lock(&m_mutex);
	ret = ioctl(dsc_dev->fd, CA_DIO_WRITE_READ, (void *)&crypto);
	pthread_mutex_unlock(&m_mutex);
    if(ret < 0) {
        ret = errno;
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "Error: ioctl fail in %s!!,"
                       " err: %s\n", __func__, strerror(ret));
        return ret;
    }
	//SL_DUMP("in:",input,total_len);
    //SL_DUMP("out:",output,total_len);
    return 0;
}


alisl_retcode alisldsc_encrypt(alisl_handle handle, 
                                    uint8_t * input, uint8_t *output,
                                    uint32_t total_len)
{

    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
//    int size = 0;

    struct ca_dio_write_read crypto;
    memset(&crypto, 0, sizeof(crypto));
    memset(output, 0, total_len);
    crypto.crypt_mode = CA_ENCRYPT;
    crypto.input = (char *)input;
    crypto.output = (char *)output;
    crypto.length = total_len;
	SL_DBG("call CA_DIO_WRITE_READ dsc_dev->fd: %d, crypto.length: %d, crypto.input: %p, crypto.output: %p,"\
				"crypto.crypt_mode: %d, crypto.mode: %d, crypto.eventfd: %d\n",
				dsc_dev->fd,crypto.length,crypto.input,crypto.output,crypto.crypt_mode,crypto.mode,crypto.eventfd);		
	pthread_mutex_lock(&m_mutex);
	ret = ioctl(dsc_dev->fd, CA_DIO_WRITE_READ, (void *)&crypto);
	pthread_mutex_unlock(&m_mutex);
    if(ret < 0) {
        ret = errno;
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "Error: ioctl fail in %s!!,"
                       " err: %s\n", __func__, strerror(ret));
        return ret;
    }
	
	//SL_DUMP("in:",input,total_len);
    //SL_DUMP("out:",output,total_len);
    return 0;

}
#define TS_SCR_BIT_EVEN      (0x02)
#define TS_SCR_BIT_ODD       (0x03)
#define TS_SCR_BIT_AUTO      (1<<2)

alisl_retcode alisldsc_set_data_type(alisl_handle handle,
                                     unsigned int data_type)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
	pthread_mutex_lock(&m_mutex);
	if(dsc_dev->ca_format_flag <= 0){/*only set one times*/
		SL_DBG("dsc_dev->fd: %d,set CA_SET_FORMAT,format: %d\n",dsc_dev->fd,data_type);
	    ret = ioctl(dsc_dev->fd, CA_SET_FORMAT, &data_type);
	    if (ret < 0) {
	        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS,"setting format: %s", strerror(errno));
	        SL_ERR("set CA_SET_FORMAT,format = %d\n",data_type);
			pthread_mutex_unlock(&m_mutex);
	        return ret;
	    }
		dsc_dev->ca_format_flag += 1;
	}
	pthread_mutex_unlock(&m_mutex);
   return 0; 
}

alisl_retcode alisldsc_attach_clear_key(alisl_handle handle,
                                        unsigned int data_type,
                                        struct ca_create_clear_key *key,
                                        unsigned short *pid_list,
                                        unsigned long pid_len,
                                     	int *key_handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
    //int i = 0;
//	int format = -1;
    if (!dsc_dev || !key
        || (CA_FORMAT_RAW != data_type && CA_FORMAT_TS188 != data_type)
        || NULL == pid_list) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    ret = alisldsc_set_data_type(handle, data_type);
    if (ret < 0)
        return ret;

    pthread_mutex_lock(&m_mutex);
    SL_DBG("call CA_CREATE_CLEAR_KEY fd:%d,key->algo:%d,key->crypt_mode:%d,key->chaining_mode:%d,key->residue_mode:%d,"\
                 "key->parity:%d,key->tsc_flag:%d,key->key_size:%d,key->valid_mask:%d\n",
                 dsc_dev->fd,key->algo,key->crypt_mode,key->chaining_mode,key->residue_mode,
                 key->parity,key->tsc_flag,key->key_size,key->valid_mask);
    SL_DUMP("key_even:",(char *)key->key_even,key->key_size);
    SL_DUMP("key_odd:",(char *)key->key_odd,key->key_size);
    SL_DUMP("iv_even:",(char *)key->iv_even,CA_IV_SIZE_MAX);
    SL_DUMP("iv_odd:",(char *)key->iv_odd,CA_IV_SIZE_MAX);
    ret = ioctl(dsc_dev->fd, CA_CREATE_CLEAR_KEY, key);/*create key handle*/
    if (ret < 0) {
        ret = errno;
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM,"CREATE_CLEAR_KEY failed : %s", strerror(errno));
		pthread_mutex_unlock(&m_mutex);
        return ret;
    }
	dsc_dev->add_pid_flag = 0;
    dsc_dev->key_hdl = ret;/* save key handle*/
	*key_handle = ret;
    SL_DBG("create clear key success!,dsc_dev->key_hdl = %d\n",ret);
    pthread_mutex_unlock(&m_mutex);

    return 0;
}

alisl_retcode alisldsc_delete_key_handle(alisl_handle handle,int key_handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;

    if (!dsc_dev || key_handle < 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

	pthread_mutex_lock(&m_mutex);
	SL_DBG("call CA_DELETE_KEY dsc_dev->fd: %d, key_handle: %d\n",dsc_dev->fd,key_handle);
    ret = ioctl(dsc_dev->fd, CA_DELETE_KEY, key_handle);
    if (ret < 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS,"setting format: %s", strerror(errno));
    }
	pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alisldsc_add_pid(alisl_handle handle, unsigned short *pid_list,unsigned long pid_len,
	int crypt_mode,int parity, int key_handle)
{
    struct ca_pid pid_info;
	alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
    int ret = 0;
	int i;
    int key_hdl = -1;
    
    if (!dsc_dev || NULL == pid_list) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }
	if(dsc_dev->add_pid_flag == 1){
		SL_DBG("PID have been added!\n");
		return 0;
	}
    if (key_handle > 0)
        key_hdl = key_handle;
    else
        key_hdl = dsc_dev->key_hdl;
    
    pthread_mutex_lock(&m_mutex);
    for(i = 0; i < pid_len; i++) {
        memset(&pid_info, 0, sizeof(struct ca_pid));
        /*
        if((crypt_mode == CA_ENCRYPT) && (parity == CA_PARITY_EVEN))
            pid_info.tsc_flag = TS_SCR_BIT_EVEN;
		else if((crypt_mode == CA_ENCRYPT) && (parity == CA_PARITY_ODD))
			pid_info.tsc_flag = TS_SCR_BIT_ODD;
        */
        //In TS encryption, we always set the TSC flag to auto mode,
        //then driver will set the TSC flag automatically according to 
        // the parity mode(when create key handle or update parameters).
        if (crypt_mode == CA_ENCRYPT)
            pid_info.tsc_flag = TS_SCR_BIT_AUTO;
        
        pid_info.key_handle = key_hdl;
        pid_info.pid = pid_list[i];
		SL_DBG("call CA_ADD_PID dsc_dev->fd:%d,pid_info.key_handle:%d,pid_info.ltsid:%d,pid_info.tsc_flag:%d,pid_info.pid:%d\n",
				dsc_dev->fd,pid_info.key_handle,pid_info.ltsid,pid_info.tsc_flag,pid_info.pid);
        ret = ioctl(dsc_dev->fd, CA_ADD_PID, (void *)&pid_info);
        if(ret < 0) {
            SL_ERR("Error: ioctl CA_ADD_PID fail !! ret:%d, errno: %x\n", ret, errno);
            pthread_mutex_unlock(&m_mutex);
            return -1;
        }
		
    }
	dsc_dev->add_pid_flag = 1;
    pthread_mutex_unlock(&m_mutex);
    return 0;
}
alisl_retcode alisldsc_delete_pid(alisl_handle handle, int key_handle,
                                  unsigned short *pid_list,unsigned long pid_len)
{
    struct ca_pid pid_info;
    int ret = 0;
	int i;
    int key_hdl = -1;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;

    if (key_handle > 0)
        key_hdl = key_handle;
    else
        key_hdl = dsc_dev->key_hdl;
    
    pthread_mutex_lock(&m_mutex);
    for(i = 0; i < pid_len; i++) {
        memset(&pid_info, 0, sizeof(struct ca_pid));
        pid_info.key_handle = key_hdl;
        pid_info.pid = pid_list[i];
		SL_DBG("call CA_DEL_PID dsc_dev->fd:%d,pid_info.key_handle:%d,pid_info.ltsid:%d,pid_info.tsc_flag:%d,pid_info.pid:%d\n",
				dsc_dev->fd,pid_info.key_handle,pid_info.ltsid,pid_info.tsc_flag,pid_info.pid);		
        ret = ioctl(dsc_dev->fd, CA_DEL_PID, (void *)&pid_info);
        if(ret < 0) {
            SL_ERR("Error: ioctl CA_ADD_PID fail !! ret:%d, errno: %x\n", ret, errno);
            pthread_mutex_unlock(&m_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

alisl_retcode alisldsc_attach_kl_key(alisl_handle handle,
                                     unsigned int data_type,
                                     struct ca_create_kl_key *key,
                                     unsigned short *pid_list,
                                     unsigned long pid_len,
                                     int *key_handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
	//int i = 0;
    if (!dsc_dev || !key
        || (CA_FORMAT_RAW != data_type && CA_FORMAT_TS188 != data_type)
        || NULL == pid_list) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }
    
    ret = alisldsc_set_data_type(handle, data_type);
    if (ret < 0)
        return ret;
    
    pthread_mutex_lock(&m_mutex);
    SL_DBG("call CA_CREATE_KL_KEY dsc_dev->fd:%d,key->algo:%d,key->kl_fd:%d,key->chaining_mode:%d,key->crypt_mode:%d,"\
                 "key->parity:%d,key->residue_mode:%d,key->tsc_flag:%d,key->valid_mask:%d\n",
                 dsc_dev->fd,key->algo,key->kl_fd,key->chaining_mode,key->crypt_mode,
                 key->parity,key->residue_mode,key->tsc_flag,key->valid_mask);

    SL_DUMP("iv_even:",(char *)key->iv_even,CA_IV_SIZE_MAX);
    SL_DUMP("iv_odd:",(char *)key->iv_odd,CA_IV_SIZE_MAX);
    ret = ioctl(dsc_dev->fd, CA_CREATE_KL_KEY, key);/*create key handle*/
    if (ret < 0) {
        ret = errno;
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM,"func:%s,__line__:%d,CREATE_KL_KEY failed : %s\n",
                       __FUNCTION__,__LINE__,strerror(errno));
		pthread_mutex_unlock(&m_mutex);
		return ret;
    }
	dsc_dev->add_pid_flag = 0;
    dsc_dev->key_hdl= ret;/* save key handle*/
	*key_handle = ret;
    SL_DBG("create clear kl_key success!,dsc_dev->key_hdl = %d\n",ret);

	pthread_mutex_unlock(&m_mutex);
    return 0;
//err:
//    ret = ioctl(dsc_dev->fd,CA_DELETE_KEY,dsc_dev->key_hdl);
//    if(-1 == ret)
//        ALISLDSC_DEBUG(ALISLDSC_ERR_CLOSEERROR, "in %s delet key handle fail,"
//                       " err: %s\n", __func__, strerror(errno));
//    dsc_dev->key_hdl = -1;
//	pthread_mutex_unlock(&m_mutex);
//    return ALISLDSC_ERR_IOACCESS;
}

alisl_retcode alisldsc_attach_otp_key(alisl_handle handle,
                                     unsigned int data_type,
                                     struct ca_create_otp_key *key,
                                     int *key_handle)
{
	alisl_retcode ret = 0;
	alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
//	int i = 0;
	if (!dsc_dev || !key
		|| (CA_FORMAT_RAW != data_type && CA_FORMAT_TS188 != data_type)) {
		ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
		__func__);
		return ALISLDSC_ERR_INVALIDPARAM;
	}
	
	ret = alisldsc_set_data_type(handle, data_type);
	if (ret < 0)
		return ret;
	
	pthread_mutex_lock(&m_mutex);
	SL_DBG("call CA_CREATE_OTP_KEY dsc_dev->fd:%d,key->algo:%d,key->chaining_mode:%d,key->crypt_mode:%d,"\
			"key->residue_mode:%d,key->otp_key_select:%d\n",
			dsc_dev->fd,key->algo,key->chaining_mode,key->crypt_mode,
			key->residue_mode,key->otp_key_select);	
	
	SL_DUMP("iv_even:",(char *)key->iv_even,CA_IV_SIZE_MAX);
	ret = ioctl(dsc_dev->fd, CA_CREATE_OTP_KEY, key);/*create key handle*/
	if (ret < 0) {
		ret = errno;
		ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM,"func:%s,__line__:%d,CREATE_KL_KEY failed : %s\n",
		__FUNCTION__,__LINE__,strerror(errno));
		pthread_mutex_unlock(&m_mutex);
		return ret;
	}
	dsc_dev->key_hdl= ret;/* save key handle*/
	*key_handle = ret;
	SL_DBG("create OTP key success!,dsc_dev->key_hdl = %d\n",ret);
	
	pthread_mutex_unlock(&m_mutex);
	return 0;
}

alisl_retcode alisldsc_update_key(alisl_handle handle, struct key_from *key_from, int key_handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
    struct ca_update_clear_key key;
    int valid = 0;

    if (!dsc_dev || !key_from) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    memset((char*)&key,0,sizeof(struct ca_update_clear_key));
	if (key_from->clear_key){
        if ((key_from->clear_key->parity == CA_PARITY_EVEN)
			|| (key_from->clear_key->parity == CA_PARITY_AUTO)){
            memcpy(key.key_even,key_from->clear_key->key_even,key_from->clear_key->key_size);
            valid |= CA_VALID_KEY_EVEN;
        }
        if ((key_from->clear_key->parity == CA_PARITY_ODD)
			|| (key_from->clear_key->parity == CA_PARITY_AUTO)){
            memcpy(key.key_odd,key_from->clear_key->key_odd,key_from->clear_key->key_size);
            valid |= CA_VALID_KEY_ODD;
        }
    }else{
        return 0;
    }
    if (key_handle > 0)
        key.key_handle = key_handle;
    else
        key.key_handle = dsc_dev->key_hdl;
    key.valid_mask = valid;
	SL_DBG("call CA_UPDATE_CLEAR_KEY dsc_dev->fd:%d,key handle:%d,key.valid: %d\n",dsc_dev->fd,key.key_handle,key.valid_mask);
    SL_DUMP("key_even:",key.key_even,key_from->clear_key->key_size);
	SL_DUMP("key_odd:",key.key_odd,key_from->clear_key->key_size);
	pthread_mutex_lock(&m_mutex);
    ret = ioctl(dsc_dev->fd, CA_UPDATE_CLEAR_KEY, &key);
    if (ret < 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS,"setting format: %s", strerror(errno));
    }
	pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alisldsc_update_param(alisl_handle handle, struct key_from *key_from, 
	int crypt_mode, int key_handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;
    struct ca_update_params update_params;
//    int key_size = 0;
//    int valid = 0;

    if (!dsc_dev || !key_from
		|| (CA_DECRYPT != crypt_mode && CA_ENCRYPT != crypt_mode)) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    memset((char*)&update_params,0,sizeof(struct ca_update_params));

    if (key_handle > 0)
        update_params.key_handle = key_handle;
    else
        update_params.key_handle = dsc_dev->key_hdl;

    if(key_from->clear_key) {
        update_params.chaining_mode = key_from->clear_key->chaining_mode;
        update_params.parity = key_from->clear_key->parity;
        update_params.crypt_mode = crypt_mode;
        update_params.residue_mode = key_from->clear_key->residue_mode;
        memcpy(update_params.iv_even,key_from->clear_key->iv_even,CA_IV_SIZE_MAX);
        memcpy(update_params.iv_odd,key_from->clear_key->iv_odd,CA_IV_SIZE_MAX);
        update_params.valid_mask = key_from->clear_key->valid_mask;
    } else if(key_from->kl_key) {
        update_params.chaining_mode = key_from->kl_key->chaining_mode;
        update_params.parity = key_from->kl_key->parity;
        update_params.crypt_mode = crypt_mode;
        update_params.residue_mode = key_from->kl_key->residue_mode;
        memcpy(update_params.iv_even,key_from->kl_key->iv_even,CA_IV_SIZE_MAX);
        memcpy(update_params.iv_odd,key_from->kl_key->iv_odd,CA_IV_SIZE_MAX);
        update_params.valid_mask = key_from->kl_key->valid_mask;
    } else if(key_from->otp_key) {
        update_params.chaining_mode = key_from->otp_key->chaining_mode;
        update_params.crypt_mode = crypt_mode;
        update_params.residue_mode = key_from->otp_key->residue_mode;
        memcpy(update_params.iv_even,key_from->otp_key->iv_even,CA_IV_SIZE_MAX);
        memcpy(update_params.iv_odd,key_from->otp_key->iv_even,CA_IV_SIZE_MAX);
        update_params.valid_mask = CA_VALID_CHAINING_MODE|CA_VALID_CRYPT_MODE|CA_VALID_KEY_EVEN|CA_VALID_RESIDUE_MODE;
        //return 0;
    } else {
        return 0;
    }
    SL_DBG("call CA_UPDATE_PARAMS dsc_dev->fd:%d,update_params.key_handle:%d,update_params.chaining_mode:%d,"\
                 "update_params.parity:%d,update_params.crypt_mode:%d,update_params.residue_mode:%d,"\
                 "update_params.tsc_flag:%d,update_params.valid_mask:%d\n",
                 dsc_dev->fd,update_params.key_handle,update_params.chaining_mode,
                 update_params.parity,update_params.crypt_mode,update_params.residue_mode,
                 update_params.tsc_flag,update_params.valid_mask);	
    SL_DUMP("iv_even:",(char *)update_params.iv_even,CA_IV_SIZE_MAX);
    SL_DUMP("iv_odd:",(char *)update_params.iv_odd,CA_IV_SIZE_MAX);
	pthread_mutex_lock(&m_mutex);
	ret = ioctl(dsc_dev->fd, CA_UPDATE_PARAMS, &update_params);
    if (ret < 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS,"setting format: %s", strerror(errno));
    }
	pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alisldsc_dsc_encrypt_bl_uk(alisl_handle handle,
        DSC_BL_UK_PARAM * bl_param)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;
	SL_DBG("call IO_DSC_ENCRYTP_BL_UK dev->fd:%d,bl_param->input_key:%p,bl_param->r_key:%p,bl_param->output_key:%p,bl_param->crypt_type:%d\n",
				dev->fd,bl_param->input_key,bl_param->r_key,bl_param->output_key,bl_param->crypt_type);
    ret = ioctl(dev->fd, IO_DSC_ENCRYTP_BL_UK, bl_param);
    if(ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "ALISLDSC ERR: in %s encrypt"
                       " BL universal key fail, ret=%d\n", __func__, ret);
    return ret;
}


alisl_retcode alisldsc_dsc_DecryptEncrypt(alisl_handle handle, DEEN_CONFIG *p_DeEn,
        uint8_t *input, uint8_t *output,
        uint32_t total_len)
{
    alisl_retcode ret = 0;
    ALI_RE_ENCRYPT dsc_param;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    if ((dev == NULL) || (p_DeEn == NULL) || (input == NULL)
        || (output == NULL)) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    dsc_param.p_deen = p_DeEn;
    dsc_param.input = input;
    dsc_param.output = output;
    dsc_param.length = total_len;
	SL_DBG("call ALI_DSC_IO_DEENCRYPT dev->fd:%d,dsc_param.p_deen:%p,dsc_param.input:%p,dsc_param.output:%p,dsc_param.length\n",
				dev->fd,dsc_param.p_deen,dsc_param.input,dsc_param.output,dsc_param.length);
    ret = ioctl(dev->fd, ALI_DSC_IO_DEENCRYPT, &dsc_param);
    if(ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "ALISLDSC ERR: in %s DeEncrypt"
                       " data fail, ret=%d\n", __func__, ret);
    return ret;
}


alisl_retcode alisl_dsc_operate_mem(alisl_handle handle, uint32_t cmd, struct ali_dsc_krec_mem *krec_mem)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;
    int i;

    switch (cmd) {
        case IO_RELEASE_KREC_SPACE:
            ret = ALISLDSC_ERR_RELMEMFAIL;
            for (i=0; i<8; i++) {
                if (dsc_mem_bm.addr[i] == krec_mem->va_mem) {
                    dsc_mem_bm.bitmap &= ~(1 << i);
                    dsc_mem_bm.addr[i] = NULL;
                    ALISLDSC_DEBUG(ALISLDSC_INFO_RELMEMSUCC, "in %s release memory"
                                   " success, addr=%x, bitmap=%x, i=%d\n",
                                   __func__, krec_mem->va_mem, dsc_mem_bm.bitmap, i);
                    ret = 0;
                    break;
                }
            }
            break;
        case IO_REQUEST_KREC_SPACE:
            krec_mem->va_mem = MAP_FAILED;
            for (i=0; i<8; i++)
                if (!(dsc_mem_bm.bitmap & (1 << i))) {
                    unsigned long kaddr;
                    ret = ioctl(dev->fd, IO_REQUEST_KREC_ADDR, &kaddr);
					SL_DBG("call IO_REQUEST_KREC_ADDR dev->fd:%d,kaddr:%ld\n",dev->fd,kaddr);
                    if(ret)
                        continue;
                    krec_mem->va_mem = dev->dsc_user_baseaddr + MEM_ALLOC_UNIT * i;
                    dsc_mem_bm.addr[i] = krec_mem->va_mem;
                    dsc_mem_bm.bitmap |= (1 << i);
                    krec_mem->pa_mem = (void *)(kaddr + MEM_ALLOC_UNIT * i);
                    ALISLDSC_DEBUG(ALISLDSC_INFO_GETMEMSUCC, "in %s get memory"
                                   " success,kaddr=%x, useraddr=%x, bitmap=%x\n",
                                   __func__, kaddr, krec_mem->va_mem, dsc_mem_bm.bitmap);
                    break;
                }

            if (krec_mem->va_mem == MAP_FAILED) {
                ALISLDSC_DEBUG(ALISLDSC_ERR_GETMEMFAIL, "in %s can't map addr,"
                               " pa=%p\n", __func__, krec_mem->pa_mem);
                krec_mem->va_mem = NULL;
                ret = ALISLDSC_ERR_IOACCESS;
            }
            break;
        default:
            ret = ALISLDSC_ERR_INVALIDPARAM;
    }
    return ret;
}

alisl_retcode alisl_dsc_get_fd(alisl_handle handle, int *dsc_fd)
{
//    alisl_retcode ret;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    *dsc_fd = dev->fd;
    SL_DBG("get dsc fd: %d\n",*dsc_fd);
    return 0;
}

