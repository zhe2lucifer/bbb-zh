/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               parser.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 14:54:00
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* share library headers */
#include <alipltflog.h>
#include <alislsysinfo.h>

/* kernel headers */
#include <ali_mtd_common.h>

/* Internal header */
#include "internal.h"


/* Mapping table from system information to ID */
static sysinfo_map_t sysinfo_ro_map[] = {
	{SYSINFO_MAC,           SYSINFO_SIZE_MAC,      STO_DEFAULT_OFFSET, true, true, "MAC"},
	{SYSINFO_HDCPKEY,       SYSINFO_SIZE_HDCPKEY,  STO_DEFAULT_OFFSET, true, true, "HDCPKEY" },
	{SYSINFO_AESKEY,        SYSINFO_SIZE_AESKEY,   STO_DEFAULT_OFFSET, true, true, "AESKEY"  },
	{SYSINFO_BOOTLOGO,      SYSINFO_SIZE_DEFAULT,  STO_DEFAULT_OFFSET, true, true, "BOOTLOGO"},
	{SYSINFO_BOOTLOADER,    SYSINFO_SIZE_DEFAULT,  STO_DEFAULT_OFFSET, true, true, "LOADER"  },
	{SYSINFO_UBOOT,         SYSINFO_SIZE_DEFAULT,  STO_DEFAULT_OFFSET, true, true, "UBOOT"   },
	{SYSINFO_UBOOTBK,       SYSINFO_SIZE_DEFAULT,  STO_DEFAULT_OFFSET, true, true, "UBOOTBK" },
};

static sysinfo_map_t sysinfo_rw_map[] = {
	{SYSINFO_UPGINFO,       SYSINFO_SIZE_UPGINFO,  STO_DEFAULT_OFFSET, true, true, "UPG INFO"},
	{SYSINFO_EXTINFO,       SYSINFO_SIZE_DEFAULT,  STO_DEFAULT_OFFSET, true, true, "EXT INFO"},
	{SYSINFO_UBOOTENV,      SYSINFO_SIZE_DEFAULT,  STO_DEFAULT_OFFSET, true, true, "UBOOT ENV"},
};


