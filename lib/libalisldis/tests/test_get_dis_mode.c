/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_get_dis_mode.c
 *  @brief      unit test for alisldis_get_real_display_mode() &
 *				alisldis_get_display_mode()
 *
 *  @version    0.9
 *  @date       4/25/2014  14:17:44
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note		fix me
 */


#include <sltest_common.h>

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(GetDisMode);

TEST_SETUP(GetDisMode)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);

    sltest_play_stream(1, "dvbs1");
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("video start play ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(GetDisMode)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);

    sltest_stop_stream();
}

TEST(GetDisMode, GetMode)
{
	enum dis_display_mode dis_mode;
	
    CHECK(ERROR_NONE == alisldis_set_aspect_mode(hd_hdl,
                                                   DIS_DM_PANSCAN,
                                                   DIS_AR_16_9));
    CHECK(ERROR_NONE == alisldis_set_aspect_mode(sd_hdl,
                                                   DIS_DM_PANSCAN,
                                                   DIS_AR_16_9));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is panscan & 16:9 ?",
                                             'y',
                                             'n'));
	CHECK(ERROR_NONE == alisldis_get_display_mode(hd_hdl, &dis_mode));
	printf("dis_mode: %d\n", dis_mode);
	CHECK(ERROR_NONE == alisldis_get_display_mode(sd_hdl, &dis_mode));
	printf("dis_mode: %d\n", dis_mode);
	CHECK(ERROR_NONE == alisldis_get_real_display_mode(hd_hdl, &dis_mode));
	printf("dis_mode: %d\n", dis_mode);
	CHECK(ERROR_NONE == alisldis_get_real_display_mode(sd_hdl, &dis_mode));
	printf("dis_mode: %d\n", dis_mode);

    CHECK(ERROR_NONE == alisldis_set_aspect_mode(hd_hdl,
                                                   DIS_DM_LETTERBOX,
                                                   DIS_AR_16_9));
    CHECK(ERROR_NONE == alisldis_set_aspect_mode(sd_hdl,
                                                   DIS_DM_LETTERBOX,
                                                   DIS_AR_16_9));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is Letterbox & 16:9 ?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(GetDisMode)
{
    RUN_TEST_CASE(GetDisMode, GetMode);
}

static void run_dis_get_dis_mode()
{
    RUN_TEST_GROUP(GetDisMode);
}

static int run_group_dis_get_dis_mode(int argc,
                                     char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_get_dis_mode);

    return 0;
}


