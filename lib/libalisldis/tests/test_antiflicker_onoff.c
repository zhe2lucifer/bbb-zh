/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file           test_antiflicker_onoff.c
 *  @brief      unit test for antiflicker on/off
 *
 *  @version        0.1
 *  @date       5/8/2014  15:10:21
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
static alisl_handle hd_hdl;
static alisl_handle sd_hdl;

TEST_GROUP(DisAntiFlicker);

TEST_SETUP(DisAntiFlicker)
{
    alisldis_open(DIS_HD_DEV, &hd_hdl);
    alisldis_open(DIS_SD_DEV, &sd_hdl);
}

TEST_TEAR_DOWN(DisAntiFlicker)
{
    alisldis_close(hd_hdl);
    alisldis_close(sd_hdl);
}

TEST(DisAntiFlicker, AntiFlicker)
{
    int i = 10, onoff;

    for (i = 10; i > 0; i --) {
        onoff = i % 2;
        CHECK(ERROR_NONE == alisldis_anti_flicker_onoff(hd_hdl, onoff));
		usleep(100 * 1000);
    }
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("The picture is normal?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(DisAntiFlicker)
{
    RUN_TEST_CASE(DisAntiFlicker, AntiFlicker);
}

static void run_dis_anti_flicker()
{
    RUN_TEST_GROUP(DisAntiFlicker);
}

static int run_group_dis_anti_flicker(int argc,
                                      char *argv[])
{
    UnityMain(argc,
              argv,
              run_dis_anti_flicker);

    return 0;
}
