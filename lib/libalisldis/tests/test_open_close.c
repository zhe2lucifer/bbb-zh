/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_open_close.c
 *  @brief			unit test for alisldis_open() & alisldis_close()
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:35:25 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <alisldis.h>
#include <alislcli.h>

TEST_GROUP(DisOpenClose);

TEST_SETUP(DisOpenClose)
{
}

TEST_TEAR_DOWN(DisOpenClose)
{

}

TEST(DisOpenClose, OpenTest)
{
    const int max_time = 5;
	int i;
	alisl_handle m_hd_dev;
	alisl_handle m_sd_dev;

    printf("max_time = %d\n", max_time);
    for (i = 0; i < max_time; i++) {
        CHECK(alisldis_open(DIS_HD_DEV, &m_hd_dev) == ERROR_NONE);
        CHECK(alisldis_open(DIS_SD_DEV, &m_sd_dev) == ERROR_NONE);
    }

    for (i = 0; i < max_time; i++) {
        CHECK(alisldis_close(m_hd_dev) == ERROR_NONE);
        CHECK(alisldis_close(m_sd_dev) == ERROR_NONE);
    }

    CHECK(alisldis_close(m_hd_dev) != ERROR_NONE);
    CHECK(alisldis_close(m_sd_dev) != ERROR_NONE);
}

TEST_GROUP_RUNNER(DisOpenClose)
{
	RUN_TEST_CASE(DisOpenClose, OpenTest);
}

static void run_dis_open_close()
{
	RUN_TEST_GROUP(DisOpenClose);
}

static int run_group_dis_open_close(int argc, char *argv[])
{
	UnityMain(argc, argv, run_dis_open_close);
	
	return 0;
}
