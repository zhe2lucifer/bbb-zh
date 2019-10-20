/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              internal data struct definition
 *
 *  @version            1.0
 *  @date               06/04/2013 02:36:31 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef __ALISLSMC_INTERNEL_H__
#define __ALISLSMC_INTERNEL_H__

#include <pthread.h>
#include <alislsmc.h>
#include <alislevent.h>

/* kernel headers */
#include <ali_smc_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SMC_STATUS_NONE         (0)
#define SMC_STATUS_ALL          (0xFFFFFFFF)
#define SMC_STATUS_CONSTRUCT    (1 << 0)
#define SMC_STATUS_OPEN         (1 << 1)
#define SMC_STATUS_START        (1 << 2)
#define SMC_STATUS_STOP         (1 << 3)
#define SMC_STATUS_CLOSE        (1 << 4)
#define SMC_STATUS_CONFIGURE    (1 << 5)
#define SMC_STATUS_MONITOR      (1 << 6)

#define STACKSIZE               0x1000
#define MAX_KUMSG_SIZE          1024

typedef struct smc_device {
	int                     fd;
	int                     id;      /**< smart card id number */
	int                     portid;  /**< portid used by netlink to transport
	                                      messages from kernel space to user space */
	pthread_t               tid;     /**< monitor thread id */
	uint32_t                status;

	smc_atr_t               atr;     /**< store answare-to-reset info */

	void                    (*callback)(void *user_data, uint32_t param);
	void					*callback_user_data;

	struct smc_device_cfg   cfg;

	int			kumsgfd; /**< used for kernel-userspace messaging */
	alisl_handle ev_handle;
	struct alislevent slev;

	uint32_t                open_cnt;
} smc_device_t;

#ifdef __cplusplus
}
#endif

#endif /* __ALISLSMC_INTERNEL_H__ */
