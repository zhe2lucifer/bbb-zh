/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_nor_flash.c
 *  @brief          unit test for nor flash
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 11:07:45 AM
 *  @revision:      none
 *
 *  @Author:        Demon Yan <demon.yan@alitech.com>
 */

static alisl_handle dev_hdl;

TEST_GROUP(NORINVALIDCALL);

TEST_SETUP(NORINVALIDCALL)
{	
    CHECK(STO_ERR_NONE == alislsto_open(&dev_hdl, STO_TYPE_NOR, O_RDWR));
}

TEST_TEAR_DOWN(NORINVALIDCALL)
{
    CHECK(STO_ERR_NONE == alislsto_close(dev_hdl, false));
}

TEST(NORINVALIDCALL, InvalidCall)
{
//#define SIZE 0x200000
//#define ADDR 0x0

	off_t ADDR = 0x0;
	size_t SIZE = 0x20000;

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

/* Test NOR flash read, write and erase */
TEST_GROUP(NORRWERASE);

TEST_SETUP(NORRWERASE)
{	
    CHECK(STO_ERR_NONE == alislsto_open(&dev_hdl, STO_TYPE_NOR, O_RDWR));
}

TEST_TEAR_DOWN(NORRWERASE)
{
    CHECK(STO_ERR_NONE == alislsto_close(dev_hdl, false));
}

TEST(NORRWERASE, ReadWrite)
{
/* define according to flash type */
//#define SIZE 0x200000
//#define ADDR 0x0

	off_t ADDR = 0x0;
	size_t SIZE = 0x20000;

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

TEST(NORRWERASE, Erase)
{
/* define according to flash type */
//#define SIZE 0x200000
//#define ADDR 0x200000

	off_t ADDR = 0x200000;
	size_t SIZE = 0x20000;

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

TEST_GROUP_RUNNER(NORRWERASE)
{
	RUN_TEST_CASE(NORINVALIDCALL, InvalidCall);

	RUN_TEST_CASE(NORRWERASE, Erase);
	RUN_TEST_CASE(NORRWERASE, ReadWrite);
}

static void run_nor_tests()
{
	RUN_TEST_GROUP(NORRWERASE);
}

static int run_group_nor_tests(int argc, char *argv[])
{
	UnityMain(argc, argv, run_nor_tests);
	
	return 0;
}

