/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file           alislstandbykit.h
 *  @brief          head file of test STANDBY function
 *
 *  @version        1.0
 *  @date           06/22/2013 02:32:35 PM
 *  @revision       none
 *
 *  @author         Alan Zhang <Alan.Zhang@alitech.com>
 */

#ifndef __ALISLSTANDBY_KIT__H_
#define __ALISLSTANDBY_KIT__H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct alislstandby_cmdparam {
	bool         standby;
	bool         tv2sat;
	unsigned int sw;
	bool         multiple;
	int          tasks;
	pid_t        child;
} alislstandby_cmdparam_t;

#ifdef __cplusplus
}
#endif

#endif
