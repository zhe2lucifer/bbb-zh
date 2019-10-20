/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               info.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:08:24
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* Upgrade header */
#include <alislupgrade.h>
#include <alislupgrade_status.h>
#include <upgrade_object.h>

/* share library headers */
#include <alipltfretcode.h>

#include "../internal.h"


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
alisl_retcode alislupg_upginfo_read(unsigned char *buf, size_t size)
{
	alislsysinfo_upginfo_read(buf, size);
	sleep(1);

	return ALISLUPG_STATUS_ERR_NONE;
}

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
alisl_retcode alislupg_upginfo_write(unsigned char *buf, size_t size)
{
	alislsysinfo_upginfo_write(buf, size);
	sleep(1);

	return ALISLUPG_STATUS_ERR_NONE;
}

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
alisl_retcode alislupg_extinfo_read(unsigned char *buf, size_t size)
{
	alislsysinfo_extinfo_read(buf, size);
	sleep(1);

	return ALISLUPG_STATUS_ERR_NONE;
}

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
alisl_retcode alislupg_extinfo_write(unsigned char *buf, size_t size)
{
	alislsysinfo_extinfo_write(buf, size);
	sleep(1);

	return ALISLUPG_STATUS_ERR_NONE;
}


