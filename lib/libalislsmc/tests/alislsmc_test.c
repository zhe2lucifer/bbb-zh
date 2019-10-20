/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved
 *
 *  @file		alislsmc_test.c
 *  @brief		unit test entry for SMC
 *
 *  @version	0.1
 *  @date		7/3/2014  18:52:49
 *
 *  @author		Peter Pan <peter.pan@alitech.com>
 *
 *  @note
 */
#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alislsmc.h>
#include <alislsmc_test.h>

#include "test_t0_transfer.c"
#include "test_read_write.c"
#include "test_reset.c"
#include "test_exist.c"
#include "test_callback.c"
#include "test_stop_start.c"
#include "test_open_close.c"

void alislsmc_test_register(struct cli_command *parent)
{
	struct cli_command *smc;

	smc = cli_register_command(parent, "1", run_group_smc_open_close,
							   CLI_CMD_MODE_SELF, "open/close");
	
	smc = cli_register_command(parent, "2", run_group_smc_stop_start,
							   CLI_CMD_MODE_SELF, "stop/start");
	
	smc = cli_register_command(parent, "3", run_group_smc_callback,
							   CLI_CMD_MODE_SELF, "test callback");

	smc = cli_register_command(parent, "4", run_group_smc_exist,
							   CLI_CMD_MODE_SELF, "test exist");

	smc = cli_register_command(parent, "5", run_group_smc_reset,
							   CLI_CMD_MODE_SELF, "reset");

	smc = cli_register_command(parent, "6", run_group_smc_t0_transfer,
							   CLI_CMD_MODE_SELF, "T0 transfer test");

	smc = cli_register_command(parent, "7", run_group_smc_read_write,
							   CLI_CMD_MODE_SELF, "read & write");
}