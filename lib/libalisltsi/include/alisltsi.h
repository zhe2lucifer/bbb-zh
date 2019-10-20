/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisltsi.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 17:01:56
 *  @revision           none
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 */


#ifndef __ALISLTSI__H_
#define __ALISLTSI__H_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

/* For TS input interface */
#define ALISL_TSI_SPI_0   0    /** <The ID of SPI_TS 0 */
#define ALISL_TSI_SPI_1   1    /** <The ID of SPI_TS 1 */
#define ALISL_TSI_SPI_TSG 2    /** <SPI input from TSG */
#define ALISL_TSI_SPI_3   3    /** <The ID of SPI_DVBS2  */
#define ALISL_TSI_SSI_0   4    /** <The ID of SSI 0 */
#define ALISL_TSI_SSI_1   5    /** <The ID of SSI 1 */
#if(SYS_CHIP_MODULE==ALI_S3602F)
#define ALISL_TSI_SSI_2   12   /** <The ID of SSI 2 */
#define ALISL_TSI_SSI_3   13   /** <The ID of SSI 3 */
#define ALISL_PARA_MODE_SRC 6 /** <The mode is TSCI_PARALLEL_MODE,and sourc from CARD B*/
#else
#define ALISL_TSI_SSI_2   6    /** <The ID of SSI 2 */
#define ALISL_TSI_SSI_3   7    /** <The ID of SSI 3 */
#define ALISL_PARA_MODE_SRC 12/** <The mode is TSCI_PARALLEL_MODE,and sourc from CARD B*/
#endif
#define ALISL_TSI_SSI2B_0 8   /** <The ID of TSI_SSI  2Bit 0*/
#define ALISL_TSI_SSI2B_1 9   /** <The ID of TSI_SSI  2Bit 1*/
#define ALISL_TSI_SSI4B_0 10  /** <The ID of TSI_SSI  4Bit 0*/
#define ALISL_TSI_SSI4B_1 11  /** <The ID of TSI_SSI  4Bit 1*/

#define ALISL_TSI_SSI2B_2 14   /** <The ID of TSI_SSI  2Bit 0*/
#define ALISL_TSI_SSI2B_3 15   /** <The ID of TSI_SSI  2Bit 1*/
#define ALISL_TSI_SSI4B_2 16  /** <The ID of TSI_SSI  4Bit 0*/
#define ALISL_TSI_SSI4B_3 17  /** <The ID of TSI_SSI  4Bit 1*/

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum tsi_id {
	TSI_ID_M3602_0 = 0,  /**< Tsi id 0, related device is S3602_0 */
	TSI_NB  ,            /**< Number of tsi */
} tsi_id_t;

enum tsi_io_command {
	TSI_IOCMD_INOUT_SET = 0,
	TSI_IOCMD_CHANNEL_SET,
	TSI_IOCMD_OUTPUT_SET,
	TSI_IOCMD_CI_LINK_MODE_SET,
	TSI_IOCMD_CI_BYPASS_SET,
	TSI_IOCMD_CI_SPI_0_1_SWAP,
	TSI_IOCMD_INPUT_GET,
	TSI_IOCMD_CHANNEL_GET,
	TSI_IOCMD_OUTPUT_GET,
	TSI_IOCMD_CI_LINK_MODE_GET,
};

/**TSI output*/
#define ALISL_TSI_TS_A     0x01   /**<TS_A is default output to DMX 0*/
#define ALISL_TSI_TS_B     0x02   /**<TS_B is default output to DMX 1*/
#define ALISL_TSI_TS_C     0x03   /**<TS_B is default output to DMX 2*/
#define ALISL_TSI_TS_D     0x04   /**<TS_B is default output to DMX 3*/

/** For DeMUX output */
#define ALISL_TSI_DMX_0    0x00   /** <The ID of DMX 0 */
#define ALISL_TSI_DMX_1    0x01   /** <The ID of DMX 1 */
#define ALISL_TSI_DMX_2    0x02   /** <The ID of DMX 2 */
#define ALISL_TSI_DMX_3    0x03   /** <The ID of DMX 3 */

/**tsi ci link mode*/
#define ALISL_MODE_PARALLEL 1
#define ALISL_MODE_CHAIN    0

/**for check tsi info for demux*/
typedef struct alisl_tsiroute_info {
	uint8_t channel_id;     /**<channel id*/
	uint8_t input_id;       /**<tsi tds_input_id*/
	uint8_t ci_mode;        /**<parallel_mode*/
} alisl_tsiroute_info_t;

/**
 *  Function Name:     alisltsi_change_tsiid
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param chg_en      tsi spi
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_change_tsiid(alisl_handle handle,int32_t chg_en);

/**
 *  Function Name:     alisltsi_check_tsi_info
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param dmx_id      dmx_id
 *  @param tsi_info    point to struct alisl_tsiroute_info_t
 *  @return            alisl_retcode
 *
 *  @author            Franky.Liang  <franky.liang@alitech.com>
 *  @date              7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_check_tsi_info(alisl_handle handle,int32_t dmx_id, alisl_tsiroute_info_t* tsi_info);

/**
 *  Function Name:      alisltsi_close
 *  @brief
 *
 *  @param handle       point to struct tsi_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_close(alisl_handle handle);

/**
 *  Function Name:     alisltsi_destruct
 *  @brief
 *
 *  @param  handle     pointer to struct tsi_device
 *  @param
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

//alisl_retcode alisltsi_destruct(alisl_handle *handle);

/**
 *  Function Name:        alisltsi_open
 *  @brief
 *
 *  @param  handle        pointer to struct tsi_device
 *  @param  id            enum tsi_id
 *  tunner_config_handle  pointer to param
 *  @return               alisl_retcode
 *
 *  @author               Franky.Liang  <franky.liang@alitech.com>
 *  @date                 7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_open(alisl_handle *handle, enum tsi_id id, void *param);
/**
 *  Function Name:     alisltsi_set_cibypass
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param ci_id       ci id
 *  @param chg_en      enable bypass
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_set_cibypass(alisl_handle handle,int32_t ci_id,uint32_t chg_en);

/**
 *  Function Name:     alisltsi_set_parallel_mode
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param para        tsi ci link mode
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_set_parallel_mode(alisl_handle handle, int32_t para);
/**
 *  Function Name:     alisltsi_setchannel
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param input_id    tds_input_id
 *  @param channel_id  channel_id for setting
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_setchannel(alisl_handle handle, uint32_t input_id, int32_t channel_id);
/**
 *  Function Name:     alisltsi_setinput
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param input_id    tds_input_id
 *  @param attrib      attribute of tsi setting
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_setinput(alisl_handle handle, uint32_t input_id, uint32_t attrib);

/**
 *  Function Name:     alisltsi_setoutput
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param dmx_id      dmx_id
 *  @param channel_id  channel_id for setting
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_setoutput(alisl_handle handle,int32_t dmx_id, int32_t channel_id);

/**
 *  Function Name:      alisltsi_construct
 *  @brief
 *
 *  @param handle       pointer to struct tsi_device
 *
 *  @return
 *
 *  @author             Franky.Liang <franky.liang@alitech.com>
 *  @date               7/23/2013, Created
 *
 *  @note
 */


//alisl_retcode alisltsi_construct(alisl_handle *handle);


#ifdef __cplusplus
}
#endif

#endif /* __ALISLTSI__H_ */
