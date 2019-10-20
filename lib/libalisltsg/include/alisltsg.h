/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisltsg.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 17:01:56
 *  @revision           none
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 */


#ifndef __ALISLTSG__H_
#define __ALISLTSG__H_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum tsg_id {
	TSG_ID_M36_0 = 0,  /**< Tsg id 0 */
	TSG_NB  ,          /**< Number of tsg */
} tsg_id_t;

/**
 *  Function Name:     alisltsg_check_remain_buf
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param pkt_cnt     remain ts pkt cnt
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note  Acturally means getting total ts pkt cnt stored in TSG buffer which
 *   has not been sent out by hw yet.
 */

alisl_retcode alisltsg_check_remain_buf(alisl_handle handle, uint32_t *byte_cnt);

/**
 *  Function Name:      alisltsg_close
 *  @brief
 *
 *  @param handle       point to struct tsg_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_close(alisl_handle *handle);

/**
 *  Function Name:      alisltsg_construct
 *  @brief
 *
 *  @param handle       pointer to struct tsg_device
 *
 *  @return
 *
 *  @author             Franky.Liang <franky.liang@alitech.com>
 *  @date               7/23/2013, Created
 *
 *  @note
 */


//alisl_retcode alisltsg_construct(alisl_handle *handle);

/**
 *  Function Name:     alisltsg_copydata
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param addr        point to transfer data addr
 *  @param addr        transfer data packet number
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_copydata(alisl_handle handle, void *addr,uint32_t pkt_cnt);

/**
 *  Function Name:     alisltsg_destruct
 *  @brief
 *
 *  @param  handle     pointer to struct tsg_device
 *  @param
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

//alisl_retcode alisltsg_destruct(alisl_handle *handle);

/**
 *  Function Name:     alisltsg_insertionstart
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param bitrate     ts bitrate
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_insertionstart(alisl_handle handle, uint32_t bitrate);

/**
 *  Function Name:     alisltsg_insertionstop
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_insertionstop(alisl_handle handle);

/**
 *  Function Name:        alisltsg_open
 *  @brief
 *
 *  @param  handle        pointer to struct tsg_device
 *  @param  id            enum tsg_id
 *  tunner_config_handle  pointer to param
 *  @return               alisl_retcode
 *
 *  @author               Franky.Liang  <franky.liang@alitech.com>
 *  @date                 7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_open(alisl_handle *handle, enum tsg_id id, void *param);

/**
 *  Function Name:     alisltsg_set_clkasync
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param clk_sel     tsg clk to be set
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_set_clkasync(alisl_handle handle, uint32_t clk_sel);


#ifdef __cplusplus
}
#endif

#endif /* __ALISLTSG__H_ */
