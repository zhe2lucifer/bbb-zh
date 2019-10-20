/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisl_sha_api.c
 *  @brief              implement SHA function, provide relevant API
 *
 *  @version            1.0
 *  @date               07/02/2013 06:33:03 PM
 *  @revision           none
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 */


/* system header */
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
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/if_alg.h>

/* module header */
#include "dsc_internal.h"
#include "alisldsc.h"

static int fd = -1;

static pthread_mutex_t  m_mutex = PTHREAD_MUTEX_INITIALIZER;
static alisldsc_dev_t * m_dev[VIRTUAL_DEV_NUM];

#define ALI_SHA_DEV     "/dev/sha"
#define MAX_SHA_COMMON_LENGTH  (1024*1024)


/**
 *  @brief              open SHA device according to the available device id
 *
 *  @param[in]          dev_id          the available SHA device id
 *  @param[in/out]      handle          point to SHA device structure
 *
 *  @return             0               if successful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/01/2013, Created
 */
alisl_retcode alisldsc_sha_open(alisl_handle *handle, uint32_t dev_id)
{
    alisl_retcode ret = 0;
    struct dsc_see_dev_hld see_dev_hld;
    alisldsc_dev_t *sha_dev = (alisldsc_dev_t *)(*handle);

    if(dev_id >= VIRTUAL_DEV_NUM) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s dev_id %d is invalid.\n",
                       __func__, dev_id);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    pthread_mutex_lock(&m_mutex);
    if(NULL == m_dev[dev_id]) {
        sha_dev = (alisldsc_dev_t *)calloc(1, sizeof(alisldsc_dev_t));
        if (sha_dev == NULL) {
            ALISLDSC_DEBUG(ALISLDSC_ERR_MALLOCFAIL, "in %s alloc memory fail.\n",
                           __func__);
            pthread_mutex_unlock(&m_mutex);
            return ALISLDSC_ERR_MALLOCFAIL;
        }

        sha_dev->type = SHA;
        sha_dev->name = ALISLDSC_SHA_DEV_NAME;
        sha_dev->open_cnt = 0;
        sha_dev->fd = 0;

        sha_dev->fd = open(sha_dev->name, O_RDWR);
        if (sha_dev->fd < 0) {
            ALISLDSC_DEBUG(ALISLDSC_ERR_CANTOPENDEV, "in %s open device %s"
                           " fail.\n", __func__, sha_dev->name);
            free(sha_dev);
            pthread_mutex_unlock(&m_mutex);
            return ALISLDSC_ERR_CANTOPENDEV;
        }
        see_dev_hld.dsc_dev_id = dev_id;
        ret = ioctl(sha_dev->fd, IO_GET_DEV_HLD, &see_dev_hld);
        if (ret) {
            ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s get SHA SEE address fail,"
                           " err: %s\n", __func__, strerror(errno));
            close(sha_dev->fd);
            sha_dev->fd = 0;
            free(sha_dev);
            pthread_mutex_unlock(&m_mutex);
            return ALISLDSC_ERR_IOACCESS;
        }
        sha_dev->dev_id = dev_id;
        /* priv need to set to SEE device address */
        sha_dev->priv = (void *)(see_dev_hld.dsc_dev_hld);
        m_dev[dev_id] = sha_dev;
    }

    m_dev[dev_id]->open_cnt++;
    *handle = (alisl_handle *)m_dev[dev_id];
    pthread_mutex_unlock(&m_mutex);
    return ret;
}


/**
 *  @brief              close SHA device
 *
 *  @param[in]          handle      point to SHA device structure
 *
 *  @return             0           if successful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/05/2013, Created
 *
 *  @note
 */
