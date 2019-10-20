/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           nim_test_autoscan.c
 *  @brief          unit test case for nim autoscan
 *
 *  @Version:       1.0
 *  @date:          05/28/2014 10:47:18 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */
#include <sltest_common.h>

static alisl_handle nim_hdl;
static sltest_config nim_test_config;
static sltest_handle nim_test_handle;

static int autoscan_callback(unsigned char status,
                             unsigned char polar,
                             unsigned int freq,
                             unsigned int sym,
                             unsigned char fec,
                             void *usr_data)
{
    printf("freq = %d, sym = %d, fec = %d, polar = %d\n",
           freq, sym, fec, polar);

    return 0;
}

TEST_GROUP(NIMAutoScan);

TEST_SETUP(NIMAutoScan)
{
    CHECK(!sltest_get_config(&nim_test_config, 1, "dvbs1"));
    CHECK(!sltest_tsi_start(&(nim_test_handle.tsi_hdl), &(nim_test_config.tsi_config)));
    CHECK(!sltest_dmx_start(&(nim_test_handle.dmx_hdl), nim_test_config.dmxID,
                            &(nim_test_config.freq_config)));
    CHECK(!alislnim_open(&nim_hdl, nim_test_config.dvbs_config.nimID));
}

TEST_TEAR_DOWN(NIMAutoScan)
{
    CHECK(!alislnim_close(nim_hdl));
	CHECK(!sltest_tsi_stop(nim_test_handle.tsi_hdl));
	CHECK(!sltest_dmx_stop(nim_test_handle.dmx_hdl));
}

TEST(NIMAutoScan, case1)
{
    slnim_config nim_config;
    slnim_freq_lock_t lock_param;
    int lock_flag = 0;
    int lock_cnt = 0;
    test_dvbs_nim_config *test_nim_config = &(nim_test_config.dvbs_config);

    memset(&nim_config, 0, sizeof(slnim_config));
    nim_config.config_dvbs.demo_config.QPSK_Config = test_nim_config->qpsk_config;
    nim_config.config_dvbs.demo_config.i2c_base_addr = test_nim_config->demod_i2c_base;
    nim_config.config_dvbs.demo_config.i2c_type_id = test_nim_config->demod_i2c_type;
    nim_config.config_dvbs.tunner_config.freq_high = test_nim_config->freq_high;
    nim_config.config_dvbs.tunner_config.freq_low = test_nim_config->freq_low;
    nim_config.config_dvbs.tunner_config.i2c_base_addr = test_nim_config->tuner_i2c_base;
    nim_config.config_dvbs.tunner_config.i2c_type_id = test_nim_config->tuner_i2c_type;
    nim_config.config_dvbs.tunner_config.tuner_id = test_nim_config->tuner_id;
    CHECK(!alislnim_set_cfg(nim_hdl, &nim_config));
    //CHECK(!alislnim_set_polar(nim_hdl, &test_nim_config->polar));

    slnim_auto_scan_t param;
    memset(&param, 0, sizeof(param));
    param.sfreq = 950;
    param.efreq = 2150;
    param.callback = autoscan_callback;
    alislnim_autoscan(nim_hdl, &param);
}

TEST_GROUP_RUNNER(NIMAutoScan)
{
    RUN_TEST_CASE(NIMAutoScan, case1);
}

static void run_autoscan_group()
{
    RUN_TEST_GROUP(NIMAutoScan);
}

static int run_nim_autoscan(int argc,
                            char *argv[])
{
    UnityMain(argc,
              argv,
              run_autoscan_group);

    return 0;
}
