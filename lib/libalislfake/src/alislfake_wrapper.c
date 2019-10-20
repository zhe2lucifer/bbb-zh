/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislfake_wrapper.c
 *  @brief          output api of FAKE share lib 
 *
 *  @version        1.0
 *  @date           05/31/2013 06:18:16 PM
 *  @revision       none
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <alipltfretcode.h>
#include <alipltflog.h>

/* Fake header */
#include <alislfake.h>

/* Internal header */
#include "internal.h"

/**
 *  Function Name:  alislfake_get_tick
 *  @brief          get gap of 2 ticks 
 *
 *  @param[in]      trace      print position of progame 
 *  @param[in]      enable     0:disable 1: enable 
 *  @param[in]      status     0:in 1:out
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note           pls find "ALISLFAKE INFO: gap %lu us" in your print
 */
alislfake_retcode alislfake_get_tick(const char *trace, const char enable,
		unsigned int status)
{
	int      ret = ALISLFAKE_ERR_NONE;
	alislfake_dev_t      dev;
	alislfake_param_t    param;
	unsigned long        tick_cur = 0;
#ifdef ENABLE_DEBUG
	static unsigned long tick_last = 0;
#endif
	if (!enable) {
		SL_WARN("Not to enable.\n");
		return ALISLFAKE_WARN_DISABLE;
	}

	memset(&dev, 0x00, sizeof(alislfake_dev_t));
	memset(&param, 0x00, sizeof(alislfake_param_t));

	param.status = ALISLFAKE_DEV_ACTIVATE;
	alislfake_construct(&dev, &param);

	ret = alislfake_ioctl(&dev, ALISLFAKE_CMD_GET_TICK, &tick_cur);
//SL_INFO will do nothing if disable DEBUG in mini build ,so the warning that "tick_last define but not used" occur
#ifdef ENABLE_DEBUG        
	if (status == ALISLFAKE_GET_TICK_OUT) {
		SL_INFO("The gap %lu us.\n", trace, tick_cur - tick_last);
	}

	tick_last = tick_cur;
#endif
	alislfake_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislfake_show_mm_ddr
 *  @brief          show memory start address
 *    
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_show_mm_ddr(void)
{
	int  ret = ALISLFAKE_ERR_NONE;
	alislfake_dev_t   dev;
	alislfake_param_t param;

	memset(&dev, 0x00, sizeof(alislfake_dev_t));
	memset(&param, 0x00, sizeof(alislfake_param_t));

	param.status = ALISLFAKE_DEV_ACTIVATE;
	alislfake_construct(&dev, &param);

	ret = alislfake_ioctl(&dev, ALISLFAKE_CMD_GET_MM_ADDR, NULL);

	alislfake_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislfake_show_mem
 *  @brief          show memory map
 *
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_show_mem(void)
{
	int   ret = ALISLFAKE_ERR_NONE;
	alislfake_dev_t   dev;
	alislfake_param_t param;

	memset(&dev, 0x00, sizeof(alislfake_dev_t));
	memset(&param, 0x00, sizeof(alislfake_param_t));

	param.status = ALISLFAKE_DEV_ACTIVATE;
	alislfake_construct(&dev, &param);

	ret = alislfake_ioctl(&dev, ALISLFAKE_CMD_SHOW_MEM, NULL);

	alislfake_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislfake_show_stack_all
 *  @brief          show stacks of all task
 *
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_show_stack_all(void)
{
	int   ret = ALISLFAKE_ERR_NONE;
	alislfake_dev_t   dev;
	alislfake_param_t param;

	memset(&dev, 0x00, sizeof(alislfake_dev_t));
	memset(&param, 0x00, sizeof(alislfake_param_t));

	param.status = ALISLFAKE_DEV_ACTIVATE;
	alislfake_construct(&dev, &param);

	ret = alislfake_ioctl(&dev, ALISLFAKE_CMD_SHOW_STACK_ALL, NULL);

	alislfake_destruct(&dev);

	return ret;
}

/**
 *  Function Name:  alislfake_show_stack_pid
 *  @brief          show stack by task id
 *
 *  @param          pid Id of task
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_show_stack_pid(pid_t pid)
{
	int ret = ALISLFAKE_ERR_NONE;
	alislfake_dev_t   dev;
	alislfake_param_t param;
	pid_t inpid = 0;

	inpid = pid;
	memset(&dev, 0x00, sizeof(alislfake_dev_t));
	memset(&param, 0x00, sizeof(alislfake_param_t));

	param.status = ALISLFAKE_DEV_ACTIVATE;
	alislfake_construct(&dev, &param);

	ret = alislfake_ioctl(&dev, ALISLFAKE_CMD_SHOW_STACK_PID, (unsigned long *)&inpid);

	alislfake_destruct(&dev);

	return ret;
}
