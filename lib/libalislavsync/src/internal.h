/**@file
 *  (c) Copyright 2014-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              head file of internal
 *
 *  @version            1.0
 *  @date               08/08/2014 10:12:58 AM
 *  @revision           none
 *
 *  @author             Wendy He <wendy.he@alitech.com>
 */

#ifndef __ALISLAVSYNC_INTERNAL__H_
#define __ALISLAVSYNC_INTERNAL__H_

#include <alislavsync.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLAVSYNC_DEV_NAME "/dev/ali_avsync0"

#define ALISLAVSYNC_INVALID_HANDLE -1

enum alislavsync_errcode
{
    AVSYNC_ERR_NOERR               = ERROR_NONE,
    AVSYNC_ERR_INVAL               = ERROR_INVAL,
    AVSYNC_ERR_FILEOPENFAILED      = ERROR_OPEN,
    AVSYNC_ERR_NOMEM               = ERROR_NOMEM,
    AVSYNC_ERR_IOCTLFAILED         = ERROR_IOCTL
};

typedef struct avsync_private 
{
	int                   fd;
	unsigned int          open_cnt;
	unsigned int          start_cnt;
	enum avsync_video_id  video_id;
} avsync_private_t;

#ifdef __cplusplus
}
#endif

#endif
