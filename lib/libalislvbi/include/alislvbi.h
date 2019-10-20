/** @file     libvbi.h
 *  @brief    include struct and function defination used in vbi share library
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef _VBI_ALISL_
#define _VBI_ALISL_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

//#include <sys_config.h>
//#include <basic_types.h>
//#include <mediatypes.h>

/* share library headers */
#include <alipltfretcode.h>


typedef enum {
    SL_IO_VBI_SET_SOURCE_TYPE,
    SL_IO_VBI_CHECK_TTX_TASK_START, 
    SL_IO_VBI_SET_CC_TYPE,
    SL_IO_VBI_GET_CC_TYPE,
    SL_IO_VBI_TTX_STOP,
} sl_vbi_io_control;

typedef enum {
    VBI_TYPE_TTX,
    VBI_TYPE_CC,
    VBI_TYPE_VPS,
    VBI_TYPE_WSS,    
} sl_vbi_source_type;

typedef void (* vbi_output_callback)(uint8_t field_polar);

enum vbi_output_device
{
	/**use hd tv encoder output*/
	VBI_OUTPUT_DEVICE_HD = 0,
	/**use sd tv encoder output*/
	VBI_OUTPUT_DEVICE_SD,
};


/**
*    @brief        vbi open
*    @author       ze.hong
*    @date         2013-6-19
*    @param[in]    handle is VBI device pointer
*    @param[out]   
*    @return       alisl_retcode
*    @note		
*
*/
alisl_retcode alislvbi_open(void **handle);

/**
 * @brief          close vbi 
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is vbi device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislvbi_close(void *handle);


/**
 * @brief         vbi io control
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cmd is io control command from uplayer
 * @param[out]    param2 param2_description
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislvbi_io_control(void *handle, uint32_t cmd, uint32_t param);

/**
 * @brief         write vbi data to TV  Encoder
 * @author        Iris.chen
 * @date          2016-10-18
 * @param[in]   handle is device pointer,
                       buffer is the data pointer
                       size  is data length
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislvbi_write(void *handle, char* buffer, uint32_t size);

/*Ignore this interface*/
alisl_retcode alislvbi_get_output_callback(void *handle, 
													vbi_output_callback *output_callback);
/*Ignore this interface*/
alisl_retcode alislvbi_set_output_device(void *handle, enum vbi_output_device output_device);

/*Ignore this interface*/
alisl_retcode alislvbi_start(void *handle);

/*Ignore this interface*/
alisl_retcode alislvbi_stop(void *handle);


#endif /*_VBI_ALISL_*/
