/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldsc.h
 *  @brief              scrambler/descrambler API (DSC)
 *
 *  @version            1.0
 *  @date               06/21/2013 04:04:49 PM
 *  @revision           none
 *
 *  @authors            Terry Wu <terry.wu@alitech.com>
 *                      Vincent Pilloux <vincent.pilloux@alitech.com>
 */

#ifndef __ALISLDSC_INTERFACE__H_
#define __ALISLDSC_INTERFACE__H_

#include <stdbool.h>
#include <alipltfretcode.h>
#include <inttypes.h>

#include <ali_dsc_common.h>
#include <ca_dsc.h>
#include <ca_sha.h>


#ifdef __cplusplus
extern "C"
{
#endif

/* valid values are: AES, TDES, CSA. For SHA, use custom API (see below) */
typedef enum WORK_SUB_MODULE algo_t;

#define SL_R2R_PID 0x1234

struct key_from {
    struct ca_create_kl_key *kl_key;
    struct ca_create_clear_key *clear_key;
    struct ca_create_otp_key *otp_key;
};

/*sha define */
#define SL_SHA1_DIGEST_SIZE         20
#define SL_SHA224_DIGEST_SIZE       28
#define SL_SHA256_DIGEST_SIZE       32
#define SL_SHA384_DIGEST_SIZE       48
#define SL_SHA512_DIGEST_SIZE       64

/*for pvr*/
#define SL_PVR_IO_UPDATE_ENC_PARAMTOR 	1
#define SL_PVR_IO_DECRYPT					2
#define SL_PVR_IO_FREE_BLOCK				3
#define SL_PVR_IO_SET_BLOCK_SIZE			4

#define SL_PVR_IO_CAPTURE_DECRYPT_RES		5
#define SL_PVR_IO_SET_DECRYPT_RES			6
#define SL_PVR_IO_RELEASE_DECRYPT_RES		7
#define SL_PVR_IO_DECRYPT_EVO				8
#define SL_PVR_IO_DECRYPT_ES_EVO            9
#define SL_PVR_IO_DECRYPT_EVO_SUB            10

/**
 *  @brief              open algorithm device according to the available
 *                      device id
 *
 *  @param[in]          dev_id     the available AES device id
 *  @param[in/out]      handle     point to the algorithm device structure
 *
 *  @return             0          if sucessful, error code otherwise
 *
 *  @author             Vincent Pilloux <vincent.pilloux@alitech.com>
 *  @date               18/03/2014, Created
  */
alisl_retcode alisldsc_algo_open(alisl_handle *handle, algo_t algo,
                                 uint32_t dev_id);

/**
 *  @brief              close algorithm device
 *
 *  @param[in]          handle      point to the algorithm device structure
 *
 *  @return             0               if sucessful, error code otherwise
 *
 *  @author             Vincent Pilloux <vincent.pilloux@alitech.com>
 *  @date               18/03/2014, Created
 */
alisl_retcode alisldsc_algo_close(alisl_handle handle);

/**
 *  @brief              algorithm ioctl
 *
 *  @param[in]          handle      point to algorithm device structure
 *  @param[in]          cmd         ioctl command
 *  @param[in/out]      param       point to parameters
 *
 *  @return                         return ioctl return value directly
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               09/04/2014, Created
 */
alisl_retcode alisldsc_algo_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param);
/**
 *  Function Name:      alisldsc_algo_decrypt
 *  @brief              decrypt data with the specified algorithm
 *
 *  @param[in]          handle          point to the 'algo' device structure
 *  @param[in]          stream_id       0~3 for TS mode,4~7 for data mode
 *  @param[in]          input           point to the data need to decrypt
 *  @param[in]          len             data lenght
 *  @param[out]         output          decrypted data
 *
 *  @return             0               if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/01/2013, Created
  */
alisl_retcode alisldsc_decrypt(alisl_handle handle,
                               uint8_t *input,uint8_t *output,
                               uint32_t total_len);

