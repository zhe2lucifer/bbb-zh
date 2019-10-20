/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_open_close.c
 *  @brief          unit test for alislvdec_open() & alislvdec_close().
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 11:07:45 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */
TEST_GROUP(VDECOpenClose);

TEST_SETUP(VDECOpenClose)
{

}

TEST_TEAR_DOWN(VDECOpenClose)
{

}

TEST(VDECOpenClose, case1)
{
	alisl_handle m_dev_hdl;
    const max_time = 10;
    int i;

    for (i = 0; i < max_time; i++) {
        CHECK(ERROR_NONE == alislvdec_open(&m_dev_hdl, 0));
    }

    for (i = 0; i < max_time; i++) {
        CHECK(ERROR_NONE == alislvdec_close(m_dev_hdl));
    }

    for (i = 0; i < max_time; i++) {
        CHECK(ERROR_NONE != alislvdec_close(m_dev_hdl));
    }
}

TEST_GROUP_RUNNER(VDECOpenClose)
{
	RUN_TEST_CASE(VDECOpenClose, case1);
}

static void run_vdec_open_close()
{
	RUN_TEST_GROUP(VDECOpenClose);
}

static int run_group_vdec_open_close(int argc, char *argv[])
{
	UnityMain(argc, argv, run_vdec_open_close);
	
	return 0;
}
