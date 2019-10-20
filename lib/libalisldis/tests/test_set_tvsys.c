/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_set_tvsys.c
 *  @brief			unit test for alisldis_set_tvsys().
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 10:38:59 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisSetTVSystem);

TEST_SETUP(DisSetTVSystem)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisSetTVSystem)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}


TEST(DisSetTVSystem, SetTVSystem)
{
	enum dis_tvsys tvsys;
	bool progressive;
	
    CHECK(ERROR_NONE == alisldis_set_tvsys(hd_hdl, DIS_TVSYS_LINE_1080_25, false));
	TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("DIS_TVSYS_LINE_1080_25 ?",
                                             'y',
                                             'n'));
    CHECK(ERROR_NONE == alisldis_get_tvsys(hd_hdl, &tvsys, &progressive));
	//CHECK(DIS_TVSYS_LINE_1080_25 == tvsys);
	//CHECK(false == progressive);

    CHECK(ERROR_NONE == alisldis_set_tvsys(hd_hdl, DIS_TVSYS_NTSC, true));
	TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("DIS_TVSYS_NTSC ?",
                                             'y',
                                             'n'));
    CHECK(ERROR_NONE == alisldis_get_tvsys(hd_hdl, &tvsys, &progressive));
	//CHECK(DIS_TVSYS_NTSC == tvsys);
	//CHECK(true == progressive);

	CHECK(ERROR_NONE == alisldis_set_tvsys(hd_hdl, DIS_TVSYS_LINE_720P_60_VESA, true));
	TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("DIS_TVSYS_LINE_720P_60_VESA ?",
                                             'y',
                                             'n'));
    CHECK(ERROR_NONE == alisldis_get_tvsys(hd_hdl, &tvsys, &progressive));
	//CHECK(DIS_TVSYS_LINE_720P_60_VESA == tvsys);
	//CHECK(true == progressive);
}

TEST_GROUP_RUNNER(DisSetTVSystem)
{
    RUN_TEST_CASE(DisSetTVSystem, SetTVSystem);
}

static void run_dis_set_tvsys()
{
    RUN_TEST_GROUP(DisSetTVSystem);
}

static int run_group_dis_set_tvsys(int argc,
                                     char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_set_tvsys);

    return 0;
}

