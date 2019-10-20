/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislscart.c
 *  @brief          Implementation of SCART share library
 *
 *  @version        1.0
 *  @date           06/21/2013 03:47:23 PM
 *  @revision       none
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <alipltflog.h>

/* SCART driver header */
#include <ali_scart_common.h>

/* SCART lib header */
#include <alislscart.h>

/* Internal header */
#include <internal.h>

/* Define SCART device */
alislscart_dev_attr_t alislscart_dev_attr[] = {
	{ALISLSCART_ID_SCART0, ALISLSCART_DEV0_NAME},
	{ALISLSCART_ID_SCART1, ALISLSCART_DEV1_NAME}
};

/**
 *  Function Name:  alislscart_construct
 *  @brief          initalize device by input parameter
 *
 *  @param          handle      point to struct SCART device 
 *  @param          param input parameter use to initalize device
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_construct(alisl_handle *handle,
                                   alislscart_param_t *param)
{
	alislscart_dev_t *dev;

	if (NULL == param) {
		SL_DBG("The input param is NULL!\n");
		return ALISLSCART_ERR_INPUT_PARAM;
	}

	dev = malloc(sizeof(alislscart_dev_t));
	if (NULL == dev) {
		SL_DBG("The malloc device fail.\n");
		return ALISLSCART_ERR_MALLOC_MEM;
	}

	dev->status = param->status;
	if (ALISLSCART_DEV_INACTIVATE == dev->status) {
		SL_DBG("The SCART dev is not activate!\n");
        free(dev);
        dev = NULL;
		return ALISLSCART_ERR_DEV_INACTIVATE;
	}
	
	dev->deviceid = param->deviceid;
	dev->dev_name = alislscart_dev_attr[dev->deviceid].dev_name;

	SL_DBG("The SCART deviceid is %d, device'name is %s\n",
	       dev->deviceid, dev->dev_name);

	*handle = dev;
    
	return 0;
}

/**
 *  Function Name:  alislscart_destruct
 *  @brief          destruct device of SCART
 *
 *  @param          handle point to struct SCART device 
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_destruct(alisl_handle *handle)
{
	alislscart_dev_t *dev = (alislscart_dev_t *)(*handle);

	if (dev != NULL) {
		free(dev);
	} else {
		SL_DBG("The dev has free.");
	}

	*handle = NULL;
	
	return 0;
}

/**
 *  Function Name:      alislscart_open
 *  @brief              open device of SCART
 *
 *  @param              handle point to struct SCART device 
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/11/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_open(alisl_handle handle)
{
	alislscart_dev_t *dev = (alislscart_dev_t *)handle;
	if (!dev) {
		return ALISLSCART_ERR_DEV_CANTOPEN;
	}

	dev->handle = open(dev->dev_name, O_RDWR);
	if (0 > dev->handle) {
		SL_DBG("The SCART dev can't open!\n");
		return ALISLSCART_ERR_DEV_CANTOPEN;
	}

	return 0;
}

/**
 *  Function Name:      alislscart_close
 *  @brief              close device of SCART
 *
 *  @param              handle point to struct SCART device 
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/11/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_close(alisl_handle handle)
{
	alislscart_dev_t *dev = (alislscart_dev_t *)handle;
	if (!dev) {
		return ALISLSCART_ERR_DEV_CANTOPEN;
	}
	
	close(dev->handle);
	dev->handle = ALISLSCART_INVALID_HANDLE;
	dev->status = ALISLSCART_DEV_INACTIVATE;
    
	return 0;
}

/**
 *  Function Name:  alislscart_ioctl
 *  @brief          choose function by ioctl
 *
 *  @param          dev description of SCART device
 *  @param          cmd command of SCART funcation
 *  @param          arg in or out param for SCART function
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_ioctl(alislscart_dev_t *dev,
                               unsigned int cmd, unsigned long arg)
{
	if (!dev) {
		return ALISLSCART_ERR_DEV_IOACCESS;
	}

	if (ALISLSCART_DEV_INACTIVATE == dev->status) {
		SL_DBG("SCART dev is not activate!\n");
		return ALISLSCART_ERR_DEV_INACTIVATE;
	}

	if (dev->handle < 0) {
		SL_DBG("The dev' handle is invalild!\n");
		return ALISLSCART_ERR_INVALID_HANDLE;
	}

	SL_DBG("The cmd: %d, The input arg: %ld\n", cmd, arg);
	
	if (ioctl(dev->handle, cmd, (void *)arg) < 0) {
		SL_DBG("The IO access is error!\n");
		return ALISLSCART_ERR_DEV_IOACCESS;
	}
	
	return 0;
}

/**
 *  Function Name:  alislscart_set_param
 *  @brief          set parameters of SCART device
 *
 *  @param          handle point to struct SCART device 
 *  @param          cmd command of SCART funcation
 *  @param          arg in or out param for SCART function
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_param(alisl_handle handle,
                                   unsigned int cmd, unsigned long arg)
{
	alislscart_err_t ret = 0;
	unsigned int c = 0;
	unsigned long p = 0;

	alislscart_dev_t *dev = (alislscart_dev_t *)handle;

	switch (cmd) {
	case ALISLSCART_CMD_SET_TV_SOURCE:     { c = SCART_TV_SOURCE;     break; }
	case ALISLSCART_CMD_SET_VCR_SOURCE:    { c = SCART_VCR_SOURCE;    break; }
	case ALISLSCART_CMD_SET_TV_ASPECT:     { c = SCART_TV_ASPECT;     break; }
	case ALISLSCART_CMD_SET_TV_FORMAT:     { c = SCART_TV_MODE;       break; }
	case ALISLSCART_CMD_GET_INPUT_STATUS:  { c = SCART_CHK_STATE;     break; }
	case ALISLSCART_CMD_ENTRY_STANDBY:     { c = SCART_ENTRY_STANDBY; break; }
	default:
		SL_DBG("The SCART command is over cover!\n");
		break;
	}
	
	switch (arg) {
	case ALISLSCART_PARAM_TV_FORMAT_RGB: { p = TV_MODE_RGB;         break; }
	case ALISLSCART_PARAM_TV_FORMAT_CVBS:{ p = TV_MODE_CVBS;        break; }
	case ALISLSCART_PARAM_ASPECT_4_3:    { p = ASPECT_4_3;          break; }
	case ALISLSCART_PARAM_ASPECT_16_9:   { p = ASPECT_16_9;         break; }
	case ALISLSCART_PARAM_TV_IN:         { p = SOURCE_TV_IN;        break; }
	case ALISLSCART_PARAM_VCR_IN:        { p = SOURCE_VCR_IN;       break; }
	case ALISLSCART_PARAM_STB_IN:        { p = SOURCE_STB_IN;       break; }
	case ALISLSCART_PARAM_POWER_ON:      { p = 1;                   break; }
	case ALISLSCART_PARAM_POWER_OFF:     { p = 0;                   break; }
	default:
		SL_DBG("The SCART param is over cover!\n");
		break;
	}

	ret = alislscart_ioctl(dev, c, p);
	
	return ret;
}

/**
 *  Function Name:  alislscart_get_status
 *  @brief          Get scart status of input source
 *
 *  @param          handle point to struct SCART device 
 *  @param          cmd command of SCART funcation
 *  @param          arg in or out param for SCART function
 
 *  @return         alisl_recode
 *
 *  @author         Jonathan Chen <jonathan.chen@alitech.com>
 *  @date           07/02/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_get_status(alisl_handle handle,
                                    unsigned int cmd, unsigned long arg)
{
	alislscart_err_t ret = 0;
	unsigned int c;

	alislscart_dev_t *dev = (alislscart_dev_t *)handle;

	switch (cmd) {
	case ALISLSCART_CMD_GET_INPUT_STATUS:     
		{
			c = SCART_CHK_STATE;
			ret = alislscart_ioctl(dev, c, arg);
			break;
		}
	default:
		SL_ERR("The input command is error\n");
		break;
	}

	
	return ret;
}
