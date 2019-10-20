/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               usb.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:13:36
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "usb.h"
#include "../../internal.h"

static usbupg_object_t object = {
	.name = UPG_OBJ_NAME,
};

static void *upgrade_obj_loop(void *desc_para)
{
	alislupg_desc_t *desc = (alislupg_desc_t *)desc_para;
	upg_object_t *object = desc->object;
	usbupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;
	upg_header_t *hdr = desc->header;
	upg_elem_t *elem = hdr->elem;
	FILE *upgpkt = NULL;
	size_t pktsz = 0;
	struct stat pktstat;
	size_t totalread = obj->upgdata_offset;
	size_t perread = UPG_BUF_SIZE;
	size_t elemread = 0;
	int ret = 0;
	int ret_len = 0;
	int writersem_val = 0;
#if UPGRADE_BUILD_CRYPTO
	void *buf = NULL;
#endif

	if (NULL == elem)
	{
		return;
	}
    
	stat(obj->pktname, &pktstat);
	upgpkt = fopen(obj->pktname, "rb");
	if (NULL == upgpkt)
	{
		SL_DBG("open usb file %s fail\n", obj->pktname);
		desc->error = ALISLUPG_ERR_OTHER;
		sem_post(&tune->sourcesem);
		goto over;
	}

	if(0 != fseek(upgpkt, obj->upgdata_offset, SEEK_SET))
	{
		SL_DBG("[%s](%d) can not seek to upgdata offset\n", __FUNCTION__, __LINE__);
		desc->error = ALISLUPG_ERR_OTHER;
		sem_post(&tune->sourcesem);
		goto over;
	}
	tune->total = pktstat.st_size;
	SL_DBG("[%s](%d) pktsz=%d, readoffset=0x%x\n",
	               __FUNCTION__, __LINE__, tune->total, totalread);

	tune->writting = true;

	while(totalread < pktstat.st_size && NULL != elem) 
	{
		if (0 == elem->imgsz) 
		{
			totalread += elem->size;
			elem = elem->next;
			continue;
		}

		while(1)
		{
			ret = sem_getvalue(&tune->writersem, &writersem_val);
			if(writersem_val>0)
			{
				sem_wait(&tune->writersem);
				break;
			}
			usleep(1000);
		}

		/*
		 * It safe to quit, due to the error is set by writter
		 * If the error hanppen, the writter should be already
		 * finish at that time
		 */

		if (ALISLUPG_ERR_NONE != desc->error)
		{
			break;
		}
		tune->elem = elem;
		tune->curoffset = elemread;
		if (elem->imgsz < UPG_BUF_SIZE)
		{
			perread = elem->imgsz;
		}
#if UPGRADE_BUILD_CRYPTO
		if (NULL != buf) 
		{
			free(tune->dest);
			tune->dest = buf;
			buf = NULL;
		}

		if (HWCRYPTO_SIGN_BYIMG == elem->sign_method) 
		{
			perread = elem->imgsz;
			if (NULL == buf)
			{
				buf = tune->dest;
			}
			tune->dest = malloc(perread);
			memset(tune->dest, 0, perread);
		}
#endif
		obj->buf = tune->dest;
		if (0 != fseek(upgpkt, elem->offset + elemread, SEEK_SET))
		{
			SL_DBG("[%s](%d) seek data fail\n", __FUNCTION__, __LINE__);
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}
		
		ret_len = fread(obj->buf, perread, 1, upgpkt);
		if (ret_len <= 0)
		{
			SL_DBG("[%s](%d) read data fail\n", __FUNCTION__, __LINE__);
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}
		elemread += perread;
		tune->size = perread;
		totalread += perread;
		SL_DBG("In %s Source perread 0x%x "
				  "total 0x%x 0x%x 0x%x elem size 0x%x 0x%x "
				  "elemread 0x%x, elem offset 0x%x part id %d, "
				  "current offset 0x%x\n", \
				  __FUNCTION__, perread, \
				  totalread, pktstat.st_size, \
				  tune->total, elem->size, \
				  elem->imgsz, elemread, \
				  elem->offset, elem->part_id, \
				  tune->curoffset);

		if (elem->imgsz - elemread < UPG_BUF_SIZE)
		{
			perread = elem->imgsz - elemread;
		}

		if (0 == perread) 
		{
			totalread += elem->size - elem->imgsz;
			elem = elem->next;
			elemread = 0;
			perread = UPG_BUF_SIZE;
		}

		desc->percent = (totalread) / (tune->total/100);
		if (totalread == tune->total)
		{
			desc->percent = 100;
		}
		SL_DBG("In %s Source read percent %d\n", __FUNCTION__, desc->percent);

		sem_post(&tune->sourcesem);
	}
	
over:

#if UPGRADE_BUILD_CRYPTO
	if (NULL != buf) 
	{
		free(tune->dest);
		tune->dest = buf;
	}
#endif

	tune->writting = false;
	if (upgpkt) {
		fclose(upgpkt);
	}

	SL_DBG("[%s(%d)] End\n", __FUNCTION__, __LINE__);
	return;
}

