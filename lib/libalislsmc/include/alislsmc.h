/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsmc.h
 *  @brief              ALi smart card function interfaces
 *
 *  @version            1.0
 *  @date               06/04/2013 12:05:43 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef __ALISLSMC__H_
#define __ALISLSMC__H_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_SMC_SLOT 2

enum smc_io_command {
	SMC_IOCMD_SET_IO_ONOFF = 0, /**< enable/disable smart card io */
	SMC_IOCMD_SET_ETU,          /**< set working etu */
	SMC_IOCMD_SET_WWT,          /**< set block waiting time, in unit of ms */
	SMC_IOCMD_SET_GUARDTIME,
	SMC_IOCMD_SET_BAUDRATE,
	SMC_IOCMD_CHECK_STATUS,
	SMC_IOCMD_CLKCHG_SPECIAL,
	SMC_IOCMD_FORCE_SETTING,
	SMC_IOCMD_SET_CWT,          /**< set character waiting time, in unit of ms */
	SMC_IOCMD_GET_F,            /**< get F factor value */
	SMC_IOCMD_GET_D,            /**< get D factor value */
	SMC_IOCMD_GET_ATR_RESULT,   /**< check ATR status */
	SMC_IOCMD_GET_HB,           /**< get History Bytes */
	SMC_IOCMD_GET_PROTOCOL,     /**< get card current protocol */
	SMC_IOCMD_SET_WCLK,         /**< set the working clock of smc, the new setting
	                                 value will be used from the next time reset */
	SMC_IOCMD_GET_CLASS,        /**< return the currently selected classs */
	SMC_IOCMD_SET_CLASS,         /**< setting new class selection if previous
	                                 select fail */
	SMC_IOCMD_SET_PROTOCOL,     /**< set protocol */
	SMC_IOCMD_GET_WCLK,         // Get smart card clock frequency in Hz
	SMC_IOCMD_SET_RESET_MODE,    /**< Set reset mode: 0: cold reset; 1: warm reset */
	SMC_IOCMD_DISABLE_PPS_ON_RESET      /*Disable PPS when execute SMC_CMD_RESET,1: disable pps; 0: enble pps*/
};

enum class_select
{
	CLASS_NONE_SELECT = 0,
	CLASS_A_SELECT,
	CLASS_B_SELECT,
	CLASS_C_SELECT
};

struct smc_device_cfg {
	uint32_t init_clk_trigger : 1;   /**< 0, use default initial clk3.579545MHz. \n
	                                      1, use configed initial clk */
	uint32_t def_etu_trigger : 1;    /**< 0, use HW detected ETU as initial ETU. \n
	                                      1, use configed ETU as initial ETU */
	uint32_t sys_clk_trigger : 1;    /**< Currently, useless */
	uint32_t gpio_cd_trigger : 1;    /**< Current down detecting, while power
	                                      off, a gpio int will notify CPU to
	                                      do deactivation */
	uint32_t gpio_cs_trigger : 1;    /**< Currently, useless */
	uint32_t force_tx_rx_trigger : 1;/**< Support TX/RX timely switch */
	uint32_t parity_disable_trigger : 1;
	                                 /**< 0, disable parity check while get ATR \n
	                                      1, enable parity check while get ATR */
	uint32_t parity_odd_trigger : 1; /**< 0, use even parity while get ATR \n
	                                      1, use odd parity while get ATR */
	uint32_t apd_disable_trigger : 1;/**< 0, enable auto pull down function
	                                         while get ATR \n
	                                      1, disable auto pull down while get ATR */
	uint32_t type_chk_trigger : 1;   /**< 0, don't care card type check \n
	                                      1, check card type is A, B or AB type
	                                         according to interface device setting */
	uint32_t warm_reset_trigger : 1; /**< 0, all the reset are cold reset \n
	                                      1, all the reset are warm reset,
	                                         except the first one */
	uint32_t gpio_vpp_trigger : 1;   /**< Use a gpio pin to provide Vpp signal */
	uint32_t disable_pps : 1;
	uint32_t invert_power : 1;
	uint32_t invert_detect : 1;
	uint32_t class_selection_supported : 1;
	                                 /**< indicate current board support
					      more than one class */
	uint32_t board_supported_class : 6;
	                                 /**< indicate classes supported by
					      current board */
	uint32_t reserved : 10;

