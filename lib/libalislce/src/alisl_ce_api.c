/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisl_ce_api.c
 *  @brief              crypto engine (CE) or key ladder (KL) API
 *
 *  @version            1.0
 *  @date               07/09/2013 09:50:28 AM
 *  @revision           none
 *
 *  @authors            Terry Wu <terry.wu@alitech.com>
 *                      Vincent Pilloux <vincent.pilloux@alitech.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <inttypes.h>

#include <alipltflog.h>
#include <alipltfretcode.h>

#include "ce_internal.h"
#include "ca_dsc.h"
#include "ca_kl.h"
#include "errno.h"
#include "alislce.h"

static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
//static alislce_dev_t *m_dev = NULL;

#define ROOTKEY_MAX 	6
static const int rootkey_to_devname[] = {
	0,//rootkey 0,0x4d
	1,//rootkey 1,0x51
	2,//rootkey 2,0x55
	5,//rootkey	3,0x59
	4,//rootkey 4,0x60
	3,//rootkey 5,0x64
};

alisl_retcode alislce_open(alisl_handle *handle,unsigned int rootkey,
	sl_ce_root_key_type rootkey_type)
{
    alisl_retcode ret = 0;
//    unsigned int ce_see_dev_hld = 0;
    alislce_dev_t *ce_dev;

    pthread_mutex_lock(&m_mutex);
    ce_dev = (alislce_dev_t *)calloc(1, sizeof(alislce_dev_t));
    if (ce_dev == NULL || rootkey > ROOTKEY_MAX) {
        SL_ERR("alloc memory fail.\n");
        ret = ALISLCE_ERR_MALLOCFAIL;
        goto err;
    }

	/*
    device name    root_key_addr     rootkey_index    kl_device
    /dev/kl5:			0x59				3			  KL4
    /dev/kl4:			0x60				4             kL5
    /dev/kl3:			0x64				5			  KL4
    /dev/kl2:			0x55				2			  KL3
    /dev/kl1:			0x51				1			  KL2
    /dev/kl0:			0x4d				0			  KL1
	*/
	if (SL_CE_ROOT_KEY_DEFAULT == rootkey_type){
    	snprintf(ce_dev->name, sizeof(ce_dev->name), "/dev/kl/kl%d",rootkey_to_devname[rootkey]);
	}else if (SL_CE_ROOT_KEY_ETSI == rootkey_type){
		snprintf(ce_dev->name, sizeof(ce_dev->name),"/dev/kl/etsi%d",rootkey_to_devname[rootkey]);
	} else if (SL_CE_ROOT_KEY_CONAXVSC == rootkey_type){
		snprintf(ce_dev->name, sizeof(ce_dev->name),"/dev/kl/vsc%d",rootkey_to_devname[rootkey]);
	} else{
		SL_ERR("alloc memory fail.\n");
        ret = ALISLCE_ERR_INVALIDPARAM;
        goto err;
	}
    ce_dev->fd = open(ce_dev->name, O_RDWR);
    SL_DBG("ce_dev->name:%s,ce fd:%d\n",ce_dev->name,ce_dev->fd);
    if(0 > ce_dev->fd) {
        SL_ERR("open kl device error!\n");
        ret = ALISLCE_ERR_IOACCESS;
        goto err;
    }

    ce_dev->type = ALISLCE_DEV_TYPE;
    ce_dev->open_cnt = 0;
    ce_dev->priv = NULL;
    ce_dev->key_hdl = -1;
	ce_dev->target_fd = -1;
    *handle = (alisl_handle)ce_dev;
    pthread_mutex_unlock(&m_mutex);
    return ret;
err:
    if(NULL != ce_dev)
        free(ce_dev);
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

#if 0
alisl_retcode alislce_open_algo(alisl_handle *handle,int ca_algo)
{
    alislce_dev_t * ce_dev = (alislce_dev_t *)handle;
    int kl_fd = -1;

    if (NULL == handle || CA_ALGO_CSA3 != ca_algo || CA_ALGO_CSA2 != ca_algo
        || CA_ALGO_AES != ca_algo || CA_ALGO_TDES != ca_algo) {
        SL_ERR("ce handle is NULL\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }
    pthread_mutex_lock(&m_mutex);
    if ((CA_ALGO_CSA3 == ca_algo) && (ce_dev->flag & (1<<CA_ALGO_CSA3) != 0)) {/*if ce device have opened ,then don't open it again*/
        kl_fd = open("/dev/kl/kl3", O_RDWR);
        if(-1 == kl_fd)
            goto error;
        if(ce_dev->flag) { /* every ce handle only open one ce algo device*/
            close(ce_dev->fd);
            ce_dev->fd = -1;
            ce_dev->flag = 0;
        }
        ce_dev->fd = kl_fd;
        ce_dev->flag |= (1<<CA_ALGO_CSA3);
    } else if ((CA_ALGO_CSA2 == ca_algo) && (ce_dev->flag & (1<<CA_ALGO_CSA2) != 0)) {
        kl_fd = open("/dev/kl/kl0", O_RDWR);
        if(-1 == kl_fd)
            goto error;
        if(ce_dev->flag) { /* every ce handle only open one ce algo device*/
            close(ce_dev->fd);
            ce_dev->fd = -1;
            ce_dev->flag = 0;
        }
        ce_dev->fd = kl_fd;
        ce_dev->flag |= (1<<CA_ALGO_CSA2);
    } else if ((CA_ALGO_AES == ca_algo) && (ce_dev->flag & (1<<CA_ALGO_AES) != 0)) {
        kl_fd = open("/dev/kl/kl1", O_RDWR);
        if(-1 == kl_fd)
            goto error;
        if(ce_dev->flag) { /* every ce handle only open one ce algo device*/
            close(ce_dev->fd);
            ce_dev->fd = -1;
            ce_dev->flag = 0;
        }
        ce_dev->fd = kl_fd;
        ce_dev->flag |= (1<<CA_ALGO_AES);
    } else if ((CA_ALGO_TDES == ca_algo) && (ce_dev->flag & (1<<CA_ALGO_TDES) != 0)) {
        kl_fd = open("/dev/kl/kl2", O_RDWR);
        if(-1 == kl_fd)
            goto error;
        if(ce_dev->flag) { /* every ce handle only open one ce algo device*/
            close(ce_dev->fd);
            ce_dev->fd = -1;
            ce_dev->flag = 0;
        }
        ce_dev->fd = kl_fd;
        ce_dev->flag |= (1<<CA_ALGO_TDES);
    }

    pthread_mutex_unlock(&m_mutex);
    return 0;

error:
    pthread_mutex_unlock(&m_mutex);
    SL_ERR("open kl algo dev is NULL,%s\n",strerror(errno));
    return ALISLCE_ERR_CANTOPENDEV;

}
#endif

alisl_retcode alislce_close(alisl_handle handle)
{
    alisl_retcode ret = 0;
    alislce_dev_t * ce_dev = (alislce_dev_t *)handle;

    if(NULL == ce_dev) {
        SL_ERR("Invalid parameters!\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    pthread_mutex_lock(&m_mutex);

    ret |= close(ce_dev->fd);
    SL_DBG("close ce,ce_dev->fd = %d,ret: %d\n",ce_dev->fd,ret);
    if (ret != 0) {
        SL_ERR("close CE failed\n");
    }

	/*if cf has been opened,then close it*/
	if(ce_dev->target_fd > 0){
		ret |= close(ce_dev->target_fd);
        SL_DBG("close ce,ce_dev->target_fd = %d,ret: %d\n",ce_dev->target_fd,ret);
	    if (ret != 0) {
	        SL_ERR("close CE failed\n");
	    }
	}
    free(ce_dev);
    ce_dev = NULL;
//
//quit:
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alislce_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param)
{
    alislce_dev_t* ce_dev = NULL;
	int ret = -1;

    if (NULL == handle) {
        SL_ERR("ce handle is NULL\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    ce_dev = (alislce_dev_t *)handle;
    SL_DBG("ioctl ce_dev->fd: %d,cmd: 0x%x,param: 0x%x\n",ce_dev->fd,cmd,param);
	pthread_mutex_lock(&m_mutex);
	ret = ioctl(ce_dev->fd, cmd, param);
	pthread_mutex_unlock(&m_mutex);
    return ret;
}

/*alisl_retcode alislce_key_load(alisl_handle handle, OTP_PARAM * otp_info)
{
    alisl_retcode ret = 0;
    alislce_ioparam_t ce_param;
    alislce_dev_t * ce_dev = (alislce_dev_t *)handle;

    if ((ce_dev == NULL) || (otp_info == NULL))
    {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    ce_param.p[0] = (uint32_t)(ce_dev->priv);
    ce_param.p[1] = (uint32_t)otp_info;
    ret = alislce_ioctl(ce_dev, IO_CE_KEY_LOAD, &ce_param);
    if (ret != 0)
    {
        SL_ERR("CE key load fail, ret=%d.\n", ret);
    }

    return ret;

}*/


alisl_retcode alislce_generate_all_level_key(alisl_handle handle,
        struct kl_gen_key *data_info)
{
    alisl_retcode ret = 0;
    alislce_dev_t * ce_dev = (alislce_dev_t *)handle;

    if ((ce_dev == NULL) || (data_info == NULL)) {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }
    SL_DBG("call KL_GEN_KEY ce_dev->fd: %d\n",ce_dev->fd);
	pthread_mutex_lock(&m_mutex);
    ret = ioctl(ce_dev->fd, KL_GEN_KEY, (void *)data_info);
	pthread_mutex_unlock(&m_mutex);
    int i = 0;
    for(i = 0; i<KL_LEVEL_MAX-1; i++) {
	SL_DBG("data_info->pk[%d]", i);
	SL_DUMP("", data_info->pk[i],KL_KEY_SIZE_MAX);
    }
    SL_DUMP("data_info->key_even", data_info->key_even,KL_KEY_SIZE_MAX);
    SL_DUMP("data_info->key_odd", data_info->key_odd,KL_KEY_SIZE_MAX);
    SL_DBG("data_info->run_parity:%d\n",data_info->run_parity);
    if (ret != 0) {
        SL_ERR("CE generate single level key fail, ret=%d.\n", ret);
    }

    return ret;
}

alisl_retcode alislce_get_otp_root_key(alisl_handle handle,
                                       OTP_PARAM *otp_info)
{
    alisl_retcode ret = 0;
    alislce_dev_t * ce_dev = (alislce_dev_t *)handle;

    if ((ce_dev == NULL) || (otp_info == NULL)) {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    /*SL_DBG("DBG: fd: %d, otp_addr: %d, otp_key_pos: %d\n", ce_dev->fd,
            otp_info->otp_addr, otp_info->otp_key_pos); */
    SL_DBG("call IO_OTP_ROOT_KEY_GET,ce_dev->fd:%d,otp_info->otp_addr:%d,otp_info->otp_key_pos:%d\n",
                ce_dev->fd,otp_info->otp_addr,otp_info->otp_key_pos);
    pthread_mutex_lock(&m_mutex);
    ret = ioctl(ce_dev->fd, IO_OTP_ROOT_KEY_GET, otp_info);
	pthread_mutex_unlock(&m_mutex);
    if (ret != 0) {
        SL_ERR("CE get OTP root key fail, ret=%d.\n", ret);
    }
    return ret;
}

alisl_retcode alislce_get_decrypt_key(alisl_handle handle,
                                      unsigned char *key)
{
    alisl_retcode ret = 0;
    alislce_dev_t * ce_dev = (alislce_dev_t *)handle;
	struct kl_dbg_key buf;
	memset(&buf,0,sizeof(struct kl_dbg_key));


    if ((handle == NULL) || (key == NULL)) {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }
    SL_DBG("call KL_DBG_KEY,ce_dev->fd:%d\n",ce_dev->fd);    
	pthread_mutex_lock(&m_mutex);
    ret = ioctl(ce_dev->fd, KL_DBG_KEY, &buf);
	pthread_mutex_unlock(&m_mutex);
    if (ret != 0) {
        SL_ERR("CE get decrypt key fail, ret=%d.\n", ret);
    }
	SL_DUMP("key_result: ",(char *)buf.buffer,16);
	memcpy(key,(unsigned char *)buf.buffer,16);
    return ret;
}


alisl_retcode alislce_get_key_fd(alisl_handle handle,
                                    unsigned long *pos_param)
{
//    alisl_retcode ret = 0;
    alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
    if (handle == NULL || NULL == pos_param) {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }
    SL_DBG("ce pos == ce fd:%d\n",ce_dev->fd);
	*pos_param = ce_dev->fd;
    return 0;

}

alisl_retcode alislce_create_key_hdl(alisl_handle handle,
                                     struct kl_config_key *cfg_kl_key)
{
    alisl_retcode ret = 0;
    alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
    if( NULL == ce_dev || NULL == cfg_kl_key) {
        SL_ERR("Invalid parameters!\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }
    if(-1 != ce_dev->key_hdl) {
        SL_DBG("kl key handle have created!\n");
        return 0;
    }
    //cfg_kl_key->level = 1; /*because driver only support level = 1,otherwise return Invalid parameter error*/
    SL_DBG("call KL_CONFIG_KEY,ce_dev->fd = %d,cfg_kl_key->level = %d,cfg_kl_key->ck_size = %d,"\
                "cfg_kl_key->ck_parity = %d,cfg_kl_key->crypt_mode = %d,cfg_kl_key->algo = %d\n",
                ce_dev->fd,cfg_kl_key->level,cfg_kl_key->ck_size,
                cfg_kl_key->ck_parity,cfg_kl_key->crypt_mode,cfg_kl_key->algo);
	pthread_mutex_lock(&m_mutex);
	ret = ioctl(ce_dev->fd, KL_CONFIG_KEY, cfg_kl_key);
	pthread_mutex_unlock(&m_mutex);
    if(ret) {
        SL_ERR("create kl key handle fail!,error string:%s\n",strerror(errno));
        return ALISLCE_ERR_IOACCESS;
    }
    ce_dev->key_hdl = 1;/*flag kl key created*/
    return 0;
}


alisl_retcode alislce_set_key_pos_idle(alisl_handle handle,
                                       enum CE_CRYPT_TARGET key_pos)
{
//    alisl_retcode ret = 0;
//    alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
    return 0;
#if 0
    if (ce_dev == NULL) {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    ret = ioctl(ce_dev->fd, IO_CRYPT_POS_SET_IDLE, key_pos);
    if (ret != 0) {
        SL_DBG("CE set key pos idle fail, ret=%d.\n", ret);
    }
    return ret;
#endif
}

alisl_retcode alislce_key_pos_set_used(alisl_handle handle,
                                       const enum CE_CRYPT_TARGET key_pos)
{
    //alisl_retcode ret = 0;
    //alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
    return 0;
#if 0
    if (ce_dev == NULL) {
        SL_ERR("param invalid\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }

    ret = ioctl(ce_dev->fd, IO_CRYPT_POS_SET_USED, key_pos);
    if (ret != 0) {
        SL_DBG("CE set key pos used fail: ret=%d\n", ret);
    }

    return ret;
#endif
}

alisl_retcode alislce_gen_hdcp_key(alisl_handle handle,
                                       struct kl_gen_hdcp_key * gen_hdcp)
{
    alisl_retcode ret = 0;
    alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
    if (handle == NULL || NULL == gen_hdcp) {
	    SL_ERR("param invalid.\n");
	    return ALISLCE_ERR_INVALIDPARAM;
    }
    SL_DBG("call KL_GEN_HDCP_KEY ce_dev->fd:%d\n",ce_dev->fd);
    SL_DUMP("encryped hdcp key:",(char *)gen_hdcp->hdcp_pk,288);
	pthread_mutex_lock(&m_mutex);
	if (ioctl(ce_dev->fd, KL_GEN_HDCP_KEY, gen_hdcp) < 0)
	{
		ret = errno;
		SL_ERR("CE set key pos used fail: %s\n", strerror(ret));
	}
	pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alislce_get_fd(alisl_handle handle)
{
	alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
	if (handle == NULL) {
        SL_ERR("param invalid.\n");
        return ALISLCE_ERR_INVALIDPARAM;
    }
	return ce_dev->fd;
}

alisl_retcode alislce_gen_one_level_plus(alisl_handle handle,
	struct kl_cw_derivation *key_config)
{
	int ret = 0;
	alislce_dev_t *ce_dev = (alislce_dev_t *)handle;
	if (!handle || !key_config) {
		SL_ERR("param invalid.\n");
		return ALISLCE_ERR_INVALIDPARAM;
	}
    SL_DBG("call KL_DERIVE_CW\n");
	SL_DBG("ce_dev->fd: %d,ce_dev->target_fd: %d,ce_dev->target_parity: %d\n",
	       ce_dev->fd,ce_dev->target_fd,ce_dev->target_parity);
	SL_DBG("key_config->key_src: %d,key_config->data_src: %d,key_config->target_fd: %d,key_config->target_parity: %d\n",
	       key_config->key_src,key_config->data_src,key_config->target_fd,key_config->target_parity);

	if(key_config->key_src == KL_KEY_HW){
		SL_DBG("key_config.key.fd: %d\n",key_config->key.fd);
		SL_DBG("key_config.key.parity: %d\n",key_config->key.parity);
	}else{
		SL_DUMP("key_config->key.buf: ",(char *)key_config->key.buf,16);
	}

	if(key_config->data_src == KL_DATA_HW){
		SL_DBG("key_config.data.fd: %d\n",key_config->data.fd);
		SL_DBG("key_config.data.parity: %d\n",key_config->data.parity);
	}else{
		SL_DUMP("key_config->data.buf: ",(char *)key_config->data.buf,16);
	}

	pthread_mutex_lock(&m_mutex);
	if (ioctl(ce_dev->fd, KL_DERIVE_CW, key_config) < 0)
	{
		ret = errno;
		SL_ERR("CE set key pos used fail: %s\n", strerror(ret));
	}
	pthread_mutex_unlock(&m_mutex);

	return ret;
}

