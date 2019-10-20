/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file       test_set_output.c
 *  @brief      unit test for set vdec output mode
 *
 *  @version    0.1
 *  @date       5/9/2014  15:18:5
 *
 *  @author     Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
#include <sltest_common.h>
#include <alisldis.h>

static alisl_handle vdec_hdl;
static sltest_config vdec_test_config;
static sltest_handle vdec_test_hdl;

TEST_GROUP(VDECOutput);

TEST_SETUP(VDECOutput)
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

TEST_TEAR_DOWN(VDECOutput)
{
    CHECK(!sltest_nim_stop(vdec_test_hdl.nim_hdl));
    CHECK(!sltest_tsi_stop(vdec_test_hdl.tsi_hdl));
    CHECK(!sltest_dmx_stop(vdec_test_hdl.dmx_hdl));
    CHECK(!sltest_snd_stop(vdec_test_hdl.snd_hdl));
    CHECK(!sltest_display_stop());
    CHECK(ERROR_NONE == alislvdec_close(vdec_hdl));
}

TEST(VDECOutput, case1)
{
    struct vdec_rect src_rect, dst_rect;
    struct vdec_out_setting out_setting;
	alisl_handle dis_hdl;

	/** start play with preview */
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    src_rect.w = 720;
    src_rect.h = 2880;
    dst_rect.w = 360;
    dst_rect.h = 1440;
    CHECK(ERROR_NONE == alislvdec_set_output_rect(vdec_hdl, &src_rect, &dst_rect));
    CHECK(ERROR_NONE == alislvdec_set_decoder(vdec_hdl,
                                                  vdec_test_config.freq_config.v_type,
                                                  1));
    CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Video start with 1/4 preview ?",
                                             'y',
                                             'n'));

	/** deview to full screen */
    dst_rect.w = 720;
    dst_rect.h = 2880;
    CHECK(ERROR_NONE == alislvdec_set_output_rect(vdec_hdl, &src_rect, &dst_rect));
	CHECK(ERROR_NONE == alisldis_open(DIS_HD_DEV, &dis_hdl));
    CHECK(ERROR_NONE == alisldis_get_tvsys(dis_hdl, &(out_setting.out_tvsys),
                                               &(out_setting.progressive)));
	CHECK(ERROR_NONE == alisldis_close(dis_hdl));
	out_setting.out_mode = VDEC_OUT_FULLVIEW;
	out_setting.smooth_switch_en = 1;
    CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));
    CHECK(ERROR_NONE == alislvdec_set_output(vdec_hdl, &out_setting, NULL, NULL));
	CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Smooth deview to fullscreen ?",
                                             'y',
                                             'n'));
	/** switch to preview mode */
    dst_rect.w = 360;
    dst_rect.h = 1440;
	CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));
    CHECK(ERROR_NONE == alislvdec_set_output_rect(vdec_hdl, &src_rect, &dst_rect));
	out_setting.out_mode = VDEC_OUT_PREVIEW;
	CHECK(ERROR_NONE == alislvdec_set_output(vdec_hdl, &out_setting, NULL, NULL));
	CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Smooth deview to 1/4 preview mode ?",
                                             'y',
                                             'n'));

	/** switch to preview mode */
    dst_rect.w = 720;
    dst_rect.h = 2880;
	CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));
    CHECK(ERROR_NONE == alislvdec_set_output_rect(vdec_hdl, &src_rect, &dst_rect));
	out_setting.out_mode = VDEC_OUT_FULLVIEW;
	CHECK(ERROR_NONE == alislvdec_set_output(vdec_hdl, &out_setting, NULL, NULL));
	CHECK(ERROR_NONE == alislvdec_start(vdec_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Smooth deview to fullscreen ?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alislvdec_stop(vdec_hdl, 0, 0));
}


TEST_GROUP_RUNNER(VDECOutput)
{
    RUN_TEST_CASE(VDECOutput, case1);
}

static void run_vdec_set_output()
{
    RUN_TEST_GROUP(VDECOutput);
}

static int run_group_vdec_set_output(int argc, char *argv[])
{
    UnityMain(argc, argv, run_vdec_set_output);

    return 0;
}
