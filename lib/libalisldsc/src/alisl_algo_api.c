/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisl_algo_api.c
 *  @brief              management of cryptographic algorithms: AES, CSA, TDES
 *
 *  @version            1.0
 *  @date               07/01/2013 02:51:28 PM
 *  @revision           none
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
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
#include <errno.h>

#include "dsc_internal.h"
#include "alisldsc.h"
#include "ca_dsc.h"

/* MAX_ALGO is 5, but RD should know that the number of algorithms managed
 * here is 3, reason is:
 * 1. DES and TDES is the same device for driver
 * 2. SHA resource is not managed in this file.
 * by B2400 Will.Qian */
#define MAX_ALGO 5

static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

/* In fact, resources are managed by ALi DSC dirver.
 * As we open a new algorithm device, we should call interface
 * alisldsc_dsc_get_free_stream_id and alisldsc_dsc_get_free_subdev first,
 * to make sure that the device can actually be used now.
 * every sub device can be opened multi-times for share library as other
 * modules do.
 * by B2400 Will.Qian */
static int dev_reserved[MAX_ALGO][VIRTUAL_DEV_NUM];

alisldsc_dev_t *g_algo_dev[MAX_ALGO][VIRTUAL_DEV_NUM];

static int g_algo_initialized = 0;

/* array must be aligned with enum WORK_SUB_MODULE */
struct algo_res {
    int algo;
    int type;
    char dev_name[20];
} algo_res[] = {
    { DES, DES, ALISLDSC_DES_DEV_NAME },
    { AES, AES, ALISLDSC_AES_DEV_NAME },
    { SHA, -1, "" },
    { TDES, DES, ALISLDSC_DES_DEV_NAME },
    { CSA, CSA, ALISLDSC_CSA_DEV_NAME }
};

#define alisldsc_algo_map(algo) ((algo_res[algo].algo == (algo)) ? algo_res[algo].type : -1)

alisl_retcode alisldsc_algo_open(alisl_handle *handle, algo_t algo, uint32_t dev_id)
{
    alisl_retcode ret = 0;
    struct dsc_see_dev_hld see_dev_hld;
    alisldsc_dev_t *algo_dev = (alisldsc_dev_t *)(*handle);
    int algo_res_type = -1, i, algo_used;

    if (!g_algo_initialized) {
        memset(dev_reserved, 0, sizeof(dev_reserved));
        memset(g_algo_dev, 0, sizeof(g_algo_dev));
        g_algo_initialized = 1;
    }

    if (dev_id >= VIRTUAL_DEV_NUM) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s: dev_id out of range\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    algo_res_type = alisldsc_algo_map(algo);
    if (algo_res_type < 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s: algo is wrong\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    pthread_mutex_lock(&m_mutex);

    if (dev_reserved[algo_res_type][dev_id] > 0) {
        *handle = g_algo_dev[algo_res_type][dev_id];
        dev_reserved[algo_res_type][dev_id]++;
        pthread_mutex_unlock(&m_mutex);
        return ret;
    }

    algo_dev = (alisldsc_dev_t *)calloc(1, sizeof(alisldsc_dev_t));
    if (!algo_dev) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_MALLOCFAIL, "in %s alloc memory fail.\n",
                       __func__);
        pthread_mutex_unlock(&m_mutex);
        return ALISLDSC_ERR_MALLOCFAIL;
    }

    for (i = 0; i < VIRTUAL_DEV_NUM; i++)
        if (dev_reserved[algo_res_type][i])
            algo_dev->fd = g_algo_dev[algo_res_type][i]->fd;

    algo_used = (algo_dev->fd != 0);

    if (!algo_used) {
        algo_dev->name = algo_res[algo].dev_name;

        algo_dev->fd = open(algo_dev->name, O_RDONLY);
        if (algo_dev->fd < 0) {
            ALISLDSC_DEBUG(ALISLDSC_ERR_CANTOPENDEV, "in %s open device %s "
                           "fail.\n", __func__, algo_dev->name);
            free(algo_dev);
            pthread_mutex_unlock(&m_mutex);
            return ALISLDSC_ERR_CANTOPENDEV;
        }
    }

    see_dev_hld.dsc_dev_id = dev_id;
    ret = ioctl(algo_dev->fd, IO_GET_DEV_HLD, &see_dev_hld);
    if (ret) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s get algo SEE address fail,"
                       " ret=%d\n", __func__, ret);

        if (!algo_used)
            close(algo_dev->fd);

        free(algo_dev);
        pthread_mutex_unlock(&m_mutex);
        return ret;
    }
    algo_dev->type = algo_res_type;
    algo_dev->dev_id = dev_id;
    /* priv need to set to SEE device address */
    algo_dev->priv = (void *)(see_dev_hld.dsc_dev_hld);

    dev_reserved[algo_res_type][dev_id]++;

    g_algo_dev[algo_res_type][dev_id] = algo_dev;
    *handle = algo_dev;
    pthread_mutex_unlock(&m_mutex);
    return ret;
}


