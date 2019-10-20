/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_get_mp_info.c
 *  @brief      unit test for alisldis_get_mp_info()
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

TEST_GROUP(DisMPInfo);

TEST_SETUP(DisMPInfo)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);

    sltest_play_stream(1, "dvbs1");
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("video start play ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(DisMPInfo)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);

    sltest_stop_stream();
}

TEST(DisMPInfo, MPInfo)
{
    struct dis_info mp_info;

    CHECK(ERROR_NONE == alisldis_get_mp_info(hd_hdl, &mp_info));
    printf("progressive: %d, gm1: %d, src_w: %d, src_h: %d, dst_w: %d, dst_h: %d, tvsys: %d\n",
           mp_info.progressive, mp_info.gma1_onoff, mp_info.source_width,
           mp_info.source_height, mp_info.dst_width, mp_info.dst_height, mp_info.tvsys);
	//CHECK(1280 == mp_info.dst_width);
	//CHECK(720 == mp_info.dst_height);

    CHECK(ERROR_NONE == alisldis_get_mp_info(sd_hdl, &mp_info));
	printf("progressive: %d, gm1: %d, src_w: %d, src_h: %d, dst_w: %d, dst_h: %d, tvsys: %d\n",
           mp_info.progressive, mp_info.gma1_onoff, mp_info.source_width,
           mp_info.source_height, mp_info.dst_width, mp_info.dst_height, mp_info.tvsys);
	//CHECK(720 == mp_info.dst_width);
	//CHECK(576 == mp_info.dst_height);
}

TEST_GROUP_RUNNER(DisMPInfo)
{
    RUN_TEST_CASE(DisMPInfo, MPInfo);
}

static void run_dis_get_mp_info()
{
    RUN_TEST_GROUP(DisMPInfo);
}

static int run_group_dis_get_mp_info(int argc,
                                     char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_get_mp_info);

    return 0;
}


