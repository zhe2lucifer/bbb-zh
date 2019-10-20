/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_ic_num.c
 *  @brief			test devices numbers
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 04:22:08 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCDeviceNum);
 
 TEST_SETUP(SOCDeviceNum)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCDeviceNum)
 {
 
 }
 
 TEST(SOCDeviceNum, DeviceNumTest)
 {
	unsigned int ret;
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_CINUM,(void *)&ret));
	printf("the ic all num is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_USBNUM,(void *)&ret));
	printf("the USB num is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_MACNUM,(void *)&ret));
	printf("the mac num is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_TUNERNUM,(void *)&ret));
	printf("the tuner num is 0x%x\n",ret);

 }
 
 TEST_GROUP_RUNNER(SOCDeviceNum)
 {
	 RUN_TEST_CASE(SOCDeviceNum, DeviceNumTest);
 }
 
 static void run_soc_device_num()
 {
	 RUN_TEST_GROUP(SOCDeviceNum);
 }
 
 static int run_group_soc_device_num(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_device_num);
 
	 return 0;
 }

