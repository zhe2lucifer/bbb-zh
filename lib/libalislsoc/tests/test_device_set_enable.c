/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_device_set_enable.c
 *  @brief			set device enable
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 05:30:40 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */

 TEST_GROUP(SOCDeviceSetEnable);
 
 TEST_SETUP(SOCDeviceSetEnable)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCDeviceSetEnable)
 {
 
 }
 
 TEST(SOCDeviceSetEnable, SetEnableTest)
 {
	unsigned int ret;
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_ETJTENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_ETJTENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_UARTENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_UARTENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_SPLENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_SPLENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_SECUENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_SECUENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_SCRAMENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_SCRAMENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_SATAENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_SATAENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_HDENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_HDENABLE is 0x%x\n",ret);
		
 }
 
 TEST_GROUP_RUNNER(SOCDeviceSetEnable)
 {
	 RUN_TEST_CASE(SOCDeviceSetEnable, SetEnableTest);
 }
 
 static void run_soc_device_set_enable()
 {
	 RUN_TEST_GROUP(SOCDeviceSetEnable);
 }
 
 static int run_group_soc_device_set_enable(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_device_set_enable);
 
	 return 0;
 }

