/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internal.h
 *  @brief              Conax Virtual Smart Card API
 *
 *  @version            1.0
 *  @date               01/09/2016 09:50:28 AM
 *  @revision           none
 *
 *  @author             Deji Aribuki <deji.aribuki@alitech.com>
 */


#ifndef __ALISLCONAXVSC_INTERNAL__H_
#define __ALISLCONAXVSC_INTERNAL__H_

#include <alislconaxvsc.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct alislconaxvsc_dev {
	int fd_smc;
	void *store_user_data;
	alislconaxvsc_store_fun_cb store_callback;
} alislconaxvsc_dev_t;


#ifdef __cplusplus
}
#endif

#endif
