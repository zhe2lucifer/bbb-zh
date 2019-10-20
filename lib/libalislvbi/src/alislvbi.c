/** @file     vbi share library file
 *  @brief    input file breif description here
 *  @author   ze.hong
 *  @date     2013-8-10 Iris.chen@20161018
 *  @version  2.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *             input file detail description here
 *             input file detail description here
 *             input file detail description here
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
#include <ali_vbi_common.h>
#include <adf_vbi.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alislvbi.h>
#include <bits_op.h>

/* local headers */
#include "internal.h"

static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;
struct vbi_private * vbi_dev = NULL;

/**
 * @brief         vbi device construct, malloc memory and init params
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     vbi is sound device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislvbi_construct(void **handle)
{

	struct vbi_private *priv ;
	
	priv = malloc(sizeof(*priv));
	if (priv == NULL) 
	{
		SL_DBG("VBI malloc memory failed!\n");
		return ERROR_NOMEM;
	}
	
	memset(priv, 0, sizeof(*priv)); 
	priv->fd = -1;
 	priv->id = -1;
 	priv->status = VBI_STATUS_CONSTRUCT;
	*handle = priv;
	
	return 0;
}


/**
 * @brief         vbi device destruct, free memory, close device handle.
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     vbi is sound device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislvbi_destruct(void **handle)
{
	struct vbi_private *priv = (struct vbi_private *)*handle;

	if (priv != NULL) 
	{		
		free(priv);
		*handle = NULL;
	}
	else
	{
		SL_DBG("vbi private is NULL before destruct");
		return 0;
	}

	return 0;
}


/**
*    @brief        vbi open
*    @author       ze.hong
*    @date         2013-6-19
*    @param[in]    handle is VBI device pointer
*    @param[out]   
*    @return       alisl_retcode
*    @note		
*
*/
alisl_retcode alislvbi_open(void **handle)
{
	struct vbi_private *priv = NULL;

	pthread_mutex_lock(&m_mutex);
	if (vbi_dev == NULL) 
	{
		alislvbi_construct((void **)&priv);
		if(NULL == priv)
		{
			pthread_mutex_unlock(&m_mutex);
			return ERROR_NOMEM;
		}
		SL_DBG("vbi open,dev_name: %s, priv->fd: %d\n","/dev/ali_vbi",priv->fd);
		if ((priv->fd = open("/dev/ali_vbi", O_RDWR)) < 0) 
		{
			SL_DBG("VBI device open failed!\n");
			
			pthread_mutex_unlock(&m_mutex);
			return ERROR_OPEN;
		}
		bit_set(priv->status, VBI_STATUS_OPEN);
		vbi_dev = priv;
	}
	
	vbi_dev->open_cnt++;
	*handle = vbi_dev;
	pthread_mutex_unlock(&m_mutex);
	return 0;
}

/**
 * @brief          close vbi 
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is vbi device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislvbi_close(void *handle)
{
	struct vbi_private *priv = (struct vbi_private *)handle;

	if (priv == NULL) 
	{
		SL_DBG("NULL VBI private data structure!\n");
		return ERROR_NOPRIV;
	}
	
	pthread_mutex_lock(&m_mutex);
	
	if(--priv->open_cnt)
	{
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}	
	SL_DBG("priv->fd: %d\n", priv->fd);
	close(priv->fd);
	priv->fd = -1;

	bit_clear(priv->status, VBI_STATUS_STOP | VBI_STATUS_OPEN);
	bit_set(priv->status, VBI_STATUS_CLOSE);
	
	alislvbi_destruct((void **)&vbi_dev);
	pthread_mutex_unlock(&m_mutex);

	return 0;
}



/**
 * @brief         vbi io control
 * @author        ze.hong
 * @date          2013-7-8   Iris.chen @20161018
 * @param[in]     cmd is io control command from uplayer
 * @param[out]    param2 param2_description
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislvbi_io_control(void *handle, uint32_t cmd, uint32_t param)
{
	struct vbi_private *priv = (struct vbi_private *)handle;
		
	if (priv == NULL)
	{
		SL_DBG("VBI NULL private data structure!\n");
		return ERROR_NOPRIV;
	}
	//printf("alislvbi_io_control:cmd=0x%x\n",cmd);
	switch (cmd)
	{
		case SL_IO_VBI_SET_SOURCE_TYPE:
			cmd = IO_VBI_SET_SOURCE_TYPE;
			SL_DBG("alislvbi_io_control:cmd=0x%x\n", cmd);

			switch (param)
			{
				case VBI_TYPE_CC:
					param = CC_VBI_TYPE;
					break;
		 		default:
					SL_ERR("input param error\n");
					return ERROR_INVAL;
					break;
			}

			break;
 		case SL_IO_VBI_CHECK_TTX_TASK_START:
			cmd = IO_VBI_CHECK_TTX_TASK_START;
			SL_DBG("alislvbi_io_control:cmd=0x%x\n", cmd);
			break;
		case SL_IO_VBI_SET_CC_TYPE:
			cmd = IO_VBI_SET_CC_TYPE;
			SL_DBG("alislvbi_io_control:cmd=0x%x\n", cmd);
			break;
		case SL_IO_VBI_GET_CC_TYPE:
			cmd = IO_VBI_GET_CC_TYPE;
			SL_DBG("alislvbi_io_control:cmd=0x%x\n", cmd);
			break;
		case SL_IO_VBI_TTX_STOP:
			cmd = IO_VBI_TTX_STOP;
			SL_DBG("alislvbi_io_control:cmd=0x%x\n", cmd);
			break;
		default:
			SL_ERR("input param error\n");
			return ERROR_INVAL;
			break;
	}
	

	SL_DBG("priv->fd: %d, cmd: 0x%x, param=%ld\n",priv->fd, cmd, param);
	if (0 != ioctl(priv->fd, cmd, param))
	{
		SL_DBG("alislvbi_io_control failed!\n");
		return ERROR_IOCTL;
	}

	return 0;
}

/**
 * @brief         write vbi data to TV  Encoder
 * @author        Iris.chen
 * @date          2016-10-18
 * @param[in]   handle is device pointer,
                       buffer is the data pointer
                       size  is data length
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislvbi_write(void *handle, char* buffer, uint32_t size)
{
	struct vbi_private *priv = (struct vbi_private *)handle;
	int ret=0;	
	
	if (priv == NULL)
	{
		SL_DBG("VBI NULL private data structure!\n");
		return ERROR_NOPRIV;
	}
	SL_DUMP("buffer: ", buffer, size);
	ret=write(priv->fd, buffer, size);
	if (0 != ret)
	{
		SL_DBG("alislvbi_write failed!\n");
		return ERROR_IOCTL;
	}
	
	return 0;
}

alisl_retcode alislvbi_set_output_device(void *handle, enum vbi_output_device output_device)
{
	/*Ignore this interface*/
	return 0;
	(void)handle;
	(void)output_device;
}

alisl_retcode alislvbi_get_output_callback(void *handle, 
													vbi_output_callback *output_callback)
{
	/*Ignore this interface*/
	return 0;
	(void)handle;
	(void)output_callback;
}

alisl_retcode alislvbi_start(void *handle)
{
	/*Ignore this interface*/
	return 0;
	(void)handle;
}

alisl_retcode alislvbi_stop(void *handle)
{
	/*Ignore this interface*/
	return 0;
	(void)handle;
}



