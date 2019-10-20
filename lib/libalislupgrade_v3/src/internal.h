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

/* Tag definition */
#define UPG_TAG_SEPARATOR        ":"
#define UPG_TAG_SCOPEGLOBAL      "g"
#define UPG_TAG_SCOPELOCAL       "l"
#define UPG_TAG_SCOPETIMEWIN     "t"
#define UPG_TAG_NOTAVAILABLE     "no"
#define UPG_TAG_NOR              "nor"
#define UPG_TAG_NAND             "nand"
#define UPG_TAG_FILE             "file"
#define UPG_TAG_IMAGE            "img"
#define UPG_TAG_DIR              "dir"
#define UPG_TAG_SIGNMETHOD1      "m1"
#define UPG_TAG_SIGNMETHOD2      "m2"
#define UPG_TAG_MONO             "whole"
#define UPG_TAG_RESERVEAREA      "ra"
#define UPG_TAG_RESERVEPART      "rp"
#define UPG_TAG_APPLEVEL         "app"
#define UPG_TAG_LOWLEVEL         "lowlevel"
#define UPG_TAG_BOOTLEVEL        "boot"
#define UPG_TAG_BIGENDIAN        "big"
#define UPG_TAG_LITTLEENDIAN     "little"
#define UPG_TAG_UPGPKGNAME       "name"
#define UPG_TAG_UPGDATAOFFSET    "offset"

/* Parameters' position for CONFIG V1 */
#define UPG_OFFSET_SIGN          12
#define UPG_OFFSET_UBOOT_LEN     196
#define UPG_OFFSET_PARTS         256
#define UPG_OFFSET_PARTSTART     260
#define UPG_OFFSET_IMGLEN        420
#define UPG_OFFSET_PKTSIZE       512
#define UPG_OFFSET_UPGPARTS      516
#define UPG_OFFSET_UPGDATAOFFSET 520
#define UPG_OFFSET_BOOTSIZE      524
#define UPG_OFFSET_ALIGNEDATTR   528
#define UPG_OFFSET_KEYSTART      688
#define UPG_OFFSET_KEYEND        692
#define UPG_OFFSET_PKTPAGESZ     696

#define UPG_MAX_PARTS            20

#define UPG_DEFAULT_SCALE        10

#define UPG_UPGLOADER_PARTS      2

#define DEFAULT_BOOTFILEDATA_START  1024

/** Easy usage */
typedef struct pmi_info_user pmi_info_t;

/** Upgrade descriptor */
typedef struct alislupg_desc {
	/** Supported upgrade source */
	int    sources : 3;
	/** Supported upgrade method */
	int    methods : 2;
	/** Selected upgrade source */
	int    source : 3;
	/** Selected upgrade method */
	int    method : 3;
	void   *object;
	int    percent;
	void   *header;
	alislupg_config_t  cfgver;
	char   config[UPG_MAX_STRLEN * 4];
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
	/** Operation to storage */
	void     *nand;
	void     *nor;
} alislupg_desc_t;

/** Parameter for writter */
typedef struct upgrade_writter {
	/** Thread writter */
	pthread_t  writter;
	void       *buf;
} upgrade_writter_t;

/** Mapping from partition to level */
typedef struct upgrade_maplevel {
	alislsto_idx_t        idx;
	alislupg_elemlevel_t  level;
} upgrade_maplevel_t;

/**
 * Upgrade status store in flash
 * This ugly structure is from an old usage
 * It'll be optimized later, maybe
 * It's defined and use by APP
 */
typedef struct upgstat
{
	unsigned int boot_status;
	unsigned int lowlevel_status;
	unsigned int application_status;
	unsigned int bootloader_upgrade;
	unsigned int lowlevel_upgrade;
	unsigned int application_upgrade;

	unsigned int bootloader_run_cnt;
	unsigned int lowlevel_run_cnt;
	unsigned int application_run_cnt;

	unsigned int reserved1;
	unsigned int reserved2;

	unsigned int need_upgrade;
	unsigned int backup_exist;
	unsigned int lowlevel_backup_exist;
	unsigned int boot_backup_exist;
	unsigned int nor_upgrade;
	unsigned int nor_reserved;
	unsigned int nor_reserved_upgrade;
	unsigned int nand_reserved;
	unsigned int nand_reserved_upgrade;
	unsigned int nand_mono_upgrade;
	unsigned int cur_uboot;
	unsigned int reserved[4];
} upgstat_t;

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
alisl_retcode upgrade_parse_config(alislupg_desc_t *desc);

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
 *  Function Name:  upgrade_parse_setstart
 *  @brief          
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          level  upgrade element level
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_parse_setstart(alislupg_desc_t *desc, alislupg_elemlevel_t level);

/**
 *  Function Name:  upgrade_parse_setover
 *  @brief          
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          level  upgrade element level
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_parse_setover(alislupg_desc_t *desc, alislupg_elemlevel_t level);

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
