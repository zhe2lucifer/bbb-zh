/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               usb.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:14:55
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#ifndef __UPGRADE_USB_OBJECT__H_
#define __UPGRADE_USB_OBJECT__H_

#include <upgrade_object.h>

/** Plugin name */
#define UPG_OBJ_NAME    "com.upgrade.standard.usb"

/** Buffer size for each read */
#define UPG_BUF_SIZE    (512 * 1024)

/** USB Upgrade object definition */
typedef struct usbupg_object {
	char            *name;
	char            config[UPG_MAX_STRLEN * 4];
	char            pktname[UPG_MAX_STRLEN * 4];
	/** Data Buffer */
	unsigned char   *buf;
	/** Thread of source */
	pthread_t       source;
	/** offset of update file */
	off_t           upgdata_offset;
} usbupg_object_t;

#endif
