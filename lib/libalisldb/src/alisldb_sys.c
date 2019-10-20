/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldb_sys.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/25/2013 11:17:32 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
 
/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alisldb.h>
#include <alisldb_sys.h>

/* local headers */
#include "internal.h"
#include "debug.h"
#include "error.h"

alisldb_dbmlsyntax_t audio_output_syntax[] = {
	{ "volumn",            DBML_UINT8, 1, offsetof(audio_output_t, volumn) },
	{ "default_volumn",    DBML_UINT8, 1, offsetof(audio_output_t, default_volumn) },
	{ "install_beep",      DBML_BOOL,  1, offsetof(audio_output_t, install_beep) },
	{ "is_ber_printf_on",  DBML_BOOL,  1, offsetof(audio_output_t, is_ber_printf_on) },
	{ "is_spdif_dolby_on", DBML_BOOL,  1, offsetof(audio_output_t, is_spdif_dolby_on) },
	{ "is_mute_state",     DBML_BOOL,  1, offsetof(audio_output_t, is_mute_state) },
	{ 0 }
};

alisldb_dbmlsyntax_t audio_description_syntax[] = {
	{ "is_default",         DBML_BOOL,  1, offsetof(audio_description_t, is_default) },
	{ "is_service_enabled", DBML_BOOL,  1, offsetof(audio_description_t, is_service_enabled) },
	{ "is_mode_on",         DBML_BOOL,  1, offsetof(audio_description_t, is_mode_on) },
	{ "volumn",             DBML_UINT8, 1, offsetof(audio_description_t, volumn) },
	{ "is_default_mode_on", DBML_BOOL,  1, offsetof(audio_description_t, is_default_mode_on) },
	{ "use_as_default",     DBML_BOOL,  1, offsetof(audio_description_t, use_as_default) },
	{ 0 }
};

/**
 *  Function Name:      alisldb_get_sys_volumn
 *  @brief              get system volumn
 *
 *  @param void
 *
 *  @return             uint8_t volumn
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/25/2013, Created
 *
 *  @note
 */
uint8_t alisldb_get_sys_volumn(void)
{
	struct audio_output ao;

	alisldb_get_sys(audio_output, volumn, ao);

	return ao.volumn;
}

/**
 *  Function Name:      alisldb_get_sys_default_volumn
 *  @brief              get system default volumn
 *
 *  @param void
 *
 *  @return             uint8_t default_volumn
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/25/2013, Created
 *
 *  @note
 */
uint8_t alisldb_get_sys_default_volumn(void)
{
	struct audio_output ao;

	alisldb_get_sys(audio_output, default_volumn, ao);

	return ao.default_volumn;
}

/**
 *  Function Name:      alisldb_set_sys_volumn
 *  @brief              set system volumn
 *
 *  @param volumn       value of volumn to be set
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/25/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_set_sys_volumn(volumn)
{
	struct audio_output ao;

	ao.volumn = volumn;

	return alisldb_set_sys(audio_output, volumn, ao);
}