	uint32_t init_clk_number;
	uint32_t *init_clk_array;
	uint32_t default_etu;
	uint32_t smc_sys_clk;
	uint32_t gpio_cd_pol : 1;        /**< Polarity of GPIO, 0 or 1 active */
	uint32_t gpio_cd_io : 1;         /**< HAL_GPIO_I_DIR or HAL_GPIO_O_DIR
	                                      in hal_gpio.h */
	uint32_t gpio_cd_pos : 14;
	uint32_t gpio_cs_pol : 1;        /**< Polarity of GPIO, 0 or 1 active */
	uint32_t gpio_cs_io : 1;         /**< HAL_GPIO_I_DIR or HAL_GPIO_O_DIR
	                                      in hal_gpio.h */
	uint32_t gpio_cs_pos : 14;

	uint8_t  force_tx_rx_cmd;
	uint8_t  force_tx_rx_cmd_len;
	uint8_t  intf_dev_type;
	uint8_t  reserved1;
	uint32_t gpio_vpp_pol : 1;       /**< Polarity of GPIO, 0 or 1 active */
	uint32_t gpio_vpp_io : 1;        /**< HAL_GPIO_I_DIR or HAL_GPIO_O_DIR
	                                      in hal_gpio.h */
	uint32_t gpio_vpp_pos : 14;
	uint32_t reserved2: 16;

	uint32_t ext_cfg_tag;
	void     *ext_cfg_pointer;
	void     (*class_select) (enum class_select);
	                                 /**< call back function for class
					      selection operation */

	int      use_default_cfg;
};

