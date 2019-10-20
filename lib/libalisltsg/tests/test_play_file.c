/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_play_file.c
 *  @brief
 *
 *  @Version:       1.0
 *  @date:          08/19/2014 02:52:53 PM
 *  @revision:      none
 *
 *  @Author:       Vedic Fu <vedic.fu@alitech.com>
 */
 
#include <sltest_common.h>
#include <alisldis.h>
#include <alislavsync.h>
#define PLAY_SIZE 		(1024*47)

	 
static alisl_handle tsg_handle;
static alisl_handle dis_hd_handle;
static alisl_handle dis_sd_handle;
static sltest_handle tsg_test_handle;
static sltest_tsg_config tsg_test_config;
static alisl_handle avsync_handle;
/*
enum tsg_id e_tsgid=TSG_ID_M36_0;
static test_tsi_config tsi_config={TSI_ID_M3602_0,ALISL_TSI_SPI_TSG,0x83,ALISL_TSI_TS_A,ALISL_TSI_DMX_3};
static test_frequency_info frequency_info={"tsg_stream.ts",NULL,NULL,2111,
										2121,4097,VDEC_DECODER_MPEG,SND_TYPE_MPEG2,NULL};
//char *play_file_path = "../share/tsg_stream.ts";*/



TEST_GROUP(TSGPlayFile);

TEST_SETUP(TSGPlayFile)
{
	CHECK(!sltest_get_tsg_config(&tsg_test_config));
	CHECK(!alisltsg_open(&tsg_handle, tsg_test_config.tsi_config.tsi_id, NULL));
    CHECK(!sltest_tsi_start(&(tsg_test_handle.tsi_hdl),&tsg_test_config.tsi_config));
	CHECK(!sltest_dmx_start(&(tsg_test_handle.dmx_hdl),tsg_test_config.dmxID,&tsg_test_config.freq_config));	
	CHECK(!alislavsync_open(&avsync_handle));
	CHECK(!alislavsync_start(avsync_handle));
	CHECK(!sltest_dis_start(&dis_hd_handle,&dis_sd_handle));	
	CHECK(!sltest_snd_start(&(tsg_test_handle.snd_hdl),30,tsg_test_config.freq_config.a_type));
	CHECK(!sltest_vdec_start(&(tsg_test_handle.vdec_hdl),dis_sd_handle,tsg_test_config.freq_config.v_type));
	CHECK(!alislavsync_set_av_sync_mode(avsync_handle,AVSYNC_AUDIO));//very important put last
}

TEST_TEAR_DOWN(TSGPlayFile)
{
	CHECK(!alisltsg_close(&tsg_handle));	
    CHECK(!sltest_tsi_stop(tsg_test_handle.tsi_hdl));
    CHECK(!sltest_dmx_stop(tsg_test_handle.dmx_hdl));
    CHECK(!alislavsync_stop(avsync_handle));
    CHECK(!alislavsync_close(avsync_handle));
    CHECK(!sltest_dis_stop(dis_hd_handle));
    CHECK(!sltest_dis_stop(dis_sd_handle));
    CHECK(!sltest_snd_stop(tsg_test_handle.snd_hdl));	
    CHECK(!sltest_vdec_stop(tsg_test_handle.vdec_hdl));

}

TEST(TSGPlayFile, case2)
{	
	
	int ret=0;
	void * addr;
	unsigned long byte_cnt=0;
	unsigned long pkt_empty;
	unsigned long len=0;

	// 1.open ts file
	FILE *ts_fp = fopen(tsg_test_config.tsg_stream_path,"r");
	if(NULL == ts_fp)
	{
		printf("file[%s] opened fail!\n",tsg_test_config.tsg_stream_path);
		CHECK(0);
	}
	// 2.mallock buffer
	addr = (void *)malloc(PLAY_SIZE);
	if(NULL == addr)
	{
		printf("fail ali_app_create_ts to malloc for addr!\n");
		CHECK(0);
	}
	memset(addr,0,PLAY_SIZE);
	
	// 3.set TSG clock
	CHECK(!alisltsg_set_clkasync(tsg_handle,0x09));

	
	while(1)
	{
		len = fread(addr,1024,47,ts_fp);
		if(len<1)
			break;
		len = len*1024;
		CHECK(!alisltsg_check_remain_buf(tsg_handle,(unsigned int*)&byte_cnt));		
		alisldmx_ioctl(tsg_test_handle.dmx_hdl,DMX_IOCMD_GET_FREE_BUF_LEN,(unsigned long)&pkt_empty);
		printf("========================DMX_IOCMD_GET_FREE_BUF_LEN pkt_empty[%d]\n",pkt_empty);
		while(pkt_empty <((len+byte_cnt)/188))
		{
			usleep(50000);
			printf("DMX_IOCMD_GET_FREE_BUF_LEN pkt_empty[%d]\n",pkt_empty);
			alisldmx_ioctl(tsg_test_handle.dmx_hdl,DMX_IOCMD_GET_FREE_BUF_LEN,(unsigned long)&pkt_empty);
			CHECK(!alisltsg_check_remain_buf(tsg_handle,(unsigned int*)&byte_cnt));
		}
		
		CHECK(!alisltsg_copydata(tsg_handle, addr, len));
	}
	printf("----------   TSG play over-------------\n");
	if(0 != fclose(ts_fp)) CHECK(0);
	if(addr) free(addr);
}

TEST_GROUP_RUNNER(TSGPlayFile)
{
	RUN_TEST_CASE(TSGPlayFile, case2);
}

static void run_tsg_play_file()
{
	RUN_TEST_GROUP(TSGPlayFile);
}

static int run_group_tsg_play_file(int argc, char *argv[])
{
	UnityMain(argc, argv, run_tsg_play_file);
	
	return 0;
}

