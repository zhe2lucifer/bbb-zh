/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislnim_test.c
 *  @brief          unit test entry for nim module
 *
 *  @Version:       1.0
 *  @date:          05/28/2014 10:43:43 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <alislcli.h>
#include <unity_fixture.h>

#include "alislnim_test.h"
#include "nim_test_autoscan.c"
#include "nim_test_lock.c"
#include "nim_test_diseqc.c"

void alislnim_test_register(struct cli_command *parent)
{
    struct cli_command *nim;

    nim = cli_register_command(parent, "1", run_nim_autoscan,
                               CLI_CMD_MODE_SELF, "autoscan");
	
    nim = cli_register_command(parent, "2", run_nim_lock,
                               CLI_CMD_MODE_SELF, "lock frequent");
    nim = cli_register_command(parent, "3", run_nim_diseqc,
                               CLI_CMD_MODE_SELF, "diseqc");
	

}
