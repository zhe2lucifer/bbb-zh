/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislfake_test.c
 *  @brief          test alislfake module code
 *
 *  @Version:       1.0
 *  @date:          04/30/2014 10:45:07 AM
 *  @revision:      none
 *
 *  @Author:        Vedic Fu <vedic.fu@alitech.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alislfake.h>
#include <alislfake_test.h>
#include <alipltfretcode.h>


TEST_GROUP(ALIFAKE);

TEST_SETUP(ALIFAKE)
{

}

TEST_TEAR_DOWN(ALIFAKE)
{

}

TEST(ALIFAKE, ALIFAKETEST)
{
    unsigned int i;
    for(i = 0; i < 5; i++)
    {
        CHECK(ERROR_NONE == alislfake_get_tick("alislfaketest", true,
                                                       ALISLFAKE_GET_TICK_IN));
    }

    for(i = 0; i < 5; i++)
    {
        CHECK(ERROR_NONE == alislfake_get_tick("alislfaketest", true,
                                                       ALISLFAKE_GET_TICK_OUT));
    }
    CHECK(ERROR_NONE == alislfake_get_tick(NULL, true,
                                                   ALISLFAKE_GET_TICK_IN));
    CHECK(ERROR_NONE != alislfake_get_tick("alislfaketest", false,
                                                   ALISLFAKE_GET_TICK_IN));
    CHECK(ERROR_NONE != alislfake_get_tick("alislfaketest", false,
                                                   ALISLFAKE_GET_TICK_OUT));
    for(i = 0; i < 5; i++)
    {
        CHECK(ERROR_NONE == alislfake_show_mm_ddr());
    }

    for(i = 0; i < 5; i++)
    {
        CHECK(ERROR_NONE == alislfake_show_mem());
    }

    for(i = 0; i < 5; i++)
    {
        CHECK(ERROR_NONE == alislfake_show_stack_pid(1));
    }


    CHECK(ERROR_NONE == alislfake_show_stack_all());
}

TEST_GROUP_RUNNER(ALIFAKE)
{
    RUN_TEST_CASE(ALIFAKE, ALIFAKETEST);
}

static void run_fake_test()
{
    RUN_TEST_GROUP(ALIFAKE);
}

static int run_group_fake_test(int argc, char *argv[])
{
    UnityMain(argc, argv, run_fake_test);

    return 0;
}


void alislfake_test_register(struct cli_command *parent)
{
    struct cli_command *fake;

    fake = cli_register_command(parent, "1", run_group_fake_test,
                                CLI_CMD_MODE_SELF, "fake all test");
}