/**
 *  Function Name:      alisldsc_aes_encrypt
 *  @brief              encrypt data with the specified algorithm
 *
 *  @param[in]          handle          point to 'algo' device structure
 *  @param[in]          stream_id       0~3 for TS mode,4~7 for data mode
 *  @param[in]          input           point to the data need to encrypt
 *  @param[in]          len             data lenght
 *  @param[out]         output          encrypted data
 *
 *  @return             0               if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/01/2013, Created
 */
alisl_retcode alisldsc_encrypt(alisl_handle handle,
                               uint8_t *input, uint8_t *output,
                               uint32_t total_len);

/**
 *  Function Name:      alisldsc_aes_create_crypt_stream
 *  @brief              create a stream handler for encryption/decryption
 *
 *  @param[in]          handle      point to 'algo' device structure
 *  @param[in/out]      key_param   point to 'algo' key parameters. The
 *                                  stream handler is returned in
 *                                  key_param->handle.
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @dat                07/02/2013, Created
 */
alisl_retcode alisldsc_algo_create_crypt_stream(alisl_handle handle,
        void *algo_param,
        KEY_PARAM *key_param);


/**
 *  Function Name:      alisldsc_algo_update_cw
 *  @brief              update key information without creating a new handler
 *                      (Behaves as alisldsc_algo_encrypt without
 *                      key handler creation)
 *
 *  @param[in]          handle      point to algo device structure
 *  @param[in]          key_from    point to algo key_from param
 *  @param[in]          key_handle  update the key belong to the key_handle
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @dat                07/02/2013, Created
 */
alisl_retcode alisldsc_update_key(alisl_handle handle,
                                  struct key_from *key_from, int key_handle);


/**
 *  Function Name:      alisldsc_algo_delete_crypt_stream
 *  @brief              delete the encryption/decryption handler
 *
 *  @param[in]          handle            point to 'algo' device structure
 *  @param[in]          stream_handler    the enc/dec stream handler to delete
 *
 *  @return             0                 if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/02/2013, Created
 */
alisl_retcode alisldsc_algo_delete_crypt_stream(alisl_handle handle,
        uint32_t stream_handler);
/**
 *  Function Name:      alisldsc_dsc_open
 *  @brief              open descrambler device (DSC)
 *
 *  @param[out]         handle        point to descramble structure
 *
 *  @return             0             if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_open(alisl_handle *handle);

/**
 *  Function Name:      alisldsc_dsc_close
 *  @brief              close descrambler device
 *
 *  @param[in]          handle      point to DSC device structure
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/05/2013, Created
 */
alisl_retcode alisldsc_dsc_close(alisl_handle handle);

/**
 *  Function Name:      alisldsc_dsc_ioctl
 *  @brief              DSC ioctl
 *
 *  @param[in]          handle      point to DSC device structure
 *  @param[in]          cmd         ioctl command
 *  @param[in/out]      param       point to parameters
 *
 *  @return                         return ioctl return value directly
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               09/04/2014, Created
 */
alisl_retcode alisldsc_dsc_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param);

/**
 *  Function Name:      alisldsc_dsc_get_free_subdev
 *  @brief              get the available sub device id (of the algorithm)
 *
 *  @param[in]          handle      point to the descramble structure
 *  @param[in]          algo        sub device algorithm
 *  @param[out]         dev_id      save the available sub device id
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/26/2013, Created
  */
alisl_retcode alisldsc_dsc_get_free_subdev(alisl_handle handle,
        algo_t algo,
        uint32_t *dev_id);

/**
 *  Function Name:      alisldsc_dsc_set_subdev_idle
 *  @brief              set the specified sub device idle
 *
 *  @param[in]          handle      point to the descramble structure
 *  @param[in]          algo        sub device algorithm
 *  @param[in]          dev_id      sub device id
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/26/2013, Created
 */
alisl_retcode alisldsc_dsc_set_subdev_idle(alisl_handle handle,
        algo_t algo,
        uint32_t dev_id);

/**
 *  Function Name:      alisldsc_dsc_get_free_stream_id
 *  @brief              get a free stream id (reservation)
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          mode        TS mode or DATA mode
 *  @param[out]         stream_id   save the free stream id
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/27/2013, Created
 */
alisl_retcode alisldsc_dsc_get_free_stream_id(alisl_handle handle,
        enum DMA_MODE mode,
        uint32_t *stream_id);
