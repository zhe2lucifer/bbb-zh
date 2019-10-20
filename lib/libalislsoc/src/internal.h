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
 *  @author             Alan Zhang<Alan.Zhang@alitech.com>
 */

#ifndef __ALISLSOC_INTERNAL__H_
#define __ALISLSOC_INTERNAL__H_

#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLSOC_DEV_NAME "/dev/ali_soc"

#define ALISLSOC_INVALID_HANDLE -1

typedef enum alislsoc_err {
	ALISLSOC_ERR_INPUT_PARAM     = ERROR_INVAL,
	ALISLSOC_ERR_MALLOC          = ERROR_NOMEM,
	ALISLSOC_ERR_CANTOPEN        = ERROR_OPEN,
	ALISLSOC_ERR_IOACCESS        = ERROR_IOCTL,
	ALISLSOC_ERR_INVALID_HANDLE  = ERROR_INVAL,
	/* Max of SCART error */
	ALISLSOC_ERR_MAX,
} alislsoc_err_t;

typedef struct alislsoc_dev {
	unsigned char *dev_name;
	int            handle;
	int            open_cnt;
	unsigned char status;
} alislsoc_dev_t;

#ifdef __cplusplus
}
#endif

#endif
