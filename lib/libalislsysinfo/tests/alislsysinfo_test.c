/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislsysinfo_test.c
 *  @brief          unit test entry for sysinfo
 *
 *  @Version:       1.0
 *  @date:          05/09/2014 11:01:01 AM
 *  @revision:      none
 *
 *  @Author:        Demon yan <demon.yan@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alislsysinfo.h>
#include <alislsysinfo_test.h>

#include "test_open_close.c"

/*
 *  @brief          register sysinfo unit test to framework
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

void alislsysinfo_test_register(struct cli_command *parent)
{
    struct cli_command *sysinfo;

    sysinfo = cli_register_command(parent, "1", run_group_sysinfo_open_close,
                                CLI_CMD_MODE_SELF, "open close sysinfo");

}