/**
 *  Function Name:      alisldsc_dsc_set_stream_id_idle
 *  @brief              free the specified stream id
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          stream_id   the specified stream id
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/27/2013, Created
 */
alisl_retcode alisldsc_dsc_set_stream_id_idle(alisl_handle handle,
        uint32_t stream_id);

/**
 *  Function Name:      alisldsc_dsc_set_stream_id_used
 *  @brief              use the specified stream id
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          stream_id   the specified stream id
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               09/05/2014, Created
 */
alisl_retcode alisldsc_dsc_set_stream_id_used(alisl_handle handle,
        uint32_t stream_id);

/**
 *  Function Name:      alisldsc_dsc_parse_stream_id
 *  @brief              Store the stream id dedicated for "live" streaming
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          stream_id   the specified stream id
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_parse_stream_id(alisl_handle handle,
        uint32_t dmx_fd);

/**
 *  Function Name:      alisldsc_dsc_get_subdev_handle
 *  @brief              get sub device handle
 *
 *  @param[in]          handle           point to descramble structure
 *  @param[in]          algo             sub device mode
 *  @param[out]         dev_handler      save sub device handler
 *
 *  @return             0                if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_get_subdev_handle(alisl_handle handle,
        algo_t algo,
        uint32_t *dev_handler);

/**
 *  Function Name:      alisldsc_dsc_set_pvr_stream_idle
 *  @brief              free pvr's stream id
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          stream_id   the specified stream id for pvr
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_set_pvr_stream_idle(alisl_handle handle,
        uint32_t stream_id);

/**
 *  Function Name:      alisldsc_dsc_set_pvr_key_param
 *  @brief              set pvr key parameters for descrambling
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          key_param   pvr key parameter
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_set_pvr_key_param(alisl_handle handle,
        DSC_PVR_KEY_PARAM *key_param);

/**
 *  Function Name:      alisldsc_dsc_encrypt_bl_uk
 *  @brief              encrypt bootloader with universal key
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          bl_param    point to booloader key param
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_encrypt_bl_uk(alisl_handle handle,
        DSC_BL_UK_PARAM *bl_param);

/**
 *  Function Name:      alisldsc_dsc_DecryptEncrypt
 *  @brief              DeEncrypt data
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          p_DeEn      point to DeEncrypt config parameter
 *  @param[in]          input       need to DeEncrypt data
 *  @param[in]          total_len   DeEncrypt data len
 *  @param[out]         output      the DeEncrypted data
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               06/28/2013, Created
 */
alisl_retcode alisldsc_dsc_DecryptEncrypt(alisl_handle handle,
        DEEN_CONFIG *p_DeEn,
        uint8_t *input, uint8_t *output,
        uint32_t total_len);
/**
 *  Function Name:      alisl_dsc_operate_mem
 *  @brief              allocate or release pvr key memory for descrambling
 *
 *  @param[in]          handle      point to descramble structure
 *  @param[in]          cmd         memory command operation
 *  @param[in]          param       memory parameters
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/01/2013, Created
 */
alisl_retcode alisl_dsc_operate_mem(alisl_handle handle, uint32_t cmd,
                                    struct ali_dsc_krec_mem *krec_mem);

/**
 *  Function Name:      alisldsc_sha_open
 *  @brief              open SHA device according to the available device id
 *
 *  @param[in]          dev_id      the available SHA device id
 *  @param[in/out]      handle      point to SHA device structure
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/01/2013, Created
 */
alisl_retcode alisldsc_sha_open(alisl_handle * handle, uint32_t dev_id);

/**
 *  Function Name:      alisldsc_sha_close
 *  @brief              close SHA device
 *
 *  @param[in]          handle      point to SHA device structure
 *
 *  @return             0           if sucessful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/05/2013, Created
 */
alisl_retcode alisldsc_sha_close(alisl_handle handle);

/**
 *  @brief              use SHA device to decrypt data
 *
 *  @param[in]          handle      point to SHA device structure
 *  @param[in]          input       point to the data need to decrypt
 *  @param[in]          data_len    the decrypt data len
 *  @param[out]         output      save the decrypted data
 *
 *  @return             0           if successful, error code otherwise
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 *  @date               07/03/2013, Created
 */
