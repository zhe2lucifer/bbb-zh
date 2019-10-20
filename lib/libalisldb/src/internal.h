/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              internal data struct definition
 *
 *  @version            1.0
 *  @date               07/16/2013 02:50:47 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef __ALISLDB_INTERNEL_H__
#define __ALISLDB_INTERNEL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define  likely(x)        __builtin_expect(!!(x), 1)
#define  unlikely(x)      __builtin_expect(!!(x), 0)

typedef struct callback_param {
	void            *arg;
	alisldb_callback callback;
	void             *cb_arg;
} callback_param_t;

#ifdef __cplusplus
}
#endif

#endif /* __ALISLDB_INTERNEL_H__ */
