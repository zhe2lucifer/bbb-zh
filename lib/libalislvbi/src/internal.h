/** @file     internal.h
 *  @brief    alislvbi internal.h
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef __ALISLVBI_INTERNEL_H__
#define __ALISLVBI_INTERNEL_H__

#include <alislvbi.h>
#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define VBI_STATUS_NONE         (0)
#define VBI_STATUS_ALL          (0xFFFFFFFF)
#define VBI_STATUS_CONSTRUCT    (1 << 0)
#define VBI_STATUS_OPEN         (1 << 1)
#define VBI_STATUS_START        (1 << 2)
#define VBI_STATUS_STOP         (1 << 3)
#define VBI_STATUS_CLOSE        (1 << 4)
#define VBI_STATUS_CONFIGURE    (1 << 5)
#define VBI_STATUS_MONITOR      (1 << 6)

#define STACKSIZE               0x1000

typedef struct vbi_private {
	int                   fd;
	int                   id;        /**< id number */
	uint32_t              status;
	unsigned int open_cnt;
} vbi_private_t;

	
#ifdef __cplusplus
}
#endif

#endif