alisl_retcode alisldsc_sha_digest(uint8_t *input,
                                  uint8_t *output, uint32_t data_len,
                                  uint32_t digest_len);


/**
 *  @brief              SHA ioctl
 *
 *  @param[in]          handle      point to SHA device structure
 *  @param[in]          cmd         ioctl command
 *  @param[in/out]      param       point to parameters
 *
 *  @return                         return ioctl return value directly
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               09/04/2014, Created
 */
alisl_retcode alisldsc_sha_ioctl(alisl_handle handle, uint32_t cmd, uint32_t param);

/**
 *  @brief              use SHA device to decrypt data without init
 *
 *  @param[in]          handle      point to SHA device structure
 *  @param[in]          input       point to the data need to decrypt
 *  @param[in]          data_len    the decrypt data len
 *  @param[out]         output      save the decrypted data
 *
 *  @return             0           if successful, error code otherwise
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               10/22/2014, Created
 */
alisl_retcode alisldsc_sha_digest_without_init(alisl_handle handle, uint8_t *input,
        uint8_t*output, uint32_t data_len);

alisl_retcode alisldsc_sha_get_buffer(alisl_handle handle, unsigned long size,  void **pp_buf);
alisl_retcode alisldsc_sha_release_buffer(alisl_handle handle, void *p_buf, unsigned long size);
alisl_retcode alisl_dsc_get_fd(alisl_handle handle, int *dsc_fd);
alisl_retcode alisldsc_attach_clear_key(alisl_handle handle,
                                        unsigned int data_type,
                                        struct ca_create_clear_key *key,
                                        unsigned short *pid_list,
                                        unsigned long pid_len,
                                     	int *key_handle);
alisl_retcode alisldsc_attach_kl_key(alisl_handle handle,
                                     unsigned int data_type,
                                     struct ca_create_kl_key *key,
                                     unsigned short *pid_list,
                                     unsigned long pid_len,
                                     int *key_handle);
alisl_retcode alisldsc_attach_otp_key(alisl_handle handle,
                                     unsigned int data_type,
                                     struct ca_create_otp_key *key,
                                     int *key_handle);

alisl_retcode alisldsc_update_param(alisl_handle handle, struct key_from *key_from, int crypt_mode, int key_handle);

alisl_retcode alisldsc_config_pvr(int *pvr_fd,
	int encrypt_fd,
	int pid_count,
	unsigned short * pid,
	int block_count);
alisl_retcode alisldsc_pvr_ioctl(int pvr_fd,unsigned int cmd, unsigned long args);
alisl_retcode alisldsc_pvr_open(alisl_handle *handle);
alisl_retcode alisldsc_pvr_close(alisl_handle handle);
alisl_retcode alisldsc_pvr_ioctl_ex(alisl_handle handle, unsigned int cmd, unsigned long param);
alisl_retcode alisldsc_pvr_fd_get(alisl_handle handle, int *pvr_fd);

alisl_retcode alisldsc_set_data_type(alisl_handle handle,unsigned int data_type);

/*
if need_map is 0, need munmap, or else need mmap. 
*/
#define MMAP_LEN (256*1024)
#define NEED_MUNMAP 0
#define NEED_MMAP 1
alisl_retcode alisldsc_pvr_mmap(alisl_handle handle, unsigned int *p_addr, unsigned int *buf_len, int need_map);
alisl_retcode alisldsc_pvr_start_block_mode(int *pvr_fd,
	unsigned long *enc_param,
	unsigned long block_count);
alisl_retcode alisldsc_pvr_free_block_mode(int *pvr_fd,int encyrpt_fd);

alisl_retcode alisldsc_add_pid(alisl_handle handle, unsigned short *pid_list,unsigned long pid_len,
	int crypt_mode,int parity, int key_handle);

alisl_retcode alisldsc_delete_pid(alisl_handle handle, int key_handle,
                                  unsigned short *pid_list,unsigned long pid_len);

alisl_retcode alisldsc_delete_key_handle(alisl_handle handle,int key_handle);
alisl_retcode alisldsc_pvr_free_resource(int *pvr_fd);


#ifdef __cplusplus
}
#endif

#endif
