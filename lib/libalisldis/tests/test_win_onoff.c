/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_win_onoff.c
 *  @brief			unit test for alisldis_win_onoff_by_layer().
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:39:28 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisWinOnoff);

TEST_SETUP(DisWinOnoff)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisWinOnoff)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisWinOnoff, OnoffTest)
{
    const int cnt = 4;
    int i = 0, on;

	for (i = 0; i < cnt; i ++) {
		on = i % 2;
    	CHECK(ERROR_NONE == alisldis_win_onoff_by_layer(hd_hdl, on, DIS_LAYER_MAIN));
		if (i % 2) {
			TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is video layer on ?",
                                             'y',
                                             'n'));
		} else {
			TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is video layer off ?",
                                             'y',
                                             'n'));
		}
	}

	for (i = 0; i < cnt; i ++) {
		on = i % 2;
		CHECK(ERROR_NONE == alisldis_win_onoff_by_layer(hd_hdl, on, DIS_LAYER_GMA1));
		if (i % 2) {
			TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is OSD layer on ?",
                                             'y',
                                             'n'));
		} else {
			TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is OSD layer off ?",
                                             'y',
                                             'n'));
		}
	}
}

TEST_GROUP_RUNNER(DisWinOnoff)
{
    RUN_TEST_CASE(DisWinOnoff, OnoffTest);
}

static void run_dis_win_onoff()
{
    RUN_TEST_GROUP(DisWinOnoff);
}

static int run_group_dis_win_onoff(int argc,
                                   char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_win_onoff);

    return 0;
}
