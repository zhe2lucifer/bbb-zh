/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislvdectest.c
 *  @brief          unit test entry for vdec
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 11:01:01 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alislvdec.h>
#include <alislvdec_test.h>

#include "test_open_close.c"
#include "test_fill_frame.c"
#include "test_capture_frame.c"
#include "test_start_stop.c"
#include "test_set_get_decoder.c"
#include "test_set_output.c"

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
void alislvdec_test_register(struct cli_command *parent)
{
    struct cli_command *vdec;

    vdec = cli_register_command(parent, "1", run_group_vdec_open_close,
                                CLI_CMD_MODE_SELF, "open close test");

    vdec = cli_register_command(parent, "2", run_group_vdec_start_stop,
                                CLI_CMD_MODE_SELF, "alislvdec_start/stop");

    vdec = cli_register_command(parent, "3", run_group_vdec_set_get_decoder,
                                CLI_CMD_MODE_SELF, "alislvdec_set/get_decoder");

    vdec = cli_register_command(parent, "4", run_group_vdec_set_output,
                                CLI_CMD_MODE_SELF, "alislvdec_set_output");

    vdec = cli_register_command(parent, "10", run_group_vdec_fill_frame,
                                CLI_CMD_MODE_SELF, "alislvdec_fill_frame");

    vdec = cli_register_command(parent, "11", run_group_vdec_capture_frame,
                                CLI_CMD_MODE_SELF, "alislvdec_capture_frame");
}





