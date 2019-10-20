/**@file
 *  (c) Copyright 2013-2063  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislotp.c
 *  @brief              Implementation of otp function
 *
 *  @version            1.0
 *  @date               07/06/2013 10:10:45 AM
 *  @revision           none
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <alipltflog.h>

/* otp driver header */
#include <alislotp.h>

/* WATCHDOG library header */
#include <ca_otp.h>

/* Internal header */
#include <internal.h>

/**
 *  Function Name:      alislotp_construct
 *  @brief              Initialization of otp device
 *
 *  @param              handle pointer to struct otp device
 *  @param              param  parameters use to Initialize devcie
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_construct(void **handle,
                                      alislotp_param_t *param)
{
	alislotp_dev_t *dev = NULL;

	if (NULL == param) {
		SL_DBG("The input parameter is NULL!\n");
		return ALISLOTP_ERR_INPUT_PARAM;
	}

	dev = (alislotp_dev_t *)malloc(sizeof(alislotp_dev_t));
	if (NULL == dev) {
		SL_DBG("In %s, Malloc memory failed!\n",
		                    __func__);
		return ALISLOTP_ERR_MALLOC;
	}

	dev->status = param->status;
	if (ALISLOTP_INACTIVATE == dev->status) {
		SL_DBG("The otp dev is not activate!\n");
        free(dev);
        dev = NULL;
		return ALISLOTP_ERR_INACTIVATE;
	}

	dev->dev_name = (unsigned char*)ALISLOTP_DEV_NAME;

	*handle = dev;

	return 0;
}

/**
 *  Function Name:      alislotp_destruct
 *  @brief              destruct otp device
 *
 *  @param              handle point to struct otp device
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_destruct(void **handle)
{
	if(NULL == *handle) return ALISLOTP_ERR_INPUT_PARAM;
	alislotp_dev_t *dev = (alislotp_dev_t *)(*handle);

	dev->handle = ALISLOTP_INVALID_HANDLE;
	dev->dev_name = 0;
	if (dev->status == ALISLOTP_ACTIVATE) {

		dev->status = ALISLOTP_INACTIVATE;
	}

	if (dev != NULL) {
		free(dev);
	} else {
		SL_DBG("The dev's memory has release.\n");
	}

	*handle = NULL;

	return 0;
}

/**
 *  Function Name:      alislotp_open
 *  @brief              open otp device
 *
 *  @param              handle point to struct otp device
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_open(void *handle)
{
	if(NULL == handle) return ALISLOTP_ERR_INPUT_PARAM;
	alislotp_dev_t *dev = (alislotp_dev_t *)handle;

	dev->handle = open((char *)dev->dev_name, O_RDWR);
	if (dev->handle < 0) {
		SL_DBG("The otp dev is not open!\n");
		return ALISLOTP_ERR_CANTOPEN;
	}

	return 0;
}

/**
 *  Function Name:      alislotp_close
 *  @brief              close otp device
 *
 *  @param              handle point to struct otp device
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_close(void *handle)
{
	if(NULL == handle) return ALISLOTP_ERR_INPUT_PARAM;
	alislotp_dev_t *dev = (alislotp_dev_t *)handle;

	close(dev->handle);

	return 0;
}

/**
 *  Function Name:      alislotp_ioctl
 *  @brief              choose function by command
 *
 *  @param              dev descirption of otp device
 *  @param              cmd command use to choose function of otp
 *  @param              arg point to parameter address
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
static alisl_retcode alislotp_ioctl(alislotp_dev_t *dev,
                                          unsigned int cmd, unsigned long arg)
{

	if(NULL == dev) return ALISLOTP_ERR_INPUT_PARAM;
	if (ALISLOTP_INACTIVATE == dev->status) {
		SL_DBG("In %s, The dev is not activate!\n",
                            __func__);
		return ALISLOTP_ERR_DEV_INACTIVAGE;
	}

	if (dev->handle < 0) {
		SL_DBG("The dev's handle is invalid!\n");
		return ALISLOTP_ERR_INVALID_HANDLE;
	}

	// SL_DBG("The command is %d, The arg is %d\n", cmd, arg);

	if (ioctl(dev->handle, cmd, (void *)arg) < 0) {
		SL_DBG("IO acess error!\n");
		return ALISLOTP_ERR_IOACCESS;
	}

	return 0;
}

/**
 *  Function Name:      alislotp_set_param
 *  @brief              set otp function by parameters
 *
 *  @param              handle  point to struct otp device
 *  @param              cmd command use to choose function of otp
 *  @param              arg point to parameter
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_set_param(void *handle, unsigned int cmd,
                                      unsigned long arg)
{
	alislotp_dev_t *dev = (alislotp_dev_t *)handle;
	alislotp_err_t ret  = 0;
	unsigned int c = 0;
	unsigned long p = 0;

	switch (cmd) {
	case ALISLOTP_CMD_READ:
		c =  ALI_OTP_READ;
		p = arg;
		break;

	case ALISLOTP_CMD_WRITE:
		c = ALI_OTP_WRITE;
		p = arg;
		break;
	default:
		SL_DBG("Command %d is over the cover!\n", cmd);
		break;
	}

	ret = alislotp_ioctl(dev, c, p);

	return ret;
}
