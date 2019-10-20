/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file           test_gma_global_alpha.c
 *  @brief          test global alpha set for GMA
 *
 *  @version        0.1
 *  @date           5/8/2014  14:58:44
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisGMAGlobalAlpha);

TEST_SETUP(DisGMAGlobalAlpha)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisGMAGlobalAlpha)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisGMAGlobalAlpha, GlobalAlpha)
{
    CHECK(ERROR_NONE == alisldis_set_global_alpha_by_layer(hd_hdl,
                                                             DIS_LAYER_GMA1,
                                                             0));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("alpha = 0?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alisldis_set_global_alpha_by_layer(hd_hdl, DIS_LAYER_GMA1,
                                                             0xff));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("alpha = 0xff?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(DisGMAGlobalAlpha)
{
    RUN_TEST_CASE(DisGMAGlobalAlpha, GlobalAlpha);
}

static void run_dis_gma_global_alpha()
{
    RUN_TEST_GROUP(DisGMAGlobalAlpha);
}

static int run_group_dis_gma_global_alpha(int argc,
                                          char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_gma_global_alpha);

    return 0;
}
