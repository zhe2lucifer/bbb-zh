/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alisldistest.c
 *  @brief          unit test pattern for sharelib display module.
 *
 *  @Version:       1.0
 *  @date:          04/22/2014 10:46:49 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alisltsi_test.h>

#include <alisltsi.h>

#if 1
#define SL_TSI_DEBUG_PRINT printf
#else
#define SL_TSI_DEBUG_PRINT(...)
#endif

int tsi_test_channel_id =0;
int front_id=0;
int dmx_test_id=0;

int ali_tsi_switch_nim()
{
	int ret=0;
	alisl_handle tsi_handler;
	ret=alisltsi_open(&tsi_handler,TSI_ID_M3602_0,0);
	if((0 != ret))
	{
		SL_TSI_DEBUG_PRINT("\n alisltsi_construct error ret[%d] tsi_handler[%d] \n",ret,tsi_handler);
		return -1;
	}
	
	CHECK(alisltsi_setinput(tsi_handler,front_id,0x83)!=0);
	CHECK(alisltsi_setchannel(tsi_handler,front_id,tsi_test_channel_id)!=0);
	CHECK(alisltsi_setoutput(tsi_handler,dmx_test_id,tsi_test_channel_id)!=0);
	CHECK(alisltsi_close(tsi_handler) != 0);
	return ret;
}

int ali_tsi_switch_tsg()
{
	int ret=0;
	alisl_handle tsi_handler;
	ret=alisltsi_open(&tsi_handler,TSI_ID_M3602_0,0);
	if((0 != ret))
	{
		SL_TSI_DEBUG_PRINT("\n alisltsi_construct error ret[%d] tsi_handler[%d] \n",ret,tsi_handler);
		return -1;
	}
	
	CHECK(alisltsi_setinput(tsi_handler,front_id,0x83)!=0);
	CHECK(alisltsi_setchannel(tsi_handler,front_id,tsi_test_channel_id)!=0);
	CHECK(alisltsi_setoutput(tsi_handler,dmx_test_id,tsi_test_channel_id)!=0);
	CHECK(alisltsi_close(tsi_handler) != 0);
	return ret;
}


TEST_GROUP(TsiOpenClose);
TEST_GROUP(tsi_switch_nim);
TEST_GROUP(tsi_switch_tsg);

TEST_SETUP(TsiOpenClose)
{
	SL_TSI_DEBUG_PRINT("TSI test start!\n");
}

TEST_SETUP(tsi_switch_nim)
{
	// 1.init tsi & dmx
	
}

TEST_SETUP(tsi_switch_tsg)
{
	// 1.init tsi & dmx
	
}

TEST_TEAR_DOWN(TsiOpenClose)
{
	SL_TSI_DEBUG_PRINT("TSI test end!\n");
}

TEST_TEAR_DOWN(tsi_switch_nim)
{
	
}

TEST_TEAR_DOWN(tsi_switch_tsg)
{
	
}

TEST(TsiOpenClose, OpenTest)
{
	int i=0;
    const int max_time = 5;
	alisl_handle tsi_handler[max_time];
	alisl_retcode sl_ret=0;
    SL_TSI_DEBUG_PRINT("max_time = %d\n", max_time);
    for (i = 0; i < max_time; i++) {
       CHECK(alisltsi_open(&tsi_handler[i],TSI_ID_M3602_0,NULL) != 0);
    }

    for (i = 0; i < max_time; i++) {
        CHECK(alisltsi_close(tsi_handler[i]) != 0);
    }
   
}

TEST(tsi_switch_nim, test)
{
    CHECK(ali_tsi_switch_nim());
}

TEST(tsi_switch_tsg, test)
{
    CHECK(ali_tsi_switch_tsg());
}


TEST_GROUP_RUNNER(TsiOpenClose)
{
	RUN_TEST_CASE(TsiOpenClose, OpenTest);
}

TEST_GROUP_RUNNER(tsi_switch_nim)
{
	RUN_TEST_CASE(tsi_switch_nim,test);
}

TEST_GROUP_RUNNER(tsi_switch_tsg)
{
	RUN_TEST_CASE(tsi_switch_tsg,test);
}

static void run_tsi_open_close()
{
	RUN_TEST_GROUP(TsiOpenClose);
}

static void run_tsi_switch_nim()
{
	RUN_TEST_GROUP(tsi_switch_nim);
}

static void run_tsi_tsg()
{
	RUN_TEST_GROUP(tsi_switch_tsg);
}

static int run_group_tsi_open_close(int argc, char *argv[])
{
	UnityMain(argc, argv, run_tsi_open_close);
	
	return 0;
}

static int run_group_tsi_nim_switch(int argc, char *argv[])
{
	CHECK(argc >= 3);
	tsi_test_channel_id =atoi(argv[0]);
	front_id=atoi(argv[1]);
	CHECK(front_id == ALISL_TSI_SPI_TSG);
	dmx_test_id=atoi(argv[2]);
	
	UnityMain(argc, argv, run_tsi_switch_nim);
	
	return 0;
}

static int run_group_tsi_tsg(int argc, char *argv[])
{
	CHECK(argc >= 3);
	tsi_test_channel_id =atoi(argv[0]);
	front_id=ALISL_TSI_SPI_TSG;
	dmx_test_id=atoi(argv[2]);
	UnityMain(argc, argv, run_tsi_tsg);
	
	return 0;
}


/*
 *  @brief          register display unit test to framework
 *
 *  @param[in]      parent      cli parent node
 *
 *  @return         void
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           04/22/2014 10:50:14 AM
 *
 *  @note
 */
void alisltsi_test_register(struct cli_command *parent)
{
    struct cli_command *tsg;
	
    tsg = cli_register_command(parent, "1", run_group_tsi_open_close,
                               CLI_CMD_MODE_SELF, "open close test");

    tsg = cli_register_command(parent, "2", run_group_tsi_nim_switch,
                               CLI_CMD_MODE_SELF, "switch to nim_x/channel_x/dmx_x");

	tsg = cli_register_command(parent, "2", run_group_tsi_tsg,
                               CLI_CMD_MODE_SELF, "switch to TSG/channel_x/dmx_x");
}