alisl_retcode alisldsc_sha_close(alisl_handle handle)
{
    uint8_t i = 0;
    alisl_retcode ret = 0;
    uint32_t dev_id = 0xff;
    alisldsc_dev_t *sha_dev = (alisldsc_dev_t *)handle;

    pthread_mutex_lock(&m_mutex);
    dev_id = sha_dev->dev_id;
    if(--sha_dev->open_cnt) {
        /* this sub-device is still in use */
        goto quit;
    } else {
        for(i = 0; i < VIRTUAL_DEV_NUM; i++) {
            /* check whether other sub-device is close */
            if(i == dev_id)
                continue;
            if(m_dev[i] != NULL)
                break;
        }
        if(i >= VIRTUAL_DEV_NUM) {
            ret = close(sha_dev->fd);
            if (ret) {
                ALISLDSC_DEBUG(ALISLDSC_ERR_CLOSEERROR, "in %s close SHA fail,"
                               " err: %s\n", __func__, strerror(errno));
            }
        }
        free(m_dev[dev_id]);
        m_dev[dev_id] = NULL;
    }
quit:
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

/**
 *  @brief              use SHA device to decrypt data
 *
 *  @param[in]          input       point to the data need to decrypt
 *  @param[in]          data_len    the decrypt data len
 *  @param[in]          digest_len    the SHA digiest length
 *  @param[out]         output      save the decrypted data
 *
 *  @return             0           if successful, error code otherwise
 *
 */
static alisl_retcode sha_device_digest(uint8_t *input,
                                  uint8_t *output, uint32_t data_len,
                                  uint32_t digest_len)
{
	int /*fd,*/ ret;
    int type;
    
	struct digest dgt;

	//fd = open(ALI_SHA_DEV, O_RDWR);
	/*if (fd < 0) {
		printf("open %s failed: %s\n", ALI_SHA_DEV,
			strerror(errno));
		return ALISLDSC_ERR_CANTOPENDEV;
	}*/
    switch (digest_len)
    {
        case  SL_SHA1_DIGEST_SIZE:
            type = CA_SHA_TYPE_1;
            break;
        case  SL_SHA224_DIGEST_SIZE:
            type = CA_SHA_TYPE_224;
            break;
        case  SL_SHA256_DIGEST_SIZE:
            type = CA_SHA_TYPE_256;
            break;
        case  SL_SHA384_DIGEST_SIZE:
            type = CA_SHA_TYPE_384;
            break;
        case  SL_SHA512_DIGEST_SIZE:
            type = CA_SHA_TYPE_512;
            break;
        default:
            break;
    }
            
	ret = ioctl(fd, CA_SHA_SET_TYPE, &type);
	if (ret) {
		printf("CA_SHA_SET_TYPE err: %s\n",
			strerror(errno));
		return ALISLDSC_ERR_IOACCESS;
	}

	memset(&dgt, 0, sizeof(dgt));

	dgt.input = input;
	dgt.output = output;
	dgt.data_len = data_len;

	ret = ioctl(fd, CA_SHA_DIGEST, &dgt);
	if (ret) {
		printf("CA_SHA_DIGEST err: %s\n",
			strerror(errno));
		return -1;
	}

	//close(fd);

	return 0;
    
}


/**
 *  @brief              use socket to decrypt data
 *
 *  @param[in]          input       point to the data need to decrypt
 *  @param[in]          data_len    the decrypt data len
 *  @param[in]          digest_len    the SHA digiest length
 *  @param[out]         output      save the decrypted data
 *
 *  @return             0           if successful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/03/2013, Created
 */
alisl_retcode alisldsc_sha_digest(uint8_t *input,
                                  uint8_t *output, uint32_t data_len,
                                  uint32_t digest_len)
{
    alisl_retcode ret = 0;
    struct sockaddr_alg sa;
    int sock_fd = -1,opfd = -1;
    //int idx;
    //int fd = -1;

    SL_DBG("input:%08x,output:%08x,digest_len:%d, digest_len=%d\n",
        input,output,digest_len,digest_len);

    if (!input || !output) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s invalid param.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }
    //M3735 can not use socket to do SHA when the data size is larger than 1M,
    // we will use 'dev/sha' device to do SHA in place. oter solution(3505) has no 
    // the device 'dev/sha'
    if (data_len > MAX_SHA_COMMON_LENGTH)
    {
        //fd = open(ALI_SHA_DEV, O_RDWR);
        if (fd >= 0)
        {
            //close(fd);
            return sha_device_digest(input, output, data_len, digest_len);
        }
    }
    
    memset(&sa, 0, sizeof(struct sockaddr_alg));
    sa.salg_family = AF_ALG;
    memcpy(sa.salg_type, "hash", strlen("hash"));

    switch(digest_len) {
        case SL_SHA1_DIGEST_SIZE:
            memcpy(sa.salg_name, ALISLDSC_SHA1_NAME , sizeof(ALISLDSC_SHA1_NAME));
            break;
        case SL_SHA224_DIGEST_SIZE:
            memcpy(sa.salg_name, ALISLDSC_SHA224_NAME , sizeof(ALISLDSC_SHA224_NAME));
            break;
        case SL_SHA256_DIGEST_SIZE:
            memcpy(sa.salg_name, ALISLDSC_SHA256_NAME , sizeof(ALISLDSC_SHA256_NAME));
            break;
        case SL_SHA384_DIGEST_SIZE:
            memcpy(sa.salg_name, ALISLDSC_SHA384_NAME , sizeof(ALISLDSC_SHA384_NAME));
            break;
        case SL_SHA512_DIGEST_SIZE:
            memcpy(sa.salg_name, ALISLDSC_SHA512_NAME , sizeof(ALISLDSC_SHA512_NAME));
            break;
        default:
            ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s invalid param.\n",
                           __func__);
            return ALISLDSC_ERR_INVALIDPARAM;
    }
    SL_DBG("sa.salg_family:%d,sa.salg_type:%s,sa.salg_name:%s\n",sa.salg_family,sa.salg_type,
                 sa.salg_name);
    sock_fd = socket(AF_ALG, SOCK_SEQPACKET, 0);
    if (sock_fd < 0) {
        ret = errno;
        SL_DBG("socket error: errno[%d]:%s\n",
                     errno, strerror(errno));
        goto out;
    }
    SL_DBG("sock_fd:%d,AF_ALG:%d\n",sock_fd,AF_ALG);
    ret = bind(sock_fd, (struct sockaddr *)&sa, sizeof(sa));
    if (ret < 0) {
        ret  = errno;
        SL_DBG("bind error: errno[%d]:%s\n",
                     ret, strerror(ret));

        goto out;
    }

    opfd = accept(sock_fd, NULL, 0);
    if (opfd < 0) {
        ret  = errno;
        SL_DBG("accept error: errno[%d]:%s\n",
                     ret, strerror(ret));
        goto out;
    }

    ret = send(opfd, input, data_len, MSG_MORE);
    if (ret < 0) {
        ret  = errno;
        SL_DBG("accept error: errno[%d]:%s\n",
                     ret, strerror(ret));
        goto out;
    }
    ret = recv(opfd, output, digest_len, 0);
    if (ret < 0) {
        ret  = errno;
        SL_DBG("accept error: errno[%d]:%s\n",
                     ret, strerror(ret));

        goto out;
    }

    ret = 0;
