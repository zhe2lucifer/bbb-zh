/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file			showstack.c
 *  @brief			
 *
 *  @version		1.0
 *  @date			06/20/2013 10:20:47 AM
 *  @revision		none
 *

 *  @author			Chen Jonathan <Jonathan.Chen@alitech.com>
 */

/* System header */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include <alipltflog.h>

/* Fake header */
#include <alislfake.h>

static void usage(void)
{
	printf("usage: ./showstack [-a|-p] [PID]");
}

int main(int argc, char *argv[])
{   
	pid_t pid;

	if (argc < 2 || '-' != argv[1][0] \
	   || ('p' != argv[1][1] && 'a' != argv[1][1])) {
		usage();

		return 0; 
	}

	if ('a' == argv[1][1]) {
		alislfake_show_stack_all();
	} else if ('p' == argv[1][1]) {
		pid = atoi(argv[2]);
		alislfake_show_stack_pid(pid);	
	} else {
		printf("Un-recoginized option %s\n", argv[1]);
	}

	return 0;
}
