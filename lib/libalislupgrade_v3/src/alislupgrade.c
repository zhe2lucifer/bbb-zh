/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislupgrade.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:15:34
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
#include "internal.h"


/**
 *  Function Name:  alislupg_construct
 *  @brief          initial upgade handle with parameters
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_construct(alisl_handle *handle)
{
	alislupg_desc_t *desc;

	desc = malloc(sizeof(*desc));
	if (desc == NULL) 
	{
		SL_ERR("malloc memory failed!\n");
		return ALISLUPG_ERR_OTHER;
	}
	memset(desc, 0, sizeof(*desc));
	*handle = desc;

	desc->sources &= ~(1 << UPG_USB | 1 << UPG_OTA | 1 << UPG_NET);
	desc->methods &= ~(1 << UPG_BYFILE | 1 << UPG_BYPART);
	desc->object = malloc(sizeof(upg_object_t));
	if (desc->object) {
		memset(desc->object, 0, sizeof(upg_object_t));
	} else {
		return ALISLUPG_ERR_OTHER;
	}

#if UPGRADE_BUILD_OTA
	desc->sources |= 1 << UPG_OTA;
	upgrade_object_detect(desc, UPG_OTA);
#endif
#if UPGRADE_BUILD_USB
	desc->sources |= 1 << UPG_USB;
	upgrade_object_detect(desc, UPG_USB);
#endif
#if UPGRADE_BUILD_NET
	desc->sources |= 1 << UPG_NET;
	upgrade_object_detect(desc, UPG_NET);
#endif

	if (0 == (desc->sources & (1 << UPG_USB | 1 << UPG_OTA | 1 << UPG_NET))) 
	{
		SL_DBG("Can't find upgrade object\n");
		return ALISLUPG_ERR_NOSUPPORTOBJ;
	}

	desc->error = ALISLUPG_ERR_NONE;

	pthread_mutex_init(&desc->msg_mutex, NULL);
	pthread_cond_init(&desc->msg_cond, NULL);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_open
 *  @brief          load upgade plugin according to param
 *
 *  @param          handle  point to descriptor of upg
 *  @param          param   upg param
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_open(alisl_handle handle, alislupg_param_t* param)
{
	alislupg_desc_t *desc;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	desc->source = param->source;
	if (0 == (desc->sources & (1 << desc->source))) 
	{
		SL_DBG("ALISLUPG INFO: Can't find required object\n");
		return ALISLUPG_ERR_NOSUPPORTOBJ;
	}
	
	strcpy(desc->config, param->config);

	if (0 == param->progress_scale)
	{
		desc->progress_scale = UPG_DEFAULT_SCALE;
	}
	else
	{
		desc->progress_scale = param->progress_scale;
	}

	upgrade_object_init(desc);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_prestart
 *  @brief          initialization before start upgrade  
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_prestart(alisl_handle handle)
{
	alislupg_desc_t *desc;
	alislupg_err_t  err = ALISLUPG_ERR_NONE;
	upg_header_t *hdr = NULL;
	alislsto_param_t stoparam;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	upg_object_t *object = desc->object;

	object->f_prestart(desc);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_start
 *  @brief          start upgrade process 
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_start(alisl_handle handle)
{
	alislupg_desc_t *desc;
	alislupg_err_t  err = ALISLUPG_ERR_NONE;
	upg_header_t *hdr = NULL;
	alislsto_param_t stoparam;
	pthread_t progressid;

	memset(&stoparam, 0, sizeof(alislsto_param_t));
	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	/* Parse upgrade header */
	desc->header = malloc(sizeof(upg_header_t));
	if (desc->header) {
		memset(desc->header, 0, sizeof(upg_header_t));
	} else {
		return ALISLUPG_ERR_OTHER;
	}
	desc->tune = malloc(sizeof(upg_tune_t));
	if (desc->tune) {
		memset(desc->tune, 0, sizeof(upg_tune_t));
	} else {
		free(desc->header);
		return ALISLUPG_ERR_OTHER;
	}
	desc->writter = malloc(sizeof(upgrade_writter_t));
	if (desc->writter) {
		memset(desc->writter, 0, sizeof(upgrade_writter_t));
	} else {
		free(desc->header);
		free(desc->tune);
		return ALISLUPG_ERR_OTHER;
	}

	/* We need the storage information for validation check */
	alislsto_open(&desc->nand, STO_TYPE_NAND, O_RDWR);
	if (desc->nand == NULL)
		SL_ERR("Open device failed!\n");

	err = upgrade_parse_config(desc);
	if (ALISLUPG_ERR_NONE != err) 
	{
		SL_DBG("ALISLUPG ERR: parse config fail\n");
		alislsto_close(desc->nand, false);
		return ALISLUPG_ERR_PARSECFGERR;
	}
	
	alislsto_get_param(desc->nand, &stoparam);
	hdr = desc->header;
	if (!stoparam.info || stoparam.info->writesize / 1024 != hdr->pagesz)
	{
		SL_DBG("ALISLUPG ERR: Mismatch page size\n");
		alislsto_close(desc->nand, false);
		return ALISLUPG_ERR_MISMATCHPGSZ;
	}

	sem_init(&desc->oversem, 0, 0);

	/* If it's a whole flash upgrade 
	if ((stoparam.maxpart <= hdr->maxpart_id) && !hdr->mono) 
	{
		SL_ERR("Too many parts in packet and it's not a mono upgrade\n");
		alislsto_close(desc->nand, false);
		err = ALISLUPG_ERR_TOOMANYPARTS;
		goto error;
	} */

	if (hdr->mono && (hdr->reqsz > stoparam.info->size)) 
	{
		SL_ERR("Need space 0x%x available 0x%x\n", \
		       hdr->reqsz, stoparam.info->size);
		err = ALISLUPG_ERR_NOTENOUGHSPACE;
		alislsto_close(desc->nand, false);
		goto error;
	}

	alislsto_close(desc->nand, false);

	if (hdr->maxpart_id + UPG_UPGLOADER_PARTS > stoparam.maxpart) 
	{
		SL_DBG("Upgrade loader will be upgrade %d %d\n",
		       hdr->maxpart_id, stoparam.maxpart);
	}


	/*
	 * Start the tune process
	 * Design
	 *
	 * upgrade_plugin   writer      flash
	 * buf    ------>  buf ------> hw
	 *
	 * The writer will use the same buffer
	 * with the plugin to save space
	 * The source will set the dest buffer for writter
	 */
	upgrade_tune_init(desc);
	upgrade_tune_setsource(desc);
	upgrade_tune_setdest(desc);
	upgrade_tune_start(desc);

	return err;

