/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               internal.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 14:46:54
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#ifndef __STORAGE_INTERNAL__H_
#define __STORAGE_INTERNAL__H_

#include <alislstorage.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define STO_SFLASH_NAME        "/dev/mtd0"
#define STO_PFLASH_NAME        "/dev/mtd1"
#define STO_ERASE_TYPE_BLOCK 0
#define STO_ERASE_TYPE_SECTOR 1


/* Device descriptor */
typedef struct alislsto_dev {
	alislsto_type_t  type;
	char             *dev_name;
	void             *attr;
	int              handle;
	mtd_info_t       *info;
	loff_t           offset;
	int              maxpart;
	uint32_t         open_cnt;
	bool             pmi;
	void             *priv;
	unsigned long    bad_cnt;
	char             *bad_map;
	mtd_info_t       *info_logic;
    int             erase_type; // 0 - erase by block, 1 - erase by 4K
} alislsto_dev_t;

/* Flash layout mapping idx -> offset */
typedef struct storage_map {
	alislsto_idx_t  idx;
	loff_t           offset;/*for bug #56342, MUST use loff_t instead of off_t.*/
	bool            readonly;
	char            *name;
	size_t          size;
	size_t          imgsz;
} storage_map_t;

/* Private parameters */
typedef struct storage_priv {
	pthread_mutex_t  rdmutex;
	pthread_mutex_t  wrmutex;
	pthread_mutex_t  seekmutex;
	pthread_mutex_t  devlock;
	loff_t           margin;
} storage_priv_t;


/* Easy usage */
typedef struct pmi_info_user pmi_info_t;


/**
 *  Function Name:  storage_parse
 *  @brief
 *
 *  @param          dev  descriptor of device
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode storage_parse(alislsto_dev_t *dev);

#ifdef __cplusplus
}
#endif

#endif