alisl_retcode alisldsc_algo_close(alisl_handle handle)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *algo_dev = (alisldsc_dev_t *)handle;
    int algo_used = 0, i;

    if (!handle) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_CLOSEERROR, "in %s close algo failed",
                       __func__);
        return ALISLDSC_ERR_INVALIDHANDLE;
    }
    pthread_mutex_lock(&m_mutex);
    dev_reserved[algo_dev->type][algo_dev->dev_id]--;

    if (dev_reserved[algo_dev->type][algo_dev->dev_id] > 0) {
        pthread_mutex_unlock(&m_mutex);
        return ret;
    }
    for (i=0; i<VIRTUAL_DEV_NUM; i++)
        if (dev_reserved[algo_dev->type][i])
            algo_used++;

    /* if no more dev_id is using the same algorithm, close algo device */
    if (!algo_used) {
        ret = close(algo_dev->fd);
        if (ret) {
            ALISLDSC_DEBUG(ALISLDSC_ERR_CLOSEERROR, "in %s close AES fail, "
                           "ret=%d\n", __func__, ret);
        }
    }
    g_algo_dev[algo_dev->type][algo_dev->dev_id] = NULL;
    free(algo_dev);
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

alisl_retcode alisldsc_algo_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param)
{
    alisldsc_dev_t *algo_dev = NULL;
    ALI_DSC_IO_PARAM ioc_param;

    if (NULL == handle) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "algorithm handle is NULL\n");
        return ALISLDSC_ERR_INVALIDPARAM;
    }
    algo_dev = (alisldsc_dev_t *)handle;

    ioc_param.dev = (void *)algo_dev->priv;
    ioc_param.ioc_param = (void *)param;
    return ioctl(algo_dev->fd, cmd, (uint32_t)&ioc_param);
}

/* for alisl DMX use only */
alisl_retcode alisldsc_algo_get_priv(alisl_handle handle, void **priv)
{
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    if (!dev || !priv)  {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    *priv = (void *)dev->priv;
    return 0;
}

alisl_retcode alisldsc_algo_delete_crypt_stream(alisl_handle handle,
        uint32_t stream_handler)
{
    alisl_retcode ret = 0;
    ALI_DSC_IO_PARAM algo_param;
    alisldsc_dev_t * algo_dev = (alisldsc_dev_t *)handle;

    if (algo_dev == NULL) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    algo_param.dev = algo_dev->priv;
    algo_param.ioc_param = (void *)stream_handler;
    ret = ioctl(algo_dev->fd, IO_DELETE_CRYPT_STREAM_CMD, &algo_param);
    if (ret != 0) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s delete crypt stream fail,"
                       " ret=%d.\n", __func__, ret);
    }
    return ret;
}

alisl_retcode alisldsc_algo_create_crypt_stream(alisl_handle handle,
        void *algo_param,
        KEY_PARAM *key_param)
{
//    alisl_retcode ret = 0;
 //   alisldsc_dev_t *dsc_dev = (alisldsc_dev_t*)handle;


    if ((handle == NULL) || (algo_param == NULL) || (key_param == NULL)) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s param invalid.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }
 

    return 0;
}

alisl_retcode alisldsc_dsc_get_free_stream_id(alisl_handle handle, enum DMA_MODE mode,
        uint32_t * stream_id)
{
    alisl_retcode ret = 0;
    ALI_DSC_ID_INFO dsc_param;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    dsc_param.mode = (uint32_t)mode;
    dsc_param.id_number = (uint32_t)stream_id;
    ret = ioctl(dev->fd, ALI_DSC_IO_GET_FREE_STREAM_ID, &dsc_param);
    if (ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s get free stream id fail,"
                       " err: %s\n", __func__, strerror(errno));
    return ret;
}


