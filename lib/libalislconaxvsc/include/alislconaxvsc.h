/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislconaxvsc.h
 *  @brief              Conax Virtual Smart Card API
 *
 *  @version            1.0
 *  @date               01/09/2016 09:50:28 AM
 *  @revision           none
 *
 *  @author             Deji Aribuki <deji.aribuki@alitech.com>
 */


#ifndef __ALISLCONAXVSC__H_
#define __ALISLCONAXVSC__H_

#include <alipltfretcode.h>
#include <inttypes.h>


#ifdef __cplusplus
extern "C"
{
#endif

#define SL_CONAXVSC_KL_PARITY_EVEN (1 << 0)
#define SL_CONAXVSC_KL_PARITY_ODD  (1 << 1)


struct alislconaxvsc_store {
    uint8_t *data;
    uint32_t data_len;
    uint8_t *key;
    uint8_t *hash;
};

typedef void (*alislconaxvsc_store_fun_cb) (
    struct alislconaxvsc_store *store,
    void *user_data
);

/**
 *  Function Name:     alislconaxvsc_open
 *  @brief             open vsc device
 *
 *  @param[out]        handle      point to VSC SL structure
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_open(alisl_handle *handle);

/**
 *  Function Name:     alislconaxvsc_close
 *  @brief             close vsc device
 *
 *  @param[in]         handle      point to VSC SL structure
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_close(alisl_handle handle);

/**
 *  Function Name:     alislconaxvsc_cmd_dispatch
 *  @brief             Exchange command with VSC
 *
 *  @param[in]         handle      point to VSC SL structure
 *  @param[in]         session_id  session identifier
 *  @param[in]         cmd         VSC command
 *  @param[in]         cmd_len     command length
 *  @param[out]        resp        command response
 *  @param[out]        resp_len    command response length
 *  @param[out]        sw_1        status word 1
 *  @param[out]        sw_2        status word 2
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_cmd_dispatch(alisl_handle handle,
	int session_id, uint8_t *cmd, int cmd_len, uint8_t *resp,
	int *resp_len, int *sw1, int *sw2);

/**
 *  Function Name:     alislconaxvsc_set_decw_key
 *  @brief             Decrypt and load VSC DECW to destination Key Ladder
 *
 *  @param[in]         handle      point to VSC SL structure
 *  @param[in]         kl_fd       key ladder file descriptor
 *  @param[in]         key_id      key identifier
 *  @param[in]         decw        encrypted code word
 *  @param[in]         parity      parity
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_set_decw_key(alisl_handle handle,
	int kl_fd, uint16_t key_id, uint8_t *decw, int parity);


/**
 *  Function Name:     alislconaxvsc_set_rec_en_key
 *  @brief             Decrypt and load VSC REC EN to destination Key Ladder
 *
 *  @param[in]         handle      point to VSC SL structure
 *  @param[in]         kl_fd       key ladder file descriptor
 *  @param[in]         en_key      key parameter
 *  @param[in]         parity      key parity
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_set_rec_en_key(alisl_handle handle,
	int kl_fd, uint8_t *en_key, int parity);

/**
 *  Function Name:     alislconaxvsc_set_uk_en_key
 *  @brief             Decrypt and load VSC UK EN to destination Key Ladder
 *
 *  @param[in]         handle      point to VSC SL structure
 *  @param[in]         kl_fd       key ladder file descriptor
 *  @param[in]         en_key      key parameter
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_set_uk_en_key(alisl_handle handle,
	int kl_fd, uint8_t *en_key);

/**
 *  Function Name:     alislconaxvsc_lib_init
 *  @brief             Initialize VSC-Lib
 *
 *  @param[in]         handle      pointer to VSC SL structure
 *  @param[in]         data        pointer to store data. Can be NULL
 *  @param[in]         key         pointer to data key
 *  @param[in]         hash        pointer to data hash
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_lib_init(alisl_handle handle, uint8_t *data,
	uint8_t *key, uint8_t *hash);

/**
 *  Function Name:     alislconaxvsc_register_callback
 *  @brief             Register store data read callback from VSC-Lib
 *
 *  @param[in]         handle      pointer to VSC SL structure
 *  @param[in]         p_user_data pointer to callback user data
 *  @param[in]         callback    pointer to callback function
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_register_callback(alisl_handle handle,
	void *p_user_data, alislconaxvsc_store_fun_cb callback);

/**
 *  Function Name:     alislconaxvsc_unregister_callback
 *  @brief             Unregister store data read callback from VSC-Lib
 *
 *  @param[in]         handle      pointer to VSC SL structure
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_unregister_callback(alisl_handle handle);

#ifdef _CAS9_VSC_RAP_ENABLE_
/**
 *  Function Name:     alislconaxvsc_get_keyid
 *  @brief             Get keyid from user
 *
 *  @param[in]         handle      pointer to VSC SL structure
 *  @param[in]         key_id      key identifier
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_get_keyid(alisl_handle handle,unsigned short* key_id);

/**
 *  Function Name:     alislconaxvsc_clear_store
 *  @brief             clear vsc store memory
 *
 *  @param[in]         handle      pointer to VSC SL structure
 *
 *  @return            0           if sucessful, error code otherwise
 */
alisl_retcode alislconaxvsc_clear_store(alisl_handle handle);
#endif
#ifdef __cplusplus
}
#endif

#endif
