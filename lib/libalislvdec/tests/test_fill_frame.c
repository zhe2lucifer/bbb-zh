/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_fill_frame.c
 *  @brief			unit test for alislvdec_fill_frame().
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 01:19:47 PM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */
#include <sltest_common.h>

static alisl_handle m_dev_hdl;

TEST_GROUP(VDECFillFrame);

TEST_SETUP(VDECFillFrame)
{
	sltest_play_stream(1, "dvbs1");

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Start play ?",
                                             'y',
                                             'n'));

	CHECK(ERROR_NONE == alislvdec_open(&m_dev_hdl, 0));
}

TEST_TEAR_DOWN(VDECFillFrame)
{
	sltest_stop_stream();

	CHECK(ERROR_NONE == alislvdec_close(m_dev_hdl));
}

TEST(VDECFillFrame, FillFrame)
{
    struct vdec_color c;
    /* struct vdec_status_info status; */

    c.y = 0;
    c.u = 0;
    c.v = 0;

    CHECK(ERROR_NONE == alislvdec_start(m_dev_hdl));

    CHECK(ERROR_NONE == alislvdec_stop(m_dev_hdl, false, true));

    CHECK(ERROR_NONE == alislvdec_fill_frame(m_dev_hdl, &c));
}

TEST_GROUP_RUNNER(VDECFillFrame)
{
	RUN_TEST_CASE(VDECFillFrame, FillFrame);
}

static void run_vdec_fill_frame()
{
	RUN_TEST_GROUP(VDECFillFrame);
}

static int run_group_vdec_fill_frame(int argc, char *argv[])
{
	UnityMain(argc, argv, run_vdec_fill_frame);
	
	return 0;
}
