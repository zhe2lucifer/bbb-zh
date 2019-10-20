/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alisltsg_test.c
 *  @brief
 *
 *  @Version:       1.0
 *  @date:          08/19/2014 02:52:53 PM
 *  @revision:      none
 *
 *  @Author:       Vedic Fu <vedic.fu@alitech.com>
 */
#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alisltsg.h>
#include <alisltsg_test.h>

#include "test_open_close.c"
#include "test_play_file.c"

/*
 *  @brief          register vdec unit test to framework
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
void alisltsg_test_register(struct cli_command *parent)
{
    struct cli_command *tsg;

    tsg = cli_register_command(parent, "1", run_group_tsg_open_close,
                                CLI_CMD_MODE_SELF, "open close test");
	
    tsg = cli_register_command(parent, "2", run_group_tsg_play_file,
                                CLI_CMD_MODE_SELF, "play ts file test");

}


