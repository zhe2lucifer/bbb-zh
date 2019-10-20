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
#include <alipltflog.h>
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "internal.h"
#include "plugins/ota/lib_ota.h"
#include "plugins/net/net_interface.h"

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
	alislupg_desc_t *desc = NULL;

	desc = malloc(sizeof(alislupg_desc_t));
	if (desc == NULL) 
	{
		SL_ERR("malloc memory failed!(alislupg_desc_t)\n");
		return ALISLUPG_ERR_OTHER;
	}
	memset(desc, 0, sizeof(alislupg_desc_t));
	*handle = desc;

	desc->sources &= ~(1 << UPG_OTA | 1 << UPG_USB | 1 << UPG_NET);
	desc->object = malloc(sizeof(upg_object_t));
	if (desc->object == NULL)
	{
		free(desc);
		*handle = NULL;
		SL_ERR("malloc memory failed!(upg_object_t)\n");
		return ALISLUPG_ERR_OTHER;
	}
	memset(desc->object, 0, sizeof(upg_object_t));

	upgrade_object_detect(desc, UPG_OTA);
	upgrade_object_detect(desc, UPG_USB);
	upgrade_object_detect(desc, UPG_NET);
	SL_DBG("desc->sources=0x%x\n", desc->sources);

	if (0 == (desc->sources & (1 << UPG_OTA | 1 << UPG_USB | 1 << UPG_NET)))
	{
		free(desc->object);
		desc->object = NULL;
		free(desc);
		*handle = NULL;
		SL_ERR("Can't find upgrade object\n");
		return ALISLUPG_ERR_NOSUPPORTOBJ;
	}

	pthread_mutex_init(&desc->msg_mutex, NULL);
	pthread_cond_init(&desc->msg_cond, NULL);
	sem_init(&desc->oversem, 0, 0);

	desc->tune = malloc(sizeof(upg_tune_t));
	if (desc->tune == NULL)
	{
		free(desc->object);
		desc->object = NULL;
		free(desc);
		*handle = NULL;
		SL_ERR("malloc memory failed!(upg_tune_t)\n");
		return ALISLUPG_ERR_OTHER;
	}
	memset(desc->tune, 0, sizeof(upg_tune_t));

	desc->writter = malloc(sizeof(upgrade_writter_t));
	if (desc->writter == NULL)
	{
		free(desc->tune);
		desc->tune = NULL;
		free(desc->object);
		desc->object = NULL;
		free(desc);
		*handle = NULL;
		SL_ERR("malloc memory failed!(upgrade_writter_t)\n");
		return ALISLUPG_ERR_OTHER;
	}
	memset(desc->writter, 0, sizeof(upgrade_writter_t));

	desc->error = ALISLUPG_ERR_NONE;

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
	alislupg_desc_t *desc = NULL;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		SL_ERR("invalid parameter!\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}
	if (param == NULL)
	{
		SL_ERR("invalid parameter!\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	desc->source = param->source;
	if (0 == (desc->sources & (1 << desc->source))) 
	{
		SL_ERR("ALISLUPG INFO: Can't find required object\n");
		return ALISLUPG_ERR_NOSUPPORTOBJ;
	}

	desc->method = param->method;
	
	strcpy(desc->config, param->config);

	if (0 == param->progress_scale)
	{
		desc->progress_scale = UPG_DEFAULT_SCALE;
	}
	else
	{
		desc->progress_scale = param->progress_scale;
	}

	return upgrade_object_init(desc);
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
	alislupg_desc_t *desc = NULL;
//	alislupg_err_t  err = ALISLUPG_ERR_NONE;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		SL_ERR("invalid parameter!\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	upg_object_t *object = (upg_object_t *)desc->object;
	if (object == NULL)
	{
		SL_ERR("invalid parameter.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	object->f_prestart(desc);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_start_post
 *  @brief          start upgrade process
 *
 *	@param          handle : upgrade handler
 *
 *  @return         alisl_retcode
 *
 *  @author         terry.wu <terry.wu@alitech.com>
 *  @date           2014-10-8
 *
 */
alisl_retcode alislupg_start(alisl_handle handle)
{
	alislupg_desc_t * desc = NULL;
	alislupg_err_t  ret = ALISLUPG_ERR_NONE;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("invalid parameter!\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	if (!desc->config_got)
	{
		if (ALISLUPG_ERR_NONE != (ret = upgrade_get_config(desc)))
		{
			SL_ERR("get config fail, ret:%d.\n", ret);
			return ret;
		}
		desc->config_got = true;
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

	return ret;
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
	alislupg_desc_t *desc = NULL;
	struct timespec timeout = {0};

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	pthread_mutex_lock(&desc->msg_mutex);
	timeout.tv_sec = time(0) + 5;
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
	alislupg_desc_t *desc = NULL;
	upg_object_t *object = NULL;

	SL_DBG("enter\n");
	
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
		upgrade_parse_destroyelem(desc);
	}
	
	SL_DBG("leave");
	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_prestop
 *  @brief          stop download package for ONE-SHOT mode
 *
 *	@param          handle : upgrade handler
 *
 *  @return         alisl_retcode
 *
 *  @author         terry.wu <terry.wu@alitech.com>
 *  @date           2014-10-8
 *
 */
alisl_retcode alislupg_prestop(alisl_handle handle)
{
	alislupg_desc_t *desc = NULL;
	upg_object_t *object = NULL;

	SL_DBG("enter");
	
	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	object = desc->object;
	object->f_prestop(desc);

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
	alislupg_desc_t *desc = NULL;
//	upg_tune_t *tune = NULL;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}
//	tune = (upg_tune_t *)desc->tune;
	
	upgrade_tune_unsetsource(desc);
	upgrade_tune_unsetdest(desc);
	upgrade_tune_waitfinish(desc);
	upgrade_tune_end(desc);
	upgrade_tune_deinit(desc);
	upgrade_parse_destroyelem(desc);

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
	alislupg_desc_t *desc = NULL;

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
	alislupg_desc_t *desc = NULL;

	desc = (alislupg_desc_t *)(*handle);
	if (desc == NULL) 
	{
		SL_ERR("Try to open before construct!\n");
		return ALISLUPG_ERR_OTHER;
	}

	if (desc->writter)
	{
		free(desc->writter);
		desc->writter = NULL;
	}
	if (desc->tune)
	{
		free(desc->tune);
		desc->tune = NULL;
	}

	sem_destroy(&desc->oversem);
	pthread_cond_destroy(&desc->msg_cond);
	pthread_mutex_destroy(&desc->msg_mutex);

	if (desc->object)
	{
		free(desc->object);
		desc->object = NULL;
	}

	free(desc);
	*handle = NULL;

	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislupg_set_upgrade_parts
 *  @brief          specify the partitions need to upgrade
 *
 *	@param          handle : upgrade handler
 *  @param          parts_name : point to the buffer contain the partitions' name
 *  @param          parts_cnt : the number of partitions need to upgrade
 *
 *  @return         alisl_retcode
 *
 *  @author         terry.wu <terry.wu@alitech.com>
 *  @date           2014-9-23
 *
 */
alisl_retcode alislupg_set_upgrade_parts(alisl_handle handle, char parts_name[][MAX_PART_NAME_LEN], uint8_t parts_cnt)
{
	int i = 0;
	alislupg_desc_t * desc = NULL;
	upg_elem_t * elem = NULL;

	desc = (alislupg_desc_t *)handle;
	if ((desc == NULL) || (parts_name == NULL))
	{
		SL_ERR("Try to open before construct.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}
	elem = (upg_elem_t *)desc->elem;

	pthread_mutex_lock(&desc->msg_mutex);
	if (parts_cnt > 0)
	{
		for (i = 0; i < parts_cnt; i++)
		{
			strcpy(elem->part_name, parts_name[i]);
			elem = elem->next;
		}
	}
	pthread_mutex_unlock(&desc->msg_mutex);

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  alislupg_get_upgrade_parts
 *  @brief          check which partitions need to upgrade
 *
 *	@param          handle : upgrade handler
 *  @param          parts_name : point to the buffer contain the partitions' name
 *  @param          parts_cnt : point to the buffer contain the number of partitions need to upgrade
 *
 *  @return         alisl_retcode
 *
 *  @author         terry.wu <terry.wu@alitech.com>
 *  @date           2014-9-23
 *
 */
alisl_retcode alislupg_get_upgrade_parts(alisl_handle handle, char parts_name[][MAX_PART_NAME_LEN], uint8_t * parts_cnt)
{
	int i = 0;
	alislupg_desc_t * desc = NULL;
	upg_elem_t * elem = NULL;
	alisl_retcode ret = ALISLUPG_ERR_NONE;

	desc = (alislupg_desc_t *)handle;

	if ((desc == NULL) || (parts_name == NULL) || (parts_cnt == NULL))
	{
		SL_ERR("Try to open before construct.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}
	
	if (!desc->config_got)
	{
		if (ALISLUPG_ERR_NONE != (ret = upgrade_get_config(desc)))
		{
			SL_ERR("get config fail, ret:%d.\n", ret);
			return ret;
		}
		desc->config_got = true;
	}	
	
	elem = (upg_elem_t *)(desc->elem);

	pthread_mutex_lock(&desc->msg_mutex);
	if (desc->upg_part_num > 0)
	{
		for (i = 0; i < desc->upg_part_num; i++)
		{
			strcpy(parts_name[i], elem->part_name);
			elem = elem->next;
		}
		*parts_cnt = desc->upg_part_num;
	}
	else
	{
		*parts_cnt = 0;		
	}
	pthread_mutex_unlock(&desc->msg_mutex);
	
	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_get_upgrade_info
 *  @brief          get the upgrade package header
 *
 *	@param          handle : upgrade handler
 *  @param          info :   return the upgrade package header
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2015-1-27
 *
 */
alisl_retcode alislupg_get_upgrade_info(alisl_handle handle, alislupg_header_t* info)
{
//	int i = 0;
	alislupg_desc_t * desc = NULL;
//	upg_elem_t * elem = NULL;
	alisl_retcode ret = ALISLUPG_ERR_NONE;

	desc = (alislupg_desc_t *)handle;

	if ((desc == NULL) || (info == NULL))
	{
		SL_ERR("Invalid param.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}
	
	if (!desc->config_got)
	{
		if (ALISLUPG_ERR_NONE != (ret = upgrade_get_config(desc)))
		{
			SL_ERR("get config fail, ret:%d.\n", ret);
			return ret;
		}
		desc->config_got = true;
	}

	memcpy(info, &desc->package_header, sizeof(alislupg_header_t));
	
	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_set_ota_dl_param
 *  @brief          specify the ota download param. if ota upgrade, we should call this function before prestar
 *
 *	@param          handle : upgrade handler
 *  @param          ota_param : ota param(oui/ hw_model/ hw_version/ sw_model/ sw_version)
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2015-3-31
 *
 */
alisl_retcode alislupg_set_ota_dl_param(alisl_handle handle, alislupg_ota_dl_param_t ota_dl_param)
{
//	int i = 0;
	alislupg_desc_t * desc = NULL;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL)
	{
		SL_ERR("Try to open before construct.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	alislupg_ota_set_dl_param(ota_dl_param.oui, 
							ota_dl_param.hw_model, ota_dl_param.hw_version, 
							ota_dl_param.sw_model, ota_dl_param.sw_version);
	
	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  alislupg_set_net_ca_path
 *  @brief          set CA path from upper middleware. For https upgrade, we should call this function before prestar
 *
 *  @param          handle : upgrade handler
 *  @param          ca_path : CA path store upg certificate
 *
 *  @return         alisl_retcode
 *
 *  @author         xavier.chiang <xavier.chiang@alitech.com>
 *  @date           2015-10-08
 *
 */
alisl_retcode alislupg_set_net_ca_path(alisl_handle handle, char *ca_path)
{
	alislupg_desc_t * desc = NULL;

	desc = (alislupg_desc_t *)handle;
	if (desc == NULL)
	{
		SL_ERR("Try to open before construct.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	net_set_ca_path(ca_path);

	return ALISLUPG_ERR_NONE;
}

