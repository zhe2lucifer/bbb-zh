/** @file     alislsdec.c
 *  @brief    alislsdec wraper functions file
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *
 *
 *
 */
/* system headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/* kernel headers */
#include <sys/ioctl.h>
#include <ali_sdec_common.h>

/* share library headers */
#include <alipltflog.h>
#include <alislsdec.h>
#include <bits_op.h>

/* local headers */
#include "internal.h"

struct sdec_private *sdec_dev = NULL;
static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief         sdec device construct
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is a void pointer
 * @param[out]
 * @return        alisl_retcode
 * @note          malloc memory for sdec device
 *                and init device.
 */
/*alisl_sdec_m36_attach*/
static alisl_retcode alislsdec_construct(void **handle)
{
	struct sdec_private *priv ;

	priv = malloc(sizeof(*priv));
	if (priv == NULL)
	{
		SL_DBG("SDEC malloc memory failed!\n");
		return ERROR_NOMEM;
	}
	memset(priv, 0, sizeof(*priv));

	priv->fd = -1;
 	priv->id = -1;
 	priv->status = SDEC_STATUS_CONSTRUCT;
 	*handle = priv;

	return ERROR_NONE;

}

/**
 * @brief         sdec device destruct
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note          free memory, close device handle
 *
 */
static alisl_retcode alislsdec_destruct(void **handle)
{
	struct sdec_private *priv = (struct sdec_private *)*handle;

	if (priv != NULL)
	{
		free(priv);
		*handle = NULL;
	}
	else
	{
		SL_DBG("sdec private is NULL before destruct");
		return ERROR_NONE;
	}

	return ERROR_NONE;
}


/**
 * @brief         open sdec device ali_sdec
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note          get sdec handle priv->fd
 *
 */
alisl_retcode alislsdec_open(void **handle)
{
	struct sdec_private *priv = NULL;

	pthread_mutex_lock(&m_mutex);
	if (sdec_dev == NULL)
	{
		alislsdec_construct((void **)&priv);

		if (NULL == priv || (priv->fd = open("/dev/ali_sdec", O_RDWR)) < 0)
		{
			SL_DBG("SDEC device open failed!\n");
			pthread_mutex_unlock(&m_mutex);
			return ERROR_OPEN;
		}

		bit_set(priv->status, SDEC_STATUS_OPEN);
		sdec_dev = priv;
	}
	sdec_dev->open_cnt++;
	*handle = sdec_dev;
	pthread_mutex_unlock(&m_mutex);
	return ERROR_NONE;
}


/**
 * @brief         close sdec handle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsdec_close(void *handle)
{
	struct sdec_private *priv = (struct sdec_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("NULL SDEC private data structure!\n");
		return ERROR_NOPRIV;
	}
	pthread_mutex_lock(&m_mutex);
	if(--priv->open_cnt)
	{
		pthread_mutex_unlock(&m_mutex);
		return ERROR_NONE;
	}
	close(priv->fd);
	priv->fd = -1;

	bit_clear(priv->status, SDEC_STATUS_STOP | SDEC_STATUS_OPEN);
	bit_set(priv->status, SDEC_STATUS_CLOSE);
	alislsdec_destruct((void **)&sdec_dev);

	pthread_mutex_unlock(&m_mutex);
	return ERROR_NONE;
}


/**
 * @brief         start sdec, ioctl transmit start parameters to driver
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer, composition_page_id and
 *                ancillary_page_id are page id from uplayer functions.
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode  alislsdec_start(void *handle,
				uint16_t composition_page_id,
				uint16_t ancillary_page_id)
{
	uint32_t arg;
	struct sdec_private *priv = (struct sdec_private *)handle;
	//int status = 0;

	if (priv == NULL)
	{
		SL_DBG("NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	bit_set(priv->status, SDEC_STATUS_START);
	arg = (uint32_t)((ancillary_page_id << 16) |composition_page_id);

	if (0 != ioctl(priv->fd, CALL_SDEC_START, (void *)arg))
	{
		SL_DBG("SDEC start error!\n");
		return ERROR_IOCTL;
	}
	return ERROR_NONE;
}


/**
 * @brief         stop sdec device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode  alislsdec_stop(void *handle)
{
	struct sdec_private *priv = (struct sdec_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("SDEC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	if (0 != ioctl(priv->fd, CALL_SDEC_STOP, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("SDEC stop error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         pause sdec device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode  alislsdec_pause(void *handle)
{
	struct sdec_private *priv = (struct sdec_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("SDEC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	if (0 != ioctl(priv->fd, CALL_SDEC_PAUSE, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("SDEC pause error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         show subtitle on osd
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsdec_show(void *handle)
{
	struct sdec_private *priv = (struct sdec_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("SDEC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	//show the subtitle on osd
	if (0 != ioctl(priv->fd, CALL_OSD_SUBT_ENTER, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("SDEC show error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         hide the subtitle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsdec_hide(void *handle)
{
	struct sdec_private *priv = (struct sdec_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("SDEC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}
	//hide the subtitle on osd
	if (0 != ioctl(priv->fd, CALL_OSD_SUBT_LEAVE, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("SDEC show error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         sdec ioctl, used for future
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     sdec device handle, ioctl cmd and param
 * @param[out]
 * @return        alisl_retcode
 * @note          no operation
 *
 */
alisl_retcode alislsdec_ioctl(void *handle, uint32_t cmd, uint32_t param)
{
	return ERROR_NONE;
}
