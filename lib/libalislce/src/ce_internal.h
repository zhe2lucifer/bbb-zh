/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               ce_internal.h
 *  @brief              crypto engine internal marco and variable definition
 *
 *  @version            1.0
 *  @date               07/08/2013 05:16:48 PM
 *  @revision           none
 *
 *  @author             Terry Wu <terry.wu@alitech.com>
 */


#ifndef __CE_INTERNAL__H_
#define __CE_INTERNAL__H_

#include <ali_ce_common.h>
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* error handler of library */
typedef enum alislce_err {
	ALISLCE_ERR_CANTOPENDEV     = ERROR_OPEN,
	ALISLCE_ERR_INVALIDPARAM    = ERROR_INVAL,
	ALISLCE_ERR_INVALIDHANDLE   = ERROR_INVAL,
	ALISLCE_ERR_IOACCESS        = ERROR_IOCTL,
	ALISLCE_ERR_NOTSUPPORTLEN   = ERROR_INVAL,
	ALISLCE_ERR_CLOSEERROR      = ERROR_CLOSE,
	ALISLCE_ERR_MALLOCFAIL      = ERROR_NOMEM,
} alislce_err_t;


/* invalid device handle */
#define ALISLCE_INVALID_HANDLE      -1
#define CE_NAME_MAX		50

/* crypto engine device structure */
typedef struct ce_dev{
	/* device type */
	int type;
	/* device file descriptor */
	int fd;
#if 0
	int flag;  /*represent fd variabe open which ce device*/
#endif
	/* device name */
	char  name[CE_NAME_MAX];
	/* reserved */
	void * priv;
	/* open count */
	int open_cnt;
	/*flag kl key whether created or not*/
	int key_hdl;
	/*only when used ali_cf_target moudle the field is used*/
	int target_fd;
	int target_parity;
}alislce_dev_t;

/* crypto engine device name */
#define ALISLCE_DEV_AES		"/dev/kl/kl1"
#define ALISLCE_DEV_TDES		"/dev/kl/kl2"
#define ALISLCE_DEV_CSA2		"/dev/kl/kl0"
#define ALISLCE_DEV_CSA3		"/dev/kl/kl3"
#define ALISLCF_DEV_NAME		"/dev/ali_cf_target"

/* crypto engine device typr */
#define ALISLCE_DEV_TYPE        0x011f0000

/* definition of io param */
typedef struct ali_ce_hld_param alislce_ioparam_t;


#ifdef __cplusplus
}
#endif


#endif
