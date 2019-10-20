/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_device_is_enable.c
 *  @brief			test device is enable
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 04:55:43 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCDeviceIsEnable);
 
 TEST_SETUP(SOCDeviceIsEnable)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCDeviceIsEnable)
 {
 
 }
 
 TEST(SOCDeviceIsEnable, IsEnableTest)
 {
	unsigned int ret;
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_FLVENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_FLVENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_VP8ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_VP8ENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_AVSENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_AVSENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_VC1ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_VC1ENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_RMVBENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_RMVBENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_MS11ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_MS11ENABLE is 0x%x\n",ret);
	
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_MS10ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_MS10ENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_MP4ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_MP4ENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_H264ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_H264ENABLE is 0x%x\n",ret);
	
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_AACENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_AACENABLE is 0x%x\n",ret);
	
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_XDPENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_XDPENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_XDENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_XDENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_DDPENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_DDPENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_AC3ENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_AC3ENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_MGENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_MGENABLE is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_ISHDENABLE,(void *)&ret));
	printf("the ALISLSOC_CMD_ISHDENABLE is 0x%x\n",ret);
	
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_NIM3501,(void *)&ret));
	printf("the ALISLSOC_CMD_NIM3501 is 0x%x\n",ret);
		
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_NIM,(void *)&ret));
	printf("the ALISLSOC_CMD_NIM is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_DACD,(void *)&ret));
	printf("the ALISLSOC_CMD_DACD is 0x%x\n",ret);
	
 }
 
 TEST_GROUP_RUNNER(SOCDeviceIsEnable)
 {
	 RUN_TEST_CASE(SOCDeviceIsEnable, IsEnableTest);
 }
 
 static void run_soc_device_is_enable()
 {
	 RUN_TEST_GROUP(SOCDeviceIsEnable);
 }
 
 static int run_group_soc_device_is_enable(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_device_is_enable);
 
	 return 0;
 }

