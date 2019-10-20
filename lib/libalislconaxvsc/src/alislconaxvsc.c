/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislconaxvsc.c
 *  @brief              Conax Virtual Smart Card API
 *
 *  @version            1.0
 *  @date               01/09/2016 09:50:28 AM
 *  @revision           none
 *
 *  @authors            Deji Aribuki <deji.aribuki@alitech.com>
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
#include <sys/epoll.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <ca_vsc.h>
#include <ca_kl.h>
#include <ca_kl_vsc.h>

#include <alipltflog.h>
#include <alipltfretcode.h>

#include <alislevent.h>
#include <alislconaxvsc.h>
#include "internal.h"

#define FD_INVALID -1

static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
static int m_store_fd = FD_INVALID;
static struct alislevent slevent;
static alisl_handle slevent_handle = NULL;


alisl_retcode alislconaxvsc_open(alisl_handle *handle)
{
    alisl_retcode ret = 0;
    alislconaxvsc_dev_t *dev;

    dev = (alislconaxvsc_dev_t *) calloc(1, sizeof(alislconaxvsc_dev_t));
    if (dev == NULL) {
        SL_ERR("calloc failed\n");
        return ENOMEM;
    }

    dev->fd_smc = open("/dev/vsc0", O_WRONLY | O_CLOEXEC);
    SL_DBG("vsc open,dev_name: %s,dev->fd_smc: %d\n","/dev/vsc0",dev->fd_smc);
    if (dev->fd_smc == FD_INVALID) {
    	ret = errno;
    	SL_ERR("open vsc0 failed\n");
    	free(dev);
    	return ret;
    }

    *handle = (alisl_handle) dev;
    return 0;
}

alisl_retcode alislconaxvsc_close(alisl_handle handle)
{
    alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }
    SL_DBG("close vsc,dev->fd_smc=%d\n",dev->fd_smc);
    close(dev->fd_smc);
    dev->fd_smc = FD_INVALID;
    free(dev);

    return 0;
}


alisl_retcode alislconaxvsc_cmd_dispatch(alisl_handle handle,
	int session_id, uint8_t *cmd, int cmd_len, uint8_t *resp,
	int *resp_len, int *sw1, int *sw2)
{
    alisl_retcode ret = 0;
    struct vsc_cmd_transfer *tr;
    alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    tr = (struct vsc_cmd_transfer *) malloc(sizeof(struct vsc_cmd_transfer));
    if (tr == NULL) {
        SL_ERR("malloc tr failed\n");
        return ENOMEM;
    }

	tr->session_id = session_id;
	memcpy(tr->command, cmd, cmd_len);
	tr->num_to_write = cmd_len;
    SL_DBG("call VSC_CMD_DISPATCH,dev->fd_smc:%d,tr->session_id:%d,tr->num_to_write:%d\n",
                dev->fd_smc,tr->session_id,tr->num_to_write);
    //SL_DUMP("command:",(char *)tr->command,tr->num_to_write);     //generally needn't dump this value
	ret = ioctl(dev->fd_smc, VSC_CMD_DISPATCH, tr);
	if (ret) {
		ret = errno;
		SL_ERR("ioctl VSC_CMD_DISPATCH failed\n");
        free(tr);
		return ret;
	}
	SL_DBG("tr->response_len:%d,tr->sw1:%d,tr->sw2:%d\n",tr->response_len,tr->sw1,tr->sw2);
    //SL_DUMP("response:",(char *)tr->response,tr->response_len);       //generally needn't dump this value
	memcpy(&resp[0], tr->response, tr->response_len);
	*resp_len = tr->response_len;
	*sw1 = tr->sw1;
	*sw2 = tr->sw2;

    free(tr);

    return 0;
}

alisl_retcode alislconaxvsc_set_decw_key(alisl_handle handle,
	int kl_fd, uint16_t key_id, uint8_t *decw, int parity)
{
	alisl_retcode ret = 0;
	struct vsc_decw_key key;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    memset(&key, 0, sizeof(struct vsc_decw_key));
	key.kl_fd = kl_fd;
	key.key_id = key_id;
	key.ck_parity = parity;
	memcpy(&key.decw_key[0], decw, VSC_KEY_DECW_SIZE);
    SL_DBG("call VSC_DECW_KEY,dev->fd_smc:%d,key.kl_fd:%d,key.key_id:%d,key.ck_parity:%d\n",
                dev->fd_smc,key.kl_fd,key.key_id,key.ck_parity);
    SL_DUMP("decw_key:",(char *)key.decw_key,VSC_KEY_DECW_SIZE);
	ret = ioctl(dev->fd_smc, VSC_DECW_KEY, &key);
	if (ret) {
		ret = errno;
		SL_ERR("ioctl VSC_DECW_KEY failed\n");
		return ret;
	}

	return 0;
}

