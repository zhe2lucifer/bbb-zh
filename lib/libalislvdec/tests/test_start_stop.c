/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_start_stop.c
 *  @brief      unit test for vdec start/stop
 *
 *  @version    0.1
 *  @date       5/9/2014  10:43:41
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */

#include <sltest_common.h>

static alisl_handle vdec_hdl;
static sltest_config vdec_test_config;
static sltest_handle vdec_test_hdl;

TEST_GROUP(VDECStartStop);

TEST_SETUP(VDECStartStop)
{
    CHECK(ERROR_NONE == alislvdec_open(&vdec_hdl, 0));

    CHECK(!sltest_display_start());
    CHECK(!sltest_get_config(&vdec_test_config, 1, "dvbs1"));
    CHECK(!sltest_nim_start_dvbs(&(vdec_test_hdl.nim_hdl),
                                 &(vdec_test_config.dvbs_config),
                                 vdec_test_config.freq_config.freq,
                                 vdec_test_config.freq_config.sym_rate,
                                 0));
    CHECK(!sltest_tsi_start(&(vdec_test_hdl.tsi_hdl),
                            &(vdec_test_config.tsi_config)));
    CHECK(!sltest_dmx_start(&(vdec_test_hdl.dmx_hdl),
                            vdec_test_config.dmxID,
                            &(vdec_test_config.freq_config)));
    CHECK(!sltest_snd_start(&(vdec_test_hdl.snd_hdl),
                            30,
                            vdec_test_config.freq_config.a_type));
    CHECK(ERROR_NONE == alislvdec_set_decoder(vdec_hdl,
                                                  vdec_test_config.freq_config.v_type,
                                                  0));

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Sound start ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(VDECStartStop)
{
    CHECK(!sltest_nim_stop(vdec_test_hdl.nim_hdl));
    CHECK(!sltest_tsi_stop(vdec_test_hdl.tsi_hdl));
    CHECK(!sltest_dmx_stop(vdec_test_hdl.dmx_hdl));
    CHECK(!sltest_snd_stop(vdec_test_hdl.snd_hdl));
	CHECK(!sltest_display_stop());
    CHECK(ERROR_NONE == alislvdec_close(vdec_hdl));
}

TEST(VDECStartStop, case1)
{
    int i;

    for (i = 6; i > 0; i --) {
        if (i % 2) {
            CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 1, 1));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("video stop with black screen ?",
                                                     'y',
                                                     'n'));
        } else {
            CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("video start ?",
                                                     'y',
                                                     'n'));
        }
    }
}

TEST(VDECStartStop, case2)
{
    int i;

    for (i = 6; i > 0; i --) {
        if (i % 2) {
            CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("video stop without black screen ?",
                                                     'y',
                                                     'n'));
        } else {
            CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("video start ?",
                                                     'y',
                                                     'n'));
        }
    }
}

TEST_GROUP_RUNNER(VDECStartStop)
{
    RUN_TEST_CASE(VDECStartStop, case1);
	RUN_TEST_CASE(VDECStartStop, case2);
}

static void run_vdec_start_stop()
{
    RUN_TEST_GROUP(VDECStartStop);
}

static int run_group_vdec_start_stop(int argc, char *argv[])
{
    UnityMain(argc, argv, run_vdec_start_stop);

    return 0;
}

