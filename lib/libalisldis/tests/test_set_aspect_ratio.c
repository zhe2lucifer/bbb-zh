/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_set_aspect_ratio.c
 *  @brief			unit test for alisldis_set_aspect_mode()
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:36:21 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisSetAspectRatio);

TEST_SETUP(DisSetAspectRatio)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisSetAspectRatio)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisSetAspectRatio, SetAspectRatio)
{
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

    CHECK(ERROR_NONE == alisldis_set_aspect_mode(hd_hdl,
                                                   DIS_DM_LETTERBOX,
                                                   DIS_AR_4_3));
    CHECK(ERROR_NONE == alisldis_set_aspect_mode(sd_hdl,
                                                   DIS_DM_LETTERBOX,
                                                   DIS_AR_4_3));

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is letterbox & 4:3 ?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alisldis_set_aspect_mode(hd_hdl,
                                                   DIS_DM_PILLBOX,
                                                   DIS_AR_4_3));
    CHECK(ERROR_NONE == alisldis_set_aspect_mode(sd_hdl,
                                                   DIS_DM_PILLBOX,
                                                   DIS_AR_4_3));

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is pillbox & 4:3 ?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alisldis_set_aspect_mode(hd_hdl,
                                                   DIS_DM_PANSCAN43ON169,
                                                   DIS_AR_4_3));
    CHECK(ERROR_NONE == alisldis_set_aspect_mode(sd_hdl,
                                                   DIS_DM_PANSCAN43ON169,
                                                   DIS_AR_4_3));

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is DIS_DM_PANSCAN43ON169 & 4:3 ?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(DisSetAspectRatio)
{
    RUN_TEST_CASE(DisSetAspectRatio, SetAspectRatio);
}

static void run_dis_set_aspectratio()
{
    RUN_TEST_GROUP(DisSetAspectRatio);
}

static int run_group_dis_set_aspectratio(int argc,
                                         char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_set_aspectratio);

    return 0;
}
