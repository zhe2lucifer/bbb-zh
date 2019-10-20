/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alipltfintf.h
 *  @brief          interface of the platform for application
 *
 *  @version        1.0
 *  @date           06/04/2013 04:58:59 PM
 *  @revision       none
 *
 *  @author         Zhao Owen <Owen.Zhao@alitech.com>
 */

#ifndef __ALIPLTF_INTERFACE__H_
#define __ALIPLTF_INTERFACE__H_

#include <alipltfretcode.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Function Name: alipltf_get_uuid
 * @brief         Generate a unique ID for aliplatform module.
 *
 * @return        the unique id
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          10/30/2013, created
 *
 * @note
 */
uint32_t alipltf_get_uuid(void);

alisl_retcode alipltf_package_info_get(char* info, int max_len);

#ifdef __cplusplus
}
#endif

#endif
