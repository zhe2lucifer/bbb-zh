/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved 
 *
 *  @file		nim_test_lock.c
 *  @brief		unit test case for nim lock frequency
 *
 *  @version	0.1
 *  @date		5/28/2014  13:54:4
 *
 *  @author		Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
#include <sltest_common.h>

static alisl_handle nim_hdl;
static sltest_config nim_test_config;
extern sltest_frontend_type frontend_type;

TEST_GROUP(NIMLock);

TEST_SETUP(NIMLock)
{
	if(frontend_type == eSL_FRONTEND_DVBS)
	{
		CHECK(!sltest_get_config(&nim_test_config, 1, "dvbs1"));
		CHECK(!alislnim_open(&nim_hdl, nim_test_config.dvbs_config.nimID));
	}
	else if(frontend_type == eSL_FRONTEND_DVBC)
	{
		CHECK(!sltest_get_config(&nim_test_config, 1, "dvbc1"));
		CHECK(!alislnim_open(&nim_hdl, nim_test_config.dvbc_config.nimID));
	}
	else
	{
		printf("invalid parameter\n");
		CHECK(0);
	}
}

TEST_TEAR_DOWN(NIMLock)
{
	CHECK(!alislnim_close(nim_hdl));
}

TEST(NIMLock, case1)
{
	slnim_config nim_config;
    slnim_freq_lock_t lock_param;
    int lock_flag = 0;
    int lock_cnt = 0;

    memset(&nim_config, 0, sizeof(slnim_config));
	if(frontend_type == eSL_FRONTEND_DVBS)
	{
		test_dvbs_nim_config *test_nim_config = &(nim_test_config.dvbs_config);
		
		nim_config.config_dvbs.demo_config.QPSK_Config = test_nim_config->qpsk_config;
		nim_config.config_dvbs.demo_config.i2c_base_addr = test_nim_config->demod_i2c_base;
		nim_config.config_dvbs.demo_config.i2c_type_id = test_nim_config->demod_i2c_type;
		nim_config.config_dvbs.tunner_config.freq_high = test_nim_config->freq_high;
		nim_config.config_dvbs.tunner_config.freq_low = test_nim_config->freq_low;
		nim_config.config_dvbs.tunner_config.i2c_base_addr = test_nim_config->tuner_i2c_base;
		nim_config.config_dvbs.tunner_config.i2c_type_id = test_nim_config->tuner_i2c_type;
		nim_config.config_dvbs.tunner_config.tuner_id = test_nim_config->tuner_id;

		
		CHECK(!alislnim_set_polar(nim_hdl, (unsigned char)test_nim_config->polar));		
		CHECK(!alislnim_set_cfg(nim_hdl, &nim_config));
	}
	else if(frontend_type == eSL_FRONTEND_DVBC)
	{
		test_dvbc_nim_config *test_nim_config = &(nim_test_config.dvbc_config);
		
		nim_config.config_dvbc.demo_config.qam_mode 		= test_nim_config->mode_value | test_nim_config->nim_clk;
		nim_config.config_dvbc.tunner_config.i2c_base_addr	= test_nim_config->tuner_i2c_base;
		nim_config.config_dvbc.tunner_config.i2c_type_id	= test_nim_config->tuner_i2c_type;
		nim_config.config_dvbc.tunner_config.tuner_id		= test_nim_config->tuner_id;
		nim_config.config_dvbc.tunner_config.rf_agc_max 	= test_nim_config->rf_agc_max;
		nim_config.config_dvbc.tunner_config.rf_agc_min 	= test_nim_config->rf_agc_min;
		nim_config.config_dvbc.tunner_config.if_agc_max 	= test_nim_config->if_agc_max;
		nim_config.config_dvbc.tunner_config.if_agc_min 	= test_nim_config->if_agc_min;
		nim_config.config_dvbc.tunner_config.agc_ref		= test_nim_config->agc_ref;
		nim_config.config_dvbc.tunner_config.tuner_crystal	= test_nim_config->tuner_crystal;
		nim_config.config_dvbc.tunner_config.chip			= test_nim_config->chip;
		nim_config.config_dvbc.tunner_config.tuner_special_config	= test_nim_config->tuner_special_config;
		nim_config.config_dvbc.tunner_config.tuner_ref_divratio = test_nim_config->tuner_ref_divratio;
		nim_config.config_dvbc.tunner_config.wtuner_if_freq = test_nim_config->wtuner_if_freq;
		nim_config.config_dvbc.tunner_config.tuner_agc_top	= test_nim_config->tuner_agc_top;
		nim_config.config_dvbc.tunner_config.tuner_step_freq	= test_nim_config->tuner_step_freq;
		nim_config.config_dvbc.tunner_config.tuner_if_freq_J83A = test_nim_config->tuner_if_freq_J83A;
		nim_config.config_dvbc.tunner_config.tuner_if_freq_J83B = test_nim_config->tuner_if_freq_J83B;
		nim_config.config_dvbc.tunner_config.tuner_if_freq_J83C = test_nim_config->tuner_if_freq_J83C;
		nim_config.config_dvbc.tunner_config.tuner_if_J83AC_type	= test_nim_config->tuner_if_J83AC_type;

		CHECK(!alislnim_set_cfg(nim_hdl, &nim_config));
		
	}
	else
	{
		printf("error.................\n");
		CHECK(0);
	}

	

    memset(&lock_param, 0, sizeof(slnim_freq_lock_t));
    lock_param.freq = nim_test_config.freq_config.freq;
    lock_param.sym = nim_test_config.freq_config.sym_rate;
    lock_param.fec = 0;	
	lock_param.modulation = nim_test_config.freq_config.modulation; 
    CHECK(!alislnim_freqlock(nim_hdl, &lock_param));

	int b_locked = 0;
    while(1) {
        if (alislnim_get_lock(nim_hdl, &lock_flag)) {
            SLTEST_DEBUG("alislnim get log error");
            break;
        }
        if (lock_flag) {
            SLTEST_DEBUG("lock success!");
			b_locked = 1;
            break;
        }
        ++ lock_cnt;
        sleep(1);
        SLTEST_DEBUG("unlock");

        if (lock_cnt > 10) {
            break;;
        }
    }
	CHECK(b_locked);
}

TEST_GROUP_RUNNER(NIMLock)
{
    RUN_TEST_CASE(NIMLock, case1);
}

static void run_lock_group()
{
    RUN_TEST_GROUP(NIMLock);
}

static int run_nim_lock(int argc,
                            char *argv[])
{
    UnityMain(argc,
              argv,
              run_lock_group);

    return 0;
}
