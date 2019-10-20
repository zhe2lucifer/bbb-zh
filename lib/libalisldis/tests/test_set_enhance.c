/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_set_enhance.c
 *  @brief			unit test for alisldis_set_enhance().
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:38:22 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisEnhance);

TEST_SETUP(DisEnhance)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisEnhance)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisEnhance, BrightnessSet)
{
    struct dis_enhance enhence;

	enhence.type = DIS_ENHANCE_BRIGHTNESS;
    enhence.brightness = 100;
    CHECK(ERROR_NONE == alisldis_set_enhance(hd_hdl, &enhence));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("brightness: 100 ?",
                                             'y',
                                             'n'));
	
    enhence.type = DIS_ENHANCE_BRIGHTNESS;
    enhence.brightness = 80;
    CHECK(ERROR_NONE == alisldis_set_enhance(hd_hdl, &enhence));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("brightness: 80 ?",
                                             'y',
                                             'n'));

    enhence.type = DIS_ENHANCE_BRIGHTNESS;
    enhence.brightness = 60;
    CHECK(ERROR_NONE == alisldis_set_enhance(hd_hdl, &enhence));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("brightness: 60 ?",
                                             'y',
                                             'n'));

    enhence.type = DIS_ENHANCE_BRIGHTNESS;
    enhence.brightness = 50;
    CHECK(ERROR_NONE == alisldis_set_enhance(hd_hdl, &enhence));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("brightness: 50 ?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(DisEnhance)
{
    RUN_TEST_CASE(DisEnhance, BrightnessSet);
}

static void run_dis_set_enhance()
{
    RUN_TEST_GROUP(DisEnhance);
}

static int run_group_dis_set_enhance(int argc,
                                     char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_set_enhance);

    return 0;
}

