/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_open_close.h
 *  @brief			unit test for alislhdmi_open() & alislhdmi_close().
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 05:38:01 PM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */
#include <alislhdmi.h>
#include <alislcli.h>

TEST_GROUP(HDMIOpenClose);

TEST_SETUP(HDMIOpenClose)
{
}

TEST_TEAR_DOWN(HDMIOpenClose)
{

}

TEST(HDMIOpenClose, case1)
{
    const int max_time = 5;
	int i;
	alisl_handle hdmi_dev[5];
	unsigned char hdcp_key[286] = {0};

    for (i = 0; i < max_time; i++) {
        CHECK(HDMI_ERROR_NONE == alislhdmi_open(&(hdmi_dev[i]), hdcp_key));
    }

    for (i = 0; i < max_time; i++) {
        CHECK(HDMI_ERROR_NONE == alislhdmi_close(hdmi_dev[i]));
    }

    CHECK(HDMI_ERROR_NONE != alislhdmi_close(hdmi_dev[0]));
}

TEST_GROUP_RUNNER(HDMIOpenClose)
{
	RUN_TEST_CASE(HDMIOpenClose, case1);
}

static void run_hdmi_open_close()
{
	RUN_TEST_GROUP(HDMIOpenClose);
}

static int run_group_hdmi_open_close(int argc, char *argv[])
{
	UnityMain(argc, argv, run_hdmi_open_close);
	
	return 0;
}

