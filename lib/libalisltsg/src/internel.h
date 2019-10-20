/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               internel.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 16:29:47
 *  @revision           none
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 */



#ifndef __ALISLTSG_INTERNEL_H__
#define __ALISLTSG_INTERNEL_H__

#include <pthread.h>
#include <alisltsg.h>
#include <bits_op.h>
#include <flag_op.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TSG_STATUS_NONE         (0)
#define TSG_STATUS_ALL          (0xFFFFFFFF)
#define TSG_STATUS_CONSTRUCT    (1 << 0)
#define TSG_STATUS_OPEN         (1 << 1)
#define TSG_STATUS_START        (1 << 2)
#define TSG_STATUS_PAUSE        (1 << 3)
#define TSG_STATUS_STOP         (1 << 4)
#define TSG_STATUS_CLOSE        (1 << 5)

struct tsg_device {
	enum tsg_id             id;             /**< tsg id number */
	struct flag             status;
	int                     fd;             /**< fd of tsg device,\n
	                                             for common control usage */
	int                     feedfd;         /**< fd of tsg device,\n
	                                             for feed data usage */
	const char              *path;
	const char              *pathfeed;      /**< when playback, we need to feed data\n
	                                             to device specified by pathfeed */
	unsigned int			open_cnt;		/*tsg count of how many user use TSG*/
} tsg_device_t;

#ifdef __cplusplus
}
#endif

#endif /* __ALISLTSG_INTERNEL_H__ */
