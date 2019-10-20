/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			alislsoc_test.c
 *  @brief			test soc module
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 01:07:30 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <alislcli.h>
#include <unity_fixture.h>

#include <alislsoc.h>
#include <alislsoc_test.h>

#include "test_read_write.c" 
#include "test_ic_id.c"
#include "test_ic_clk.c"
#include "test_ic_device_num.c"
#include "test_device_is_enable.c"
#include "test_device_set_enable.c"
#include "test_bonding_reboot.c"
#include "test_ioctl_read_write.c"
 
 
 
void alislsoc_test_register(struct cli_command *parent)
{
	struct cli_command *soc;

	soc = cli_register_command(parent, "1", run_group_soc_read_write,
							CLI_CMD_MODE_SELF, "soc read write test");

	soc = cli_register_command(parent, "2", run_group_soc_id,
							CLI_CMD_MODE_SELF, "soc some id test");
	soc = cli_register_command(parent, "3", run_group_soc_clk,
							CLI_CMD_MODE_SELF, "cpu dram clock test");

	soc = cli_register_command(parent, "4", run_group_soc_device_num,
							CLI_CMD_MODE_SELF, "device and it's number test");

	soc = cli_register_command(parent, "5", run_group_soc_device_is_enable,
							CLI_CMD_MODE_SELF, "judge device is enable test");

	soc = cli_register_command(parent, "6", run_group_soc_device_set_enable,
							CLI_CMD_MODE_SELF, "enable device test");

	soc = cli_register_command(parent, "7", run_group_soc_reboot,
							CLI_CMD_MODE_SELF, "bonding reboot test");

	soc = cli_register_command(parent, "8", run_group_soc_ioctl_read_write,
							CLI_CMD_MODE_SELF, "read write via ioctl test");



}


