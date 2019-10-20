/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file			alislfakekit.h
 *  @brief			Headfile Test function of FAKE share lib 
 *
 *  @version		1.0
 *  @date			05/31/2013 01:55:49 PM
 *  @revision		none
 *
 *  @author			Chen Jonathan <Jonathan.Chen@alitech.com>
 */

#ifndef __ALISLFAKE_KIT__H_
#define __ALISLFAKE_KIT__H_

#ifdef __cplusplus
extern "C"
{
#endif

#define ALISLFAKE_TEST_DELAY_1000_US 1000
#define ALISLFAKE_TEST_DELAY_10000_US 10000

typedef struct alislfake_cmdparam {
	unsigned int    cmd;
	pid_t           pid;
	bool            multiple;
	int             tasks;
	pid_t           child;
} alislfake_cmdparam_t;

typedef enum alislfake_cmd_type {
	ALISLFAKE_GET_TICKS = 1,
	ALISLFAKE_SHOW_START_MEM,
	ALISLFAKE_SHOW_MEM_MAP,    
	ALISLFAKE_SHOW_STACK_ALL,
	ALISLFAKE_SHOW_STACK_PID,
} alislfake_cmd_type_t;

#ifdef __cplusplus
}
#endif

#endif

