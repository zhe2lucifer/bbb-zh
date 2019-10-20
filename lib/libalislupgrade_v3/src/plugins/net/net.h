/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               net.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:10:42
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#ifndef __UPGRADE_NET_OBJECT__H_
#define __UPGRADE_NET_OBJECT__H_

#include <curl/curl.h>

#include <upgrade_object.h>

/* Plugin name */
#define UPG_OBJ_NAME    "com.upgrade.standard.net"

/* Buffer size for each read */
#define UPG_BUF_SIZE    (512 * 1024)

/* NET Upgrade object definition */
typedef struct netupg_object {
	char           *name;
	char           config[UPG_MAX_STRLEN * 4];
	char           pktname[UPG_MAX_STRLEN * 4];
	unsigned char  *buf;   /* Data Buffer */
	pthread_t      source; /* Thread of source */
	off_t          upgdata_offset;
	char           *url;   /* URL of data */
} netupg_object_t;

#endif
