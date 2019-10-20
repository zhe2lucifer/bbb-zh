/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_get_aspect.c
 *  @brief      unit test for alisldis_get_aspect() &
 *				alisldis_get_src_aspect()
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

TEST_GROUP(GetDisAspect);

TEST_SETUP(GetDisAspect)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);

    sltest_play_stream(1, "dvbs1");
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("video start play ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(GetDisAspect)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);

    sltest_stop_stream();
}

TEST(GetDisAspect, GetAspect)
{
	enum dis_aspect_ratio aspect;
	
    CHECK(ERROR_NONE == alisldis_get_aspect(hd_hdl, &aspect));
	printf("aspect = %d\n", aspect);

	CHECK(ERROR_NONE == alisldis_get_src_aspect(hd_hdl, &aspect));
	printf("aspect = %d\n", aspect);
}

TEST_GROUP_RUNNER(GetDisAspect)
{
    RUN_TEST_CASE(GetDisAspect, GetAspect);
}

static void run_dis_get_aspect()
{
    RUN_TEST_GROUP(GetDisAspect);
}

static int run_group_dis_get_aspect(int argc,
                                     char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_get_aspect);

    return 0;
}


