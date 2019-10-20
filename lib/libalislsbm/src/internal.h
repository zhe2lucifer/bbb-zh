/**@file
 *  (c) Copyright 2014-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              head file of internal
 *
 *  @version            1.0
 *  @date               08/13/2014 16:12:58 PM
 *  @revision           none
 *
 *  @author             Wendy He <wendy.he@alitech.com>
 */

#ifndef __ALISLSBM_INTERNAL__H_
#define __ALISLSBM_INTERNAL__H_

#include <alipltflog.h>
#include <alislsbm.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef struct dev_sbm {
	enum sbm_id     id;
	const char      *path;
} dev_sbm_t;

struct dev_sbm dev_sbm[SBM_NUM_SBM] = {
	{SBM_ID_SBM0, "/dev/ali_sbm0"},
	{SBM_ID_SBM1, "/dev/ali_sbm1"},
	{SBM_ID_SBM2, "/dev/ali_sbm2"},
	{SBM_ID_SBM3, "/dev/ali_sbm3"},
	{SBM_ID_SBM4, "/dev/ali_sbm4"},
	{SBM_ID_SBM5, "/dev/ali_sbm5"},
	{SBM_ID_SBM6, "/dev/ali_sbm6"},
	{SBM_ID_SBM7, "/dev/ali_sbm7"},
	{SBM_ID_SBM8, "/dev/ali_sbm8"},
	{SBM_ID_SBM9, "/dev/ali_sbm9"},
	{SBM_ID_SBM10, "/dev/ali_sbm10"},
	{SBM_ID_SBM11, "/dev/ali_sbm11"}
};

enum alislsbm_errcode
{
    SBM_ERR_NOERR               = ERROR_NONE,
    SBM_ERR_INVAL               = ERROR_INVAL,
    SBM_ERR_FILEOPENFAILED      = ERROR_OPEN,
    SBM_ERR_NOMEM               = ERROR_NOMEM,
    SBM_ERR_IOCTLFAILED         = ERROR_IOCTL,
    SBM_ERR_FAILED              = ERROR_FAILED
};

typedef struct sbm_private 
{
	int                   fd;
	enum sbm_id           id;
	unsigned int          open_cnt;
} sbm_private_t;

#ifdef __cplusplus
}
#endif

#endif
