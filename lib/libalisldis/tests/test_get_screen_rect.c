/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_get_screen_rect.c
 *  @brief      unit test for alisldis_get_mp_screen_rect()
 *
 *  @version    0.9
 *  @date       4/25/2014  14:17:44
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */


#include <sltest_common.h>

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisScreenRect);

TEST_SETUP(DisScreenRect)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);

    sltest_play_stream(1, "dvbs1");
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("video start play ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(DisScreenRect)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);

    sltest_stop_stream();
}

TEST(DisScreenRect, GetScreenSize)
{
    struct dis_rect srect = {0, 0, 720, 2880};
    struct dis_rect drect = {0, 0, 360, 1440};
    struct dis_rect result;

    CHECK(ERROR_NONE == alisldis_get_mp_screen_rect(hd_hdl, &result));
	printf("x = %d, y = %d, w = %d, h = %d\n", result.x, result.y, result.w, result.h);
    CHECK(1280 == result.w);
	CHECK(720 == result.h);

    CHECK(ERROR_NONE == alisldis_zoom_by_layer(hd_hdl,
                                                 &srect,
                                                 &drect,
                                                 DIS_LAYER_MAIN));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("1/4 screen ?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alisldis_get_mp_screen_rect(hd_hdl, &result));
	printf("x = %d, y = %d, w = %d, h = %d\n", result.x, result.y, result.w, result.h);
    CHECK(640 == result.w);
	CHECK(360 == result.h);

	drect.w = 720;
    drect.h = 2880;
    srect.w = 720;
    srect.h = 2880;
    CHECK(ERROR_NONE == alisldis_zoom_by_layer(hd_hdl,
                                                 &srect,
                                                 &drect,
                                                 DIS_LAYER_MAIN));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("full screen ?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alisldis_get_mp_screen_rect(hd_hdl, &result));
	CHECK(1280 == result.w);
	CHECK(720 == result.h);
}

TEST_GROUP_RUNNER(DisScreenRect)
{
    RUN_TEST_CASE(DisScreenRect, GetScreenSize);
}

static void run_dis_get_screen_rect()
{
    RUN_TEST_GROUP(DisScreenRect);
}

static int run_group_dis_get_screen_rect(int argc,
                                         char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_get_screen_rect);

    return 0;
}


