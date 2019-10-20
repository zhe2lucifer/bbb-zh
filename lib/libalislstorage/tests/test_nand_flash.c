/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_nand_flash.c
 *  @brief          unit test for nand flash
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 11:07:45 AM
 *  @revision:      none
 *
 *  @Author:        Demon Yan <demon.yan@alitech.com>
 */

static alisl_handle dev_hdl = NULL;

TEST_GROUP(NANDINVALIDCALL);

TEST_SETUP(NANDINVALIDCALL)
{	
    CHECK(STO_ERR_NONE == alislsto_open(&dev_hdl, STO_TYPE_NAND, O_RDWR));
}

TEST_TEAR_DOWN(NANDINVALIDCALL)
{
    CHECK(STO_ERR_NONE == alislsto_close(dev_hdl, false));
}

TEST(NANDINVALIDCALL, InvalidCall)
{
//#define SIZE 0x200000
//#define ADDR 0x7500000

	off_t ADDR = 0x7500000;
	size_t SIZE = 0x200000;

	int   i = 0;
	off_t addr = 0;
	char  rbuf[SIZE];
	char  wbuf[SIZE];
	
	memset(rbuf, 0x0, SIZE);
	memset(wbuf, 0xAA, SIZE);

	CHECK(STO_ERR_NONE != alislsto_set_offset(NULL, ADDR, true));
	CHECK(STO_ERR_NONE != alislsto_get_offset(NULL, &addr));
	CHECK(STO_ERR_NONE != alislsto_get_offset(dev_hdl, NULL));

	CHECK(STO_ERR_NONE != alislsto_write(NULL, wbuf, SIZE));
	CHECK(STO_ERR_NONE != alislsto_write(dev_hdl, NULL, SIZE));

	CHECK(STO_ERR_NONE != alislsto_read(NULL, rbuf, SIZE));
	CHECK(STO_ERR_NONE != alislsto_read(dev_hdl, NULL, SIZE));
	
	CHECK(STO_ERR_NONE != alislsto_erase(NULL, ADDR, SIZE));
}

/* Test NAND flash open, close and erase */
TEST_GROUP(NANDRWERASE);

TEST_SETUP(NANDRWERASE)
{	
    CHECK(STO_ERR_NONE == alislsto_open(&dev_hdl, STO_TYPE_NAND, O_RDWR));
}

TEST_TEAR_DOWN(NANDRWERASE)
{
    CHECK(STO_ERR_NONE == alislsto_close(dev_hdl, false));
}

TEST(NANDRWERASE, ReadWrite)
{
/* define according to flash type */
//#define SIZE 0x200000
//#define ADDR 0x7500000

	off_t ADDR = 0x7500000;
	size_t SIZE = 0x200000;
	
	int   i = 0;
	off_t addr = 0;
	char  rbuf[SIZE];
	char  wbuf[SIZE];
	
	memset(rbuf, 0x0, SIZE);
	memset(wbuf, 0xAA, SIZE);

	CHECK(STO_ERR_NONE == alislsto_set_offset(dev_hdl, ADDR, true));
	CHECK(STO_ERR_NONE == alislsto_get_offset(dev_hdl, &addr));
	CHECK(ADDR == addr);
	CHECK(SIZE == alislsto_write(dev_hdl, wbuf, SIZE));

	CHECK(STO_ERR_NONE == alislsto_set_offset(dev_hdl, ADDR, true));
	CHECK(STO_ERR_NONE == alislsto_get_offset(dev_hdl, &addr));
	CHECK(ADDR == addr)
	CHECK(SIZE == alislsto_read(dev_hdl, rbuf, SIZE));
	
	TEST_ASSERT_EQUAL_INT8_ARRAY(wbuf, rbuf, SIZE);	
}

TEST(NANDRWERASE, Erase)
{
/* define according to flash type */
//#define SIZE 0x200000
//#define ADDR 0x7800000

	off_t ADDR = 0x7800000;
	size_t SIZE = 0x200000;

	int   i = 0;
	off_t addr = 0;
	char  rbuf[SIZE];
	char  buf[SIZE];
	
	memset(rbuf, 0x0, SIZE);
	memset(buf, 0xFF, SIZE);

	CHECK(STO_ERR_NONE == alislsto_erase(dev_hdl, ADDR, SIZE));

	CHECK(STO_ERR_NONE == alislsto_set_offset(dev_hdl, ADDR, true));
	CHECK(STO_ERR_NONE == alislsto_get_offset(dev_hdl, &addr));
	CHECK(ADDR == addr);
	CHECK(SIZE == alislsto_read(dev_hdl, rbuf, SIZE));
	
	TEST_ASSERT_EQUAL_INT8_ARRAY(buf, rbuf, SIZE);	
}

TEST_GROUP_RUNNER(NANDRWERASE)
{
	RUN_TEST_CASE(NANDINVALIDCALL, InvalidCall);

	RUN_TEST_CASE(NANDRWERASE, Erase);
	RUN_TEST_CASE(NANDRWERASE, ReadWrite);
}

static void run_nand_tests()
{
	RUN_TEST_GROUP(NANDRWERASE);
}

static int run_group_nand_tests(int argc, char *argv[])
{
	UnityMain(argc, argv, run_nand_tests);
	
	return 0;
}
