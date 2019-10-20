/** @file     alislsbm.h
 *  @brief    include struct and function defination used in 
 *            share library of share buffer memory
 *  @author   wendy.he
 *  @date     2014-8-13
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2014-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef _SBM_ALISL_
#define _SBM_ALISL_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum sbm_id
{
    SBM_ID_SBM0 = 0,
    SBM_ID_SBM1,
    SBM_ID_SBM2,
    SBM_ID_SBM3,
    SBM_ID_SBM4,
    SBM_ID_SBM5,
    SBM_ID_SBM6,
    SBM_ID_SBM7,
    SBM_ID_SBM8,
    SBM_ID_SBM9,
    SBM_ID_SBM10,
    SBM_ID_SBM11,
    SBM_NUM_SBM  /**< Number of demux */
}sbm_id_t;

typedef enum sbm_wrap_mode
{
    SBM_WRAP_MODE_NORMAL = 0,
    SBM_WRAP_MODE_PACKET
}sbm_wrap_mode_t;


typedef enum sbm_lock_mode
{
    SBM_LOCK_MODE_MUTEX_LOCK = 0,
    SBM_LOCK_MODE_SPIN_LOCK
}sbm_lock_mode_t;

typedef struct sbm_buf_config 
{
    unsigned int buffer_addr;
    unsigned int buffer_size;
    unsigned int block_size;
    unsigned int reserve_size;
    enum sbm_wrap_mode wrap_mode;
    enum sbm_lock_mode lock_mode;
}sbm_buf_config_t;

/**  
 * @brief         Open ali_sbm(xx) for common usage. 
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[in]     id               sbm device id
 * @param[out]   
 * @return        alisl_retcode    0:success, other: fail
 * @note
 *
 */
alisl_retcode alislsbm_open(alisl_handle *handle, enum sbm_id id);

/**
 * @brief         close sbm device
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_close(alisl_handle handle);

/**
 * @brief         create sbm               
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[in]     id               sbm device id
 * @param[in]     config           sbm config
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_create(alisl_handle handle,
    const struct sbm_buf_config *config);

/**
 * @brief         destroy sbm
 *                The sbm should be destroyed when no longer needed
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_destroy(alisl_handle handle);

/**
 * @brief         reset sbm
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_reset(alisl_handle handle);

/**
 * @brief         write data to sbm
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[in]     buf              data to write
 * @param[in]     len              buf size
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_write(alisl_handle handle, unsigned char *buf, unsigned long size);

/**
 * @brief         get valid size
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm valid size
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_valid_size(alisl_handle handle, unsigned long* size);

/**
 * @brief         get free size
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm free size
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_free_size(alisl_handle handle, unsigned long* size);
    
/**
 * @brief         get total size
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm total size
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_total_size(alisl_handle handle, unsigned long* size);

/**
 * @brief         get packet number
 * @author        wendy.he
 * @date          2014-8-13
 * @param[in]     handle           sbm device pointer
 * @param[out]    size             sbm packet number
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislsbm_get_pkt_num(alisl_handle handle, unsigned int* num);
    
#ifdef __cplusplus
}
#endif

#endif /*_SBM_ALISL_*/
