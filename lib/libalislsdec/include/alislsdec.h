/** @file     alislsdec.h
 *  @brief    define sdec share library struct and functions
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
 * @brief         open sdec device ali_sdec
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          get sdec handle priv->fd
 *
 */
alisl_retcode alislsdec_open(void **handle);


/**
 * @brief         close sdec handle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislsdec_close(void *handle);


/**
 * @brief         start sdec, ioctl transmit start parameters to driver
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer, composition_page_id and
 *                ancillary_page_id are page id from uplayer functions.
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode  alislsdec_start(void *handle, uint16_t composition_page_id,
				uint16_t ancillary_page_id);


/**
 * @brief         stop sdec device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode  alislsdec_stop(void *handle);

/**
 * @brief         pause sdec device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode  alislsdec_pause(void *handle);

/**
 * @brief         show subtitle on osd
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislsdec_show(void *handle);


/**
 * @brief         hide the subtitle
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sdec device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislsdec_hide(void *handle);


/**
 * @brief         sdec ioctl, used for future
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     sdec device handle, ioctl cmd and param
 * @param[out]    
 * @return        alisl_retcode
 * @note          no operation
 *
 */
alisl_retcode alislsdec_ioctl(void *handle, uint32_t cmd, uint32_t param);


#ifdef __cplusplus
}
#endif
#endif /*__ALISL_SDEC_H__*/






