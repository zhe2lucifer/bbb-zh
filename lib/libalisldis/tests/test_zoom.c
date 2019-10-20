/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_zoom.c
 *  @brief			unit test for alisldis_zoom_by_layer.
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:39:52 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

#include <sltest_common.h>

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisZoom);

TEST_SETUP(DisZoom)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);

	sltest_play_stream(1, "dvbs1");
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("video start play ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(DisZoom)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);

	sltest_stop_stream();
}

TEST(DisZoom, TestZoom)
{
    struct dis_rect srect = {0, 0, 720, 2880};
    struct dis_rect drect = {0, 0, 360, 1440};

    CHECK(ERROR_NONE == alisldis_zoom_by_layer(hd_hdl,
                                                 &srect,
                                                 &drect,
                                                 DIS_LAYER_MAIN));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("1/4 screen ?",
                                             'y',
                                             'n'));

	drect.w = 180;
    drect.h = 720;
	CHECK(ERROR_NONE == alisldis_zoom_by_layer(hd_hdl,
                                                 &srect,
                                                 &drect,
                                                 DIS_LAYER_MAIN));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("1/16 screen ?",
                                             'y',
                                             'n'));
	
    drect.w = 720;
    drect.h = 2880;
    CHECK(ERROR_NONE == alisldis_zoom_by_layer(hd_hdl,
                                                 &srect,
                                                 &drect,
                                                 DIS_LAYER_MAIN));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("full screen ?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(DisZoom)
{
    RUN_TEST_CASE(DisZoom, TestZoom);
}

static void run_dis_zoom()
{
    RUN_TEST_GROUP(DisZoom);
}

static int run_group_dis_zoom(int argc,
                              char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_zoom);

    return 0;
}


