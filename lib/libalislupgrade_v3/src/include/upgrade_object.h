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

#define UPG_CONFIGV1_SIZE      1024
/* Definition of UPG param */
#define UPG_MAX_CFGLEN         4096

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



/** Element definition to of packet organization */
typedef enum upg_endian {
	UPG_BIG_ENDIAN,
	UPG_LITTLE_ENDIAN,
} upg_endian_t;

/** Page size of the packet */
typedef enum upg_pgsz {
	UPG_PGSZ_2K = 2,
	UPG_PGSZ_4K = 4,
	UPG_PGSZ_8K = 8,
	UPG_PGSZ_16K = 16,
} upg_pgsz_t;

/** Configuration for config V2 */
typedef enum upg_elemscope {
	UPG_SCOPE_GLOBAL,
	UPG_SCOPE_LOCAL,
	UPG_SCOPE_TIMEWINDOW,
} upg_elemscope_t;

/** Destination flash type */
typedef enum upg_elemdest {
	UPG_DEST_NOR,
	UPG_DEST_NAND,
} upg_elemdest_t;

typedef enum upg_elemtype {
	UPG_TYPE_FILE,
	UPG_TYPE_IMAGE,
	UPG_TYPE_DIR,
} upg_elemtype_t;

/** Upgrade filesystem */
typedef enum upg_fstype {
	UPG_FS_EXT4,
	UPG_FS_YAFFS2,
	UPG_FS_CRYPTO_EXT4,
} upg_fstype_t;

typedef enum upg_destreserve {
	UPG_RESERVE_NONE,
	UPG_RESERVE_AREA,
	UPG_RESERVE_PART,
} upg_destreserve_t;

/** Each element attribute of upgrade */
typedef struct upg_elem {
	upg_elemscope_t        scope;
	upg_elemdest_t         dest;
	upg_elemtype_t         type;
	char                   dir[UPG_MAX_STRLEN * 4];
	int                    img_partno;
	upg_fstype_t           fs;
#if UPGRADE_BUILD_CRYPTO
	/** If signed, it'll be encrypted too */
	hwcrypto_signmethod_t  sign_method;
	hwcrypto_encmethod_t   encrypt_method;
#endif
	upg_destreserve_t      reserve;
	off_t                  reserve_start;
	off_t                  reserve_end;
	int                    reserve_partno;
	/** Size aligned to 128K of image size */
	size_t                 size; 
	char                   name[UPG_MAX_STRLEN];
	/** Upgrade time window for scheduled upgrade */
	char                   time_start[UPG_MAX_STRLEN];
	char                   time_end[UPG_MAX_STRLEN];
	/** Offset of item in upgrade image */
	off_t                  offset;
	/** Offset of the part in Nand -- used for MONO upgrade */
	off_t                  dstoffset;
	alislupg_elemlevel_t   level;
	char                   sn_begin[UPG_MAX_STRLEN];
	char                   sn_end[UPG_MAX_STRLEN];
	/** Parameter for config V1 & V2 */
	size_t                 partsz;
	size_t                 imgsz;
	int                    part_id;
	bool                   private_part;
	struct upg_elem        *next;
} upg_elem_t;

/** Header of upgrade packet */
typedef struct upg_header {
	upg_elem_t      *elem;
	upg_endian_t    host_endian;
	size_t          reqsz;
	/** Upgrade whole or not */
	bool            mono;
	size_t          upgdatasz;
	int             maxpart_id;
	/** Member for config V1 */
	unsigned long   sign;
	int             parts;
	/** Upgrade package size */
	size_t          pktsz;
	/** Upgrade partition number */
	int             upgparts;
	off_t           upgdata_offset;
	size_t          bootfile_size;
	size_t          bootfile_imgsz;
	size_t          ubootfile_imgsz;
	off_t           fskey_start;
	off_t           fskey_end;
	upg_pgsz_t      pagesz;
	/** Member for config V2 */
	char            pktname[UPG_MAX_STRLEN * 4];
} upg_header_t;

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
	func_setoffset  f_setupgdata_offset;
	func_getoffset  f_getupgdata_offset;
} upg_object_t;

/** Tune for writter */
typedef struct upg_tune {
	bool         writting;
	bool         unique;
	void         *source;
	void         *dest;
	size_t       size;
	size_t       total;
	sem_t        writersem;
	sem_t        sourcesem;
	/** Current element */
	upg_elem_t   *elem;
	/** Current offset in the element */
	off_t        curoffset;

	pthread_t    monitor;
} upg_tune_t;


#ifdef __cplusplus
}
#endif

#endif
