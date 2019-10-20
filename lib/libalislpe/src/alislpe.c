/** @file     alislpe.c
 *  @brief    play engine device function wrapping 
 *  @author   wendy.he
 *  @date     2014-8-12
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2014-2999 Copyright (C)
 */

/* system headers */
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

/* kernel headers */
#include <ali_pe_common.h>

/* share library headers */
#include <alipltflog.h>
#include <alislpe.h>

/* local headers */
#include "internal.h"

static struct pe_private *m_pe_dev = NULL;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief         pe device construct, malloc memory and init params
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle is pe device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislpe_construct(void **handle)
{
	struct pe_private *priv ;
		
	priv = malloc(sizeof(*priv));
	if (priv == NULL) {
		SL_DBG("pe malloc memory failed!\n");
		return PE_ERR_NOMEM;
	}
	
	memset(priv, 0, sizeof(*priv)); 
	priv->fd = -1;	
	*handle = priv;
	
	return PE_ERR_NOERR;
}

/**
 * @brief         pe device destruct, free memory, close device handle.
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle is pe device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislpe_destruct(void **handle)
{
	free(*handle);
	*handle = NULL;
	
	return PE_ERR_NOERR;
}


/**  
 * @brief         Open ali_pe0 for common usage. 
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle           pe device pointer
 * @param[out]   
 * @return        alisl_retcode    0:success, other: fail
 * @note
 *
 */
alisl_retcode alislpe_open(alisl_handle *handle)
{
	struct pe_private *priv = NULL;

	pthread_mutex_lock(&m_mutex);
	if (m_pe_dev == NULL) {
		alislpe_construct((void **)&priv);
		if(NULL == priv) {
			pthread_mutex_unlock(&m_mutex);
			return PE_ERR_NOMEM;
		}
		if ((priv->fd = open(ALISLPE_DEV_NAME, O_RDWR)) < 0) {
			SL_DBG("pe device open failed!\n");
			pthread_mutex_unlock(&m_mutex);
			return PE_ERR_FILEOPENFAILED;
		}
		m_pe_dev = priv;
	}
	m_pe_dev->open_cnt++;
	*handle = m_pe_dev;
	SL_DBG("pe_dev:%p pe_dev->fd:%d\n", m_pe_dev, m_pe_dev->fd);

	pthread_mutex_unlock(&m_mutex);
	return PE_ERR_NOERR;
}


/**
 * @brief         close pe device
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle           pe device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislpe_close(alisl_handle handle)
{
	struct pe_private *priv = (struct pe_private *)handle;

	if (priv == NULL) {
		SL_DBG("PE NULL private data structure!\n");
		return PE_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);
	
	if(--priv->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return PE_ERR_NOERR;
	}

	close(priv->fd);
	priv->fd = -1;
	alislpe_destruct((void **)&priv);
	m_pe_dev = NULL;
	pthread_mutex_unlock(&m_mutex);
	return PE_ERR_NOERR;
}

/**
 * @brief         get pe memory info
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle           pe device pointer
 * @param[out]    mem_info         pe memory info
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislpe_get_pe_mem_info(alisl_handle handle, 
    struct pe_mem_info* mem_info)
{
   	struct pe_private *priv = (struct pe_private *)handle;
    struct ali_pe_mem_info info;
    
	if (priv == NULL || mem_info == NULL) {
		SL_DBG("PE NULL data structure!\n");
		return PE_ERR_INVAL;
	}
	
	if (ioctl(priv->fd, ALIPEIO_MISC_GET_PE_MEM_INFO, &info)) {
		return PE_ERR_IOCTLFAILED;
	}
    mem_info->mem_start = info.mem_start;
    mem_info->mem_size = info.mem_size;
	return PE_ERR_NOERR; 
}
