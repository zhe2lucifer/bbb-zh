/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               upgrade_object.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:08:05
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#ifndef _UPGRADE_OBJECT__H_
#define _UPGRADE_OBJECT__H_

/* share library headers */
#include <alipltfretcode.h>
#include <alislupgrade.h>
#include "../internal.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Standard way of plugins' name definition */
#define UPG_PLUGIN_USBOBJ      "com.upgrade.standard.usb"
#define UPG_PLUGIN_OTAOBJ      "com.upgrade.standard.ota"
#define UPG_PLUGIN_NETOBJ      "com.upgrade.standard.net"

/* Symbol in the plugin for register / unregister */
#define UPG_PLUGIN_REGISTER    "upgrade_obj_register"
#define UPG_PLUGIN_UNREGISTER  "upgrade_obj_unregister"

#define DATA_ALIGN(size, boundary) \
    (((size) + ((boundary) - 1)) & ~((boundary) - 1))

/* Align the size to 128K */
#if 0
#define UPG_ALIGN_128K(size) \
	({ \
		size_t alignsize = 0; \
		alignsize = (size) + (128 * 1024 - ((size) % (128 * 1024))); \
		alignsize; \
	})
#else
#define UPG_ALIGN_128K(size) \
	(DATA_ALIGN(size, 128*1024))
#define UPG_ALIGN_4K(size) \
	(DATA_ALIGN(size, 4*1024))
#endif

/* Function type in the plugin */
typedef alisl_retcode (*func_register)(alislupg_desc_t *);
/** Function pointer for getting upgrade configuration */
typedef size_t (*func_getcfg)(alislupg_desc_t *, unsigned char *, size_t);
typedef alisl_retcode (*func_operation)(alislupg_desc_t *);
typedef alisl_retcode (*func_setname)(alislupg_desc_t *, char *);
typedef alisl_retcode (*func_setoffset)(alislupg_desc_t *, off_t);
typedef alisl_retcode (*func_getoffset)(alislupg_desc_t *);

/** Common upgrade object definition */
typedef struct upg_object {
	void            *handle;
	void            *plugin;
	func_register   f_register;
	func_register   f_unregister;
	func_getcfg     f_getcfg;
	func_operation  f_prestart;
	func_operation  f_prestop;
	func_operation  f_start;
	func_operation  f_stop;
	func_setname    f_setpktname;
} upg_object_t;

/** Tune for writter */
typedef struct upg_tune {
	/* buffer for storing data in tune between reader and writer*/
	void         *buf;
	/* buffer size in tune */
	size_t       size;
	/* total size of data need to be upgraded */
	size_t       total;
	/* sems */
	sem_t        writersem;
	sem_t        sourcesem;
	/** Current element */
	upg_elem_t   *elem;
} upg_tune_t;


#ifdef __cplusplus
}
#endif

#endif
