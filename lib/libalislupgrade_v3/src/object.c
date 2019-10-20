/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               object.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:18:43
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

/* System header */
#include <dlfcn.h>

/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "internal.h"

/* Cast the plugin file name */
#define UPG_CAST_PLUGINDIR(path, source) \
	do { \
		strcpy(path, UPGRADE_PLUGINS_DIR"/"); \
		strcat(path, UPG_PLUGIN_PREFIX); \
		if (UPG_USB == source) \
			strcat(path, UPG_PLUGIN_USB); \
		else if (UPG_OTA == source) \
			strcat(path, UPG_PLUGIN_OTA); \
		else if (UPG_NET == source) \
			strcat(path, UPG_PLUGIN_NET); \
		strcat(path, UPG_PLUGIN_SUFFIX); \
	} while (0)


/**
 *  Function Name:  upgrade_object_detect
 *  @brief          
 *
 *  @param          desc    descriptor with upgrade parameters
 *  @param          source  upgrade source
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_object_detect(alislupg_desc_t *desc, alislupg_source_t source)
{
	char           path[UPG_MAX_STRLEN * 4];
	void           *handle = NULL;
	func_register  func = NULL;

	/* We already have a valid object */
	if ((0 != (desc->source & source)) && (NULL != desc->object))
	{
		return ALISLUPG_ERR_NONE;
	}
    
	UPG_CAST_PLUGINDIR(path, source);
    
	if (NULL == (handle = dlopen(path, RTLD_NOW))) 
	{
		SL_DBG("In %s Can't load plugin: %s\nBecause: %s\n", __FUNCTION__, path, dlerror());
		desc->sources &= ~(1 << source);
		return ALISLUPG_ERR_NOSUPPORTOBJ;
	}

#if 0 /* not to reg/unreg plugin */
	if (NULL == (func = dlsym(handle, UPG_PLUGIN_REGISTER))) 
	{
		SL_DBG("In %s Can't find register for\n%s\n", __FUNCTION__, path);
		desc->sources &= ~(1 << source);
		dlclose(handle);
		return ALISLUPG_ERR_NOPLUGINREGISTER;
	}

	if (ALISLUPG_ERR_NONE != func(desc)) 
	{
		SL_DBG("In %s Plugins register fail\n%s\n", __FUNCTION__, path);
		desc->sources &= ~(1 << source);
		dlclose(handle);
		return ALISLUPG_ERR_PLUGINREGERR;
	}

	if (NULL == (func = dlsym(handle, UPG_PLUGIN_UNREGISTER))) 
	{
		SL_DBG("In %s Can't find unregister for\n%s\n", __FUNCTION__, path);
	}

	if (ALISLUPG_ERR_NONE != func(desc)) 
	{
		SL_DBG("In %s Plugins unregister fail\n%s\n", __FUNCTION__, path);
	}
#endif	

	desc->sources |= (1 << source);
	dlclose(handle);
	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_object_init
 *  @brief          
 *
 *  @param          desc   descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_object_init(alislupg_desc_t *desc)
{
	char          path[UPG_MAX_STRLEN * 4];
	upg_object_t  *object = NULL;
	void          *handle = NULL;
	func_register f_register = NULL;

	if (0 == desc->sources & (1 << desc->source))
	{
		return ALISLUPG_ERR_NOSUPPORTOBJ;
	}

	/* Initialize plugin elements */
	UPG_CAST_PLUGINDIR(path, desc->source);
	handle = dlopen(path, RTLD_LAZY);
	f_register = dlsym(handle, UPG_PLUGIN_REGISTER);
    
	/* Register the object */
	f_register(desc);

	object = (upg_object_t *)desc->object;
	object->handle = handle;
	object->f_register = f_register;
	object->f_unregister = dlsym(handle, UPG_PLUGIN_UNREGISTER);

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_object_deinit
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
alisl_retcode upgrade_object_deinit(alislupg_desc_t *desc)
{
	upg_object_t  *object = (upg_object_t *)desc->object;

	object->f_unregister(desc);
	dlclose(object->handle);
	object->handle = NULL;

	return ALISLUPG_ERR_NONE;
}
