/**@file
 *  (c) Copyright 2013-2063  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislsoc.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/06/2013 10:13:40 AM
 *  @revision           none
 *
 *  @author             Alan Zhang<alan.zhang@alitech.com>
 */

#ifndef __ALISLSOC_INTERFACE__H_
#define __ALISLSOC_INTERFACE__H_
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum alislsoc_status {
	ALISLSOC_INACTIVATE,
	ALISLSOC_ACTIVATE
} alislsoc_status_t;

typedef enum alislsoc_cmd {
	ALISLSOC_CMD_READ, 
	ALISLSOC_CMD_WRITE,
	ALISLSOC_CMD_CHIPID,
	ALISLSOC_CMD_PRODUCTID,
	ALISLSOC_CMD_GETBONDING,
	ALISLSOC_CMD_REVID,
	ALISLSOC_CMD_CPUCLK,
	ALISLSOC_CMD_DRAMCLK,
	ALISLSOC_CMD_HDENABLE,
	ALISLSOC_CMD_C3603PDC,  /* C3603 product */
	ALISLSOC_CMD_USBNUM,
	ALISLSOC_CMD_USBPORT,   /* USB port enable */
	ALISLSOC_CMD_NIM3501,   /* support M3501 NIM */
	ALISLSOC_CMD_NIM,       /* NIM support */
	ALISLSOC_CMD_CINUM,     /* get CI num */
	ALISLSOC_CMD_MACNUM,    /* get MAC num */
	ALISLSOC_CMD_TUNERNUM,  /* get tuner num */
	ALISLSOC_CMD_ISHDENABLE,/* is HD enable */
	ALISLSOC_CMD_SATAENABLE,  /* SATA enable */
	ALISLSOC_CMD_SCRAMENABLE,
	ALISLSOC_CMD_SECUENABLE,
	ALISLSOC_CMD_SPLENABLE,
	ALISLSOC_CMD_UARTENABLE,
	ALISLSOC_CMD_ETJTENABLE,
	ALISLSOC_CMD_MGENABLE, 
	ALISLSOC_CMD_AC3ENABLE,
	ALISLSOC_CMD_DDPENABLE,
	ALISLSOC_CMD_XDENABLE,
	ALISLSOC_CMD_XDPENABLE,
	ALISLSOC_CMD_AACENABLE,
	ALISLSOC_CMD_H264ENABLE,
	ALISLSOC_CMD_MP4ENABLE,
	ALISLSOC_CMD_MS10ENABLE,
	ALISLSOC_CMD_MS11ENABLE,
	ALISLSOC_CMD_RMVBENABLE,
	ALISLSOC_CMD_VC1ENABLE,
	ALISLSOC_CMD_AVSENABLE,
	ALISLSOC_CMD_VP8ENABLE,
	ALISLSOC_CMD_FLVENABLE,
	ALISLSOC_CMD_RBGT,          /* reboot get timer */
	ALISLSOC_CMD_ETSTANDBY,     /* enter standby */
	ALISLSOC_CMD_REBOOT,
	ALISLSOC_CMD_DACD,          /* dsc access ce is disable */
	ALISLSOC_CMD_NULL,
} alislsoc_cmd_t;

typedef enum alislsoc_io_param {
	ALISLSOC_PARAM_ENABLE,
	ALISLSOC_PARAM_DISABLE
} alislsoc_io_param_t;

typedef struct alislsoc_param {
	unsigned char status;
	unsigned char rev;
} alislsoc_param_t;

/**
 *  Function Name:      alislsoc_op_read
 *  @brief              read soc data
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_read(unsigned char *dst, unsigned char *buf, int len);

/**
 *  Function Name:      alislsoc_op_write
 *  @brief              write soc data
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_write(unsigned char *dst, unsigned char *buf, int len);

/**
 *  Function Name:      alislsoc_op_reboot
 *  @brief              hardware watchdog reboot system
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_reboot();


/**
 *  Function Name:      alislsoc_op_ioctl
 *  @brief              uniform interface
 *
 *  @para1            alislsoc_cmd
 *  @para2            para
 *                        when alislsoc_cmd is ALISLSOC_CMD_READ ALISLSOC_CMD_WRITE
 *				ALISLSOC_CMD_USBPORT ALISLSOC_CMD_RBGT  ALISLSOC_CMD_ETSTANDBY
 *				you need input relevant struct address eg: alislsoc_cmd is ALISLSOC_CMD_READ,
 *				you need input a  soc_op_paras stuct address,when para is others alislsoc_cmd,
 *				you need input a unsigned int value address for receive the return parameter
 *				
 *
 *  @return             alisl_retcode 0:right
 *
 *  @author             Vedic Fu <vedic.fu@alitech.com>
 *  @date               05/15/2014, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_ioctl(alislsoc_cmd_t cmd,void *para);

#ifdef __cplusplus
}
#endif

#endif
