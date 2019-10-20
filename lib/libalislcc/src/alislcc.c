/** @file     alislcc.c
 *  @brief    alislcc wraper functions file
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
#include <bits_op.h>

/* share library headers */
#include <alipltflog.h>
#include <alislcc.h>

/* local headers */
#include "internal.h"

struct cc_private *cc_dev = NULL;
static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief         cc device construct
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is a void pointer
 * @param[out]
 * @return        alisl_retcode
 * @note          malloc memory for cc device
 *                and init device.
 */
/*alisl_cc_m36_attach*/
static alisl_retcode alislcc_construct(void **handle)
{
	struct cc_private *priv ;

	priv = malloc(sizeof(*priv));
	if (priv == NULL)
	{
		SL_DBG("CC malloc memory failed!\n");
		return ERROR_NOMEM;
	}
	memset(priv, 0, sizeof(*priv));

	priv->fd = -1;
 	priv->id = -1;
 	priv->status = CC_STATUS_CONSTRUCT;
 	*handle = priv;

	return ERROR_NONE;

}

/**
 * @brief         cc device destruct
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note          free memory, close device handle
 *
 */
static alisl_retcode alislcc_destruct(void **handle)
{
	struct cc_private *priv = (struct cc_private *)*handle;

	if (priv != NULL)
	{
		free(priv);
		*handle = NULL;
	}
	else
	{
		SL_DBG("cc private is NULL before destruct");
		return ERROR_NONE;
	}

	return ERROR_NONE;
}


/**
 * @brief         open cc device ali_cc
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note          get cc handle priv->fd
 *
 */
alisl_retcode alislcc_open(void **handle)
{
	struct cc_private *priv = NULL;

	pthread_mutex_lock(&m_mutex);
	if (cc_dev == NULL)
	{
		alislcc_construct((void **)&priv);

		if (NULL == priv || (priv->fd = open("/dev/ali_isdbtcc", O_RDWR)) < 0)
		{
			SL_DBG("CC device open failed!\n");
			pthread_mutex_unlock(&m_mutex);
			return ERROR_OPEN;
		}

		bit_set(priv->status, CC_STATUS_OPEN);
		cc_dev = priv;
	}
	cc_dev->open_cnt++;
	*handle = cc_dev;
	pthread_mutex_unlock(&m_mutex);
	return ERROR_NONE;
}


/**
 * @brief         close cc handle
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislcc_close(void *handle)
{
	struct cc_private *priv = (struct cc_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("NULL CC private data structure!\n");
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

	bit_clear(priv->status, CC_STATUS_STOP | CC_STATUS_OPEN);
	bit_set(priv->status, CC_STATUS_CLOSE);
	alislcc_destruct((void **)&cc_dev);

	pthread_mutex_unlock(&m_mutex);
	return ERROR_NONE;
}


/**
 * @brief         start cc, ioctl transmit start parameters to driver
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer, composition_page_id and
 *                ancillary_page_id are page id from uplayer functions.
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode  alislcc_start(void *handle,
				uint16_t composition_page_id,
				uint16_t ancillary_page_id)
{
	uint32_t arg;
	struct cc_private *priv = (struct cc_private *)handle;
	//int status = 0;

	if (priv == NULL)
	{
		SL_DBG("NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	bit_set(priv->status, CC_STATUS_START);
	arg = (uint32_t)((ancillary_page_id << 16) |composition_page_id);

	if (0 != ioctl(priv->fd, CALL_SDEC_START, (void *)arg))
	{
		SL_DBG("CC start error!\n");
		return ERROR_IOCTL;
	}
	return ERROR_NONE;
}


/**
 * @brief         stop cc device
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode  alislcc_stop(void *handle)
{
	struct cc_private *priv = (struct cc_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("CC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	if (0 != ioctl(priv->fd, CALL_SDEC_STOP, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("CC stop error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         pause cc device
 * @author        tom.xie
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode  alislcc_pause(void *handle)
{
	struct cc_private *priv = (struct cc_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("CC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	if (0 != ioctl(priv->fd, CALL_SDEC_PAUSE, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("CC pause error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         show subtitle on osd
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislcc_show(void *handle)
{
	struct cc_private *priv = (struct cc_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("CC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}

	//show the subtitle on osd
	if (0 != ioctl(priv->fd, CALL_OSD_ISDBT_CC_ENTER, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("CC show error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         hide the subtitle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislcc_hide(void *handle)
{
	struct cc_private *priv = (struct cc_private *)handle;

	if (priv == NULL)
	{
		SL_DBG("CC NULL private data structure!\n");
		return ERROR_NOPRIV;
	}
	//hide the subtitle on osd
	if (0 != ioctl(priv->fd, CALL_OSD_ISDBT_CC_LEASE, (void *)NULL))
	{
		return ERROR_IOCTL;
		SL_DBG("CC show error!\n");
	}
	return ERROR_NONE;

}

/**
 * @brief         cc ioctl, used for future
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cc device handle, ioctl cmd and param
 * @param[out]
 * @return        alisl_retcode
 * @note          no operation
 *
 */
alisl_retcode alislcc_ioctl(void *handle, uint32_t cmd, uint32_t param)
{
	return ERROR_NONE;
}

