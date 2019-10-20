/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               internel.h
 *  @brief
 *
 *  @version            1.0
 *  @date               06/15/2013 09:20:42 AM
 *  @revision           none
 *
 *  @author             Glen Dai <glen.dai@alitech.com>
 */

#ifndef __ALISLDIS_INTERNAL_H__
#define __ALISLDIS_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <ali_video_common.h>

#include "alisldis.h"

struct alisldis_dev
{
    int fd_vpo;
    int fd_gma1;
	int fd_gma2;
    enum dis_dev_id dev_id;
    pthread_mutex_t  *mutex;
    unsigned int video_enhance_max;
};

#define GE_SET_SCALE_MODE       0x04 // GE_SCALE_FILTER or GE_SCALE_DUPLICATE
#define GE_SET_SCALE_PARAM      0x05 // Set arbitrary scale param. see struct gma_scale_param_t

#ifdef __cplusplus
}
#endif

#endif    /* __ALISLDIS_INTERNAL_H__ */
