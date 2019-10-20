/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file           internal.h
 *  @brief          head file of internal
 *
 *  @version        1.0
 *  @date           06/21/2013 05:06:34 PM
 *  @revision       none
 *
 *  @author	        Chen Jonathan <Jonathan.Chen@alitech.com>
 */

#ifndef __ALISLSCART_INTERNAL__H_
#define __ALISLSCART_INTERNAL__H_

#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLSCART_DEV0_NAME  "/dev/ali_scart"
#define ALISLSCART_DEV1_NAME  "/dev/ali_scart1"

#define ALISLSCART_INVALID_HANDLE -1

typedef enum alislscart_err {
	ALISLSCART_ERR_INPUT_PARAM    = ERROR_INVAL,
	ALISLSCART_ERR_MALLOC_MEM     = ERROR_NOMEM,
	ALISLSCART_ERR_DEV_INACTIVATE = ERROR_NOOPEN,
	ALISLSCART_ERR_DEV_CANTOPEN   = ERROR_OPEN,
	ALISLSCART_ERR_DEV_IOACCESS   = ERROR_IOCTL,
	ALISLSCART_ERR_INVALID_HANDLE = ERROR_INVAL,
	ALISLSCART_ERR_TV_STATUS      = ERROR_STATUS
} alislscart_err_t;

/* Description of dev */
typedef struct alislscart_dev {
	enum scart_deviceid deviceid;
	const char          *dev_name;
	unsigned char       status;
	int                 handle;
} alislscart_dev_t;

typedef struct alislscart_dev_attr {
	enum scart_deviceid deviceid;
	const char          *dev_name;
} alislscart_dev_attr_t;

#ifdef __cplusplus
}
#endif

#endif

