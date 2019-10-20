/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislscart_wrapper.c
 *  @brief          api of SCART share library
 *
 *  @version        1.0
 *  @date           06/21/2013 05:09:54 PM
 *  @revision       none
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 */

/*System header */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alipltflog.h>

/* SCRAT header */
#include <alislscart.h>

/* Internal header */
#include <internal.h>

/**
 *  Function Name:  alislscart_set_tv_in_source
 *  @brief          set input source of TV
 *
 *  @param          source    input source of TV is VCR or STB
 *  @param          deviceid  SCART device id
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_tv_in_source(alislscart_param_input_source_t source,
                                          enum scart_deviceid deviceid)
{
	alislscart_err_t   ret = 0;
	void *dev = NULL;
	alislscart_param_t param;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status   = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	ret = alislscart_set_param(dev, ALISLSCART_CMD_SET_TV_SOURCE, source);
	if (0 != ret) {
		SL_DBG("Can't set VCR input source!\n");
	}

	alislscart_close(dev);
	alislscart_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislscart_set_vcr_in_source
 *  @brief          set vcr input source 
 *
 *  @param          source   input source of VCR is TV or STB
 *  @param          deviceid SCART device id
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_vcr_in_source(alislscart_param_input_source_t source,
                                           enum scart_deviceid deviceid)
{
	alislscart_err_t   ret = 0;
	void *dev = NULL;
	alislscart_param_t param;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	ret = alislscart_set_param(dev, ALISLSCART_CMD_SET_VCR_SOURCE, source);
	if (0 != ret) {
		SL_DBG("Can's set VCR input source!\n");
	}

	alislscart_close(dev);
	alislscart_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislscart_set_tv_video_out_aspect
 *  @brief          set tv video output aspect 
 *
 *  @param          aspect  parameter of aspect is 4:3 or 16:9
 *  @param          deviceid SCART device id
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_tv_video_out_aspect(alislscart_param_aspect_t aspect,
                                                 enum scart_deviceid deviceid)
{
	alislscart_err_t   ret = 0;
	void *dev = NULL;
	alislscart_param_t param;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	ret = alislscart_set_param(dev, ALISLSCART_CMD_SET_TV_ASPECT, aspect);

	alislscart_close(dev);
	alislscart_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislscart_set_vcr_video_out_aspect
 *  @brief          aspect switch
 *
 *  @param          aspect  parameter of aspect is 4:3 or 16:9
 *  @param          deviceid SCART device id
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_vcr_video_out_aspect(alislscart_param_aspect_t aspect,
                                                  enum scart_deviceid deviceid)
{
	alislscart_err_t   ret = 0;
	void *dev = NULL;
	alislscart_param_t param;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	ret = alislscart_set_param(dev, ALISLSCART_CMD_SET_VCR_ASPECT, aspect);
	if (0 != ret) {
		SL_DBG("Can not set VCR aspect!\n");
	}

	alislscart_close(dev);
	alislscart_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislscart_set_tv_video_out_format
 *  @brief          set TV video output format
 *
 *  @param          format TV video format is RGB and CVBS
 *  @param          deviceid SCART device id
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_tv_video_out_format(alislscart_param_tv_format_t format,
                                                 enum scart_deviceid deviceid)
{
	alislscart_err_t   ret = 0;
	void *dev = NULL;
	alislscart_param_t param;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	if (alislscart_is_tv_in_status(deviceid) == true) {
		ret = alislscart_set_param(dev, ALISLSCART_CMD_SET_TV_FORMAT, format);
	} else {
		ret = ALISLSCART_ERR_TV_STATUS;
	}

	alislscart_close(dev);
	alislscart_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislscart_is_tv_in_status
 *  @brief          check whether input source is TV
 * 
 *  @param          deviceid SCART device id
 *
 *  @return         true or false
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
bool alislscart_is_tv_in_status(enum scart_deviceid deviceid)
{
	void *dev = NULL;
	alislscart_param_t param;
	unsigned long status = 0;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	alislscart_get_status(dev, ALISLSCART_CMD_GET_INPUT_STATUS, (unsigned long)&status);

	alislscart_close(dev);
	alislscart_destruct(&dev);

	if (status & ALISLSCART_STATUS_TV_IN) {
		return true;
	} else {
		return false;
	}
}

/**
 *  Function Name:  alislscart_is_vcr_in_status
 *  @brief          check whether input source is VCR
 * 
 *  @param          deviceid SCART device id
 *
 *  @return         true or false
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/28/2013, Created
 *
 *  @note
 */
bool alislscart_is_vcr_in_status(enum scart_deviceid deviceid)
{
	void *dev = NULL;
	alislscart_param_t param;
	unsigned long status = 0;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	alislscart_get_status(dev, ALISLSCART_CMD_GET_INPUT_STATUS, (unsigned long)&status);

	alislscart_close(dev);
	alislscart_destruct(&dev);

	if (status & ALISLSCART_STATUS_VCR_IN) {
		return true;
	} else {
		return false;
	}
}

/**
 *  Function Name:  alislscart_power_onoff
 *  @brief          SCART device power on or off
 *
 *  @param          sw SCART switch between power on and off, 0: off 1: on
 *  @param          deviceid SCART device id
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_power_onoff(alislscart_param_power_onoff_t sw,
                                     enum scart_deviceid deviceid)
{
	alislscart_err_t ret = 0;
	void *dev = NULL;
	alislscart_param_t param;

	memset(&param, 0x00, sizeof(alislscart_param_t));

	param.deviceid = deviceid;
	param.status = ALISLSCART_DEV_ACTIVATE;
	alislscart_construct(&dev, &param);
	alislscart_open(dev);

	ret = alislscart_set_param(dev, ALISLSCART_CMD_ENTRY_STANDBY, sw);

	alislscart_close(dev);
	alislscart_destruct(&dev);

	return ret;
}
