/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               internal.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:16:39
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#ifndef _UPGRADE_INTERNAL__H_
#define _UPGRADE_INTERNAL__H_

#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislupgrade.h>
#include <upgrade_object.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* The way to find the plugins */
#define UPG_PLUGIN_PREFIX        "libalislupgrade"

#define UPG_PLUGIN_USB           "usb"
#define UPG_PLUGIN_OTA           "ota"
#define UPG_PLUGIN_NET           "net"

#define UPG_PLUGIN_SUFFIX        ".so"

#define UPG_DEFAULT_SCALE        10

/* sflash param area related */
#define SFLASH_NAME                                 "sflash"
/* boot param area related */
#define BOOT_NAME                                   "boot"
#define BOOT_NAME_BAK                               "bootbak"
/* boot param without PMI */
#define SCS_HASH_LEVEL1_LEN                         0x130
#define SCS_SIG_LEN                                 0x100
#define SCS_PARAMS_AREA_START                       (SCS_HASH_LEVEL1_LEN + SCS_SIG_LEN)
#define SCS_TOTAL_AREA_LEN                          0x60
#define SCS_NAND_BOOT_FLASH_MEM_PARAMS_LEN          0x40
#define BOOT_PARAM_TOTAL_SIZE                       0x5d0
/* boot param with PMI */
#define TOTAL_PMI_NUM                               4
#define PAGES_OF_EACH_PMI                           256
#define PMI_DATA_SIZE                               (8*1024)

/** Each element attribute of upgrade */
typedef struct upg_elem {
	/** Part name */
	char             part_name[MAX_PART_NAME_LEN];
	/** Image size */
	size_t           img_size; 
	/** Image offset in the upg package */
	off_t            img_offset;
	/*  Next */
	struct upg_elem *next;
} upg_elem_t;

/** Upgrade descriptor */
typedef struct alislupg_desc {
	/** Supported upgrade source (NET/USB/OTA) */
	int    sources;
	/** Selected upgrade source (NET/USB/OTA)*/
	int    source;
	/** Selected upgrade method (OneShot/PieceByPiece) */
	int    method;
	void   *object;
	int    percent;
	char   config[UPG_MAX_STRLEN * 4];
	bool   config_got;
	upg_elem_t *elem;
	uint32_t    upg_part_num;
	/** Package header */
	alislupg_header_t package_header;
	/** Tune for whole process */
	void   *tune;
	/** For status notification */
	pthread_cond_t     msg_cond;
	pthread_mutex_t    msg_mutex;
	int                progress_scale;
	/** For error notification - Happen when error or finish */
	alislupg_err_t     error;
	/** Writter object */
	void   *writter;
	/** Semaphore for finish */
	sem_t  oversem;
} alislupg_desc_t;

/** Parameter for writter */
typedef struct upgrade_writter {
	pthread_t  tid;
	void       *buf;
} upgrade_writter_t;


/**
 *  Function Name:  upgrade_get_config
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
alisl_retcode upgrade_get_config(alislupg_desc_t *desc);

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
void upgrade_parse_destroyelem(alislupg_desc_t *desc);

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
alisl_retcode upgrade_parse_updatepmi(alislupg_desc_t *desc);

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
alisl_retcode upgrade_object_detect(alislupg_desc_t *desc, alislupg_source_t source);

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
alisl_retcode upgrade_object_init(alislupg_desc_t *desc);

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
alisl_retcode upgrade_object_deinit(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_init
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
alisl_retcode upgrade_tune_init(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_deinit
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
alisl_retcode upgrade_tune_deinit(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_waitfinish
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
alisl_retcode upgrade_tune_waitfinish(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_setsource
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
alisl_retcode upgrade_tune_setsource(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_setdest
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
alisl_retcode upgrade_tune_setdest(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_unsetsource
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
alisl_retcode upgrade_tune_unsetsource(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_unsetdest
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
alisl_retcode upgrade_tune_unsetdest(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_start
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
alisl_retcode upgrade_tune_start(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_tune_end
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
alisl_retcode upgrade_tune_end(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_writter_start
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
alisl_retcode upgrade_writter_start(alislupg_desc_t *desc);

/**
 *  Function Name:  upgrade_writter_stop
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
alisl_retcode upgrade_writter_stop(alislupg_desc_t *desc);

#ifdef __cplusplus
}
#endif

#endif
