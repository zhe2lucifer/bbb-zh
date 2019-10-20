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
#include <config.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* kernel headers */
#include <ali_mtd_common.h>

/* Internal header */
#include "internal.h"

#if 0
#define UPG_TAIL_LIST(head, list, elem) \
	do { \
		if (NULL == list) \
			head = list = elem; \
		else { \
			list->next = elem; \
			list = elem; \
		} \
	} while (0)
#endif

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
	upg_elem_t *element = desc->elem;
	upg_elem_t *dstelem = malloc(sizeof(upg_elem_t));

	if (dstelem == NULL)
	{
		SL_ERR("%s malloc fail.\n", __FUNCTION__);
		return;
	}

	memset(dstelem, 0, sizeof(upg_elem_t));
	memcpy(dstelem, elem, sizeof(upg_elem_t));

	if (NULL == element)
	{
		desc->elem = dstelem;
		return;
	}

	while (NULL != element->next)
	{
		element = element->next;
	}

	element->next = dstelem;
}
#if 0
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
	upg_elem_t *elem = desc->elem;
	upg_elem_t *prev = NULL;

	if (NULL == elem)
	{
		return;
	}

	prev = elem;
	while (NULL != elem->next)
	{
		prev = elem;
		elem = elem->next;
	}

	free(elem);
	prev->next = NULL;
}
#endif

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
	upg_elem_t *elem = desc->elem;
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

	desc->elem = NULL;
}

static void __upgrade_parse_dumpelem(alislupg_desc_t *desc)
{
	upg_elem_t *elem = desc->elem;

	if (NULL == elem) return;

	SL_DBG("Dump upgrade elems ... \n");
	do
	{
		SL_DBG("-------------------------------------\n");
		SL_DBG("Partition Name............%s\n", elem->part_name);
		SL_DBG("Partition Image Size..............0x%x\n", elem->img_size);
		SL_DBG("Partition Image Offset(From Begin Of Package).....0x%x\n", elem->img_offset);
		SL_DBG("-------------------------------------\n");
		elem = elem->next;
	}
	while (elem != NULL);
}

static void __upgrade_parse_dumphdr(alislupg_desc_t *desc)
{
	SL_DBG("Dump upgrade header ... \n");
	SL_DBG("-------------------------------------\n");
	SL_DBG("Config Name.............%s\n", desc->config);
	SL_DBG("Source..................%s\n", (UPG_USB == desc->source) ? "usb" : \
	               ((UPG_NET == desc->source) ? "net" : \
	                ((UPG_OTA == desc->source) ? "ota" : "NULL")));
	SL_DBG("Parts to be Upgrade.....%d\n", desc->upg_part_num);
	SL_DBG("-------------------------------------\n");

	__upgrade_parse_dumpelem(desc);
}


static alisl_retcode __upgrade_parse_config(alislupg_desc_t *desc, \
                unsigned char *buf, \
                size_t size)
{
	upg_elem_t *elem = NULL;
	//off_t elemoffset = 0;
	size_t part_size = 0;
	int i = 0;
//	uint32_t off_set = 0;
//	uint32_t total_upgpart_len = 0;
	alislupg_header_t header = {0};

	elem = malloc(sizeof(upg_elem_t));
	if (elem == NULL)
	{
		return ALISLUPG_ERR_NOTENOUGHSPACE;
	}

	memcpy(&header, buf, sizeof(alislupg_header_t));
	memset(elem, 0, sizeof(upg_elem_t));
	memcpy(&desc->package_header, buf, sizeof(alislupg_header_t));//store the whole header to desc

	desc->upg_part_num = header.part_cnt;

	SL_DBG("\n header content : upg_version(%d), package_version(%d)\n",
			header.upg_ver, header.package_ver);
	if (header.upg_ver != 40)
	{
		SL_ERR("In %s, invalid package format\n",__FUNCTION__);
		free(elem);
		return ALISLUPG_ERR_INVALIDPACKAGE;
	}

	memset(elem, 0x00, sizeof(upg_elem_t));

	/* upgrade partitions */
	i = 0;
	while (i < desc->upg_part_num)
	{
		memset(elem, 0, sizeof(upg_elem_t));

		memcpy(elem->part_name, header.image_info[i].image_name, MAX_PART_NAME_LEN);
		elem->img_offset =  header.image_info[i].image_offset;
		elem->img_size = header.image_info[i].image_size;

		alislsto_get_partsize(elem->part_name, &part_size);
		if (elem->img_size > part_size)
		{
			SL_ERR("In %s, partition [%s] image size(0x%x) larger than partition size(0x%x)\n",
			               __FUNCTION__, elem->part_name, elem->img_size, part_size);
			free(elem);
			return ALISLUPG_ERR_NOTENOUGHSPACE;
		}

		upgrade_parse_addelem(desc, elem);

		i++;
	}

#if ENABLE_DEBUG
	__upgrade_parse_dumphdr(desc);
#endif

	free(elem);
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
static alisl_retcode upgrade_parse_config(alislupg_desc_t *desc)
{
	unsigned char  *buf = NULL;
	upg_object_t   *object = NULL;
	int            len = 0;
	int            trytimes = 0;
	alislupg_err_t err = ALISLUPG_ERR_NONE;
	int            config_len;

//	if (desc == NULL)
//	{
//		SL_ERR("%s invalid parameter.\n", __FUNCTION__);
//		return ALISLUPG_ERR_INVALIDPARAM;
//	}

	object = (upg_object_t *)desc->object;
	if (object == NULL)
	{
		SL_ERR("%s invalid parameter.\n", __FUNCTION__);
		return ALISLUPG_ERR_INVALIDPARAM;
	}
	if (NULL == object->f_getcfg)
	{
		SL_ERR("In %s Fail to get getcfg handler!\n", __FUNCTION__);
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	config_len = sizeof(alislupg_header_t);

	buf = malloc(config_len);
	if (buf == NULL)
	{
		return ALISLUPG_ERR_NOTENOUGHSPACE;
	}

	memset(buf, 0, config_len);

	//while (trytimes < 5)
	while (trytimes < 1)//the plugin will handle the retry, so not retry here
	{
		len = object->f_getcfg(desc, buf, config_len);
		/* retry when get config fail */
		if (len <= 0)
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
		SL_ERR("In %s Fail to get config\n", __FUNCTION__);
		free(buf);
		return ALISLUPG_ERR_CANTGETCFG;
	}

	err = __upgrade_parse_config(desc, buf, len);

	object->f_setpktname(desc, desc->config);

	free(buf);
	return err;
}

/**
 *  Function Name:  upgrade_get_config
 *  @brief          get config
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode upgrade_get_config(alislupg_desc_t *desc)
{
	alislupg_err_t  err = ALISLUPG_ERR_NONE;

	if (desc == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	err = upgrade_parse_config(desc);
	#if 0
	if (ALISLUPG_ERR_NONE != err)
	{
		SL_DBG("Parse config fail\n");
		//alislsto_close(desc->nand, false);
		return ALISLUPG_ERR_PARSECFGERR;
	}
	#endif

	return err;
}



