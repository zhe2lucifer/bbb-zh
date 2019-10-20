/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file			internal.h
 *  @brief		    Internal headfile of FAKE share lib
 *
 *  @version		1.0
 *  @date			05/31/2013 01:55:49 PM
 *  @revision		none
 *
 *  @author			Chen Jonathan <Jonathan.Chen@alitech.com>
 */

#ifndef __ALISLFAKE_INTERNAL__H_
#define __ALISLFAKE_INTERNAL__H_

#include "alipltfretcode.h"
#include "alipltflog.h"
#include "alisl_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLFAKE_DEV_NAME        "/dev/ali_fake_trace"

#define ALISLFAKE_INVALID_HANDLE  -1

/* Error handler of library */
enum {
	ALISLFAKE_ERR_NONE              = ERROR_NONE,
	ALISLFAKE_ERR_CANTOPEN_DEV      = ERROR_OPEN,
	ALISLFAKE_ERR_DEV_INACTIVATE    = ERROR_NOSTART,
	ALISLFAKE_ERR_IOACCESS          = ERROR_IOCTL,
	ALISLFAKE_ERR_INPUT_PARAM       = ERROR_INVAL,
	ALISLFAKE_ERR_NOTSUPPORT_CMD    = ERROR_INVAL,
	ALISLFAKE_ERR_INVALID_HANDLE    = ERROR_INVAL,
	ALISLFAKE_WARN_DISABLE          = ERROR_NOOPEN
};

#ifdef __cplusplus
}
#endif

#endif

