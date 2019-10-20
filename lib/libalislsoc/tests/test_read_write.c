/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_read_write.c
 *  @brief			test read and write interface function
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 03:34:41 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCReadWrite);
 
 TEST_SETUP(SOCReadWrite)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCReadWrite)
 {
 
 }
 
 TEST(SOCReadWrite, ReadWriteTest)
 {
	int i;
	unsigned char soc_read_buf[8]={0};
	unsigned char soc_write_buf[8]={0};
	memset(soc_write_buf,0x44,8);
	
	printf("address 0xb80000d4\n");
	CHECK( !alislsoc_op_write((unsigned char *)0xb80000d4,soc_write_buf,4));
	CHECK( !alislsoc_op_read(soc_read_buf,(unsigned char *)0xb80000d4,4));
		
	printf("soc_write_buf 4bytes buffer follow:\n");
	for(i=0;i<4;i++)
	{
		printf("0x%.2x\n",soc_write_buf[i]);
	}
	printf("soc_read_buf 4bytes buffer follow:\n");
	for(i=0;i<4;i++)
	{
		printf("0x%.2x\n",soc_read_buf[i]);
	}	
	CHECK(!memcmp(soc_read_buf,soc_write_buf,4));

	memset(soc_read_buf,0,8);  //need clear

	CHECK( !alislsoc_op_write((unsigned char *)0xb80000d4,soc_write_buf,8));
	CHECK( !alislsoc_op_read(soc_read_buf,(unsigned char *)0xb80000d4,8));

	printf("soc_write_buf 4bytes buffer follow:\n");
	for(i=0;i<8;i++)
	{
	 printf("0x%.2x\n",soc_write_buf[i]);
	}
	printf("soc_read_buf 4bytes buffer follow:\n");
	for(i=0;i<8;i++)
	{
	 printf("0x%.2x\n",soc_read_buf[i]);
	}	 
	CHECK(!memcmp(soc_read_buf,soc_write_buf,8));
 }
 
 TEST_GROUP_RUNNER(SOCReadWrite)
 {
	 RUN_TEST_CASE(SOCReadWrite, ReadWriteTest);
 }
 
 static void run_soc_read_write()
 {
	 RUN_TEST_GROUP(SOCReadWrite);
 }
 
 static int run_group_soc_read_write(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_read_write);
 
	 return 0;
 }
 

