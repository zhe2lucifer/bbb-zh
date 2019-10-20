/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           sldmx_test_openclose.c
 *  @brief          
 *
 *  @version        1.0
 *  @date           04/30/2014 02:17:08 PM
 *  @revision       none
 *
 *  @author         Christian xie <christian.xie@alitech.com>
 */
#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <alisldmx.h>

TEST_GROUP(DmxOpenClose);

TEST_SETUP(DmxOpenClose)
{
}

TEST_TEAR_DOWN(DmxOpenClose)
{

}

TEST(DmxOpenClose, OpenTest)
{
    const int max_time = 5;
	int i,j;
	alisl_handle dmx_dev;
    
    for (i = (int *)DMX_ID_DEMUX0; i < (int *)DMX_NB_DEMUX; i++)
	{		
    	printf("DEMUX%d OpenClose Test(%d times)\n", i, max_time);
		for (j = 0; j < max_time; j++) 
		{
        	CHECK(alisldmx_open(&dmx_dev, i, 0) == 0); 
		} 
    	for (j = 0; j < max_time; j++) 
		{
        	CHECK(alisldmx_close(dmx_dev) == 0);
    	}
	}
}

TEST_GROUP_RUNNER(DmxOpenClose)
{
	RUN_TEST_CASE(DmxOpenClose, OpenTest);
}

static void run_dmx_open_close()
{
	RUN_TEST_GROUP(DmxOpenClose);
}
static int run_group_dmx_open_close(int argc, char *argv[]) {
	UnityMain(argc, argv, run_dmx_open_close);
	
	return 0;
}
