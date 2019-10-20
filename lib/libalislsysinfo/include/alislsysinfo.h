/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsysinfo.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 14:49:22
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#ifndef __SYSINFO_INTERFACE__H_
#define __SYSINFO_INTERFACE__H_

#include <stdio.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <mtd/mtd-user.h>


/* share library headers */
#include <alipltfretcode.h>
#include <alislstorage.h>


#ifdef __cplusplus
extern "C"
{
#endif

#define SYSINFO_SIZE_MAX        4096

/* Interface error handler */
typedef enum alislsysinfo_err {
	SYSINFO_ERR_NONE,
	SYSINFO_ERR_INVALIDAREA,
	SYSINFO_ERR_NULLBUF,
	SYSINFO_ERR_CANTSEEKSYSINFO,
	SYSINFO_ERR_CANTOPENDEV,
} alislsysinfo_err_t;

/**
 *  Function Name:  alislsysinfo_mac_read
 *  @brief          
 *
 *  @param          buf  mac buffer
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_mac_read(unsigned char *buf);

/**
 *  Function Name:  alislsysinfo_mac_write
 *  @brief          
 *
 *  @param          buf  mac buffer
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_mac_write(unsigned char *buf);

/**
 *  Function Name:  alislsysinfo_upginfo_read
 *  @brief          
 *
 *  @param          buf   upginfo buffer
 *  @param          size  read size
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_upginfo_read(unsigned char *buf, size_t size);


/**
 *  Function Name:  alislsysinfo_upginfo_write
 *  @brief          
 *
 *  @param          buf   write buffer
 *  @param          size  write size
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsysinfo_upginfo_write(unsigned char *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
