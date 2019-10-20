/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alisldistest.c
 *  @brief          unit test pattern for sharelib display module.
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

#include <alisldis.h>
#include <alisldis_test.h>

#include "test_open_close.c"
#include "test_set_aspect_ratio.c"
#include "test_set_enhance.c"
#include "test_set_bg.c"
#include "test_set_tvsys.c"
#include "test_win_onoff.c"
#include "test_zoom.c"
#include "test_get_screen_rect.c"
#include "test_get_mp_info.c"
#include "test_get_dis_mode.c"
#include "test_get_aspect.c"
#include "test_gma_scale.c"
#include "test_gma_global_alpha.c"
#include "test_antiflicker_onoff.c"

/*
 *  @brief          register display unit test to framework
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
void alisldis_test_register(struct cli_command *parent)
{
    struct cli_command *dis;

    dis = cli_register_command(parent, "1", run_group_dis_open_close,
                               CLI_CMD_MODE_SELF, "open close test");

    dis = cli_register_command(parent, "2", run_group_dis_set_aspectratio,
                               CLI_CMD_MODE_SELF, "set display aspectratio");

    dis = cli_register_command(parent, "3", run_group_dis_set_enhance,
                               CLI_CMD_MODE_SELF, "set display enhance");

    dis = cli_register_command(parent, "4", run_group_dis_bg,
                               CLI_CMD_MODE_SELF, "set display background color");

    dis = cli_register_command(parent, "5", run_group_dis_set_tvsys,
                               CLI_CMD_MODE_SELF, "set tv system");

    dis = cli_register_command(parent, "6", run_group_dis_win_onoff,
                               CLI_CMD_MODE_SELF, "win on/off by layer");

    dis = cli_register_command(parent, "7", run_group_dis_zoom,
                               CLI_CMD_MODE_SELF, "zoom");

    dis = cli_register_command(parent, "8", run_group_dis_get_screen_rect,
                               CLI_CMD_MODE_SELF, "alisldis_get_mp_screen_rect");

    dis = cli_register_command(parent, "9", run_group_dis_get_mp_info,
                               CLI_CMD_MODE_SELF, "alisldis_get_mp_info");

    dis = cli_register_command(parent, "10", run_group_dis_get_dis_mode,
                               CLI_CMD_MODE_SELF, "alisldis_get_dis_mode");

    dis = cli_register_command(parent, "11", run_group_dis_get_aspect,
                               CLI_CMD_MODE_SELF, "alisldis_get_aspect");

    dis = cli_register_command(parent, "12", run_group_dis_gma_scale,
                               CLI_CMD_MODE_SELF, "alisldis_gma_scale");

    dis = cli_register_command(parent, "13", run_group_dis_gma_global_alpha,
                               CLI_CMD_MODE_SELF, "set global alpha for GMA");

    dis = cli_register_command(parent, "14", run_group_dis_anti_flicker,
                               CLI_CMD_MODE_SELF, "alisldis_anti_flicker_onoff");
}


