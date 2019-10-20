/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislupgrade.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:01:51
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#ifndef __UPGRADE_INTERFACE__H_
#define __UPGRADE_INTERFACE__H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <semaphore.h>
#include <pthread.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alislstorage.h>
#include <alislsysinfo.h>
#if UPGRADE_BUILD_CRYPTO
#include <alislhwcrypto.h>
#endif
#include <alislupgrade_status.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * define BUILD macro here right now, need modify later
 */
#define UPGRADE_BUILD_USB         1
#define UPGRADE_BUILD_NET         1
#define UPGRADE_BUILD_OTA         1
#define UPGRADE_BUILD_INFO        1
#define UPGRADE_BUILD_CRYPTO      0
#define UPGRADE_BUILD_CONFIGV2    0
#define UPGRADE_BUILD_STEPSTATUS  0

/** upgrade configuration directory */
#define UPGRADE_CONFIG_DIR "/usr/lib/upgrade/conf"
/** upgrade plugins directory */
#define UPGRADE_PLUGINS_DIR "/usr/lib"

#define UPG_MAX_STRLEN            64

/** Configuration parser choice, only support V1 now */
typedef enum alislupg_config {
	UPG_CFG_V1,
	UPG_CFG_V2,
} alislupg_config_t;

/** Error handler of library */
typedef enum alislupg_err {
	ALISLUPG_ERR_NONE,
	ALISLUPG_ERR_NOSUPPORTOBJ,
	ALISLUPG_ERR_NOPLUGINREGISTER,
	ALISLUPG_ERR_PLUGINREGERR,
	ALISLUPG_ERR_PARSECFGERR,
	ALISLUPG_ERR_CANTGETCFG,
	ALISLUPG_ERR_TOOMANYPARTS,
	ALISLUPG_ERR_NOTENOUGHSPACE,
	ALISLUPG_ERR_MISMATCHPGSZ,
	ALISLUPG_ERR_VERIFYFAIL,
	ALISLUPG_ERR_OTHER,
} alislupg_err_t;

/** From where the data to be got */
typedef enum alislupg_source {
	UPG_OTA,
	UPG_USB,
	UPG_NET,
} alislupg_source_t;

/** Which way to overwrite the storage */
typedef enum alislupg_method {
	UPG_BYFILE,
	UPG_BYPART,
} alislupg_method_t;

/** Parameter to initialization the upgrade descriptor */
typedef struct alislupg_param {
	/** Selected upgrade source */
	int    source : 3;
	/** Selected upgrade method */
	int    method : 3;
	/** Net upgrade used */
	char   *url;
	bool   useproxy;
	char   *proxy;
	/**
	 * If the configuration file is available
	 * Used by USB and network upgrade.
	 * For USB and network, the config will
	 * be the file name of configuration file
	 * in version 2, and will be the package name
	 * in version 1.
	 */
	char   *config;
	/** Configuration version */
	alislupg_config_t   cfgver;
	int     progress_scale;
} alislupg_param_t;


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
alisl_retcode alislupg_construct(alisl_handle *handle);

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
alisl_retcode alislupg_open(alisl_handle handle, alislupg_param_t* param);

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
alisl_retcode alislupg_prestart(alisl_handle handle);

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
alisl_retcode alislupg_start(alisl_handle handle);

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
alisl_retcode alislupg_wait_status(alisl_handle handle, int *p_percent, int *p_error);


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
alisl_retcode alislupg_abort(alisl_handle handle);

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
alisl_retcode alislupg_stop(alisl_handle handle);

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
alisl_retcode alislupg_close(alisl_handle handle);

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
alisl_retcode alislupg_destruct(alisl_handle *handle);

#ifdef __cplusplus
}
#endif

#endif
