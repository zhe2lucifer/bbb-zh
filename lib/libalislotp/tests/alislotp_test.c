/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislotp_test.c
 *  @brief
 *
 *  @Version:       1.0
 *  @date:          04/25/2014 02:52:53 PM
 *  @revision:      none
 *
 *  @Author:       Vedic Fu <vedic.fu@alitech.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <alislcli.h>
#include <unity_fixture.h>

#include <alislotp.h>
#include <alislotp_test.h>



TEST_GROUP(OTPReadWrite);

TEST_SETUP(OTPReadWrite)
{

}

TEST_TEAR_DOWN(OTPReadWrite)
{

}

TEST(OTPReadWrite, ReadWriteTest)
{
    /* M3515 offset+len < 0x400(1024)
	*  allow test mem 0x350~0x400
	*
	* M3921  offset+len < 0x400(1024)
	* allow test mem 0x358~0x36c
	*/
	unsigned char i=0;
	unsigned char *pbuf=NULL;
	unsigned char buf_read[16] = {0};
	unsigned char buf_write[16] = {0};
	memset(buf_read,0,16);
	memset(buf_write,0x22,16);
	
    CHECK(alislotp_op_write(1024, buf_write, 4));
    CHECK(alislotp_op_write(0, buf_write, 0));
    CHECK(alislotp_op_read(1024, buf_read, 4));
    CHECK(alislotp_op_read(0, buf_read, 0));
	
    CHECK(!alislotp_op_write(0x358, buf_write, 4));
    CHECK(!alislotp_op_read(0x358, buf_read, 4));
	printf("write 4bytes buffer follow:\n");
	pbuf = buf_write;
	for(i=0;i<4;i++)
	{
		printf("0x%.2x\n",*pbuf++);
	}
	printf("read 4bytes buffer follow:\n");
	pbuf = buf_read;
	for(i=0;i<4;i++)
	{
		printf("0x%.2x\n",*pbuf++);
	}	
    CHECK(!memcmp(buf_write,buf_read,4));
	
	memset(buf_read,0,16);
	
    CHECK(!alislotp_op_write(0x358, buf_write, 16));
    CHECK(!alislotp_op_read(0x358, buf_read, 16));

	printf("write 16bytes buffer follow:\n");
	pbuf = buf_write;
	for(i=0;i<16;i++)
	{
		printf("0x%.2x\n",*pbuf++);
	}
	printf("read 16bytes buffer follow:\n");
	pbuf = buf_read;
	for(i=0;i<16;i++)
	{
		printf("0x%.2x\n",*pbuf++);
	}		
    CHECK(!memcmp(buf_write,buf_read,16));
}

TEST_GROUP_RUNNER(OTPReadWrite)
{
    RUN_TEST_CASE(OTPReadWrite, ReadWriteTest);
}

static void run_otp_read_write()
{
    RUN_TEST_GROUP(OTPReadWrite);
}

static int run_group_otp_read_write(int argc, char *argv[])
{
    UnityMain(argc, argv, run_otp_read_write);

    return 0;
}


void alislotp_test_register(struct cli_command *parent)
{
    struct cli_command *otp;

    otp = cli_register_command(parent, "1", run_group_otp_read_write,
                               CLI_CMD_MODE_SELF, "read write test");
}


