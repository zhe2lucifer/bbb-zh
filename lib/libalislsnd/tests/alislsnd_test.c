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

#include <alislsnd.h>
#include <alislsnd_test.h>

#include "snd_test_audio_start_stop.c"
#include "snd_test_audio_track.c"
#include "snd_test_audio_output_fomat.c"
#include "snd_test_audio_volume.c"
#include "snd_test_audio_sync_mode.c"
#include "snd_test_audio_mute.c"

void alislsnd_test_register(struct cli_command *parent)
{
    struct cli_command *snd;

    snd = cli_register_command(parent, "1", run_group_snd_start_stop,
                               CLI_CMD_MODE_SELF, "start  start stop test");

    snd = cli_register_command(parent, "2", run_group_snd_audio_track,
                               CLI_CMD_MODE_SELF, "audio  track test");

	snd = cli_register_command(parent, "3", run_group_snd_output_format,
						   CLI_CMD_MODE_SELF, "audio  output format test");
	
	snd = cli_register_command(parent, "4", run_group_snd_volume,
						   CLI_CMD_MODE_SELF, "audio  volume test");

	snd = cli_register_command(parent, "5", run_group_snd_av_sync_mode,
					   CLI_CMD_MODE_SELF, "audio  av sync mode test");
	
	snd = cli_register_command(parent, "6", run_group_snd_audio_mute,
					   CLI_CMD_MODE_SELF, "audio  mute test");

}





