/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_switch_status.c
 *  @brief      unit test for hdmi switch status
 *
 *  @version    0.1
 *  @date       5/14/2014  17:14:1
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note       This file provide some test cases to test the switch function\n
 *              of HDMI output.
 */
#include <alislhdmi.h>
#include <alislcli.h>
#include <sltest_common.h>

static alisl_handle hdmi_dev = NULL;
static sltest_config hdmi_test_config;
static sltest_handle hdmi_test_hdl;
static alisl_handle dis_hd_hdl = NULL;
static alisl_handle dis_sd_hdl = NULL;

TEST_GROUP(HDMIOnOff);

TEST_SETUP(HDMIOnOff)
{
    unsigned char hdcp_key[286] = {0};

    CHECK(!sltest_get_config(&hdmi_test_config, 1, "dvbs1"));
    CHECK(!sltest_nim_start_dvbs(&(hdmi_test_hdl.nim_hdl),
                                 &(hdmi_test_config.dvbs_config),
                                 hdmi_test_config.freq_config.freq,
                                 hdmi_test_config.freq_config.sym_rate,
                                 0));
    CHECK(!sltest_tsi_start(&(hdmi_test_hdl.tsi_hdl),
                            &(hdmi_test_config.tsi_config)));
    CHECK(!sltest_dmx_start(&(hdmi_test_hdl.dmx_hdl),
                            hdmi_test_config.dmxID,
                            &(hdmi_test_config.freq_config)));
    CHECK(!sltest_snd_start(&(hdmi_test_hdl.snd_hdl),
                            30,
                            hdmi_test_config.freq_config.a_type));
    CHECK(!sltest_vdec_start(&(hdmi_test_hdl.vdec_hdl),
                             NULL,
                             hdmi_test_config.freq_config.v_type));
    CHECK(!sltest_hdmi_start(&hdmi_dev, hdcp_key));
    CHECK(!sltest_dis_start(&dis_hd_hdl, &dis_sd_hdl));

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Playing start ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(HDMIOnOff)
{
    CHECK(!sltest_nim_stop(hdmi_test_hdl.nim_hdl));
    CHECK(!sltest_tsi_stop(hdmi_test_hdl.tsi_hdl));
    CHECK(!sltest_dmx_stop(hdmi_test_hdl.dmx_hdl));
    CHECK(!sltest_snd_stop(hdmi_test_hdl.snd_hdl));
    CHECK(!sltest_vdec_stop(hdmi_test_hdl.vdec_hdl));
    CHECK(!sltest_dis_stop(dis_hd_hdl));
    CHECK(!sltest_hdmi_stop(hdmi_dev));
}

TEST(HDMIOnOff, case1)
{
    int i = 6;
    unsigned int status;

    for (i = 4; i > 0; i --) {
        if (i % 2) {
            CHECK(HDMI_ERROR_NONE == alislhdmi_set_switch_status(hdmi_dev, 1));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("HDMI output ?",
                                                     'y',
                                                     'n'));
            CHECK(HDMI_ERROR_NONE == alislhdmi_get_switch_status(hdmi_dev, &status));
            CHECK(1 == status);
        } else {
            CHECK(HDMI_ERROR_NONE == alislhdmi_set_switch_status(hdmi_dev, 0));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("No HDMI output ?",
                                                     'y',
                                                     'n'));
            CHECK(HDMI_ERROR_NONE == alislhdmi_get_switch_status(hdmi_dev, &status));
            CHECK(0 == status);
        }
    }

    for (i = 4; i > 0; i --) {
        if (i % 2) {
            CHECK(HDMI_ERROR_NONE == alislhdmi_set_audio_status(hdmi_dev, 1));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("HDMI audio output ?",
                                                     'y',
                                                     'n'));
            CHECK(HDMI_ERROR_NONE == alislhdmi_get_audio_status(hdmi_dev, &status));
            CHECK(1 == status);
        } else {
            CHECK(HDMI_ERROR_NONE == alislhdmi_set_audio_status(hdmi_dev, 0));
            TEST_ASSERT_BYTES_EQUAL('y',
                                    cli_check_result("No HDMI audio output ?",
                                                     'y',
                                                     'n'));
            CHECK(HDMI_ERROR_NONE == alislhdmi_get_audio_status(hdmi_dev, &status));
            CHECK(0 == status);
        }
    }
}

TEST_GROUP_RUNNER(HDMIOnOff)
{
    RUN_TEST_CASE(HDMIOnOff, case1);
}

static void run_hdmi_switch_status()
{
    RUN_TEST_GROUP(HDMIOnOff);
}

static int run_group_hdmi_switch_status(int argc, char *argv[])
{
    UnityMain(argc, argv, run_hdmi_switch_status);

    return 0;
}