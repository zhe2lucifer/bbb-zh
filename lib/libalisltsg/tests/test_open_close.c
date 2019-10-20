/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_open_close.c
 *  @brief
 *
 *  @Version:       1.0
 *  @date:          08/19/2014 02:52:53 PM
 *  @revision:      none
 *
 *  @Author:       Vedic Fu <vedic.fu@alitech.com>
 */
#include <sltest_common.h>
 
static sltest_tsg_config tsg_test_config;
static alisl_handle tsg_handle=NULL;

TEST_GROUP(TSGOpenClose);

TEST_SETUP(TSGOpenClose)
{
	CHECK(!sltest_get_tsg_config(&tsg_test_config));
}

TEST_TEAR_DOWN(TSGOpenClose)
{

}

TEST(TSGOpenClose, case1)
{
    int max_time = 10;
	int i;
    for (i = 0; i < max_time; i++) {
        CHECK(0 == alisltsg_open(&tsg_handle, tsg_test_config.tsi_config.tsi_id, NULL));
    }
	
    for (i = 0; i < max_time; i++) {
        CHECK(0 == alisltsg_close(&tsg_handle));
    }
	
    for (i = 0; i < max_time; i++) {
        CHECK(0 != alisltsg_close(&tsg_handle));
    }

}

TEST_GROUP_RUNNER(TSGOpenClose)
{
	RUN_TEST_CASE(TSGOpenClose, case1);
}

static void run_tsg_open_close()
{
	RUN_TEST_GROUP(TSGOpenClose);
}

static int run_group_tsg_open_close(int argc, char *argv[])
{
	UnityMain(argc, argv, run_tsg_open_close);
	
	return 0;
}
