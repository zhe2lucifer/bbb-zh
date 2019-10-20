/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislotpkit.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/06/2013 10:14:28 AM
 *  @revision           none
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 */


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct alislotp_cmdparam {
	bool         reboot;
	bool         start;
	bool         stop;
	bool         feed;
	unsigned int time;
	bool         multiple;
	int          tasks;
	pid_t        child;
} alislotp_cmdparam_t;
#ifdef __cplusplus
}
#endif

