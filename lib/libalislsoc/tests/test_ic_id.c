/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_ic_id.c
 *  @brief			test some id
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 03:41:17 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCIDValue);
 
 TEST_SETUP(SOCIDValue)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCIDValue)
 {
 
 }
 
 TEST(SOCIDValue, IDValueTest)
 {
	unsigned int ret;
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_CHIPID,(void *)&ret));
	printf("the chip ID is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_PRODUCTID,(void *)&ret));
	printf("the product ID is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_REVID,(void *)&ret));
	printf("the revervion ID is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_C3603PDC,(void *)&ret));
	printf("the C3603 product ID is 0x%x\n",ret);

 }
 
 TEST_GROUP_RUNNER(SOCIDValue)
 {
	 RUN_TEST_CASE(SOCIDValue, IDValueTest);
 }
 
 static void run_soc_id()
 {
	 RUN_TEST_GROUP(SOCIDValue);
 }
 
 static int run_group_soc_id(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_id);
 
	 return 0;
 }

