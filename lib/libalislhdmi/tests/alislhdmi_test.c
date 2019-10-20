/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislhdmi_test.c
 *  @brief          unit test pattern for sharelib hdmi module.
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

#include <alislhdmi.h>
#include <alislhdmi_test.h>

#include "test_open_close.c"
#include "test_switch_status.c"

/*
 *  @brief          register hdmi unit test to framework
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
void alislhdmi_test_register(struct cli_command *parent)
{
    struct cli_command *hdmi;

    hdmi = cli_register_command(parent, "1", run_group_hdmi_open_close,
                                CLI_CMD_MODE_SELF, "open close test");
    hdmi = cli_register_command(parent, "2", run_group_hdmi_switch_status,
                                CLI_CMD_MODE_SELF, "alislhdmi_set/get_switch_status");
}


