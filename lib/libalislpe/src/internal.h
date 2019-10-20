/**@file
 *  (c) Copyright 2014-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              head file of internal
 *
 *  @version            1.0
 *  @date               08/12/2014 10:12:58 AM
 *  @revision           none
 *
 *  @author             Wendy He <wendy.he@alitech.com>
 */

#ifndef __ALISLPE_INTERNAL__H_
#define __ALISLPE_INTERNAL__H_

#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLPE_DEV_NAME "/dev/ali_pe0"

enum alislpe_errcode
{
    PE_ERR_NOERR               = ERROR_NONE,
    PE_ERR_INVAL               = ERROR_INVAL,
    PE_ERR_FILEOPENFAILED      = ERROR_OPEN,
    PE_ERR_NOMEM               = ERROR_NOMEM,
    PE_ERR_IOCTLFAILED         = ERROR_IOCTL
};

typedef struct pe_private 
{
	int                   fd;
	unsigned int          open_cnt;
} pe_private_t;

#ifdef __cplusplus
}
#endif

#endif
