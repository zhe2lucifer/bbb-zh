/** @file     alislcc.h
 *  @brief    define cc share library struct and functions
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef _SDEC_ALISL_
#define _SDEC_ALISL_
	
	/* system headers */
#include <inttypes.h>
#include <stdbool.h>
	
	/* share library headers */
#include <alipltfretcode.h>

#define SDEC_CALL                          0x00010000
#define CALL_SDEC_START                   (SDEC_CALL+1)
#define CALL_SDEC_STOP                    (SDEC_CALL+2)
#define CALL_SDEC_PAUSE                   (SDEC_CALL+3)

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief         open cc device ali_cc
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          get cc handle priv->fd
 *
 */
alisl_retcode alislcc_open(void **handle);


/**
 * @brief         close cc handle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislcc_close(void *handle);


/**
 * @brief         start cc, ioctl transmit start parameters to driver
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer, composition_page_id and
 *                ancillary_page_id are page id from uplayer functions.
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode  alislcc_start(void *handle, uint16_t composition_page_id,
				uint16_t ancillary_page_id);


/**
 * @brief         stop cc device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode  alislcc_stop(void *handle);

/**
 * @brief         pause cc device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode  alislcc_pause(void *handle);

/**
 * @brief         show subtitle on osd
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislcc_show(void *handle);


/**
 * @brief         hide the subtitle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is cc device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislcc_hide(void *handle);


/**
 * @brief         cc ioctl, used for future
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cc device handle, ioctl cmd and param
 * @param[out]    
 * @return        alisl_retcode
 * @note          no operation
 *
 */
alisl_retcode alislcc_ioctl(void *handle, uint32_t cmd, uint32_t param);


#ifdef __cplusplus
}
#endif
#endif /*__ALISL_SDEC_H__*/






