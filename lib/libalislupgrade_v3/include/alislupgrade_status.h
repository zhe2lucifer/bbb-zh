/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislupgrade_status.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:02:54
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#ifndef __UPGRADE_STATUS__H_
#define __UPGRADE_STATUS__H_

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** upgrade level */
typedef enum alislupg_elemlevel {
	UPG_LEVEL_NONE,
	UPG_LEVEL_BOOT,
	UPG_LEVEL_LOWLEVEL,
	UPG_LEVEL_APPLICATION,
} alislupg_elemlevel_t;

/** Status to be updated */
typedef enum alislupg_status_runvalue {
	ALISLUPG_RUN_ENTER = 0x00,
	ALISLUPG_RUN_START = 0x01,
	ALISLUPG_RUN_OVER  = 0x02,
	ALISLUPG_RUN_NONE  = 0x03,
	ALISLUPG_RUN_MAX   = 0x06,
} alislupg_status_runvalue_t;

/** Upgrade value */
typedef enum alislupg_status_upgvalue {
	ALISLUPG_UPG_NO    = 0x00,
	ALISLUPG_UPG_START = 0x01,
	ALISLUPG_UPG_OVER  = 0x02,
	ALISLUPG_UPG_MAX   = 0x06,
	ALISLUPG_UPG_NONE  = 0x07,
} alislupg_status_upgvalue_t;

/** Upgrade stage */
typedef enum alislupg_status_stage {
	ALISLUPG_STAGE_UPGRADE,
	ALISLUPG_STAGE_RUN,
} alislupg_status_stage_t;

/** Upgrade indicator */
typedef struct alislupg_status_param {
	alislupg_status_runvalue_t  runvalue;
	alislupg_status_stage_t     stage;
	alislupg_elemlevel_t        level;
	alislupg_status_upgvalue_t  upgvalue;
} alislupg_status_param_t;

/** Error handler */
typedef enum alislupg_status_err {
	ALISLUPG_STATUS_ERR_NONE,
} alislupg_status_err_t;

/**
 *  Function Name:  alislupg_upginfo_read
 *  @brief          
 *
 *  @param          buf  upg status info buffer
 *  @param          size upg status info buffer size
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode alislupg_upginfo_read(unsigned char *buf, size_t size);


/**
 *  Function Name:  alislupg_upginfo_write
 *  @brief          
 *
 *  @param          buf  upg status info buffer
 *  @param          size upg status info buffer size
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode alislupg_upginfo_write(unsigned char *buf, size_t size);


/**
 *  Function Name:  alislupg_extinfo_read
 *  @brief          
 *
 *  @param          buf  upg ext info buffer
 *  @param          size upg ext info buffer size
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode alislupg_extinfo_read(unsigned char *buf, size_t size);

/**
 *  Function Name:  alislupg_upginfo_write
 *  @brief          
 *
 *  @param          buf  upg ext info buffer
 *  @param          size upg ext info buffer size
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode alislupg_extinfo_write(unsigned char *buf, size_t size);


#ifdef __cplusplus
}
#endif

#endif
