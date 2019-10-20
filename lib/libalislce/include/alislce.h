/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislce.h
 *  @brief              crypto engine (CE) or key ladder (KL) API
 *
 *  @version            1.0
 *  @date               07/08/2013 04:23:14 PM
 *  @revision           none
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 */


#ifndef __ALISLCE_INTERFACE__H_
#define __ALISLCE_INTERFACE__H_

#include <stdbool.h>
#include <alipltfretcode.h>
#include <inttypes.h>

#include "ali_ce_common.h"
#include "ca_dsc.h"
#include "ca_kl.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum sl_ce_root_key_type{
	SL_CE_ROOT_KEY_DEFAULT,
	SL_CE_ROOT_KEY_ETSI,
	SL_CE_ROOT_KEY_CONAXVSC
}sl_ce_root_key_type;

enum sl_ce_key_parity{
	SL_CE_KEY_PARITY_ODD,
	SL_CE_KEY_PARITY_EVEN,
};
/**
 *  Function Name:      alislce_open
 *  @brief              open crypto engine device 
 *
 *  @param[out]         handle      point to CE structure
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/09/2013, Created
 */
alisl_retcode alislce_open(alisl_handle *handle,unsigned int rootkey,
		sl_ce_root_key_type rootkey_type);


#if 0
alisl_retcode alislce_open_algo(alisl_handle *handle,int ca_algo);
#endif
/**
 *  Function Name:      alislce_close
 *  @brief              close crypto engine 
 *
 *  @param[in]          handle      point to CE device structure
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/09/2013, Created
  */
alisl_retcode alislce_close(alisl_handle handle);

/**
 *  Function Name:      alislce_ioctl
 *  @brief              crypto engine ioctl
 *
 *  @param[in]          handle      point to CE device structure
 *  @param[in]          cmd         ioctl command
 *  @param[in/out]      param       point to parameters
 *
 *  @return             0           return ioctl return value directly
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               09/05/2014, Created
  */
alisl_retcode alislce_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param);

/**
 *  Function Name:      alislce_key_load
 *  @brief              load a key from OTP (unused for the moment)
 *
 *  @param[in]          handle      point to CE device structure
 *  @param[in]          otp_info    point to OTP parameter
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/09/2013, Created
 */
//alisl_retcode alislce_key_load(alisl_handle handle, OTP_PARAM *otp_info);

/**
 *  Function Name:      alislce_generate_all_level_key
 *  @brief              derivate an encrypted key (on one level only). The
 *                      inputs are the encrypted key and the 'main' key. The 
 *                      'main' key can be either the root key (in OTP) or one
 *                      of its derivated version.
 *
 *  @param[in]          handle       point to CE device structure
 *  @param[in]          data_info    point to CE data info
 *
 *  @return             0            if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/09/2013, Created
 */
alisl_retcode alislce_generate_all_level_key(alisl_handle handle,
                                                struct kl_gen_key *data_info);

/**
 *  Function Name:      alislce_get_otp_root_key
 *  @brief              copy the root key from otp to the CE
 *
 *  @param[in]          handle      point to CE device structure
 *  @param[in]          otp_info    specify otp address and key pos
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/10/2013, Created
 */
alisl_retcode alislce_get_otp_root_key(alisl_handle handle,
				       OTP_PARAM *otp_info);

/**
 *  Function Name:      alislce_get_decrypt_key
 *  @brief              if the CE is not protected by OTPs, read the decrypted 
 *                      key. (Used for debugging).
 *
 *  @param[in]          handle      point to CE device structure
 *  @param[in/out]      key_info    specify the key len and mode, 
 *                                  and save the decrypt key
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/10/2013, Created
 */
alisl_retcode alislce_get_decrypt_key(alisl_handle handle,
									  unsigned char *key);

/**
 *  Function Name:      alislce_find_free_pos
 *  @brief              find an available key position in CE to store
 *                      the future generated key (using 
 *                      alislce_generate_all_level_key)
 *
 *  @param[in]          handle      point to CE device structure
 *  @param[in/out]      pos_param   specify key derivation level, and save the 
 *                                  available key position
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/10/2013, Created
 */
alisl_retcode alislce_get_key_fd(alisl_handle handle,
				    unsigned long *pos_param);


alisl_retcode alislce_create_key_hdl(alisl_handle handle,
                                       struct kl_config_key *cfg_kl_key);
alisl_retcode alislce_gen_hdcp_key(alisl_handle handle,
                                       struct kl_gen_hdcp_key * gen_hdcp);

alisl_retcode alislcf_set_target_pos(alisl_handle handle, enum sl_ce_key_parity sl_ce_parity);
alisl_retcode alislce_get_fd(alisl_handle handle);
alisl_retcode alislce_gen_one_level_plus(alisl_handle handle,
	struct kl_cw_derivation *key_config);
alisl_retcode alislcf_get_target_dev_param(alisl_handle handle, 
	struct kl_cw_derivation *key_config);

#ifdef __cplusplus
}
#endif

#endif
