/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           pltftest.c
 *  @brief          ALi share library unit test suite
 *
 *  @Version:       1.0
 *  @date:          04/04/2014 04:47:43 PM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <alisldis_test.h>
#include <alislvdec_test.h>
#include <alisldmx_test.h>
#include <alislfake_test.h>
#include <alislhdmi_test.h>
#include <alislotp_test.h>
#include <alislsoc_test.h>
#include <sltest_common.h>
#include <alislsnd_test.h>
#include <alislsysinfo_test.h>
#include <alislsto_test.h>
#include <alislsdec_test.h>
#include <alislnim_test.h>
#include <alislsmc_test.h>

extern sltest_board_type board_type;
extern sltest_frontend_type frontend_type;

static int start_play(int argc, char *argv[])
{
	int nim_cnt = 1;

	if (argc < 2) {
		printf("bad parameter!\n");
		return -1;
	}

	if (argc > 2) {
		if (!strcmp(argv[2], "nim2")) {
			nim_cnt = 2;
		}
	}
	sltest_play_stream(nim_cnt, argv[1]);
}

static int stop_play(int argc, char *argv[])
{
	sltest_stop_stream();
}

/*
 *  @brief          unit test entry
 *
 *  @param[in]      argc    paramter number
 *  @param[in]      argv    paramter list
 *
 *  @return         0
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           04/04/2014 04:48:33 PM
 *
 *  @note
 */
int main(int argc, char *argv[])
{
	struct cli_command *a;

	int select;

	printf("\n------------ Board Type ------------\n");
	printf("\n1 \t\t M3515 demo board\n");
	printf("2 \t\t M3733 demo board\n");
	printf("3 \t\t M3715 demo board\n");
	printf("\n------------------------------------\n\n");
	printf("Please select your board type: ");
	
RESELECT_BOARD:
	select = getchar();
	if ('1' == select) {
		board_type = eSL_BOARD_3515_DEMO;
	} else if ('2' == select) {
		board_type = eSL_BOARD_3733_DEMO;
	} else if ('3' == select) {
		board_type = eSL_BOARD_3715_DEMO;
	} else {
		goto RESELECT_BOARD;
	}

	printf("\n----------- frontend Type ----------\n");
	printf("\n1 \t\t DVB-S2\n");
	printf("2 \t\t DVB-C\n");
	printf("\n------------------------------------\n\n");
	printf("Please select your frontend type: ");

RESELECT_NIM:
	select = getchar();
	if ('1' == select) {
		frontend_type = eSL_FRONTEND_DVBS;
	} else if ('2' == select) {
		frontend_type = eSL_FRONTEND_DVBC;
	} else {
		goto RESELECT_NIM;
	}

	cli_init();
	cli_set_hostname("aliplatform");
	cli_ignore_check_result(0);

	/** register a command to start a live stream */
	cli_register_command(NULL, "play", start_play,
						 CLI_CMD_MODE_SELF,
						 "start play a live stream, example: \"play dvbs1/dvbc1 nim2\"");

	/** register a command to stop the current stream */
	cli_register_command(NULL, "stop", stop_play,
						 CLI_CMD_MODE_SELF, "stop the playback");

	/** register display module unit test */
	a = cli_register_command(NULL, "dis", NULL,
							 CLI_CMD_MODE_SELF, "Display module test");
	alisldis_test_register(a);

	/** register vdec module unit test */
	a = cli_register_command(NULL, "vdec", NULL,
							 CLI_CMD_MODE_SELF, "VDEC module test");
	alislvdec_test_register(a);

	/** register dmx  module unit test */
	a = cli_register_command(NULL, "dmx", NULL,
							 CLI_CMD_MODE_SELF, "DMX module test");
	alisldmx_test_register(a);

	/** register otp module unit test */
	a = cli_register_command(NULL, "otp", NULL,
							 CLI_CMD_MODE_SELF, "OTP module test");
	alislotp_test_register(a);

	/** register fake module unit test */
	a = cli_register_command(NULL, "fake", NULL,
							 CLI_CMD_MODE_SELF, "Fake module test");
	alislfake_test_register(a);

	/** register sysinfo  module unit test */
	a = cli_register_command(NULL, "sysinfo", NULL,
							 CLI_CMD_MODE_SELF, "Sysinfo module test");
	alislsysinfo_test_register(a);

	/** register hdmi module unit test */
	a = cli_register_command(NULL, "hdmi", NULL,
							 CLI_CMD_MODE_SELF, "HDMI module test");
	alislhdmi_test_register(a);

	/** register snd  module unit test */
	a = cli_register_command(NULL, "snd", NULL,
							 CLI_CMD_MODE_SELF, "SND module test");
	alislsnd_test_register(a);

	/** register soc module unit test */
	a = cli_register_command(NULL, "soc", NULL,
							 CLI_CMD_MODE_SELF, "SOC module test");
	alislsoc_test_register(a);

	/** register storage module unit test */
	a = cli_register_command(NULL, "sto", NULL,
							 CLI_CMD_MODE_SELF, "Storage module test");
	alislsto_test_register(a);
	/** register sdec module unit test */
	a = cli_register_command(NULL, "sdec", NULL,
							 CLI_CMD_MODE_SELF, "sdec module test");
	alislsdec_test_register(a);

	/** register snd  module unit test */
	a = cli_register_command(NULL, "vbi", NULL,
							 CLI_CMD_MODE_SELF, "VBI module test");
	alislvbi_test_register(a);

	/** register nim module unit test */
	a = cli_register_command(NULL, "nim", NULL,
							 CLI_CMD_MODE_SELF, "NIM module test");
	alislnim_test_register(a);

	/** register smc module unit test */
	a = cli_register_command(NULL, "smc", NULL,
							 CLI_CMD_MODE_SELF, "SMC module test");
	alislsmc_test_register(a);

	/** register tsg module unit test */
	a = cli_register_command(NULL, "tsg", NULL,
							 CLI_CMD_MODE_SELF, "TSG module test");
	alisltsg_test_register(a);


	cli_loop();

	stop_play(1, NULL);
	sltest_display_stop();

	return 0;
}


