/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               internal.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 14:52:51
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#ifndef __SYSINFO_INTERNAL__H_
#define __SYSINFO_INTERNAL__H_

#include <alislsysinfo.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Elements' length definition in system information */
#define SYSINFO_SIZE_DEFAULT    0x0
#define SYSINFO_SIZE_MAC        0x6
#define SYSINFO_SIZE_HDCPKEY    0x120
#define SYSINFO_SIZE_AESKEY     0x10

#define SYSINFO_SIZE_UPGINFO    0x200


/* Elements in system information area */
typedef enum alislsysinfo_idx {
	SYSINFO_MAC,
	SYSINFO_HDCPKEY,
	SYSINFO_AESKEY,
	SYSINFO_BOOTLOGO,
	SYSINFO_BOOTLOADER,
	SYSINFO_UBOOT,
	SYSINFO_UBOOTBK,
	SYSINFO_ROMAX,

	SYSINFO_SHAREMTD = 64,
	SYSINFO_UPGINFO,
	SYSINFO_EXTINFO,
	SYSINFO_UBOOTENV,
	SYSINFO_RWMAX,
} alislsysinfo_idx_t;

/* System information parameter */
typedef struct alislsysinfo_param {
	alislsysinfo_idx_t  idx;
	unsigned char       *buf;
	size_t              size;
} alislsysinfo_param_t;

/* System information descriptor */
typedef struct alislsysinfo_desc {
	alislsysinfo_idx_t  idx;
	/* Start of system information area */
	off_t               start;

	size_t              size;
	unsigned char       *buf;
	void                *priv;
} alislsysinfo_desc_t;

/* Mapping from NAND layout to system information ID */
typedef struct sysinfo_map {
	alislsysinfo_idx_t  idx;
	size_t              size;
	/* Offset to the start of system information area */
	loff_t              offset;
	/* If it's a block monopolized elements */
	bool                blockmono;
	/* If it's block aligned */
	bool                blockaligned;
	char                *name;
} sysinfo_map_t;

/* Definition of area in system area */
#define SYSINFO_BOOTLOADER_ADDR_POS         100
#define SYSINFO_MAC_ADDR_POS                160
#define SYSINFO_HDCPKEY_ADDR_POS            164            
#define SYSINFO_BOOTLOGO_ADDR_POS           172
#define SYSINFO_UPGINFO_ADDR_POS            184
#define SYSINFO_UBOOTENV_ADDR_POS           188
#define SYSINFO_UBOOT_ADDR_POS              192
#define SYSINFO_UBOOTBK_ADDR_POS            200
#define SYSINFO_ECCSIZE_POS                 28

/* Each key space in key area */
#define SYSINFO_PER_KEYSPACE                (8 * 1024)

/* Easy usage */
typedef struct pmi_info_user pmi_info_t;


/**
 *  Function Name:  sysinfo_parse
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode sysinfo_parse(alislsysinfo_desc_t *desc);


/**
 *  Function Name:  sysinfo_getmap
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         sysinfo map
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
sysinfo_map_t *sysinfo_getmap(alislsysinfo_desc_t *desc);


/**
 *  Function Name:  sysinfo_getidxbase
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         index base
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
int sysinfo_getidxbase(alislsysinfo_desc_t *desc);


/**
 *  Function Name:  sysinfo_setsize
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *  @param          size  set size
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
void sysinfo_setsize(alislsysinfo_desc_t *desc, size_t size);

#ifdef __cplusplus
}
#endif

#endif
