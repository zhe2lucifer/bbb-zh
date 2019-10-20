/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_ioctl_read_write.c
 *  @brief			test read write via alislsoc_op_ioctl inerface function
 *
 *	@Version:		1.0
 *	@date:			05/15/2014 09:10:51 AM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCIoctlReadWrite);
 
 TEST_SETUP(SOCIoctlReadWrite)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCIoctlReadWrite)
 {
 
 }
 
 TEST(SOCIoctlReadWrite, IoctlReadWriteTest)
 {
	int i;
	unsigned char soc_read_buf[8]={0};
	unsigned char soc_write_buf[8]={0};
	unsigned char *mem_add = (unsigned char *)0xb80000d4;
	unsigned char *p_read, *p_write;
	memset(soc_write_buf,0x66,8);
	p_read = &soc_read_buf[0];
	p_write = &soc_write_buf[0];
	
	printf("address 0xb80000d4\n");
	
	struct soc_test_paras paras_read = {p_read,mem_add,4};
	struct soc_test_paras paras_write = {mem_add,p_write,4};

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_WRITE,(void *)&paras_write));
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_READ,(void *)&paras_read));
		
	printf("soc_write_buf 4 bytes buffer follow:\n");
	for(i=0;i<4;i++)
	{
		printf("0x%.2x\n",soc_write_buf[i]);
	}
	printf("soc_read_buf 4 bytes buffer follow:\n");
	for(i=0;i<4;i++)
	{
		printf("0x%.2x\n",soc_read_buf[i]);
	}	
	CHECK(!memcmp(soc_read_buf,soc_write_buf,4));

	memset(soc_read_buf,0,8);  //need clear
	paras_read.len = 8;
	paras_write.len = 8;

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_WRITE,(void *)&paras_write));
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_READ,(void *)&paras_read));

	printf("soc_write_buf 8 bytes buffer follow:\n");
	for(i=0;i<8;i++)
	{
	 printf("0x%.2x\n",soc_write_buf[i]);
	}
	printf("soc_read_buf 8 bytes buffer follow:\n");
	for(i=0;i<8;i++)
	{
	 printf("0x%.2x\n",soc_read_buf[i]);
	}	 
	CHECK(!memcmp(soc_read_buf,soc_write_buf,8));

	printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
	
 }
 
 TEST_GROUP_RUNNER(SOCIoctlReadWrite)
 {
	 RUN_TEST_CASE(SOCIoctlReadWrite, IoctlReadWriteTest);
 }
 
 static void run_soc_ioctl_read_write()
 {
	 RUN_TEST_GROUP(SOCIoctlReadWrite);
 }
 
 static int run_group_soc_ioctl_read_write(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_ioctl_read_write);
 
	 return 0;
 }
 

