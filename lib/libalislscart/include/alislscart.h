/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file        alislscart.h
 *  @brief       head file of SCART share libary
 *
 *  @version     1.0
 *  @date        06/21/2013 04:29:35 PM
 *  @revision    none
 *
 *  @author      Chen Jonathan <Jonathan.Chen@alitech.com>
 */

#ifndef __ALISLSCART_INTERFACE__H_
#define __ALISLSCART_INTERFACE__H_

#include <stdbool.h>
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Struct define */

/* Id of SCART device */
typedef enum scart_deviceid {
	ALISLSCART_ID_SCART0 = 0,
	ALISLSCART_ID_SCART1
} scart_deviceid_t;

/* Status of SCART dev */
typedef enum alislscart_status {
	ALISLSCART_DEV_INACTIVATE = 0,
	ALISLSCART_DEV_ACTIVATE
} alislfake_status_t;

/* Define command types */
typedef enum alislscart_cmd {
	ALISLSCART_CMD_SET_TV_SOURCE,       /**< Switch input source on TV mode\n  */
	ALISLSCART_CMD_SET_VCR_SOURCE,      /**< Switch input source on VCR mode\n  */
	ALISLSCART_CMD_SET_TV_ASPECT,       /**< Switch aspect on TV mode\n */
	ALISLSCART_CMD_SET_VCR_ASPECT,
	ALISLSCART_CMD_SET_TV_FORMAT,       /**< Switch TV mode between RGB and CVBS\n */
	ALISLSCART_CMD_GET_INPUT_STATUS,    /**< Check SCART input source status\n  */
	ALISLSCART_CMD_ENTRY_STANDBY        /**< Power on or off SCART */
} alislscart_cmd_t;

/* paramteters of input source */
typedef enum alislscart_param_input_source {
	ALISLSCART_PARAM_VCR_IN = 0,
	ALISLSCART_PARAM_STB_IN,
	ALISLSCART_PARAM_TV_IN
}alislscart_param_input_source_t;

/* paramteters of aspect */
typedef enum alislscart_param_aspect {
	ALISLSCART_PARAM_ASPECT_4_3  = 3,
	ALISLSCART_PARAM_ASPECT_16_9
} alislscart_param_aspect_t;

/* paramteters of tv format */
typedef enum alislscart_param_tv_format {
	ALISLSCART_PARAM_TV_FORMAT_RGB = 5,
	ALISLSCART_PARAM_TV_FORMAT_CVBS
} alislscart_param_tv_format_t;

typedef enum alislscart_param_power_onoff {
	ALISLSCART_PARAM_POWER_ON = 7,
	ALISLSCART_PARAM_POWER_OFF
} alislscart_param_power_onoff_t;

/* SCART input source status */
typedef enum alislscart_in_status {
	ALISLSCART_STATUS_VCR_IN,           /**< The Scart input source is VCR */
	ALISLSCART_STATUS_TV_IN             /**< The Scart input source is TV\n */
} alislscart_in_status_t;

/* Input param for SCART dev */
typedef struct alislscart_param {
	unsigned char deviceid;             /**< SCART deviceid,support 2 devices now */
	unsigned char status;               /**< SCART device status: activate or inactivate */
} alislscart_param_t;

/**
 *  Function Name:  alislscart_construct
 *  @brief          initalize device by input parameter
 *
 *  @param          handle  point to struct SCART device 
 *  @param          param input parameter use to initalize device
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_construct(void **handle,
                                   alislscart_param_t *param);

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
alisl_retcode alislscart_destruct(void **handle);

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
alisl_retcode alislscart_open(void *handle);

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
alisl_retcode alislscart_close(void *handle);
                         
/**
 *  Function Name:  alislscart_set_param
 *  @brief          set parameters of SCART device
 *
 *  @param          handle point to struct SCART device 
 *  @param          cmd commond of SCART funcation
 *  @param          arg in or out param for SCART function
 *
 *  @return         alisl_retcode
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_set_param(void *handle,
                                   unsigned int cmd, unsigned long arg);

/**
 *  Function Name:  alislscart_get_status
 *  @brief          Get scart status of input source
 *
 *  @param          handle point device of SCART 
 *  @param          cmd commond of SCART funcation
 *  @param          arg in or out param for SCART function
 
 *  @return         alisl_recode
 *
 *  @author         Jonathan Chen <jonathan.chen@alitech.com>
 *  @date           07/02/2013, Created
 *
 *  @note
 */
alisl_retcode alislscart_get_status(void *handle, 
                                    unsigned int cmd, unsigned long arg);

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
                                          enum scart_deviceid deviceid);

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
                                           enum scart_deviceid deviceid);

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
                                                  enum scart_deviceid deviceid);

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
                                                 enum scart_deviceid deviceid);

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
                                                 enum scart_deviceid deviceid);

/**
 *  Function Name:  alislscart_is_tv_in_status
 *  @brief          check whether input source is tv
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
bool alislscart_is_tv_in_status(enum scart_deviceid deviceid);

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
bool alislscart_is_vcr_in_status(enum scart_deviceid deviceid);

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
                                     enum scart_deviceid deviceid);

#ifdef __cplusplus
}
#endif

#endif
