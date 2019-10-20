/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislsdec_test.c
 *  @brief          unit test pattern for sharelib sdec module.
 *
 *  @Version:       1.0
 *  @date:          04/22/2014 10:46:49 AM
 *  @revision:      none
 *
 *  @Author:        Lisa Liu <lisa.liu@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alislsdec.h>
#include <alislsdec_test.h>

#include "test_open_close.c"

/*
 *  @brief          register sdec unit test to framework
 *
 *  @param[in]      parent      cli parent node
 *
 *  @return         void
 *
 *  @author         Lisa Liu <lisa.liu@alitech.com>
 *
 *  @note
 */
void alislsdec_test_register(struct cli_command *parent)
{
    struct cli_command *sdec;

    sdec = cli_register_command(parent, "1",run_group_sdec_open_close,
                                CLI_CMD_MODE_SELF, "open close test");
}


