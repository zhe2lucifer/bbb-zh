/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file			alislfake.h
 *  @brief			Headfile of FAKE share lib function
 *
 *  @version		1.0
 *  @date			05/31/2013 01:55:49 PM
 *  @revision		none
 *
 *  @author			Chen Jonathan <Jonathan.Chen@alitech.com>
 */

#ifndef __ALISLFAKE_INTERFACE__H_
#define __ALISLFAKE_INTERFACE__H_

#ifdef __cplusplus
extern "C"
{
#endif


/* struct define */

/* alislfake ret code*/
typedef int alislfake_retcode;

/* status of fake dev */
typedef enum alislfake_status {
	ALISLFAKE_DEV_INACTIVATE,
	ALISLFAKE_DEV_ACTIVATE,
} alislfake_status_t;

/* CMD of fake lib*/
typedef enum alislfake_cmd {
	ALISLFAKE_CMD_GET_MM_ADDR,
	ALISLFAKE_CMD_SHOW_MEM,
	ALISLFAKE_CMD_GET_TICK,
	ALISLFAKE_CMD_SHOW_STACK_ALL,
	ALISLFAKE_CMD_SHOW_STACK_PID,

	/* CMD MAX VALUE */
	ALISLFAKE_CMD_MAX,
} alislfake_cmd_t;

/* description of fake dev */
typedef struct alislfake_dev {
	char *dev_name;
	int  handle;
	unsigned char status;         //0:inactivate 1:activate
} alislfake_dev_t;

/* param of fun */
typedef struct alislfake_param {
	unsigned char status;         //0:inactivate 1:activate
	unsigned char rev;
} alislfake_param_t;

/* get_tick param of fun */
typedef enum alislfake_get_tick_status {
	ALISLFAKE_GET_TICK_IN,      
	ALISLFAKE_GET_TICK_OUT,
} alislfake_get_tick_status_t;

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
		alislfake_param_t *param);

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
alislfake_retcode alislfake_destruct(alislfake_dev_t *dev);

/**
 *  Function Name:  alislfake_ioctl
 *  @brief          dev ioctl
 *
 *  @param[in]      dev  descriptor with FAKE attribute
 *  @param[in]      cmd  choose function of FAKE 
 *  @param[in]      arg  return or input param                  
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_ioctl(alislfake_dev_t *dev, unsigned int cmd, 
		unsigned long *arg);
/**
 *  Function Name:  alislfake_get_tick
 *  @brief          get gap of 2 ticks 
 *
 *  @param[in]      trace      print position of progame 
 *  @param[in]      enable     0:enable 1:disable 
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
		unsigned int status);

/**
 *  Function Name:  alislfake_show_mm_ddr
 *  @brief          show memory map start address
 * 
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_show_mm_ddr(void);

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
alislfake_retcode alislfake_show_mem(void);

/**
 *  Function Name:  alislfake_show_stack_all
 *  @brief          show all stack of task
 *  
 *
 *  @return         FAKE error NO.
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
alislfake_retcode alislfake_show_stack_all(void);

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
alislfake_retcode alislfake_show_stack_pid(pid_t pid);

#ifdef __cplusplus
}
#endif

#endif