out:
    if (opfd > 0)
        close(opfd);
    if (sock_fd > 0)
        close(sock_fd);

    return ret;
}

/**
 *  @brief              SHA ioctl
 *
 *  @param[in]          handle      point to SHA device structure
 *  @param[in]          cmd         ioctl command
 *  @param[in/out]      param       point to parameters
 *
 *  @return                         return ioctl return value directly
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               09/04/2014, Created
 */
alisl_retcode alisldsc_sha_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param)
{
    alisldsc_dev_t *sha_dev = NULL;
    ALI_DSC_IO_PARAM ioc_param;

    if (NULL == handle) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "SHA handle is NULL\n");
        return ALISLDSC_ERR_INVALIDPARAM;
    }
    sha_dev = (alisldsc_dev_t *)handle;

    ioc_param.dev = (void *)sha_dev->priv;
    ioc_param.ioc_param = (void *)param;
    return ioctl(sha_dev->fd, cmd, (uint32_t)&ioc_param);
}

/**
 *  @brief              use SHA device to decrypt data without init
 *
 *  @param[in]          handle      point to SHA device structure
 *  @param[in]          input       point to the data need to decrypt
 *  @param[in]          data_len    the decrypt data len
 *  @param[out]         output      save the decrypted data
 *
 *  @return             0           if successful, error code otherwise
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               10/22/2014, Created
 */
