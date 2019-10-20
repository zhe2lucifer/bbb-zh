/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux Team(alitech.com).
 *  All rights reserved 
 *
 *  @file			alislsnd_test.c
 *  @brief		test ali share libray unit test
 *
 *  @version		1.0
 *  @date		5/12/2014 
 *
 *  @author		Andy yu <Andy.yu@alitech.com>
 *
 *  @note
 */



#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <sltest_common.h>

#include <alislvbi.h>
#include <alislvbi_test.h>
#include "vbi_test_start_stop.c"

void alislvbi_test_register(struct cli_command *parent)
{
    struct cli_command *vbi;

    vbi = cli_register_command(parent, "1", run_group_vbi_start_stop,
                               CLI_CMD_MODE_SELF, "vbi start stop test");

}

