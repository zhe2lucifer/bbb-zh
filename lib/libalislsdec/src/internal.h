/** @file     internal.h
 *  @brief    alislsdec internal.h
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef __ALISLSDEC_INTERNEL_H__
#define __ALISLSDEC_INTERNEL_H__

#include <alislsdec.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SDEC_STATUS_NONE         (0)
#define SDEC_STATUS_ALL          (0xFFFFFFFF)
#define SDEC_STATUS_CONSTRUCT    (1 << 0)
#define SDEC_STATUS_OPEN         (1 << 1)
#define SDEC_STATUS_START        (1 << 2)
#define SDEC_STATUS_STOP         (1 << 3)
#define SDEC_STATUS_CLOSE        (1 << 4)
#define SDEC_STATUS_CONFIGURE    (1 << 5)
#define SDEC_STATUS_MONITOR      (1 << 6)

struct sdec_private
{
	int                   fd;
	int                   id;    /**<id number */
	uint32_t              status;
	unsigned int open_cnt;
}sdec_private_t;

#ifdef __cplusplus
}
#endif

#endif
