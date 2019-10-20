/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               parser.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:19:19
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* kernel headers */
#include <ali_mtd_common.h>
#include <config.h>

/* Internal header */
#include "internal.h"


#define UPG_TAIL_LIST(head, list, elem) \
	do { \
		if (NULL == list) \
			head = list = elem; \
		else { \
			list->next = elem; \
			list = elem; \
		} \
	} while (0)

#define __ba_2_ul(ba, endian) \
	({ \
		unsigned long val = 0; \
		if (__machine_endian() != endian) \
			val = (ba)[0] << 24 | (ba)[1] << 16 | \
			      (ba)[2] << 8 | (ba)[3] << 0; \
		else \
			val = (ba)[0] << 0 | (ba)[1] << 8 | \
			      (ba)[2] << 16 | (ba)[3] << 24; \
		val; \
	})

#define UPG_INVALID_SIZE(size) \
	({ \
		((0 == size) || (-1 == size)); \
	})


/**
 *  Function Name:  upgrade_parse_addelem
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *  @param          elem  upgrade element
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static void upgrade_parse_addelem(alislupg_desc_t *desc, upg_elem_t *elem)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *element = hdr->elem;
	upg_elem_t *dstelem = malloc(sizeof(upg_elem_t));

	if (!dstelem) {
		return;
	}
	memset(dstelem, 0, sizeof(upg_elem_t));
	memcpy(dstelem, elem, sizeof(upg_elem_t));

	if (NULL == element)
	{
		hdr->elem = dstelem;
		return;
	}

	while (NULL != element->next)
	{
		element = element->next;
	}

	element->next = dstelem;
}

/**
 *  Function Name:  upgrade_parse_delelem
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static void upgrade_parse_delelem(alislupg_desc_t *desc)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *elem = hdr->elem;
	upg_elem_t *prev = NULL;

	if (NULL == elem)
	{
		return;
	}

	while (NULL != elem->next)
	{
		prev = elem;
		elem = elem->next;
	}

	free(elem);
	if (prev) {
		prev->next = NULL;
	}
}


/**
 *  Function Name:  upgrade_parse_destroyelem
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
void upgrade_parse_destroyelem(alislupg_desc_t *desc)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *elem = hdr->elem;
	upg_elem_t *next = NULL;

	if (NULL == elem)
	{
		return;
	}

	while (NULL != elem)
	{
		next = elem->next;
		free(elem);
		elem = next;
	}

	hdr->elem = NULL;
}



/**
 *  Function Name:  upgrade_parse_sortelem
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static void upgrade_parse_sortelem(alislupg_desc_t *desc)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *elem = hdr->elem;
	upg_elem_t *appelem = NULL, *apphead = NULL;
	upg_elem_t *lowelem = NULL, *lowhead = NULL;
	upg_elem_t *bootelem = NULL, *boothead = NULL;

	if (NULL == elem)
	{
		return;
	}

	while (NULL != elem)
	{
		if (UPG_LEVEL_APPLICATION == elem->level)
		{
			UPG_TAIL_LIST(apphead, appelem, elem);
		}

		if (UPG_LEVEL_LOWLEVEL == elem->level)
		{
			UPG_TAIL_LIST(lowhead, lowelem, elem);
		}

		if (UPG_LEVEL_BOOT == elem->level)
		{
			UPG_TAIL_LIST(boothead, bootelem, elem);
		}

		elem = elem->next;
	}

	hdr->elem = NULL;

	/* To be a whole list */
	if (NULL != apphead)
	{
		appelem->next = NULL;
		hdr->elem = apphead;
		appelem->next = lowhead;
	}

	if (NULL != lowhead)
	{
		lowelem->next = NULL;

		if (NULL == hdr->elem)
		{
			hdr->elem = lowhead;
		}

		lowelem->next = bootelem;
	}

	if (NULL != boothead)
	{
		bootelem->next = NULL;

		if (NULL == hdr->elem)
		{
			hdr->elem = boothead;
		}
	}
}

