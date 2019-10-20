/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               main_entry.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               01/27/2014 09:19:11 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */
#ifdef _TDS_
#include <sys_config.h>
#include <types.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include <aliunity.h>
#include <unity_fixture.h>

static void runAllTests()
{
	RUN_TEST_GROUP(mygroup);
	RUN_TEST_GROUP(UnityFixture);
	RUN_TEST_GROUP(UnityCommandOptions);
	RUN_TEST_GROUP(LeakDetection)
}

static int example_test1(int argc, char* argv[])
{
	UnityMain(argc, argv, runAllTests);

	return 0;
}

int main(int argc, char **argv)
{
	example_test1(argc, argv);

	return 0;
}