alisl_retcode alislconaxvsc_set_rec_en_key(alisl_handle handle,
	int kl_fd, uint8_t *en_key, int parity)
{
	alisl_retcode ret = 0;
	struct kl_vsc_rec_en_key key;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    memset(&key, 0, sizeof(struct kl_vsc_rec_en_key));
	memcpy(&key.en_key[0], en_key, KL_VSC_REC_KEY_SIZE);
	key.parity = parity;
    SL_DBG("call KL_VSC_REC_EN_KEY,kl_fd: %d,key.parity: %d\n",kl_fd,key.parity);
    SL_DUMP("en_key:",(char *)key.en_key,KL_VSC_REC_KEY_SIZE);
	ret = ioctl(kl_fd, KL_VSC_REC_EN_KEY, &key);
	if (ret) {
		ret = errno;
		SL_ERR("ioctl KL_VSC_REC_EN_KEY failed\n");
		return ret;
	}

	return 0;
}

alisl_retcode alislconaxvsc_set_uk_en_key(alisl_handle handle,
	int kl_fd, uint8_t *en_key)
{
	alisl_retcode ret = 0;
	struct kl_vsc_uk_en_key key;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    memset(&key, 0, sizeof(struct kl_vsc_uk_en_key));
	memcpy(&key.en_key[0], en_key, KL_VSC_UK_KEY_SIZE);
    SL_DBG("call KL_VSC_UK_EN_KEY,kl_fd: %d\n",kl_fd);
    SL_DUMP("en_key:",(char *)key.en_key,KL_VSC_UK_KEY_SIZE);
	ret = ioctl(kl_fd, KL_VSC_UK_EN_KEY, &key);
	if (ret) {
		ret = errno;
		SL_ERR("ioctl KL_VSC_UK_EN_KEY failed\n");
		return ret;
	}

	return 0;
}

alisl_retcode alislconaxvsc_lib_init(alisl_handle handle, uint8_t *data,
	uint8_t *key, uint8_t *hash)
{
	int ret = 0;
	int fd;
	struct vsc_lib_init_params init;
	struct vsc_store store;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    pthread_mutex_lock(&m_mutex);

    fd = open("/dev/vsc_store0", O_RDWR | O_CLOEXEC);
    SL_DBG("open dev_name: %s,fd: %d\n","/dev/vsc_store0",fd);
    if (fd == FD_INVALID) {
    	ret = errno;
    	SL_ERR("open vsc_store0 failed\n");
    	return ret;
    }

    if (data) {
    	if (!key || !hash) {
    		SL_ERR("key, hash parameters null\n");
    		goto exit;
    	}

    	memset(&store, 0, sizeof(struct vsc_store));
    	store.data = data;
    	store.data_len = VSC_STORE_DATA_SIZE;
    	memcpy(store.random_key, key, VSC_STORE_KEY_SIZE);
    	memcpy(store.hash, hash, VSC_STORE_HASH_SIZE);
        SL_DBG("call VSC_STORE_WRITE,fd:%d,store.data:%p,store.data_len:%ld\n",fd,store.data,store.data_len);
        SL_DUMP("random_key:",(char *)store.random_key,VSC_STORE_KEY_SIZE);
        SL_DUMP("hash:",(char *)store.hash,VSC_STORE_HASH_SIZE);
		ret = ioctl(fd, VSC_STORE_WRITE, &store);
		if (ret) {
			ret = errno;
			SL_ERR("ioctl VSC_STORE_WRITE failed\n");
			goto exit;
		}
    }

    memset(&init, 0, sizeof(struct vsc_lib_init_params));
	ret = ioctl(fd, VSC_LIB_INIT, &init);
    SL_DBG("call VSC_LIB_INIT,fd:%d,init.len:%d,ret:%d\n",fd,init.len,ret);
	if (ret) {
		ret = errno;
		SL_ERR("ioctl VSC_LIB_INIT failed\n");
		goto exit;
	}

	close(fd);
	pthread_mutex_unlock(&m_mutex);
	return 0;

exit:
	close(fd);
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

static void *alislconaxvsc_callback_func(void *param)
{
	int ret = 0;
	struct vsc_store store;
    struct alislconaxvsc_store st;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) param;

    if (dev == NULL || !dev->store_callback) {
    	SL_ERR("dev is null\n");
    	return NULL;
    }

    pthread_mutex_lock(&m_mutex);

	memset(&store, 0, sizeof(struct vsc_store));
	store.data_len = VSC_STORE_DATA_SIZE;
	store.data = malloc(VSC_STORE_DATA_SIZE);
	if (!store.data) {
		SL_ERR("malloc failed");
		goto exit;
	}

	ret = ioctl(m_store_fd, VSC_STORE_READ, &store);
    SL_DBG("call VSC_STORE_READ,m_store_fd:%d,store.data:%p,store.data_len:%ld,ret:%d\n",m_store_fd,store.data,store.data_len,ret);
    SL_DUMP("random_key:",(char *)store.random_key,VSC_STORE_KEY_SIZE);
    SL_DUMP("hash:",(char *)store.hash,VSC_STORE_HASH_SIZE);
	if (ret) {
		SL_ERR("ioctl VSC_STORE_READ failed\n");
		goto exit;
	}

    st.data = store.data;
    st.data_len = store.data_len;
    st.key = store.random_key;
    st.hash = store.hash;

	dev->store_callback(&st, dev->store_user_data);

exit:
	free(store.data);
	pthread_mutex_unlock(&m_mutex);
	return NULL;
}

