/** @file     internal.h
 *  @brief    alislcc internal.h
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef __ALISLCC_INTERNEL_H__
#define __ALISLCC_INTERNEL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define CC_STATUS_NONE         (0)
#define CC_STATUS_ALL          (0xFFFFFFFF)
#define CC_STATUS_CONSTRUCT    (1 << 0)
#define CC_STATUS_OPEN         (1 << 1)
#define CC_STATUS_START        (1 << 2)
#define CC_STATUS_STOP         (1 << 3)
#define CC_STATUS_CLOSE        (1 << 4)
#define CC_STATUS_CONFIGURE    (1 << 5)
#define CC_STATUS_MONITOR      (1 << 6)

struct cc_private
{
	int                   fd;
	int                   id;    /**<id number */	
	uint32_t              status;	
	unsigned int open_cnt;
}cc_private_t;

#ifdef __cplusplus
}
#endif

#endif
