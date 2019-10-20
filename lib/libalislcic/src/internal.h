/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              internal data struct definition
 *
 *  @version            1.0
 *  @date               07/04/2013 04:52:15 PM
 *  @revision           none
 *
 *  @author             Stephen Xiao <stephen.xiao@alitech.com>
 */

#ifndef __ALISLCIC_INTERNEL_H__
#define __ALISLCIC_INTERNEL_H__

#include <inttypes.h>
#include <alipltfretcode.h>
/* kernel headers */
#include <dvb_ca_common.h>

#include <alipltflog.h>
#include <ali_cic.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLCIC_DEV_NAME       "/dev/ali_cic"

#define ALISLCIC_STATUS_NONE         (0)
#define ALISLCIC_STATUS_ALL          (0xFFFFFFFF)
#define ALISLCIC_STATUS_CONSTRUCT    (1 << 0)
#define ALISLCIC_STATUS_OPEN         (1 << 1)
#define ALISLCIC_STATUS_START        (1 << 2)
#define ALISLCIC_STATUS_STOP         (1 << 3)
#define ALISLCIC_STATUS_CLOSE        (1 << 4)
#define ALISLCIC_STATUS_CONFIGURE    (1 << 5)
#define ALISLCIC_STATUS_MONITOR      (1 << 6)

/* Internal definition of invalid handle */
#define ALISLCIC_INVALID_HANDLE	-1

/* Error handler of library */
enum {
	ALISLCIC_ERR_CANTOPENDEV		= ERROR_OPEN,
	ALISLCIC_ERR_INVALIDPARAM		= ERROR_INVAL,
	ALISLCIC_ERR_INVALIDHANDLE		= ERROR_INVAL,
	ALISLCIC_ERR_IOACCESS			= ERROR_IOCTL,
	ALISLCIC_ERR_NOTSUPPORTLEN		= ERROR_INVAL,
	ALISLCIC_ERR_MALLOCFAILED		= ERROR_NOMEM,
};

typedef struct cic_device {

	int       fd;
	uint32_t  status;

	struct cic_msg cic_msg;               /**< A message to/from a CI-CAM */
	struct cic_slot_info cic_slot_info;   /**< slot interface types and info */
} cic_device_t;


#ifdef __cplusplus
}
#endif

#endif /* __ALISLCIC_INTERNEL_H__ */ 
