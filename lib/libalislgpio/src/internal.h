/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              internal data struct definition
 *
 *  @version            1.0
 *  @date               07/14/2016 02:36:31 PM
 *  @revision           none
 *
 *  @author             Steven Zhang <steven.zhang@alitech.com>
 */

#ifndef __ALISLGPIO_INTERNEL_H__
#define __ALISLGPIO_INTERNEL_H__

#include <pthread.h>
#include <alislgpio.h>

/* kernel headers */
#include <ali_gpio_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GPIO_STATUS_NONE         (0)
#define GPIO_STATUS_ALL          (0xFFFFFFFF)
#define GPIO_STATUS_CONSTRUCT    (1 << 0)
#define GPIO_STATUS_OPEN         (1 << 1)
#define GPIO_STATUS_START        (1 << 2)
#define GPIO_STATUS_STOP         (1 << 3)
#define GPIO_STATUS_CLOSE        (1 << 4)
#define GPIO_STATUS_CONFIGURE    (1 << 5)
#define GPIO_STATUS_MONITOR      (1 << 6)

#define STACKSIZE               0x1000
#define MAX_NETLINK_MSG_SIZE    1024

typedef struct gpio_sl_dev {
	int                     fd;
	int                     portid;  /**< portid used by netlink to transport
	                                      messages from kernel space to user space */
	uint32_t                status;
	gpio_cbfuc             handle_msg;

    int kumsg_fd;/**< used for kernel-userspace messaging */

	uint32_t                open_cnt;
	
    alisl_handle gpio_event_handle;
} gpio_sl_dev;

#define MAX_KUMSG_SIZE          1024

#ifdef __cplusplus
}
#endif

#endif /* __ALISLGPIO_INTERNEL_H__ */
