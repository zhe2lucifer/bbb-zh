/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               error.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/16/2013 01:54:39 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef __ALISLDB_ERROR_H__
#define __ALISLDB_ERROR_H__

#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  Function Name:      alisldb_register_errorcode
 *  @brief              register error code of module smart-card
 *
 *  @param              void
 *
 *  @return             0 this will always success
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/16/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_register_errorcode(void);

/**
 *  Function Name:      alisldb_unregister_errorcode
 *  @brief              unregister error code of module smart-card
 *
 *  @param              void
 *
 *  @return             0 this will always success
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/16/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_unregister_errorcode(void);

#ifdef __cplusplus
}
#endif

#endif /* __ALISLDB_ERROR_H__ */
