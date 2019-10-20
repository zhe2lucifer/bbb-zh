/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file			internal.h
 *  @brief			internal definition of trng device module
 *
 *  @version		1.0
 *  @date			06/04/2013 02:57:43 PM
 *  @revision		none
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 */

#ifndef __ALITRNG_INTERNAL__H_
#define __ALITRNG_INTERNAL__H_

#include <ali_trng_common.h>

#include <alisltrng.h>
#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLTRNG_DEV_NAME			"/dev/hwrng" //"/dev/ali_trng_0"

/* Internal definition of invalid handle */
#define ALISLTRNG_INVALID_HANDLE	-1

/* Definition of io param */
typedef struct ALI_TRNG_GET_64BITS alisltrng_ioparam_t;

#ifdef __cplusplus
}
#endif

#endif