error:
	sem_destroy(&desc->oversem);
	free(desc->writter);
	free(desc->tune);
	free(desc->header);

out:
	return err;
}

/*
 *  Function Name:  alislupg_wait_status
 *  @brief          wait for upgrade status update
 *
 *  @param          handle    descriptor with upgrade parameters
 *  @param          p_percent percent of upgrade process
 *  @param          p_error   error code of upgrade process
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_wait_status(alisl_handle handle, int *p_percent, int *p_error)
{
	alisl_retcode ret = ALISLUPG_ERR_NONE;
	alislupg_desc_t *desc;
	struct timespec timeout;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	pthread_mutex_lock(&desc->msg_mutex);
	/* Enlarge timeout from 5s to 20s due to https security upgrade need. */
	timeout.tv_sec = time(0) + 20;
	timeout.tv_nsec = 0; 
	if (ALISLUPG_ERR_NONE != desc->error)
	{
		SL_ERR("wait cond, desc->percent=%d, p_percent=%d\n", \
		desc->percent, *p_percent);
	}
	else
	{
		pthread_cond_timedwait(&desc->msg_cond, &desc->msg_mutex, &timeout);
		*p_percent = desc->percent;
		SL_DBG("wait cond, desc->percent=%d, p_percent=%d\n", \
		desc->percent, *p_percent);
	}

	*p_error = desc->error;
	ret = desc->error;
	pthread_mutex_unlock(&desc->msg_mutex);

	return ret;
}


/**
 *  Function Name:  alislupg_abort
 *  @brief          abort upgrade process 
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_abort(alisl_handle handle)
{
	alislupg_desc_t *desc;
	upg_object_t *object;

	SL_DBG("enter");
	
	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	object = desc->object;
	object->f_prestop(desc);

	if(NULL != desc->tune)
	{
		upgrade_tune_unsetsource(desc);
		upgrade_tune_unsetdest(desc);
		upgrade_tune_end(desc);
		upgrade_tune_deinit(desc);
		sem_destroy(&desc->oversem);
		upgrade_parse_destroyelem(desc);

		free(desc->writter);
		desc->writter = NULL;
		free(desc->tune);
		desc->tune = NULL;
		free(desc->header);
		desc->header = NULL;
	}
	
	SL_DBG("leave");
	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_stop
 *  @brief          stop upgrade process
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_stop(alisl_handle handle)
{
	alislupg_desc_t *desc;
	//upg_tune_t *tune = desc->tune;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}
	
	upgrade_tune_unsetsource(desc);
	upgrade_tune_unsetdest(desc);
	upgrade_tune_waitfinish(desc);
	upgrade_tune_end(desc);
	upgrade_tune_deinit(desc);
	sem_destroy(&desc->oversem);
	upgrade_parse_destroyelem(desc);

	free(desc->writter);
	desc->writter = NULL;
	free(desc->tune);
	desc->tune = NULL;
	free(desc->header);
	desc->header = NULL;
	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_close
 *  @brief          close upgrade handle 
 *
 *  @param          handle  point to descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_close(alisl_handle handle)
{
	alislupg_desc_t *desc;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}
	
	upgrade_object_deinit(desc);
	
	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislupg_destruct
 *  @brief          free upgrade handle 
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislupg_destruct(alisl_handle *handle)
{	
	alislupg_desc_t *desc;

	desc = (alislupg_desc_t *)(*handle);
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	pthread_cond_destroy(&desc->msg_cond);
	pthread_mutex_destroy(&desc->msg_mutex);

	free(desc->object);
	desc->object = NULL;

	free(desc);
	*handle = NULL;

	return STO_ERR_NONE;
}