static inline bool __machine_endian(void)
{
	unsigned long x = 0x12345678;

	return (0x12 == (x & 0x000000ff));

//	if (0x78 == (x & 0x000000ff))
//	{
//		return UPG_BIG_ENDIAN;
//	}
//	else if (0x12 == (x & 0x000000ff))
//	{
//		return UPG_LITTLE_ENDIAN;
//	}
}


static void __upgrade_parse_dumpelem(alislupg_desc_t *desc)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *elem = hdr->elem;

	if (NULL == elem) return;

	SL_DBG("In %s Dump upgrade elems ... \n", __FUNCTION__);

	do
	{
		SL_DBG("-------------------------------------\n");
		SL_DBG("Partition Size..........0x%x\n", elem->partsz);
		SL_DBG("Image Size..............0x%x\n", elem->imgsz);
		SL_DBG("Alinged 128K Size.......0x%x\n", elem->size);
		SL_DBG("Partition ID............%d\n", elem->part_id);
		SL_DBG("Private Area............%s\n", elem->private_part ? "yes" : "no");
		SL_DBG("Dest Offset.............0x%x\n", elem->dstoffset);
		SL_DBG("Element Data Offset.....0x%x %s\n", \
		               elem->offset, \
		               (elem->offset % (128 * 1024)) ? "abnormal" : "normal");
		SL_DBG("Element Level...........%s\n", (UPG_LEVEL_BOOT == elem->level) ? "boot" : \
		               ((UPG_LEVEL_LOWLEVEL == elem->level) ? "lowlevel" : \
		                ((UPG_LEVEL_APPLICATION == elem->level) ? "application" : \
		                 ((UPG_LEVEL_NONE == elem->level) ? "none" : "NULL"))));
		SL_DBG("-------------------------------------\n");
		elem = elem->next;
	}
	while (elem != NULL);
}

static void __upgrade_parse_dumphdr(alislupg_desc_t *desc)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;

	SL_DBG("In %s Dump upgrade header ... \n", __FUNCTION__);
	SL_DBG("-------------------------------------\n");
	SL_DBG("Config Version..........V%d\n", desc->cfgver + 1);
	SL_DBG("Config Name.............%s\n", desc->config);
	SL_DBG("Source..................%s\n", (UPG_USB == desc->source) ? "usb" : \
	               ((UPG_NET == desc->source) ? "net" : \
	                ((UPG_OTA == desc->source) ? "ota" : "NULL")));
	SL_DBG("Sign....................0x%x(V1)\n", hdr->sign);
	SL_DBG("Total Parts.............%d(V1)\n", hdr->parts);
	SL_DBG("Packet Size.............0x%x(V1)\n", hdr->pktsz);
	SL_DBG("Parts to be Upgrade.....%d(V1)\n", hdr->upgparts);
	SL_DBG("Data Offset.............0x%x\n", hdr->upgdata_offset);
	SL_DBG("Bootfile Size...........0x%x(V1)\n", hdr->bootfile_imgsz);
	SL_DBG("Bootfile Align Size.....0x%x(V1)\n", hdr->bootfile_size);
	SL_DBG("Ubootfile Size..........0x%x(V1)\n", hdr->ubootfile_imgsz);
	SL_DBG("File System Key Start...0x%x\n", hdr->fskey_start);
	SL_DBG("File System Key End.....0x%x\n", hdr->fskey_end);
	SL_DBG("Page Size of Flash......%d\n", hdr->pagesz);
	SL_DBG("Upgrade Whole Flash.....%s\n", hdr->mono ? "yes" : "no");
	SL_DBG("Upgrade Data Size.......0x%x\n", hdr->upgdatasz);
	SL_DBG("Upgrade Packet Name.....%s\n", hdr->pktname);
	SL_DBG("Max Part to Upgrade.....%d\n", hdr->maxpart_id);
	SL_DBG("-------------------------------------\n");

	__upgrade_parse_dumpelem(desc);
}

