/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			nim_test_diseqc.c
 *  @brief			test diseqc 
 *
 *	@Version:		1.0
 *	@date:			08/12/2014 02:20:29 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
#include <sltest_common.h>

static alisl_handle nim_hdl;
static sltest_config nim_test_config;
static unsigned char reset[3] = { 0xE0, 0x01, 0x01 };
static unsigned char commands[][4] = 	{
								{ 0xE0, 0x10, 0x38, 0xf0 },
								{ 0xE0, 0x10, 0x38, 0xf2 },
								{ 0xE0, 0x10, 0x38, 0xf1 },
								{ 0xE0, 0x10, 0x38, 0xf3 },
								{ 0xE0, 0x10, 0x38, 0xf4 },
								{ 0xE0, 0x10, 0x38, 0xf6 },
								{ 0xE0, 0x10, 0x38, 0xf5 },
								{ 0xE0, 0x10, 0x38, 0xf7 },
								{ 0xE0, 0x10, 0x38, 0xf8 },
								{ 0xE0, 0x10, 0x38, 0xfa },
								{ 0xE0, 0x10, 0x38, 0xf9 },
								{ 0xE0, 0x10, 0x38, 0xfb },
								{ 0xE0, 0x10, 0x38, 0xfc },
								{ 0xE0, 0x10, 0x38, 0xfd },
								{ 0xE0, 0x10, 0x38, 0xff },
								{ 0xE0, 0x10, 0x38, 0xfe },
								};


TEST_GROUP(NIMDiseqc);

TEST_SETUP(NIMDiseqc)
{
		CHECK(!sltest_get_config(&nim_test_config, 1, "dvbs1"));
		CHECK(!alislnim_open(&nim_hdl, nim_test_config.dvbs_config.nimID));		
}

TEST_TEAR_DOWN(NIMDiseqc)
{
	
	CHECK(!alislnim_close(nim_hdl));
}

TEST(NIMDiseqc, case3)
{
	slnim_config nim_config;
	slnim_diseqc_operate_t diseqc_param;
	int i;
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
		
	CHECK(!alislnim_set_polar(nim_hdl, (unsigned char)test_nim_config->polar));		
	CHECK(!alislnim_set_cfg(nim_hdl, &nim_config));

	for(i=0;i<16;i++)
	{
loop_check:
		memset(&diseqc_param,0,sizeof(slnim_diseqc_operate_t));
		diseqc_param.mode = SLNIM_DISEQC_MODE_BYTES;
		diseqc_param.cnt = 3;
		diseqc_param.cmd = reset;	
		CHECK(!alislnim_diseqc_operate(nim_hdl,&diseqc_param));
		
		memset(&diseqc_param,0,sizeof(slnim_diseqc_operate_t));
		diseqc_param.mode = SLNIM_DISEQC_MODE_BYTES;
		diseqc_param.cnt = 4;
		diseqc_param.cmd = commands[i];
		CHECK(!alislnim_diseqc_operate(nim_hdl,&diseqc_param));
		
		memset(&diseqc_param,0,sizeof(slnim_diseqc_operate_t));
		diseqc_param.mode = SLNIM_DISEQC_MODE_22KOFF;
		diseqc_param.cnt = 0;
		diseqc_param.cmd = NULL;
		CHECK(!alislnim_diseqc_operate(nim_hdl,&diseqc_param));
		
		memset(&diseqc_param,0,sizeof(slnim_diseqc_operate_t));
		diseqc_param.mode = commands[i][3] & 0x01 ? SLNIM_DISEQC_MODE_22KON : SLNIM_DISEQC_MODE_22KOFF;
		diseqc_param.cnt = 0;
		diseqc_param.cmd = NULL;
		CHECK(!alislnim_diseqc_operate(nim_hdl,&diseqc_param));

		printf("usage: the command is correct? y/n\n");
		if(getchar()=='y') ;
		else goto loop_check;
	}
	

	
}

TEST_GROUP_RUNNER(NIMDiseqc)
{
    RUN_TEST_CASE(NIMDiseqc, case3);
}

static void run_diseqc_group()
{
    RUN_TEST_GROUP(NIMDiseqc);
}

static int run_nim_diseqc(int argc,
                            char *argv[])
{
    UnityMain(argc,
              argv,
              run_diseqc_group);

    return 0;
}

