/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              head file of internal
 *
 *  @version            1.0
 *  @date               07/06/2013 10:12:58 AM
 *  @revision           none
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 */

#ifndef __ALISLWATCHDOG_INTERNAL__H_
#define __ALISLWATCHDOG_INTERNAL__H_

#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLWATCHDOG_DEV_NAME "/dev/watchdog"

#define ALISLWATCHDOG_INVALID_HANDLE -1

typedef enum alislwatchdog_err {
	ALISLWATCHDOG_ERR_INPUT_PARAM     = ERROR_INVAL,
	ALISLWATCHDOG_ERR_MALLOC          = ERROR_NOMEM,
	ALISLWATCHDOG_ERR_CANTOPEN        = ERROR_OPEN,
	ALISLWATCHDOG_ERR_IOACCESS        = ERROR_IOCTL,
	ALISLWATCHDOG_ERR_INVALID_HANDLE  = ERROR_INVAL,
} alislwatchdog_err_t;

typedef struct alislwatchdog_dev {
	unsigned char *dev_name;
	int            handle;
	unsigned int   open_cnt;
} alislwatchdog_dev_t;

#ifdef __cplusplus
}
#endif

#endif