static upgrade_maplevel_t upg_maplevel[] =
{
	{STO_PMI,         UPG_LEVEL_NONE		},
	{STO_PRIVATE,     UPG_LEVEL_BOOT		},
	{STO_KERNEL,      UPG_LEVEL_LOWLEVEL    },
	{STO_SEE,         UPG_LEVEL_LOWLEVEL    },
	{STO_ROOTFS,      UPG_LEVEL_APPLICATION },
	{STO_APPDATA,     UPG_LEVEL_APPLICATION },
	{STO_USERFS,      UPG_LEVEL_APPLICATION },
	{STO_RESERVED1,   UPG_LEVEL_APPLICATION },
	{STO_RESERVED2,   UPG_LEVEL_APPLICATION },
	{STO_RESERVED3,   UPG_LEVEL_APPLICATION },
	{STO_RESERVED4,   UPG_LEVEL_APPLICATION	},
	{STO_RESERVED5,   UPG_LEVEL_APPLICATION	},
	{STO_UBOOT,       UPG_LEVEL_NONE		},
	{STO_UBOOT_BAK,   UPG_LEVEL_NONE		},
};

static alisl_retcode __upgrade_parse_configv1(alislupg_desc_t *desc, \
                unsigned char *buf, \
                size_t size)
{
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *elem = malloc(sizeof(upg_elem_t));
	upgstat_t stat;
	off_t elemoffset = 0;
	size_t part_off, part_size;
	int i = 0;
	size_t base = alislsto_tell_ext(desc->nand, STO_KERNEL, STO_PART_OFFSET);

	hdr->host_endian = UPG_BIG_ENDIAN;
	hdr->sign = __ba_2_ul(&buf[UPG_OFFSET_SIGN], hdr->host_endian);
	hdr->parts = __ba_2_ul(&buf[UPG_OFFSET_PARTS], hdr->host_endian);
	hdr->pktsz = __ba_2_ul(&buf[UPG_OFFSET_PKTSIZE], hdr->host_endian);
	hdr->upgparts = __ba_2_ul(&buf[UPG_OFFSET_UPGPARTS], hdr->host_endian);
	hdr->upgdata_offset = __ba_2_ul(&buf[UPG_OFFSET_UPGDATAOFFSET], hdr->host_endian);
	hdr->bootfile_imgsz = __ba_2_ul(&buf[UPG_OFFSET_BOOTSIZE], hdr->host_endian);
	hdr->bootfile_size = UPG_ALIGN_128K(hdr->bootfile_imgsz);
	hdr->ubootfile_imgsz = __ba_2_ul(&buf[UPG_OFFSET_UBOOT_LEN], hdr->host_endian);
	hdr->fskey_start = __ba_2_ul(&buf[UPG_OFFSET_KEYSTART], hdr->host_endian);
	hdr->fskey_end = __ba_2_ul(&buf[UPG_OFFSET_KEYEND], hdr->host_endian);
	hdr->pagesz = __ba_2_ul(&buf[UPG_OFFSET_PKTPAGESZ], hdr->host_endian);
	hdr->upgdatasz = hdr->pktsz - hdr->upgdata_offset;
	strcpy(hdr->pktname, desc->config);

	if (hdr->parts == hdr->upgparts + 1)
	{
		hdr->mono = true;
	}

	elemoffset = hdr->upgdata_offset;

	if (elem) {
		memset(elem, 0x00, sizeof(upg_elem_t));
	} else {
		return ALISLUPG_ERR_OTHER;
	}
	hdr->maxpart_id = 0;
	hdr->reqsz = 0;

	/* upgrade uboot */
	if (hdr->ubootfile_imgsz > 0)
	{
		alislupg_upginfo_read((unsigned char *)&stat, sizeof(upgstat_t));
		SL_DBG("In %s(%d), current uboot = 0x%x\n",
		               __FUNCTION__, __LINE__, stat.cur_uboot);

		elem->imgsz  = hdr->ubootfile_imgsz;

		/* boot file include uboot upgrade content */
		elem->size   = UPG_ALIGN_128K(elem->imgsz + DEFAULT_BOOTFILEDATA_START) \
		               - DEFAULT_BOOTFILEDATA_START;
		elem->offset = elemoffset + DEFAULT_BOOTFILEDATA_START;
		SL_DBG("In %s(%d), img size = 0x%x, align size = 0x%x, read offset = 0x%x\n", \
		               __FUNCTION__, __LINE__, elem->imgsz, elem->size, elem->offset);

		elem->part_id = stat.cur_uboot ? STO_UBOOT : STO_UBOOT_BAK;
		elem->partsz = alislsto_tell_ext(desc->nand, elem->part_id, STO_PART_SIZE);
		elem->dstoffset = alislsto_tell_ext(desc->nand, elem->part_id, STO_PART_OFFSET);
		elem->private_part = true;

		SL_DBG("In %s(%d), offset = 0x%x, partsz = 0x%x\n",
		               __FUNCTION__, __LINE__, elem->dstoffset, elem->partsz);
		upgrade_parse_addelem(desc, elem);
	}

	/* upgrade firmware */
	while (i < UPG_MAX_PARTS)
	{
		memset(elem, 0, sizeof(upg_elem_t));
		elem->partsz = __ba_2_ul(&buf[UPG_OFFSET_PARTSTART + i * 8 + 4], hdr->host_endian);
		elem->imgsz = __ba_2_ul(&buf[UPG_OFFSET_IMGLEN + i * 4], hdr->host_endian);
		elem->size = UPG_ALIGN_128K(elem->imgsz);
		elem->offset = elemoffset;

		if (0 == i)
		{
			/*
			   actually not private part upg image contain in upg file,
				so just set the next elemoffset to the part1
			*/
			elem->private_part = true;
			elemoffset += hdr->bootfile_size;
		}
		else
		{
			elem->private_part = false;
			elemoffset += elem->size;
		}

		if ((UPG_INVALID_SIZE(elem->partsz)) || (UPG_INVALID_SIZE(elem->imgsz)))
		{
			if (0 != i)
			{
				if (hdr->mono)
				{
					SL_ERR("In %s A hole is found "
					               "in a mono upgrade packet %d\n", \
					               __FUNCTION__, i);
				}

				i++;
				continue;
			}
		}

		elem->partsz *= 1024;
		hdr->reqsz += elem->partsz;
		/* Consider about the PMI index */
		elem->part_id = i + 1;

		if (elem->part_id > hdr->maxpart_id)
		{
			hdr->maxpart_id = elem->part_id;
		}

		/* exclude private partition */
		if (i != 0)
		{
			/* if it's a mono upgrade */
			if (hdr->mono)
			{
				elem->dstoffset = base;
				base += elem->partsz;
			}
			else
			{
				part_off = alislsto_tell_ext(desc->nand, elem->part_id, STO_PART_OFFSET);
				part_size = alislsto_tell_ext(desc->nand, elem->part_id, STO_PART_SIZE);

				/*
				* make sure that if it's not a mono upgrade,
				* new partition cannot be larger than original partition.
				* otherwise, new partition maybe overwrite next partition.
				*/
				if (elem->partsz > part_size)
				{
					SL_ERR("In %s, partition %d larger than "
					               " original partition, upgsize=0x%x originsize=0x%x\n", \
					               __FUNCTION__, elem->part_id, elem->partsz, part_size);
					return ALISLUPG_ERR_NOTENOUGHSPACE;
				}

				/* remain partition offset and size unchanged */
				elem->dstoffset = part_off;
				elem->partsz = part_size;

				SL_DBG("In %s(%d), offset=0x%x partsz=0x%x\n", \
				               __FUNCTION__, __LINE__, elem->dstoffset, elem->partsz);
			}
		}

		/* Level for sorting */
		elem->level = upg_maplevel[i + 1].level;

		/* Never upgrade image taged as NONE */
		if (UPG_LEVEL_NONE != elem->level)
		{
			upgrade_parse_addelem(desc, elem);
		}

		i++;
	}

#if ENABLE_DEBUG
	__upgrade_parse_dumphdr(desc);
#endif

#if 0   //evan.wu, netupg can not support sortelem right now
	upgrade_parse_sortelem(desc);

#if ENABLE_DEBUG
	__upgrade_parse_dumphdr(desc);
#endif
#endif

	free(elem);
	return ALISLUPG_ERR_NONE;
}

