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

#ifndef __ALISLOTP_INTERNAL__H_
#define __ALISLOTP_INTERNAL__H_

#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLOTP_DEV_NAME "/dev/otp0"

#define ALISLOTP_INVALID_HANDLE -1

typedef enum alislotp_err {
	ALISLOTP_ERR_INPUT_PARAM     = ERROR_INVAL,
	ALISLOTP_ERR_MALLOC          = ERROR_NOMEM,
	ALISLOTP_ERR_INACTIVATE      = ERROR_NOOPEN,
	ALISLOTP_ERR_CANTOPEN        = ERROR_OPEN,
	ALISLOTP_ERR_IOACCESS        = ERROR_IOCTL,
	ALISLOTP_ERR_DEV_INACTIVAGE  = ERROR_STATUS,
	ALISLOTP_ERR_INVALID_HANDLE  = ERROR_INVAL,
} alislotp_err_t;

typedef struct alislotp_dev {
	unsigned char *dev_name;
	int            handle;
	unsigned char  status;
} alislotp_dev_t;

#ifdef __cplusplus
}
#endif

#endif
