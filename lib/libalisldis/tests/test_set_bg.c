/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_set_bg.c
 *  @brief			unit test for alisldis_set_bgcolor().
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:37:21 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisBg);

TEST_SETUP(DisBg)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisBg)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisBg, BgSet)
{
    struct dis_color c = {0, 0, 0};

	/** hide video layer first. */
	CHECK(ERROR_NONE == alisldis_win_onoff_by_layer(hd_hdl, 0, DIS_LAYER_MAIN));

    CHECK(ERROR_NONE == alisldis_set_bgcolor(hd_hdl, &c));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("bg: (0, 0, 0)?",
                                             'y',
                                             'n'));

    c.cb = 0;
    c.cr = 100;
    c.y = 20;
    CHECK(ERROR_NONE == alisldis_set_bgcolor(hd_hdl, &c));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("bg: (0, 100, 20)?",
                                             'y',
                                             'n'));

	CHECK(ERROR_NONE == alisldis_win_onoff_by_layer(hd_hdl, 1, DIS_LAYER_MAIN));
}

TEST_GROUP_RUNNER(DisBg)
{
    RUN_TEST_CASE(DisBg, BgSet);
}

static void run_dis_set_bg()
{
    RUN_TEST_GROUP(DisBg);
}

static int run_group_dis_bg(int argc,
                            char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_set_bg);

    return 0;
}
