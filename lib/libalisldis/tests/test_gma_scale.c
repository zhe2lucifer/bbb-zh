/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_gma_scale.c
 *  @brief      test gma scale
 *
 *  @version    0.1
 *  @date       5/8/2014  10:22:54
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisGMAScale);

TEST_SETUP(DisGMAScale)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisGMAScale)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisGMAScale, GMAScale)
{
	struct scale_param scale_p;

	scale_p.h_src = 720;
	scale_p.v_src = 1280;
	scale_p.h_dst = 360;
	scale_p.v_dst = 640;
    CHECK(ERROR_NONE == alisldis_gma_scale(hd_hdl, &scale_p, DIS_LAYER_GMA1));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("1/4 OSD?",
                                             'y',
                                             'n'));

	scale_p.h_src = 360;
	scale_p.v_src = 640;
	scale_p.h_dst = 720;
	scale_p.v_dst = 1280;
    CHECK(ERROR_NONE == alisldis_gma_scale(hd_hdl, &scale_p, DIS_LAYER_GMA1));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Full OSD?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(DisGMAScale)
{
    RUN_TEST_CASE(DisGMAScale, GMAScale);
}

static void run_dis_gma_scale()
{
    RUN_TEST_GROUP(DisGMAScale);
}

static int run_group_dis_gma_scale(int argc,
                            char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_gma_scale);

    return 0;
}

