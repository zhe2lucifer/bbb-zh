/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldb_sys.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/25/2013 02:35:41 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */
#ifndef __ALISLDB_SYS_H__
#define __ALISLDB_SYS_H__

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <alisldb.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define alisldb_get_sys(_type, elem, t) ({ \
	struct alisldb_dbml dbml; \
	char sql[256]; \
 \
	dbml.syntax = _type##_syntax; \
	dbml.data = &(t); \
 \
	memset(sql, 0, sizeof(sql)); \
	snprintf(sql, sizeof(sql), "select %s from %s", #elem, #_type); \
 \
	alisldb_get_value("sys_db", sql, &dbml, NULL, NULL); \
})

#define alisldb_set_sys(_type, elem, t) ({ \
	struct alisldb_dbml dbml; \
	char sql[256]; \
 \
	if (strcmp("*", #elem) == 0) { \
		dbml.syntax = _type##_syntax; \
	} else { \
		struct alisldb_dbmlsyntax mysyntax[] = { \
			{ #elem, DBML_NONE, 1, 0 }, \
			{ 0 } \
		}; \
		alisldb_dbmlsyntax_t *syntax; \
		for (syntax = _type##_syntax; \
		     syntax != NULL && syntax->fieldname != NULL; \
		     syntax++) { \
			if (!strcmp(syntax->fieldname, #elem)) { \
				mysyntax[0].type = syntax->type; \
				mysyntax[0].nb_elem = syntax->nb_elem; \
				mysyntax[0].data_offset = syntax->data_offset; \
				break; \
			} \
		} \
 \
		dbml.syntax = mysyntax; \
	} \
 \
	dbml.data = &(t); \
 \
	memset(sql, 0, sizeof(sql)); \
	snprintf(sql, sizeof(sql), "update %s", #_type); \
	alisldb_set_value("sys_db", sql, &dbml); \
})

typedef struct audio_output {
	uint8_t         volumn;
	uint8_t         default_volumn;
	bool            install_beep;
	bool            is_ber_printf_on;
	bool            is_spdif_dolby_on;
	bool            is_mute_state;
} audio_output_t;
extern alisldb_dbmlsyntax_t audio_output_syntax[];

typedef struct audio_description {
	bool            is_default;
	bool            is_service_enabled;
	bool            is_mode_on;
	uint8_t         volumn;
	bool            is_default_mode_on;
	bool            use_as_default;
} audio_description_t;
extern alisldb_dbmlsyntax_t audio_description_syntax[];

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
uint8_t alisldb_get_sys_volumn(void);

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
uint8_t alisldb_get_sys_default_volumn(void);

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
alisl_retcode alisldb_set_sys_volumn(volumn);

#ifdef __cplusplus
}
#endif

#endif /* __ALISLDB_SYS_H__ */
