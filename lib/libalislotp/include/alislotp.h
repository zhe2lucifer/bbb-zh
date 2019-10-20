/**@file
 *  (c) Copyright 2013-2063  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislotp.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/06/2013 10:13:40 AM
 *  @revision           none
 *
 *  @author             Alan Zhang<alan.zhang@alitech.com>
 */

#ifndef __ALISLOTP_INTERFACE__H_
#define __ALISLOTP_INTERFACE__H_

#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum alislotp_status {
	ALISLOTP_INACTIVATE,
	ALISLOTP_ACTIVATE
} alislotp_status_t;

typedef enum alislotp_cmd {
	ALISLOTP_CMD_READ,
	ALISLOTP_CMD_WRITE,
} alislotp_cmd_t;

typedef enum alislotp_io_param {
	ALISLOTP_PARAM_ENABLE,
	ALISLOTP_PARAM_DISABLE
} alislotp_io_param_t;

typedef struct alislotp_param {
	unsigned char status;
	unsigned char rev;
} alislotp_param_t;

/**
 *  Function Name:      alislotp_op_read
 *  @brief              read otp data
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_op_read(unsigned long offset, unsigned char *buf, int len);

/**
 *  Function Name:      alislotp_op_write
 *  @brief              write otp data
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_op_write(unsigned long offset, unsigned char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
