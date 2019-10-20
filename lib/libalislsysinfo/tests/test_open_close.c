/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_open_close.c
 *  @brief          unit test for alislsysinfo_open() & alislsysinfo_close().
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 11:07:45 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */
TEST_GROUP(MACReadWrite);

TEST_SETUP(MACReadWrite)
{

}

TEST_TEAR_DOWN(MACReadWrite)
{

}

TEST(MACReadWrite, ReadWriteTest)
{
	alisl_handle m_dev_hdl;
    const max_time = 10;
    int i;

    for (i = 0; i < max_time; i++) {
        CHECK(alislsysinfo_mac_read(NULL) == SYSINFO_ERR_NONE);
    }

    for (i = 0; i < max_time; i++) {
        CHECK(alislsysinfo_mac_write(NULL) == SYSINFO_ERR_NONE);
    }
}

TEST_GROUP_RUNNER(MACReadWrite)
{
	RUN_TEST_CASE(MACReadWrite, ReadWriteTest);
}

static void run_mac_read_write()
{
	RUN_TEST_GROUP(MACReadWrite);
}

static int run_group_sysinfo_open_close(int argc, char *argv[])
{
	UnityMain(argc, argv, run_mac_read_write);
	
	return 0;
}
