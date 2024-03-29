/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file                       platform.c
 *  @brief                      implementation of ALi platform function
 *
 *  @version                    1.0
 *  @date                       06/03/2013 02:38:49 PM
 *  @revision                   none
 *
 *  @author                     Zhao Owen <Owen.Zhao@alitech.com>
 *
 *  Function description:
 *  1  Provide interface to application to register different module log
 */

#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../version_autogenerated.h"
#include "alipltfretcode.h"
#include "alipltflog.h"
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
uint32_t alipltf_get_uuid(void)
{
    static uint32_t unique_id = 0;

    return (getpid() << 16) | (unique_id++ & 0xffff);
}

alisl_retcode alipltf_package_info_get(char* info, int max_len)
{
	char* package_name = "aliplatform";
	if ((0 == strlen(BR2_ALIPLATFORM_AUTOGENERATE_VERSION)) ||
		 (0 == strlen(BR2_ALIPLATFORM_COMPILE_DATETIME))) {
		SL_ERR("pacakge information is NULL!\n");
		return ERROR_INVAL;
	}

	if (NULL == info){
		SL_ERR("pointer is NULL!\n");
		return ERROR_INVAL;
	}

	memset(info, 0x00, max_len);
	//get aliplatform package tag
	snprintf(info, max_len, "%s : %s\n", package_name, BR2_ALIPLATFORM_AUTOGENERATE_VERSION);
	if (max_len < strlen(BR2_ALIPLATFORM_AUTOGENERATE_VERSION)) {
		SL_ERR("the max_len less then the string length!\n");
		return ERROR_INVAL;
	}
	//get aliplatform compile datetime.
	snprintf(info + strlen(info), max_len - strlen(info), "%s : %s\n", package_name, BR2_ALIPLATFORM_COMPILE_DATETIME);
	if ((max_len - strlen(info)) < strlen(BR2_ALIPLATFORM_COMPILE_DATETIME)) {
		SL_ERR("the max_len less then the string length!\n");
		return ERROR_INVAL;
	}

	return ERROR_NONE;
}
