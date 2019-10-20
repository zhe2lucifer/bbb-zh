/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislnim_example.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 17:53:19
 *  @revision           none
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 */

/* system headers */
#include <argp.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alislnim.h>

#if 1
int main (int argc, char **argv)
{
	alisl_handle handle = 0;
	alisl_handle handle2 = 0;
	int ret = 0;
	slnim_config nim_config;
	slnim_freq_lock_t lock_param;
	unsigned int lock_flag = 0;
	unsigned int sym = 0;
	unsigned int snr = 0;
	unsigned int ber = 0;
	unsigned int agc = 0;
	unsigned int freq = 0;


	//3516 the first nim
	ret = alislnim_open(&handle,NIM_ID_M3281_0);
	if((0 != ret) || (0 == handle))
	{
		printf("open errror %s,%d\n",__FUNCTION__,__LINE__);
		return 0;
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	memset(&nim_config,0,sizeof(slnim_config));

	//config demo
	nim_config.config_dvbc.tunner_config.tuner_id = NIM_TUNNER_MXL603;
	nim_config.config_dvbc.tunner_config.agc_ref = 0x80;
	nim_config.config_dvbc.tunner_config.rf_agc_min = 0x2A;
	nim_config.config_dvbc.tunner_config.rf_agc_max = 0xBA;
	nim_config.config_dvbc.tunner_config.if_agc_max = 0xFE;
	nim_config.config_dvbc.tunner_config.if_agc_min = 0x01;
	nim_config.config_dvbc.tunner_config.tuner_crystal = 16;
	nim_config.config_dvbc.tunner_config.i2c_base_addr = 0xc2;
	nim_config.config_dvbc.tunner_config.tuner_special_config = 0x01;
	nim_config.config_dvbc.tunner_config.wtuner_if_freq = 5000;
	nim_config.config_dvbc.tunner_config.tuner_ref_divratio = 64;
	nim_config.config_dvbc.tunner_config.tuner_agc_top = 1;
	nim_config.config_dvbc.tunner_config.tuner_step_freq = 62.5;
	nim_config.config_dvbc.tunner_config.i2c_type_id = NIM_I2C_TYPE_SCB0;
	nim_config.config_dvbc.tunner_config.tuner_if_freq_J83A = 5000;
	nim_config.config_dvbc.tunner_config.tuner_if_freq_J83B = 5380;
	nim_config.config_dvbc.tunner_config.tuner_if_freq_J83C = 5000;
	nim_config.config_dvbc.demo_config.qam_mode = NIM_DVBC_MODE_J83AC | NIM_DEMO_SAMPLE_CLK_27M;

	ret = alislnim_set_cfg(handle,&nim_config);

	memset(&lock_param,0,sizeof(slnim_freq_lock_t));
	lock_param.freq = 55400;
	lock_param.sym = 6875;
	lock_param.fec = 0;
	lock_param.modulation = 8;
	ret = alislnim_freqlock(handle,&lock_param);
	if(0 != ret)
	{
		printf("alislnim_freqlock error \n");
		//return 0;
	}
	while(1)
	{
		ret = alislnim_get_lock(handle,&lock_flag);
		if(0 != ret)
		{
			printf("alislnim_get_lock error \n");
			return 0;
		}
		if(lock_flag)
		{
			break;
		}
		sleep(1);
		printf("unlock \n");
	}
	printf("ok,lock it now\n");
//	alislnim_get_sym(handle,&sym);
//	alislnim_get_ber(handle,&ber);
//	alislnim_get_agc(handle,&agc);
//	alislnim_get_snr(handle,&snr);
//	alislnim_get_freq(handle,&freq);
	printf("sym = %d,ber =%d,agc =%d,snr=%d,freq=%d\n",sym,ber,agc,snr,freq);
	alislnim_close(handle);
	return 0;
}

#else
static int32_t autoscan_callback(uint8_t status, uint8_t polar, uint32_t freq, uint32_t sym, uint8_t fec, void *usr_data)
{
	printf("%s line %d,status = %d,polar = %d,freq = %d,sym = %d,fec = %d\n",__func__,__LINE__,status,polar,freq,sym,fec);
	return 0;

}

static test_auto_scan(int handle)
{
	slnim_auto_scan_t param ={0};
	int polar = 0;

	param.callback = autoscan_callback;
	param.sfreq = 950;
	param.efreq = 2150;
	param.callback = autoscan_callback;
	alislnim_autoscan(handle,&param);
	printf("test_auto_scan end \n");

}

int main (int argc, char **argv)
{
	int handle = 0;
	int handle1 = 0;
	int handle2 = 0;
	int ret = 0;
	slnim_config nim_config;
	slnim_freq_lock_t lock_param;
	unsigned int lock_flag = 0;
	unsigned int sym = 0;
	unsigned int snr = 0;
	unsigned int ber = 0;
	unsigned int agc = 0;
	unsigned int freq = 0;
	//3516 the first nim
	ret = alislnim_open(&handle1,NIM_ID_M3503_0);
	if((0 != ret) || (0 == handle1))
	{
		printf("open errror %s,%d\n",__FUNCTION__,__LINE__);
		return 0;
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	memset(&nim_config,0,sizeof(slnim_config));
	//config demo
	nim_config.config_dvbs.demo_config.i2c_base_addr = 0xe6;
	nim_config.config_dvbs.demo_config.i2c_type_id = NIM_I2C_TYPE_SCB0;
	nim_config.config_dvbs.demo_config.QPSK_Config = 0xe9;
	//config tunner
	nim_config.config_dvbs.tunner_config.freq_high = 2200;
	nim_config.config_dvbs.tunner_config.freq_low = 900;
	nim_config.config_dvbs.tunner_config.i2c_base_addr = 0xc0;
	nim_config.config_dvbs.tunner_config.i2c_type_id = NIM_I2C_TYPE_SCB0;
	nim_config.config_dvbs.tunner_config.tuner_id = NIM_TUNNER_AV_2012;
	ret = alislnim_set_cfg(handle1,&nim_config);
	//3516 the secode nim
	ret = alislnim_open(&handle2,NIM_ID_M3501_0);
	if((0 != ret) || (0 == handle2))
	{
		printf("open errror %s,%d\n",__FUNCTION__,__LINE__);
		return 0;
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	memset(&nim_config,0,sizeof(slnim_config));
	//config demo
	nim_config.config_dvbs.demo_config.i2c_base_addr = 0x66;
	nim_config.config_dvbs.demo_config.i2c_type_id = NIM_I2C_TYPE_GPIO1;
	nim_config.config_dvbs.demo_config.QPSK_Config = 0x69;
	//config tunner
	nim_config.config_dvbs.tunner_config.freq_high = 2200;
	nim_config.config_dvbs.tunner_config.freq_low = 900;
	nim_config.config_dvbs.tunner_config.i2c_base_addr = 0xc0;
	nim_config.config_dvbs.tunner_config.i2c_type_id = NIM_I2C_TYPE_GPIO1;
	nim_config.config_dvbs.tunner_config.tuner_id = NIM_TUNNER_AV_2012;

	ret = alislnim_set_cfg(handle2,&nim_config);
	if(0 != ret)
	{
		printf("alislnim_open error \n");
		return 0;
	}
	memset(&lock_param,0,sizeof(slnim_freq_lock_t));
	lock_param.freq = 1500;
	lock_param.sym = 25000;
	lock_param.fec = 0;
	#if 1
	handle = handle1;
	#else
	handle = handle2;
	#endif
	ret = alislnim_freqlock(handle,&lock_param);
	if(0 != ret)
	{
		printf("alislnim_freqlock error \n");
		return 0;
	}
	while(1)
	{
		ret = alislnim_get_lock(handle,&lock_flag);
		if(0 != ret)
		{
			printf("alislnim_get_lock error \n");
			return 0;
		}
		if(lock_flag)
		{
			break;
		}
		sleep(1);
		printf("unlock \n");
	}
	printf("ok,lock it now\n");
	alislnim_get_sym(handle,&sym);
	alislnim_get_ber(handle,&ber);
	alislnim_get_agc(handle,&agc);
	alislnim_get_snr(handle,&snr);
	alislnim_get_freq(handle,&freq);
	printf("sym = %d,ber =%d,agc =%d,snr=%d,freq=%d\n",sym,ber,agc,snr,freq);
	test_auto_scan(handle);
	alislnim_close(handle);
	return 0;
}
#endif