static alisl_retcode __upgrade_parse_configv2(alislupg_desc_t *desc, \
                unsigned char *buf, \
                size_t size)
{
	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_parse_config
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_parse_config(alislupg_desc_t *desc)
{
	unsigned char  *buf = malloc(UPG_MAX_CFGLEN);
	upg_object_t   *object = (upg_object_t *)desc->object;
	upg_header_t   *hdr = (upg_header_t *)desc->header;
	int            len = 0;
	int            trytimes = 0;
	alislupg_err_t err = ALISLUPG_ERR_NONE;

	if (NULL == object->f_getcfg)
	{
		SL_ERR("In %s Fail to get getcfg handler!\n", __FUNCTION__);
		free(buf);
		return ALISLUPG_ERR_OTHER;
	}

	if (buf) {
		memset(buf, 0, UPG_MAX_CFGLEN);
	} else {
		return ALISLUPG_ERR_OTHER;
	}

	while (true) 
	{
		len = object->f_getcfg(desc, buf, UPG_MAX_CFGLEN);
		/* retry when get config fail */
		if (len <= 0 && trytimes <= 5)
		{
			trytimes++;
			SL_ERR("In %s Fail to get config, try times: %d\n", __FUNCTION__, trytimes);
		}
		else
		{
			break;
		}
	}

	if (len <= 0)
	{
		SL_ERR("In %s Fail to get config %s\n", \
		               __FUNCTION__, desc->config);
		free(buf);
		return ALISLUPG_ERR_CANTGETCFG;
	}

	if (UPG_CFG_V1 == desc->cfgver)
	{
		err = __upgrade_parse_configv1(desc, buf, len);
	}
	else if (UPG_CFG_V2 == desc->cfgver)
	{
		err = __upgrade_parse_configv2(desc, buf, len);
	}

	object->f_setpktname(desc, hdr->pktname);

	/* set the start offset when read */
	if (hdr->ubootfile_imgsz > 0)
		object->f_setupgdata_offset(desc, hdr->upgdata_offset + DEFAULT_BOOTFILEDATA_START);
	else
		object->f_setupgdata_offset(desc, hdr->upgdata_offset + hdr->bootfile_size);

	free(buf);
	return err;
}


/**
 *  Function Name:  upgrade_parse_updatepmi
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_parse_updatepmi(alislupg_desc_t *desc)
{
	pmi_info_t *pmi_info = NULL;
	char pmi_new[STO_HEADER_SIZE];
	upg_header_t *hdr = (upg_header_t *)desc->header;
	upg_elem_t *elem = hdr->elem;
	upgstat_t stat;
	int eccsz = 0;
	int shift = 0;
	int i = 0, ret;
	off_t dstoffset;
	size_t partsz, imgsz;
	unsigned char *buf = NULL;
	
	SL_DBG("[%s(%d)] going to updata pmi\n", __FUNCTION__, __LINE__);

	pmi_info = (pmi_info_t *)malloc(sizeof(pmi_info_t));
	if (pmi_info) {
		memset(pmi_info->buf_start, 0x00, STO_HEADER_SIZE);
	} else {
		return ALISLUPG_ERR_OTHER;
	}
	pmi_info->offset = 0;
	pmi_info->length = STO_HEADER_SIZE;

	ret = alislsto_get_pmi(desc->nand, pmi_info);
	if (ret != STO_ERR_NONE)
	{
		SL_ERR("[%s(%d)] get pmi failed\n", __FUNCTION__, __LINE__);
		free(pmi_info);
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

#if 0

	for (i = 0; i < UPG_MAX_PARTS; i++)
	{
		SL_DBG("[%s(%d)] i=%d, partstart=0x%x, partsz=0x%x, imagesz=0x%x\n",
		               __FUNCTION__, __LINE__, i,
		               *(off_t *)(&buf[STO_IMAGE_BASEADDR_POS + i * 8]) << shift,
		               *(size_t *)(&buf[STO_IMAGE_BASEADDR_POS + i * 8 + 4]) << shift,
		               *(size_t *)(&buf[STO_IMAGELEN_BASEADDR_POS + i * 4])
		              );
	}

#endif

	memset(pmi_new, 0x00, STO_HEADER_SIZE);
	memcpy(pmi_new, pmi_info->buf_start, STO_HEADER_SIZE);

	while (elem)
	{
		if (elem->private_part)
		{
			SL_DBG("[%s(%d)] private partition\n",
			               __FUNCTION__, __LINE__);

			/* update uboot image size */
			if (elem->part_id == STO_UBOOT || elem->part_id == STO_UBOOT_BAK)
			{
				imgsz = elem->imgsz;
				i = elem->part_id - STO_UBOOT;
				SL_DBG("[%s(%d)] update uboot image size: 0x%x\n",
				               __FUNCTION__, __LINE__, imgsz);

				memcpy(pmi_new + (STO_UBOOTADDR_POS + i * 8 + 4), &imgsz, 4);
			}

			elem = elem->next;
			continue;
		}

		i = elem->part_id;
		/* partition start */
		SL_DBG("[%s(%d)] i=%d, dstoffset=0x%x, oldoffset=0x%x\n",
		               __FUNCTION__, __LINE__, i, elem->dstoffset,
		               *(off_t *)(&buf[STO_IMAGE_BASEADDR_POS + (i - 1) * 8]) << shift);
		dstoffset = elem->dstoffset >> shift;
		memcpy(pmi_new + (STO_IMAGE_BASEADDR_POS + (i - 1) * 8), &dstoffset, 4);
		/* partition size */
		SL_DBG("[%s(%d)] i=%d, partsz=0x%x, oldpartsz=0x%x\n",
		               __FUNCTION__, __LINE__, i, elem->partsz,
		               *(size_t *)(&buf[STO_IMAGE_BASEADDR_POS + (i - 1) * 8 + 4]) << shift);
		partsz = elem->partsz >> shift;
		memcpy(pmi_new + (STO_IMAGE_BASEADDR_POS + (i - 1) * 8 + 4), &partsz, 4);
		/* partition real len */
		SL_DBG("i=%d, imgsz=0x%x, oldimagsz=0x%x\n",
		               i, elem->imgsz,
		               *(size_t *)(&buf[STO_IMAGELEN_BASEADDR_POS + (i - 1) * 4]));
		imgsz = elem->imgsz;
		memcpy(pmi_new + (STO_IMAGELEN_BASEADDR_POS + (i - 1) * 4), &imgsz, 4);

		elem = elem->next;
	}

	memcpy(pmi_info->buf_start, pmi_new, STO_HEADER_SIZE);
	pmi_info->length = STO_HEADER_SIZE;

	SL_DBG("offset:%d length:%d\n", pmi_info->offset, pmi_info->length);

	ret = alislsto_set_pmi(desc->nand, pmi_info);
	if (ret != STO_ERR_NONE)
	{
		SL_ERR("[%s(%d)] set pmi failed\n", __FUNCTION__, __LINE__);
		free(pmi_info);
		return ret;
	}

	free(pmi_info);

	return ALISLUPG_ERR_NONE;
}