static alisl_retcode upgrade_obj_prestart(alislupg_desc_t *desc)
{
	/* not do anything for usb */

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_prestop(alislupg_desc_t *desc)
{
	/* not do anything for usb */

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	usbupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;

	if (NULL == tune->dest)
	{
		tune->dest = malloc(UPG_BUF_SIZE);
		if (tune->dest) {
			memset(tune->dest, 0, UPG_BUF_SIZE);
		} else {
			return ALISLUPG_ERR_OTHER;
		}
	}

	pthread_create(&obj->source, NULL, upgrade_obj_loop, desc);

	return ALISLUPG_ERR_NONE;
}


static alisl_retcode upgrade_obj_stop(alislupg_desc_t *desc)
{
	upg_tune_t *tune = desc->tune;

	if (NULL != tune->dest)
	{
		free(tune->dest);
	}

	tune->dest = NULL;

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_obj_getcfg
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *  @param          buf   store the configuration
 *  @param          max   max byte which could be store in the buf
 *
 *  @return         
 *                  Actual byte of configuration
 *                  If the return value > max, then the buf should be
 *                  reallocate the call this function again
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static size_t upgrade_obj_getcfg(alislupg_desc_t *desc, \
								 unsigned char *buf, \
								 size_t max)
{
	int             size = 0;
	upg_object_t    *obj = (upg_object_t *)desc->object;
	usbupg_object_t *object = (usbupg_object_t *)obj->plugin;
	FILE            *cfg = NULL;
	struct stat     stat;

	if (0 == strlen(object->config))
	{
		return 0;
	}

	lstat(object->config, &stat);

	if (0 == stat.st_size)
	{
		return 0;
	}

	if (UPG_CFG_V2 == desc->cfgver && stat.st_size > max) 
	{
		if (stat.st_size > UPG_MAX_CFGLEN)
		{
			return -EINVAL;
		}
		else
		{
			return stat.st_size;
		}
	}
	else if (UPG_CFG_V1 == desc->cfgver && UPG_CONFIGV1_SIZE > max)
	{
		return UPG_CONFIGV1_SIZE;
	}

	/*
	 * The parser need binary content
	 */
	if (NULL == (cfg = fopen(object->config, "rb")))
	{
		return -EINVAL;
	}

	if (UPG_CFG_V2 == desc->cfgver)
	{
		size = stat.st_size;
	}
	else if (UPG_CFG_V1 == desc->cfgver)
	{
		size = UPG_CONFIGV1_SIZE;
	}

	size = fread(buf, size, 1, cfg);

over:
	fclose(cfg);
	return size;
}


/**
 *  Function Name:  upgrade_obj_setdataoffset
 *  @brief          Common register function of upgrade plugin
 *
 *  @param          desc    descriptor with upgrade parameters
 *  @param          offset  offset
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode upgrade_obj_setdataoffset(alislupg_desc_t *desc, off_t offset)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	usbupg_object_t *obj = object->plugin;

	obj->upgdata_offset = offset;

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_getdataoffset(alislupg_desc_t *desc)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	usbupg_object_t *obj = object->plugin;

	return obj->upgdata_offset;
}

/**
 *  Function Name:  upgrade_obj_setpktname
 *  @brief          
 *
 *  @param          desc  descriptor with upgrade parameters
 *  @param          name  package name
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode upgrade_obj_setpktname(alislupg_desc_t *desc, char *name)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	usbupg_object_t *obj = object->plugin;

	strcpy(obj->pktname, name);

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_obj_register
 *  @brief          Common register function of upgrade plugin
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          param  param of upgrade
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  For a USB upgrade, we need the configuration file
 *  firstly. Supposely, the upgrade image will be in
 *  the same directory as the configuration file.
 */
alisl_retcode upgrade_obj_register(alislupg_desc_t *desc)
{
	upg_object_t *obj = (upg_object_t *)desc->object;

	if (UPG_USB == desc->source) 
	{
		strcpy(object.config, desc->config);
		obj->plugin = &object;
		obj->f_getcfg = upgrade_obj_getcfg;
		obj->f_prestart = upgrade_obj_prestart;
		obj->f_prestop = upgrade_obj_prestop;
		obj->f_start = upgrade_obj_start;
		obj->f_stop = upgrade_obj_stop;
		obj->f_setpktname = upgrade_obj_setpktname;
		obj->f_setupgdata_offset = upgrade_obj_setdataoffset;
		obj->f_getupgdata_offset = upgrade_obj_getdataoffset;
	}
	SL_DBG("In %s Object %s got\n", __FUNCTION__, object.name);

	desc->percent = 0;

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_obj_unregister
 *  @brief          Common unregister function of upgrade plugin
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          param  param of upgrade
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode upgrade_obj_unregister(alislupg_desc_t *desc, alislupg_param_t *param)
{

	if (UPG_USB == desc->source)
	{
		desc->object = NULL;
	}
    
	return ALISLUPG_ERR_NONE;
}
