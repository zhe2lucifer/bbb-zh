/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislfake.c
 *  @brief          Implementation of FAKE share lib 
 *
 *  @version        1.0
 *  @date           05/31/2013 06:18:16 PM
 *  @revision       none
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fcntl.h>

/* Fake driver header */
#include <ali_fake_common.h>

/* Platformlog err header */
#include <alipltfretcode.h>

/* Platformlog header */
#include <alipltflog.h>

/* Fake header */
#include <alislfake.h>

/* Internal header */
#include "internal.h"

/* Interface of fake driver cmd */
static unsigned int fake_driver_cmd_table[] = {
	FAKE_TRACE_GET_MM_ADDR,
	FAKE_TRACE_SHOW_MEMORY,
	FAKE_TRACE_GET_TICK,
	FAKE_TRACE_SHOW_STACK_ALL,
	FAKE_TRACE_SHOW_STACK_PID,
};

/**
 *  Function Name:  alislfake_construct
 *  @brief          prepare need param for dev
 *
 *  @param          dev		descriptor with FAKE attribute
 *  @param          param   init dev 
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_construct(alislfake_dev_t *dev, 
		alislfake_param_t *param)
{ 
	if (NULL == param) {
		SL_ERR("Param is NULL!\n");
		return ALISLFAKE_ERR_INPUT_PARAM;
	}

	dev->status = param->status;
	if (ALISLFAKE_DEV_INACTIVATE == dev->status) {
		SL_ERR("DEV isn't activate!\n");
		return ALISLFAKE_ERR_DEV_INACTIVATE;
	}

	dev->dev_name = ALISLFAKE_DEV_NAME;
	dev->handle = open(dev->dev_name, O_RDONLY);
	if (dev->handle < 0) {
		SL_ERR("DEV can't open!\n");
		return ALISLFAKE_ERR_CANTOPEN_DEV;
	}

	return ALISLFAKE_ERR_NONE;
}

/**
 *  Function Name:  alislfake_destruct
 *  @brief          destroy dev
 *
 *  @param[in]      dev		descriptor with FAKE attribute
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_destruct(alislfake_dev_t *dev)
{
	close(dev->handle);
	dev->handle = ALISLFAKE_INVALID_HANDLE;
	return ALISLFAKE_ERR_NONE;
}

/**
 *  Function Name:  alislfake_ioctl
 *  @brief          dev ioctl
 *
 *  @param[in]      dev	 descriptor with FAKE attribute
 *  @param[in]      cmd  choose function of FAKE  
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_ioctl(alislfake_dev_t *dev, unsigned int cmd, 
		unsigned long *arg)
{

	if (ALISLFAKE_DEV_INACTIVATE == dev->status) {
		SL_ERR("DEV isn't activate!\n");
		return ALISLFAKE_ERR_DEV_INACTIVATE;
	}

	if (ALISLFAKE_CMD_MAX <= cmd) {
		SL_ERR("Cmd is error! Cmd value: %d.\n", cmd);
		return ALISLFAKE_ERR_NOTSUPPORT_CMD;
	}

	if (dev->handle < 0) {
		SL_ERR("Invalid device handle!\n");
		return ALISLFAKE_ERR_INVALID_HANDLE;
	}

	/* find cmd of driver define by table */
	cmd = fake_driver_cmd_table[cmd];

	if (ioctl(dev->handle, cmd, arg) < 0) {
		SL_ERR("IO access error!\n");
		return ALISLFAKE_ERR_IOACCESS;
	}

	return ALISLFAKE_ERR_NONE;
}
