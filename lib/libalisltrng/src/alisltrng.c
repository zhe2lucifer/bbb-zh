/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file           trng.c
 *  @brief          implementation of ALi trng hardware interface
 *
 *  @version        1.0
 *  @date           06/03/2013 11:16:20 AM
 *  @revision       none
 *
 *  @author         Zhao Owen <Owen.Zhao@alitech.com>
 */

/* System header */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <ali_trng_common.h>

#include <alipltflog.h>

/* TRNG header */
#include <alisltrng.h>

/* Internal header */
#include "internal.h"

/**
 *  Function Name:	alisltrng_construct
 *  @brief			construct ALi hardware trng descriptor	
 *
 *  @param			dev		descriptor of the ALi trng
 *  @param			param	parameter used to initialize trng descriptor
 *
 *  @return			0		successful
 *  @return			others	error code
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/03/2013, Created
 *
 *  @note			
 */
alisl_retcode alisltrng_construct(alisltrng_dev_t *dev, 
								  alisltrng_param_t *param)
{
	dev->buf = NULL;
	dev->dev_name = ALISLTRNG_DEV_NAME;
	dev->handle = open(dev->dev_name, O_RDONLY);

	if (NULL != param)
		dev->series = param->series;

	if (dev->handle < 0) {
		SL_ERR("Can't open device %s\n", dev->dev_name);
		return ERROR_NOOPEN;
	}

	return 0;
}


/**
 *  Function Name:	alisltrng_destruct
 *  @brief			destroy the ALi trng descriptor
 *
 *  @param			dev		descriptor of the ALi trng
 *
 *  @return			always return 0 for successful
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_destruct(alisltrng_dev_t *dev)
{
	close(dev->handle);
	dev->handle = ALISLTRNG_INVALID_HANDLE;
	return 0;
}

/**
 *  Function Name:	alisltrng_ioctl
 *  @brief			command to ALi trng device	
 *
 *  @param			dev		descriptor of ALi trng device
 *  @param			cmd		command to device
 *  @param			param	parameter of the command
 *
 *  @return			0		successful
 *  @return			others	trng error code
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_ioctl(alisltrng_dev_t *dev, 
							  unsigned int cmd,
							  void *param)
{
	alisl_retcode ret = 0;
	alisltrng_ioparam_t *ioparam = (alisltrng_ioparam_t *)param;

	if (!(ioparam) || !(ioparam->data)) {
		SL_ERR("Null parameters\n");
		return ERROR_INVAL;
	}

	if (dev->handle < 0) {
		SL_ERR("Invalid device handle\n");
		return ERROR_INVAL;
	}

	ret = ioctl(dev->handle, cmd, ioparam);
	if (ret < 0) {
		SL_ERR("Command IO error %d\n", cmd);
		return ERROR_IOCTL;
	}

	return 0;
}

