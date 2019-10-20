/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_open_close.h
 *  @brief          unit test for alislsdec_open() & alislsdec_close().
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 05:38:01 PM
 *  @revision:      none
 *
 *  @Author:        Lisa Liu
 */
#include <alislsdec.h>
#include <alisldmx.h>
#include <alislcli.h>
#include "error.h"

TEST_GROUP(SdecOpenClose);

TEST_SETUP(SdecOpenClose)
{
}

TEST_TEAR_DOWN(SdecOpenClose)
{

}

TEST(SdecOpenClose, case1)
{
    alisl_handle subt_handle;
    alisl_handle dmx_handle;
    dmx_id_t eDmxID = DMX_ID_DEMUX0;
    dmx_channel_attr_t stDmxAttr;
    //aui_subt_start_param start_param;
    alisl_retcode ret = ERROR_NONE;
    uint32_t dmx_chan_id = 0;
    uint32_t subt_pid = 0;
    uint32_t comppage_id = 0;
    uint32_t ancillary_page_id = 0;

    /*****open subt****/
    printf("sdec open\n");
    ret = alislsdec_open(&subt_handle);
    CHECK(ERROR_NONE == ret);

    /***start dmx****/
    printf("dmx open\n");
    ret = alisldmx_open(&dmx_handle, eDmxID, 0);
    CHECK(ERROR_NONE == ret);
    /*****config dmx****/
    memset(&stDmxAttr, 0, sizeof(dmx_channel_attr_t));
    stDmxAttr.stream = DMX_STREAM_SUBTITLE;
    ret = alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_STREAM, &(dmx_chan_id));
    CHECK(ERROR_NONE == ret);
    subt_pid = 1015;
    ret = alisldmx_set_channel_pid(dmx_handle, dmx_chan_id, subt_pid);
    CHECK(ERROR_NONE == ret);
    ret = alisldmx_set_channel_attr(dmx_handle, dmx_chan_id, &(stDmxAttr));
    CHECK(ERROR_NONE == ret);
    ret = alisldmx_control_channel(dmx_handle, dmx_chan_id, DMX_CTRL_ENABLE);
    CHECK(ERROR_NONE == ret);

    /***start subt**/
    printf("start\n");
    comppage_id = 2;
    ancillary_page_id = 2;
    ret = alislsdec_start(subt_handle, comppage_id, ancillary_page_id);
    CHECK(ERROR_NONE == ret);

    /****show subt****/
    printf("show\n");
    ret = alislsdec_show(subt_handle);
    CHECK(ERROR_NONE == ret);
    usleep(10000);

    /***hide subt**/
    printf("hide\n");
    ret = alislsdec_hide(subt_handle);
    CHECK(ERROR_NONE == ret);
    usleep(10000);

    /***show subt**/
    printf("show\n");
    ret = alislsdec_show(subt_handle);
    CHECK(ERROR_NONE == ret);
    usleep(10000);

    /***stop subt**/
    printf("stop\n");
    ret = alislsdec_stop(subt_handle);
    CHECK(ERROR_NONE == ret);
    usleep(10000);

    /***start subt**/

    memset(&stDmxAttr, 0, sizeof(dmx_channel_attr_t));
    stDmxAttr.stream = DMX_STREAM_SUBTITLE;
    alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_STREAM, &(dmx_chan_id));
    subt_pid = 1015;
    alisldmx_set_channel_pid(dmx_handle, dmx_chan_id, subt_pid);
    alisldmx_set_channel_attr(dmx_handle, dmx_chan_id, &(stDmxAttr));
    alisldmx_control_channel(dmx_handle, dmx_chan_id, DMX_CTRL_ENABLE);
    /***start subt**/
    printf("start\n");
    comppage_id = 2;
    ancillary_page_id = 2;
    ret = alislsdec_start(subt_handle, comppage_id, ancillary_page_id);
    CHECK(ERROR_NONE == ret);

    /****show subt****/
    printf("show\n");
    ret = alislsdec_show(subt_handle);
    CHECK(ERROR_NONE == ret);
    usleep(10000);

    CHECK(ERROR_NONE == ret);
}

TEST_GROUP_RUNNER(SdecOpenClose)
{
    RUN_TEST_CASE(SdecOpenClose, case1);
}

static void run_sdec_open_close(SdecOpenClose)
{
    RUN_TEST_GROUP(SdecOpenClose);
}

static int run_group_sdec_open_close(int argc, char *argv[])
{
    UnityMain(argc, argv, run_sdec_open_close);

    return 0;
}