alisl_retcode alislconaxvsc_register_callback(alisl_handle handle,
	void *p_user_data, alislconaxvsc_store_fun_cb callback)
{
	int ret = 0;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    pthread_mutex_lock(&m_mutex);

    if (m_store_fd != FD_INVALID) {
    	ret = EINVAL;
     	SL_ERR("store monitor running\n");
    	goto exit;
    }

    m_store_fd = open("/dev/vsc_store0", O_RDWR | O_CLOEXEC);
    SL_DBG("open dev_name: %s,m_store_fd: %d\n","/dev/vsc_store0",m_store_fd);
    if (m_store_fd == FD_INVALID) {
    	ret = errno;
    	SL_ERR("open vsc_store0 failed\n");
    	goto exit;
    }

    slevent.cb = alislconaxvsc_callback_func;
    slevent.data = (void *) dev;
    slevent.events = EPOLLIN;
    slevent.fd = m_store_fd;

    ret = alislevent_open(&slevent_handle);
    if (ret) {
     	SL_ERR("alislevent_open failed\n");
    	goto exit;
    }

    ret = alislevent_add(slevent_handle, &slevent);
    if (ret) {
      	SL_ERR("alislevent_add failed\n");
    	goto exit;
    }

    dev->store_user_data = p_user_data;
    dev->store_callback = callback;

	pthread_mutex_unlock(&m_mutex);
	return 0;

exit:
	close(m_store_fd);
	m_store_fd = FD_INVALID;
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

alisl_retcode alislconaxvsc_unregister_callback(alisl_handle handle)
{
	int ret = 0;
	alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }

    pthread_mutex_lock(&m_mutex);

    if (m_store_fd == FD_INVALID || !dev->store_callback) {
    	ret = EINVAL;
     	SL_ERR("store monitor not running\n");
    	goto exit;
    }

    alislevent_del(slevent_handle, &slevent);
    memset(&slevent, 0, sizeof(struct alislevent));
    alislevent_close(&slevent_handle);
    slevent_handle = NULL;
    close(m_store_fd);
    SL_DBG("close m_store_fd = %d\n",m_store_fd);
    m_store_fd = FD_INVALID;
    dev->store_user_data = NULL;
    dev->store_callback = NULL;

exit:
	pthread_mutex_unlock(&m_mutex);
	return ret;
}
#ifdef _CAS9_VSC_RAP_ENABLE_
alisl_retcode alislconaxvsc_get_keyid(alisl_handle handle,unsigned short* key_id)
{
    alisl_retcode ret = 0;
    alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }
    SL_DBG("call VSC_LIB_GET_KEY,dev->fd_smc:%d,key_id:%p\n",dev->fd_smc,key_id);
    ret = ioctl(dev->fd_smc, VSC_LIB_GET_KEY, key_id);
    if (ret) {
    	ret = errno;
    	SL_ERR("ioctl VSC_LIB_GET_KEY failed\n");
    	return ret;
    }
    return 0;
}

alisl_retcode alislconaxvsc_clear_store(alisl_handle handle)
{
    alisl_retcode ret = 0;
    alislconaxvsc_dev_t *dev = (alislconaxvsc_dev_t *) handle;

    if (dev == NULL) {
    	SL_ERR("dev is null\n");
    	return EINVAL;
    }
    SL_DBG("call VSC_CLEAR_STORE,dev->fd_smc: %d,param: 0\n",dev->fd_smc);
    ioctl(dev->fd_smc, VSC_CLEAR_STORE, 0);
    return 0;
}
#endif