/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislsockit.c
 *  @brief              test watchdog base funcation 
 *
 *  @version            1.0
 *  @date               07/06/2013 10:14:08 AM
 *  @revision           none
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 */

/* System Header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> //relevant to openrate file  like macro "O_RDWR"
#include <linux/ioctl.h>

/* Kit Private header */
#include <alislsockit.h>
#include <ali_soc_common.h>
#include <alislsoc.h>

int main(int argc, char *argv[])
{
	printf("reboot test=======================\n");
	alislsoc_op_reboot();
	return 0;
}
