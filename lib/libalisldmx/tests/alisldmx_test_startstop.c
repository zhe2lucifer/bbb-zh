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

alisl_handle dmx0_dev;
alisl_handle dmx1_dev;
alisl_handle dmx2_dev;

TEST_GROUP(DmxStartStop);

TEST_SETUP(DmxStartStop)
{
	alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0);
	alisldmx_open(&dmx1_dev, DMX_ID_DEMUX1, 0);
	alisldmx_open(&dmx2_dev, DMX_ID_DEMUX2, 0);
}

TEST_TEAR_DOWN(DmxStartStop)
{
	alisldmx_close(dmx0_dev);
	alisldmx_close(dmx1_dev);
	alisldmx_close(dmx2_dev);
}

TEST(DmxStartStop, StartStop)
{
	CHECK(alisldmx_start(dmx0_dev) == 0); 
    CHECK(alisldmx_stop(dmx0_dev) == 0);

	CHECK(alisldmx_start(dmx1_dev) == 0); 
    CHECK(alisldmx_stop(dmx1_dev) == 0);
	
	CHECK(alisldmx_start(dmx2_dev) == 0); 
    CHECK(alisldmx_stop(dmx2_dev) == 0);
}

TEST_GROUP_RUNNER(DmxStartStop)
{
	RUN_TEST_CASE(DmxStartStop, StartStop);
}

static void run_dmx_start_stop()
{
	RUN_TEST_GROUP(DmxStartStop);
}
static int run_group_dmx_start_stop(int argc, char *argv[]) {
	UnityMain(argc, argv, run_dmx_start_stop);
	
	return 0;
}
