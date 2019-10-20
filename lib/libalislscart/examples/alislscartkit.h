/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file           alislscartkit.h
 *  @brief          head file of test SCART function
 *
 *  @version        1.0
 *  @date           06/22/2013 02:32:35 PM
 *  @revision       none
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 */

#ifndef __ALISLSCART_KIT__H_
#define __ALISLSCART_KIT__H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct alislscart_cmdparam {
	bool         standby;
	bool         aspect;
	unsigned int sw;
	bool         multiple;
	int          tasks;
	pid_t        child;
} alislscart_cmdparam_t;

#ifdef __cplusplus
}
#endif

#endif
