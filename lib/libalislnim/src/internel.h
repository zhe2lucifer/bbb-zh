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



#ifndef __ALISLNIM_INTERNEL_H__
#define __ALISLNIM_INTERNEL_H__

#include <pthread.h>
#include <alislnim.h>

/* kernel headers */
//#include <ali_netlink_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define NIM_STATUS_NONE         (0)
#define NIM_STATUS_ALL          (0xFFFFFFFF)
#define NIM_STATUS_CONSTRUCT    (1 << 0)
#define NIM_STATUS_OPEN         (1 << 1)
#define NIM_STATUS_START        (1 << 2)
#define NIM_STATUS_PAUSE        (1 << 3)
#define NIM_STATUS_STOP         (1 << 4)
#define NIM_STATUS_CLOSE        (1 << 5)
#define NIM_STATUS_AS           (1 << 6) /* auto-scan */

#define MAX_NETLINK_MSG_SIZE    1024

typedef struct netlink_resource {
	int                   sockfd;

	/**
	 * socket pid is used to communicate between APP and driver
	 */
	int                   sockpid;

	struct sockaddr_nl    saddr;    /**< source sockaddr */
	struct sockaddr_nl    daddr;    /**< destination sockaddr */

	struct nlmsghdr       *msghdr;  /**< buffer for message header and payload */
	struct msghdr         msg;
	struct iovec          iov;
} netlink_resource_t ;

struct nim_device {
	enum nim_id             id;             /**< nim id number */
	struct flag             status;
	int                     fd;             /**< fd of nim device,
	                                             for common control usage */
	int                     feedfd;         /**< fd of nim device,
	                                             for feed data usage */
	const char              *path;
	const char              *pathfeed;      /**< when playback, we need to feed data
	                                             to device specified by pathfeed */
//	struct netlink_resource  nl;            /**< used for netlink implementation */
//	int                      portid;        /**< portid used by netlink to transport\n
//	                                    messages from kernel space to user space */
//	pthread_t                tid;          /**< monitor thread id */

	int                      type;

	struct slnim_signal_info info;

	int			kumsgfd; /**< used for kernel-userspace messaging */

} nim_device_t;


/*
 * !!!!! To be moved to kernel dvb_frontend_common.h !!!!!!!
 */

/* Field modulation */
#define TPS_CONST_AUTO     0
#define TPS_CONST_DQPSK    0x02
#define TPS_CONST_QPSK     0x04
#define TPS_CONST_16QAM    0x10
#define TPS_CONST_64QAM    0x40

/* Field fec */
#define FEC_AUTO  10
#define FEC_1_2   0
#define FEC_2_3   1
#define FEC_3_4   2
#define FEC_5_6   3
#define FEC_7_8   4

/* Field guard_interval */
#define GUARD_AUTO 0
#define GUARD_1_32 0x20
#define GUARD_1_16 0x10
#define GUARD_1_8  0x08
#define GUARD_1_4  0x04

/* Field fft_mode */
#define MODE_AUTO  0
#define MODE_2K    0x02
#define MODE_8K    0x08
#define MODE_4K    0x04

/* Field bandwidth */
#define BW_6M    6
#define BW_7M    7
#define BW_8M    8

/* Field inverse : spectrum invertion */
#define SPEC_NORM     0
#define SPEC_INVERTED 1
#define SPEC_AUTO     2


#ifdef __cplusplus
}
#endif

#endif /* __ALISLNIM_INTERNEL_H__ */