alisl_retcode alisldsc_dsc_parse_stream_id(alisl_handle handle, uint32_t dmx_fd)
{
    (void)(handle);
    (void)(dmx_fd);
    return 0;
}


alisl_retcode alisldsc_dsc_get_subdev_handle(alisl_handle handle,
        algo_t algo,
        uint32_t *dev_handler)
{
    alisl_retcode ret = 0;
    uint32_t cmd;
    alisldsc_dev_t * dev = (alisldsc_dev_t *)handle;

    switch(algo) {
        case DES:
        case TDES:
            cmd = IO_DSC_GET_DES_HANDLE;
            break;
        case AES:
            cmd = IO_DSC_GET_AES_HANDLE;
            break;
        case SHA:
            cmd = IO_DSC_GET_SHA_HANDLE;
            break;
        case CSA:
            cmd = IO_DSC_GET_CSA_HANDLE;
            break;
        default:
            cmd = 0xff;
            ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s don't have this"
                           " sub device, sub_mode=%d\n", __func__, algo);
            break;
    }

    ret = ioctl(dev->fd, cmd, dev_handler);
    if (ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s get sub device"
                       " handle fail, ret=%d\n", __func__, ret);
    return ret;
}



alisl_retcode alisldsc_dsc_set_subdev_idle(alisl_handle handle,
        algo_t algo,
        uint32_t dev_id)
{
    alisl_retcode ret = 0;
    ALI_DSC_ID_INFO dsc_param;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    dsc_param.mode = (uint32_t)algo;
    dsc_param.id_number = dev_id;
    ret = ioctl(dev->fd, ALI_DSC_IO_SET_SUB_DEVICE_ID_IDLE, &dsc_param);
    if (ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s set sub device idle fail,"
                       " ret=%d\n", __func__, ret);
    return ret;
}





alisl_retcode alisldsc_dsc_set_stream_id_idle(alisl_handle handle, uint32_t stream_id)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    ret = ioctl(dev->fd, ALI_DSC_IO_SET_STREAM_ID_IDLE, stream_id);
    if (ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s set stream id idle fail,"
                       " err: %s\n", __func__, strerror(errno));
    return ret;
}

alisl_retcode alisldsc_dsc_set_stream_id_used(alisl_handle handle, uint32_t stream_id)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    ret = ioctl(dev->fd, ALI_DSC_IO_SET_STREAM_ID_USED, stream_id);
    if (ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s set stream id idle fail,"
                       " err: %s\n", __func__, strerror(errno));
    return ret;
}


alisl_retcode alisldsc_dsc_set_pvr_stream_idle(alisl_handle handle,
        uint32_t stream_id)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    ret = ioctl(dev->fd, IO_DSC_SET_PVR_KEY_IDLE, stream_id);
    if (ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s clear descramble's pvr"
                       " key fail, ret=%d\n", __func__, ret);
    return ret;
}


alisl_retcode alisldsc_dsc_set_pvr_key_param(alisl_handle handle,
        DSC_PVR_KEY_PARAM *key_param)
{
    alisl_retcode ret = 0;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    ret = ioctl(dev->fd, IO_DSC_SET_PVR_KEY_PARAM, key_param);
    if(ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s set pvr key param fail,"
                       " ret=%d\n", __func__, ret);
    return ret;
}

alisl_retcode alisldsc_dsc_get_free_subdev(alisl_handle handle,
        algo_t algo,
        uint32_t *dev_id)
{
    alisl_retcode ret = 0;
    uint32_t device_id = 0xff;
    ALI_DSC_ID_INFO dsc_param;
    alisldsc_dev_t *dev = (alisldsc_dev_t *)handle;

    dsc_param.mode = (uint32_t)algo;
    dsc_param.id_number = (uint32_t)&device_id;
    ret = ioctl(dev->fd, ALI_DSC_IO_GET_FREE_SUB_DEVICE_ID, &dsc_param);
    if(ret)
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s get subdev fail, err: %s\n",
                       __func__, strerror(errno));
    else
        *dev_id = device_id;
    return ret;
}
