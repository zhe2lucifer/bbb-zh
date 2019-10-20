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

#ifdef __cplusplus
extern "C"
{
#endif

/** upgrade plugins directory */
#define UPGRADE_PLUGINS_DIR "/usr/lib"

#define UPG_MAX_STRLEN            64


/**************************************************
 * Share with package tool 
 *************************************************/
#define MAX_PART_NAME_LEN 24
#define MAX_PART          64 
#define OTA_TS_MODULE_ZISE       (0x100000) //spec fix
#define OTA_TS_MODULE_VALIE_ZISE (0xFE200)  //spec fix
#define OTA_DOWNLOAD_BUFFER_SIZE (10*OTA_TS_MODULE_ZISE) /* should be mulriple of OTA_TS_MODULE_ZISE */
	
typedef struct alislupg_header_imageinfo {
    char image_name[MAX_PART_NAME_LEN];
    int  image_offset;
    int  image_size;
} alislupg_header_imageinfo_t;

typedef struct alislupg_header {
    int  upg_ver;
    /** package version, including all groups version.
      * the store method deside by user, such as:
      * package_ver = (bootver<<16 | upgfwver<<8 | mainfwver<<0)
      */
    int  package_ver; 
    /** nor+nand part count */
    int  part_cnt;    
	/* ota_down_buf_size is the buffer size(M) for stb temply stroes the ota ts data in the RAM
	 * The size is desided by the user
	 */
    int  ota_down_buf_size;
    int  reserve2;
    int  reserve3;
    int  reserve4;
    alislupg_header_imageinfo_t image_info[MAX_PART];
} alislupg_header_t;
/**************************************************
 * Share with package tool end
 *************************************************/
 

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
	ALISLUPG_ERR_INVALIDPARAM,
	ALISLUPG_ERR_FILEOPERATEFAIL,
	ALISLUPG_ERR_DOWNLOADFAIL,
	ALISLUPG_ERR_INVALIDPACKAGE,
	ALISLUPG_ERR_OTHER,
} alislupg_err_t;

/** From where the data to be got */
typedef enum alislupg_source {
	UPG_OTA,
	UPG_USB,
	UPG_NET,
} alislupg_source_t;

typedef enum alislupg_method {
	PIECE_BY_PIECE,
	ONE_SHOT,
} alislupg_method_t;

/** Parameter to initialization the upgrade descriptor */
typedef struct alislupg_param {
	/** Selected upgrade source (NET/USB/OTA) */
	int    source;
	/** Selected upgrade method (OneShot/PieceByPiece) */
	int    method;
 	/** ConfigFile */
	char  *config;
	/** Progress scale */
	int    progress_scale;
} alislupg_param_t;

typedef struct alislupg_ota_dl_param {
	unsigned long  oui;
	unsigned short hw_model;
	unsigned short hw_version;
	unsigned short sw_model;
	unsigned short sw_version;
} alislupg_ota_dl_param_t;

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
 *  @brief          just for OneShot, pre-download all the data to DRAM before burn,  
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
 *  @brief          
 *                  OneShot:
 *                    get data from DRAM which pre-downloaded by alislupg_prestart and burn
 *                  PieceByPiece:
 *                    get data from source(NET/USB/OTA) and burn
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
alisl_retcode alislupg_prestop(alisl_handle handle);

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
alisl_retcode alislupg_get_upgrade_parts(alisl_handle handle, char parts_name[][MAX_PART_NAME_LEN], uint8_t * parts_cnt);

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
alisl_retcode alislupg_set_upgrade_parts(alisl_handle handle, char parts_name[][MAX_PART_NAME_LEN], uint8_t parts_cnt);

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
alisl_retcode alislupg_get_upgrade_info(alisl_handle handle, alislupg_header_t* info);

/**
 *  Function Name:  alislupg_set_ota_dl_param
 *  @brief          specify the ota download param
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
alisl_retcode alislupg_set_ota_dl_param(alisl_handle handle, alislupg_ota_dl_param_t ota_dl_param);


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
alisl_retcode alislupg_set_net_ca_path(alisl_handle handle, char *ca_path);


#ifdef __cplusplus
}
#endif

#endif
