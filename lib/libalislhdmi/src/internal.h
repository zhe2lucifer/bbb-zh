/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           internal.h
 *  @brief
 *
 *  @version        1.0
 *  @date           20/11/2013
 *  @revision       none
 *
 *  @author         bill.kuo <bill.kuo@alitech.com>
 */

#ifndef _INTERNAL_H_
#define _INTERNAL_H_

/* system headerfiles */
//#include <sys/socket.h>         /* for socket program */
//#include <linux/netlink.h>      /* for netlink mechanism using */
#include <inttypes.h>           /* for basic data type, such as uint32_t */
#include <stdbool.h>
#include <pthread.h>

/* ALI common headerfiles */
//#include <ali_netlink_common.h>
#include <ali_media_common.h>
#include <ali_video_common.h>
#include <ali_hdmi_common.h>
#include <hdmi_io_common.h>

/* shared library headerfiles */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislevent.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ALISL_HDMI_EDID_UNREADY             0
#define ALISL_HDMI_EDID_READY               1

#define MAX_NETLINK_MSG_SIZE      1024
#define HDCP_KEY_BUF_LEN          288
#define ALI_HDMI_DEVICE           "/dev/ali_hdmi_device"
#define HDCP_KSV_OFFSET           1
#define HDCP_KSV_SIZE             5
#define HDCP_ENCRYP_DATA_OFFSET   6
#define HDCP_ENCRYP_DATA_LEN      280
#define HDMI_DEV_NAME_STRING_LEN  16

typedef struct msg_desc {
    uint8_t type;
    uint16_t len;
    uint8_t *data;
} st_msg_desc;
#if 0
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
#endif
typedef struct hdmi_device {
    struct  hdmi_device   *next;          /**< Unused */
    uint32_t    type;                     /**< Type of HDMI device */
    uint32_t ref_cnt;                     /**< Reference for multiuse */
    int8_t name[HDMI_DEV_NAME_STRING_LEN];/**< Name of HDMI device */
    uint32_t flags;                       /**< Status of HDMI device */
    int handle;                           /**< Handle of HDMI device */
    uint8_t *hdcp_key;                    /**< Cotainer for decrypted HDCP key */
    pthread_mutex_t *lock;                /**< Mutex lock for pthread safety */
    hdmi_edid_callback edid_ready;        /**< Callback function used when EDID is ready */
    hdmi_hotplug_callback hotplug_out;    /**< Callback function used when HDMI device plugs in */
    hdmi_hotplug_callback hotplug_in;     /**< Callback function used when HDMI device plugs out */
    hdmi_cec_callback cec_receive;        /**< Callback function used when receive CEC message */
    hdmi_hdcp_fail_callback hdcp_fail;    /**< Callback function used when hdcp fail */
	//struct netlink_resource nl;           /**< used for netlink implementation */
	//pthread_t               tid;          /**< monitor thread id */
	void * alislevent_hld;
	struct alislevent  slev; /** use epoll instead of thread */
	int			kumsgfd; /**< used for kernel-userspace messaging */
} st_hdmi_device;

#ifdef __cplusplus
}
#endif

#endif
