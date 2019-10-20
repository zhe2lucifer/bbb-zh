/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislsto_test.c
 *  @brief          unit test entry for sto
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

#include <alislstorage.h>
#include <alislsto_test.h>

#include "test_nor_flash.c"
#include "test_nand_flash.c"

/*
 *  @brief          register sto unit test to framework
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


void alislsto_test_register(struct cli_command *parent)
{
    struct cli_command *sto;

    sto = cli_register_command(parent, "1", run_group_nor_tests,
                                CLI_CMD_MODE_SELF, "test nor flash");

    sto = cli_register_command(parent, "2", run_group_nand_tests,
                                CLI_CMD_MODE_SELF, "test nand flash");
}