/**
 *  Function Name:      alislsmc_open
 *  @brief              open actual hardware device
 *
 *  @param handle       pointer to module handle
 *  @param id           actual smart card id number that would open.
 *                      usually this id is 0, but if hardware have several
 *                      cards and driver supports different id, then this id
 *                      could be 0 or 1 or 2 or others.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_open(alisl_handle *handle, int id);

/**
 *  Function Name:      alislsmc_ioctl
 *  @brief              provide misc function method
 *
 *  @param handle       pointer to module handle
 *  @param cmd          command, refer to enum smc_io_command
 *  @param param        parameter for related command
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_ioctl(alisl_handle handle,
                             unsigned int cmd, unsigned long param);

/**
 *  Function Name:      alislsmc_set_cfg
 *  @brief              Set the card parameter
 *
 *  @param handle       pointer to module handle
 *  @param cfg          pointer to struct smc_device_cfg
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/05/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_set_cfg(alisl_handle handle,
                               struct smc_device_cfg *cfg);

/**
 *  Function Name:      alislsmc_set_defcfg
 *  @brief              Set default card parameter
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/05/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_set_defcfg(alisl_handle handle);

/**
 *  Function Name:      alislsmc_register_callback
 *  @brief              register a callback function which will be called when
 *                      monitor receive messages
 *
 *  @param handle       pointer to module handle
 *  @param p_user_data	user data from upper layer which will be transfered to upper layer by the callback.
 *  @param callback     function pointer
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_register_callback(alisl_handle handle,
											void *p_user_data,
											void (*callback)(void *user_data, uint32_t param));

/**
 *  Function Name:      alislsmc_start
 *  @brief              make hardware or driver start to work
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_start(alisl_handle handle);

/**
 *  Function Name:      alislsmc_card_exist
 *  @brief              check if there is card exist
 *
 *  @param handle       pointer to module handle
 *
 *  @retval 0           no card
 *  @retval !0          card exist
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
bool alislsmc_card_exist(alisl_handle handle);

/**
 *  Function Name:      alislsmc_reset
 *  @brief              reset smart card
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_reset(alisl_handle handle);

/**
 *  Function Name:      alislsmc_get_atr
 *  @brief              get answer-to-reset info
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer to store answer-to-reset information
 *  @param size         buffer size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_get_atr(alisl_handle handle,
                               void *buf, unsigned short *size);

/**
 *  Function Name:      alislsmc_deactive
 *  @brief              deactive smart card
 *
 *  @param handle       pointer to module handle
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_deactive(alisl_handle handle);

/**
 *  Function Name:      alislsmc_raw_read
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer to store raw data
 *  @param size         size to read
 *  @param actlen       actual size that have really read
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_raw_read(alisl_handle handle,
				void *buf, size_t size, size_t *actlen);

/**
 *  Function Name:      alislsmc_raw_write
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer stored raw data to be written
 *  @param size         size of buffer
 *  @param actlen       actual size that have written
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_raw_write(alisl_handle handle,
				void *buf, size_t size, size_t *actlen);

/**
 *  Function Name:      alislsmc_raw_fifo_write
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer stored raw data to be written
 *  @param size         size of buffer
 *  @param actlen       actual size that have written
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_raw_fifo_write(alisl_handle handle,
				void *buf, size_t size, size_t *actlen);

/**
 *  Function Name:      alislsmc_iso_transfer
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param command      iso transfer command
 *  @param nwrite       iso transfer command size
 *  @param response     iso transfer response
 *  @param nread        iso transfer response size
 *  @param actlen       actual response size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_iso_transfer(alisl_handle handle,
				void *command, size_t nwrite,
				void *response, size_t nread,
				size_t *actlen);

/**
 *  Function Name:      alislsmc_iso_transfer_t1
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param command      iso transfer t1 command
 *  @param nwrite       iso transfer t1 command size
 *  @param response     iso transfer t1 response
 *  @param nread        iso transfer t1 response size
 *  @param actlen       actual response size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_iso_transfer_t1(alisl_handle handle,
				void *command, size_t nwrite,
				void *response, size_t nread,
				size_t *actlen);

/**
 *  Function Name:      alislsmc_iso_transfer_t14
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param command      iso transfer t14 command
 *  @param nwrite       iso transfer t14 command size
 *  @param response     iso transfer t14 response
 *  @param nread        iso transfer t14 response size
 *  @param actlen       actual response size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_iso_transfer_t14(alisl_handle handle,
				void *command, size_t nwrite,
				void *response, size_t nread,
				size_t *actlen);

/**
 *  Function Name:      alislsmc_t1_transfer
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param dad
 *  @param sendbuf
 *  @param sendlen
 *  @param recvbuf
 *  @param recvlen
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_t1_transfer(alisl_handle handle,
				uint8_t dad,
				const void *sendbuf, size_t sendlen,
				void *recvbuf, size_t recvlen);

/**
 *  Function Name:      alislsmc_t1_xcv
 *  @brief

 *  @param handle       pointer to module handle
 *  @param sblock
 *  @param slen
 *  @param rblock
 *  @param rmax
 *  @param actlen
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_t1_xcv(alisl_handle handle,
				void *sblock, size_t slen,
				void *rblock, size_t rmax,
				size_t *actlen);

/**
 *  Function Name:      alislsmc_t1_negociate_ifsd
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param dad
 *  @param ifsd
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_t1_negociate_ifsd(alisl_handle handle,
				uint32_t dad, uint32_t ifsd);

/**
 *  Function Name:      alislsmc_abort
 *  @brief              make hardware or driver exit work mode
 *                      by unblock way
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_abort(alisl_handle handle);

/**
 *  Function Name:      alislsmc_stop
 *  @brief              make hardware or driver exit work mode
 *                      by block way
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_stop(alisl_handle handle);

/**
 *  Function Name:      alislsmc_close
 *  @brief              close hardware device
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_close(alisl_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* __ALISLSMC__H_ */
