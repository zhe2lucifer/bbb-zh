/** @file     alislpe.h
 *  @brief    include struct and function defination used in play engine share library
 *  @author   wendy.he
 *  @date     2014-8-12
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2014-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef _PE_ALISL_
#define _PE_ALISL_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct pe_mem_info
{
	void *mem_start;
	unsigned long mem_size;
}pe_mem_info_t;

/**  
 * @brief         Open ali_pe0 for common usage. 
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle           pe device pointer
 * @param[out]   
 * @return        alisl_retcode    0:success, other: fail
 * @note
 *
 */
alisl_retcode alislpe_open(alisl_handle *handle);

/**
 * @brief         close pe device
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle           pe device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislpe_close(alisl_handle handle);

/**
 * @brief         get pe memory info
 * @author        wendy.he
 * @date          2014-8-12
 * @param[in]     handle           pe device pointer
 * @param[out]    mem_info         pe memory info
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislpe_get_pe_mem_info(alisl_handle handle, 
    struct pe_mem_info* mem_info);

#ifdef __cplusplus
}
#endif

#endif /*_PE_ALISL_*/
