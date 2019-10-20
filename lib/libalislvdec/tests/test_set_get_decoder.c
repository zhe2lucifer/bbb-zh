/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_set_get_decoder.c
 *  @brief      unit test for set and get vdec decoder type
 *
 *  @version    0.1
 *  @date       5/9/2014  14:18:24
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
#include <sltest_common.h>

static alisl_handle vdec_hdl;
static sltest_config vdec_test_config;
static sltest_handle vdec_test_hdl;

TEST_GROUP(VDECDecoder);

TEST_SETUP(VDECDecoder)
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

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Sound start ?",
                                             'y',
                                             'n'));
}

TEST_TEAR_DOWN(VDECDecoder)
{
    CHECK(!sltest_nim_stop(vdec_test_hdl.nim_hdl));
    CHECK(!sltest_tsi_stop(vdec_test_hdl.tsi_hdl));
    CHECK(!sltest_dmx_stop(vdec_test_hdl.dmx_hdl));
    CHECK(!sltest_snd_stop(vdec_test_hdl.snd_hdl));
	CHECK(!sltest_display_stop());
    CHECK(ERROR_NONE == alislvdec_close(vdec_hdl));
}

TEST(VDECDecoder, case1)
{
	enum vdec_decoder_type decoder_type;

	/** set the wrong video type */
	CHECK(ERROR_NONE == alislvdec_set_decoder(vdec_hdl,
                                                  VDEC_DECODER_AVC,
                                                  1));
    CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
	TEST_ASSERT_BYTES_EQUAL('n',
                            cli_check_result("Wrong video type, video start ?",
                                             'y',
                                             'n'));
	CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));

	/** set the right video type */
    CHECK(ERROR_NONE == alislvdec_set_decoder(vdec_hdl,
                                                  vdec_test_config.freq_config.v_type,
                                                  1));
    CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Video start ?",
                                             'y',
                                             'n'));
	CHECK(ERROR_NONE == alislvdec_get_decoder(vdec_hdl, &decoder_type));
	CHECK(VDEC_DECODER_MPEG == decoder_type);
	
	CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));
}


TEST_GROUP_RUNNER(VDECDecoder)
{
    RUN_TEST_CASE(VDECDecoder, case1);
}

static void run_vdec_set_get_decoder()
{
    RUN_TEST_GROUP(VDECDecoder);
}

static int run_group_vdec_set_get_decoder(int argc, char *argv[])
{
    UnityMain(argc, argv, run_vdec_set_get_decoder);

    return 0;
}
