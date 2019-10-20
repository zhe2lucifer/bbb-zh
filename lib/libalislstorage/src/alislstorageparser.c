/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               parser.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 14:47:20
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

/* share library headers */
#include <alipltfretcode.h>
#include <alipltflog.h>
#include <alislstorage.h>

/* kernel headers */
#include <ali_mtd_common.h>

/* Internal header */
#include "internal.h"


/**
 *  Function Name:  storage_parse
 *  @brief          
 *
 *  @param          dev  descriptor of device
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode storage_parse(alislsto_dev_t *dev)
{
	pmi_info_t *pmi_info = NULL;
	int i = 0;
	storage_map_t *map = (storage_map_t *)dev->attr;
	unsigned char *buf = NULL;
	int eccsz = 0;
	int shift = 0;
	alisl_retcode ret = 0;

	pmi_info = (pmi_info_t *)malloc(sizeof(pmi_info_t));
	if (NULL == pmi_info) {
		SL_DBG("Malloc failed\n");
		return STO_ERR_MEM;
	}
	memset(pmi_info->buf_start, 0x00, STO_HEADER_SIZE);
	pmi_info->offset = 0;
	pmi_info->length = STO_HEADER_SIZE;

	ret = alislsto_get_pmi(dev, pmi_info);
	if (ret != STO_ERR_NONE)
	{
		SL_DBG("Can not get PMI, ret: %d\n", ret);
		free(pmi_info);
		pmi_info = NULL;
		return ret;
	}

	buf = pmi_info->buf_start;

	eccsz = *(unsigned long *)&buf[STO_ECCSIZE_POS];
	if (1024 == eccsz) 
	{
		shift = 10;
	}
	else 
	{
		shift = 9;
	}
    
	SL_DBG("NAND header parser running...\n");
	for (i = STO_KERNEL; i < STO_UBOOT; i++) 
	{
		map[i].offset = *(off_t *)(&buf[STO_IMAGE_BASEADDR_POS + (i - STO_KERNEL + 1) * 8]) << shift;
		map[i].imgsz = *(size_t *)(&buf[STO_IMAGELEN_BASEADDR_POS + (i - STO_KERNEL + 1) * 4]);
		map[i].size = *(size_t *)(&buf[STO_PARTLEN_BASEADDR_POS + (i - STO_KERNEL + 1) * 8]) << shift;
		if (0 != map[i].size)
		{
			dev->maxpart = i;
		}

		SL_DBG("-----------------------------\n");
		SL_DBG("Name...................%s\n", map[i].name);
		SL_DBG("Offset.................0x%x\n", map[i].offset);
		SL_DBG("Size...................0x%x\n", map[i].size);
		SL_DBG("Image Length...........0x%x\n", map[i].imgsz);
		SL_DBG("-----------------------------\n");
	}

	for (i = STO_UBOOT; i < STO_NANDMAX; i++)
	{
		map[i].offset = *(off_t *)(&buf[STO_UBOOTADDR_POS + (i - STO_UBOOT) * 8]);
		map[i].imgsz  = *(size_t *)(&buf[STO_UBOOTADDR_POS + (i - STO_UBOOT) * 8 + 4]);

		/* uboot partition size equal difference between uboot begin address */
		map[i].size= *(off_t *)(&buf[STO_UBOOTADDR_POS + 8]) - *(off_t *)(&buf[STO_UBOOTADDR_POS]);
        
		SL_DBG("-----------------------------\n");
		SL_DBG("Name...................%s\n", map[i].name);
		SL_DBG("Offset.................0x%x\n", map[i].offset);
		SL_DBG("Size...................0x%x\n", map[i].size);
		SL_DBG("Image Length...........0x%x\n", map[i].imgsz);
		SL_DBG("-----------------------------\n");
	}

	SL_DBG("NAND header parser end\n");
	SL_DBG("NAND max part id %d\n", dev->maxpart);

	map[STO_PMI].offset = 0;
	/* Value in PMI is absolute address */
	map[STO_PRIVATE].offset = 0;
	map[STO_PRIVATE].size = dev->info->size;

	free(pmi_info);

	return STO_ERR_NONE;
}

