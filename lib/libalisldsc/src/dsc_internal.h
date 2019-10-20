/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               dsc_internal.h
 *  @brief              Marco, structure definition of descramble device.
 *
 *  @version            1.0
 *  @date               06/21/2013 05:29:38 PM
 *  @revision           none
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 */

#ifndef __DSC_INTERNAL__H_
#define __DSC_INTERNAL__H_

#include <alipltflog.h>
#include <alipltfretcode.h>
#include <ali_dsc_common.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define NO_USED(x) ((void)x)

/* error handler of library */
typedef enum alisldsc_err {
	ALISLDSC_ERR_CANTOPENDEV    = ERROR_OPEN,
	ALISLDSC_ERR_INVALIDPARAM   = ERROR_INVAL,
	ALISLDSC_ERR_INVALIDHANDLE  = ERROR_INVAL,
	ALISLDSC_ERR_IOACCESS       = ERROR_IOCTL,
	ALISLDSC_ERR_NOTSUPPORTLEN  = ERROR_INVAL,
	ALISLDSC_ERR_CLOSEERROR     = ERROR_CLOSE,
	ALISLDSC_ERR_MMAPERROR      = ERROR_MMAP,
	ALISLDSC_ERR_GETMEMFAIL     = ERROR_GETMEM,
	ALISLDSC_ERR_RELMEMFAIL     = ERROR_RELMEM,
	ALISLDSC_ERR_MALLOCFAIL     = ERROR_NOMEM
} alisldsc_err_t;

#define ALISLDSC_DEBUG(err, ...) SL_DBG(__VA_ARGS__)

/* descramble device structure */
typedef struct dsc_dev {
	/* device type : CSA/DES/AES/SHA/DSC */
	int type;
	/* device file descriptor */
	int fd;
	/* device name */
	char * name;
	/* reserved */
	void * priv;
	/* open count */
	int open_cnt;
	uint32_t dev_id;
	int key_hdl;
	/* descramble userspace start address */
	void *dsc_user_baseaddr;
	int ca_format_flag;
	int add_pid_flag;
} alisldsc_dev_t;

/* descramble device name */
#define ALISLDSC_CSA_DEV_NAME       "/dev/ali_csa_0"
#define ALISLDSC_AES_DEV_NAME       "/dev/ali_aes_0"
#define ALISLDSC_DES_DEV_NAME       "/dev/ali_des_0"
#define ALISLDSC_SHA_DEV_NAME       "/dev/ali_sha_0"

#define ALISLDSC_SHA1_NAME			"ali-sha1"
#define ALISLDSC_SHA224_NAME		"ali-sha224"
#define ALISLDSC_SHA256_NAME		"ali-sha256"
#define ALISLDSC_SHA384_NAME		"ali-sha384"
#define ALISLDSC_SHA512_NAME		"ali-sha512"


#define ALISLDSC_DSC_DEV_NAME       "/dev/dsc0"

/* descramble device type */
#define ALISLDSC_DEV_TYPE_DSC       0x011e0000

/* invalid descramble sub device id */
#define INVALID_DSC_SUB_DEV_ID       0xff

/* invalid descramble stream id */
#define INVALID_DSC_STREAM_ID        0xff

/* add for get kernel memory */
#ifndef MEM_ALLOC_UNIT
#define MEM_ALLOC_UNIT              128*1024
#endif

#define DSC_MEM_SIZE                1024*1024
struct dsc_mem_bitmap {
	void * addr[8];
	unsigned char bitmap;
};

#ifdef __cplusplus
}
#endif

#endif