/**
 *  Function Name:  sysinfo_parse
 *  @brief          
 *
 *  @param          desc  descriptor of system information
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode sysinfo_parse(alislsysinfo_desc_t *desc)
{
	void *dev = desc->priv;
	alislsysinfo_err_t err = SYSINFO_ERR_NONE;
	pmi_info_t *pmiinfo = NULL;
	int ret;
	unsigned char *buf = NULL;
	int eccsz = 0;
	int shift = 0;
	alislsto_param_t stoparam;

	memset(&stoparam,0,sizeof(alislsto_param_t));

	pmiinfo = (pmi_info_t *)malloc(sizeof(pmi_info_t));
	if (pmiinfo) {
		memset(pmiinfo->buf_start, 0x00, STO_HEADER_SIZE);
	} else {
		return SYSINFO_ERR_NULLBUF;
	}
	pmiinfo->offset = 0;
	pmiinfo->length = STO_HEADER_SIZE;

	ret = alislsto_get_pmi(dev, pmiinfo);
	if (ret != STO_ERR_NONE)
	{
		SL_ERR("get pmi failed\n");
		free(pmiinfo);
		return ret;
	}
    
	buf = pmiinfo->buf_start;
	
	eccsz = *(unsigned long *)&buf[SYSINFO_ECCSIZE_POS];
	if (1024 == eccsz) 
	{
		shift = 10;
	}
	else 
	{
		shift = 9;
	}
	SL_DBG("System Info parser enter %p\n", buf);

	sysinfo_ro_map[SYSINFO_MAC].offset = *(unsigned long *)&buf[SYSINFO_MAC_ADDR_POS];
	sysinfo_ro_map[SYSINFO_HDCPKEY].offset = *(unsigned long *)&buf[SYSINFO_HDCPKEY_ADDR_POS];
	sysinfo_ro_map[SYSINFO_AESKEY].offset = sysinfo_ro_map[SYSINFO_HDCPKEY].offset + SYSINFO_PER_KEYSPACE;
	sysinfo_ro_map[SYSINFO_BOOTLOGO].offset = *(unsigned long *)&buf[SYSINFO_BOOTLOGO_ADDR_POS];
	sysinfo_ro_map[SYSINFO_BOOTLOADER].offset = (*(unsigned long *)&buf[SYSINFO_BOOTLOADER_ADDR_POS]) << shift;
	sysinfo_ro_map[SYSINFO_UBOOT].offset = *(unsigned long *)&buf[SYSINFO_UBOOT_ADDR_POS];
	sysinfo_ro_map[SYSINFO_UBOOT].size = *(unsigned long *)&buf[SYSINFO_UBOOT_ADDR_POS + 4];
	sysinfo_ro_map[SYSINFO_UBOOTBK].offset = *(unsigned long *)&buf[SYSINFO_UBOOTBK_ADDR_POS];
	sysinfo_ro_map[SYSINFO_UBOOTBK].size = *(unsigned long *)&buf[SYSINFO_UBOOTBK_ADDR_POS + 4];
	
//SL_DBG will do nothing if disable DEBUG in Mini build,thus the warning that 'i  set but not used' occur
#if ENABLE_DEBUG
	int i = 0;
	SL_DBG("System Info parse RO...\n");
	for (i = SYSINFO_MAC; i < SYSINFO_ROMAX; i++) 
	{
		SL_DBG("-----------------------------\n");
		SL_DBG("Name................%s\n", sysinfo_ro_map[i].name);
		SL_DBG("Offset..............0x%x\n", sysinfo_ro_map[i].offset);
		SL_DBG("Size................0x%x\n", sysinfo_ro_map[i].size);
		SL_DBG("-----------------------------\n");
	}
	SL_DBG("In %s System Info end\n", __FUNCTION__);
#endif

	sysinfo_rw_map[SYSINFO_UPGINFO - SYSINFO_SHAREMTD - 1].offset = *(unsigned long *)&buf[SYSINFO_UPGINFO_ADDR_POS];
	alislsto_get_param(dev, &stoparam);
	sysinfo_rw_map[SYSINFO_EXTINFO - SYSINFO_SHAREMTD - 1].offset = *(unsigned long *)&buf[SYSINFO_UPGINFO_ADDR_POS] + stoparam.info->erasesize;
	sysinfo_rw_map[SYSINFO_UBOOTENV - SYSINFO_SHAREMTD - 1].offset = *(unsigned long *)&buf[SYSINFO_UBOOTENV_ADDR_POS];

#if ENABLE_DEBUG
	SL_DBG("System Info parse RW...\n");
	for (i = SYSINFO_UPGINFO; i < SYSINFO_RWMAX; i++) 
	{
		SL_DBG("-----------------------------\n");
		SL_DBG("Name................%s\n", sysinfo_rw_map[i - SYSINFO_UPGINFO].name);
		SL_DBG("Offset..............0x%x\n", sysinfo_rw_map[i - SYSINFO_UPGINFO].offset);
		SL_DBG("Size................0x%x\n", sysinfo_rw_map[i - SYSINFO_UPGINFO].size);
		SL_DBG("-----------------------------\n");
	}
	SL_DBG("System Info end\n");
#endif

	free(pmiinfo);

	return err;
}


/**
 *  Function Name:  sysinfo_getmap
 *  @brief          
 *
 *  @param          desc  descriptor of system information    
 *
 *  @return         sysinfo map
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
sysinfo_map_t *sysinfo_getmap(alislsysinfo_desc_t *desc)
{
	if (desc->idx < SYSINFO_SHAREMTD)
	{
		return sysinfo_ro_map;
	}
	else
	{
		return sysinfo_rw_map;
	}

	return NULL;
}


/**
 *  Function Name:  sysinfo_getidxbase
 *  @brief          
 *
 *  @param          desc  descriptor of system information    
 *
 *  @return         index base
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
int sysinfo_getidxbase(alislsysinfo_desc_t *desc)
{
	if (desc->idx < SYSINFO_SHAREMTD)
		return 0;
	else
		return SYSINFO_SHAREMTD + 1;
}


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
 *  @date           2013-6-27
 *
 */
void sysinfo_setsize(alislsysinfo_desc_t *desc, size_t size)
{
	if (desc->idx < SYSINFO_SHAREMTD)
	{
		sysinfo_ro_map[desc->idx].size = size;
	}
	else
	{
		sysinfo_rw_map[desc->idx - SYSINFO_SHAREMTD - 1].size = size;
	}

	return;
}
