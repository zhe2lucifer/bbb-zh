/** @file     alislsbm.c
 *  @brief    share buffer memory device function wrapping 
 *  @author   wendy.he
 *  @date     2014-8-13
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2014-2999 Copyright (C)
 */

/* system headers */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>

/* kernel headers */
#include <ali_sbm_common.h>

/* share library headers */
#include <alipltflog.h>
#include <alislsbm.h>

/* local headers */
#include "internal.h"

static struct sbm_private *m_sbm_dev[SBM_NUM_SBM] = {0};
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief         sbm device construct, malloc memory and init params
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle is sbm device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislsbm_construct(alisl_handle *handle)
{
	struct sbm_private *priv ;
		
	priv = malloc(sizeof(*priv));
	if (priv == NULL) {
		SL_ERR("sbm malloc memory failed!\n");
		return SBM_ERR_NOMEM;
	}
	
	memset(priv, 0, sizeof(*priv)); 
	priv->fd = -1;	
	*handle = priv;
	
	return SBM_ERR_NOERR;
}

/**
 * @brief         sbm device destruct, free memory, close device handle.
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle is sbm device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislsbm_destruct(alisl_handle *handle)
{
	free(*handle);
	*handle = NULL;
	
	return SBM_ERR_NOERR;
}

/**  
 * @brief         Open ali_sbm(xx) for common usage. 
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[in]     id               sbm device id
 * @param[out]   
 * @return        alisl_retcode    0:success, other: fail
 * @note
 *
 */
alisl_retcode alislsbm_open(alisl_handle *handle, enum sbm_id id)
{
	struct sbm_private *dev = NULL;
	alisl_retcode retcode;
	int i;
    
	for (i=0; i<ARRAY_SIZE(dev_sbm); i++) {
		if (id == dev_sbm[i].id) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev_sbm)) {
		SL_ERR("Invalidate sbm id!\n");
		return SBM_ERR_INVAL;
	}

	pthread_mutex_lock(&m_mutex);

	if (m_sbm_dev[i] == NULL) {
		if (alislsbm_construct((alisl_handle *)&dev)) {
			retcode = SBM_ERR_NOMEM;
			goto exit_fail;
		}
		dev->id = id;
	    dev->fd = open(dev_sbm[i].path, O_RDWR);
	    if (dev->fd < 0) {
		    SL_ERR("%s open failed!\n", dev_sbm[i].path);
		    retcode = SBM_ERR_FILEOPENFAILED;
		    goto exit_fail;
	    }
	    m_sbm_dev[i] = dev;
	} else {
		dev = m_sbm_dev[i];
	}

	dev->open_cnt++;
	*handle = dev;
	pthread_mutex_unlock(&m_mutex);
	return SBM_ERR_NOERR;

exit_fail:
    if (dev != NULL)
        free(dev);
    *handle = NULL;
    pthread_mutex_unlock(&m_mutex);
    return retcode;
}
/**
 * @brief         close sbm device
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_close(alisl_handle handle)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
	int i;

    if (dev == NULL) {
        SL_ERR("NULL dev data structure!\n");
        return SBM_ERR_INVAL;
    }
    
	for (i=0; i<ARRAY_SIZE(dev_sbm); i++) {
		if (dev->id == dev_sbm[i].id) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev_sbm)) {
		SL_ERR("Invalidate demux id!\n");
		return SBM_ERR_INVAL;
	}
	
	pthread_mutex_lock(&m_mutex);
	SL_DBG("dev:%p open_cnt:%d\n",dev,dev->open_cnt);
	if (--dev->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return SBM_ERR_NOERR;
	}
	close(dev->fd);
	dev->fd = -1;
	alislsbm_destruct((alisl_handle *)&dev);
	m_sbm_dev[i] = NULL;
	pthread_mutex_unlock(&m_mutex);

	return SBM_ERR_NOERR;
}

/**
 * @brief         create sbm               
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[in]     id               sbm device id
 * @param[in]     config           sbm config
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_create(alisl_handle handle,
    const struct sbm_buf_config *config)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    struct sbm_config sbm_info;
    
	if (dev == NULL || config == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	
    sbm_info.buffer_addr = config->buffer_addr;
    sbm_info.buffer_size = config->buffer_size;
    sbm_info.block_size = config->block_size;
    sbm_info.reserve_size = config->reserve_size;
    sbm_info.wrap_mode = config->wrap_mode;
    sbm_info.lock_mode = config->lock_mode;
    
	if(ioctl(dev->fd, SBMIO_CREATE_SBM, &sbm_info)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}

/**
 * @brief         destroy sbm
 *                The sbm should be destroyed when no longer needed
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_destroy(alisl_handle handle)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    
	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	    
	if(ioctl(dev->fd, SBMIO_DESTROY_SBM)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}

/**
 * @brief         reset sbm
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_reset(alisl_handle handle)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    
	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	    
	if(ioctl(dev->fd, SBMIO_RESET_SBM)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}

/**
 * @brief         write data to sbm
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[in]     buf              data to write
 * @param[in]     len              buf size
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_write(alisl_handle handle, unsigned char *buf, unsigned long size)
{
   	struct sbm_private *dev = (struct sbm_private *)handle;
    unsigned int write_size;

	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
    write_size = write(dev->fd, buf, size);
    if(write_size != size) {
        return SBM_ERR_FAILED;
    }

	return SBM_ERR_NOERR; 
}


/**
 * @brief         get valid size
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm valid size
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_valid_size(alisl_handle handle, unsigned long* size)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    
	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	    
	if(ioctl(dev->fd, SBMIO_SHOW_VALID_SIZE, size)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}

/**
 * @brief         get free size
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm free size
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_free_size(alisl_handle handle, unsigned long * size)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    
	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	    
	if(ioctl(dev->fd, SBMIO_SHOW_FREE_SIZE, size)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}

/**
 * @brief         get total size
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm total size
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_total_size(alisl_handle handle, unsigned long* size)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    
	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	    
	if(ioctl(dev->fd, SBMIO_SHOW_TOTAL_SIZE, size)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}

/**
 * @brief         get packet number
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm packet number
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_pkt_num(alisl_handle handle, unsigned int* num)
{
	struct sbm_private *dev = (struct sbm_private *)handle;
    
	if (dev == NULL) {
		SL_ERR("NULL dev data structure!\n");
		return SBM_ERR_INVAL;
	}
	    
	if(ioctl(dev->fd, SBMIO_SHOW_PKT_NUM, num)){
        return SBM_ERR_IOCTLFAILED;
	}

	return SBM_ERR_NOERR;
}