alisl_retcode alisldsc_sha_digest_without_init(alisl_handle handle, uint8_t *input,
        uint8_t *output, uint32_t data_len)
{
    alisl_retcode ret = 0;
    ALI_SHA_DIGEST sha_digest;
    alisldsc_dev_t * sha_dev = (alisldsc_dev_t *)handle;

    if (!sha_dev || !input || !output) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s invalid param.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    sha_digest.dev = sha_dev->priv;
    sha_digest.input = input;
    sha_digest.output = output;
    sha_digest.length = data_len;
    ret = ioctl(sha_dev->fd, ALI_DSC_IO_SHA_DIGEST, &sha_digest);
    if (ret) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_IOACCESS, "in %s SHA decrypt fail,"
                       " err: %s\n", __func__, strerror(errno));
    }

    return ret;
}


alisl_retcode alisldsc_sha_get_buffer(alisl_handle handle, unsigned long size,  void **pp_buf)
{
    // the parameter handle can be NULL so far
    
    alisldsc_dev_t * sha_dev = (alisldsc_dev_t *)handle;
    alisl_retcode ret = 0;
	void *mmap_addr = NULL;
    
    //int fd = -1; #change local variable to global
    
    if (!size) {
        ALISLDSC_DEBUG(ALISLDSC_ERR_INVALIDPARAM, "in %s invalid param.\n",
                       __func__);
        return ALISLDSC_ERR_INVALIDPARAM;
    }

    if (sha_dev)
    {
        fd = sha_dev->fd;
    }
    else
    {
        fd = open(ALI_SHA_DEV, O_RDWR);
        if (fd < 0)
        {
            ALISLDSC_DEBUG(ALISLDSC_ERR_CANTOPENDEV, "in %s open device %s"
                           " fail.\n", __func__, ALI_SHA_DEV);
            return ALISLDSC_ERR_CANTOPENDEV;
        }
    }
    mmap_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, \
                   MAP_SHARED, fd, 0);
    SL_DBG("mmap_addr = 0x%x\n", mmap_addr);
    
    if(mmap_addr == MAP_FAILED)
    {
        printf("mmap failed, error: %s\n", strerror(errno));
		return ALISLDSC_ERR_MMAPERROR;
    }
    *pp_buf = mmap_addr;

    /*if (!sha_dev && fd >= 0)
    {
        close(fd);
    }*/

    if(!(*pp_buf)) {
        ret = ALISLDSC_ERR_MALLOCFAIL;
    }

    return ret;
}

alisl_retcode alisldsc_sha_release_buffer(alisl_handle handle, void *p_buf, unsigned long size)
{
    int ret = -1;
    //int fd = -1;
    
    ret = munmap(p_buf,size);
	
    if (NULL == handle)
    {
        //may be the handle is NULL.
        
        //check if there have the SHA device.
        //fd = open(ALI_SHA_DEV, O_RDWR);
        if (fd < 0)
        {
            ALISLDSC_DEBUG(ALISLDSC_ERR_CANTOPENDEV, "in %s open device %s"
                           " fail.\n", __func__, ALI_SHA_DEV);
            return ALISLDSC_ERR_CANTOPENDEV;
        }
        close(fd);
    }
	
    return (alisl_retcode)ret;
}

