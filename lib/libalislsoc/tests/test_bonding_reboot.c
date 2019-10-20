/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_bonding_reboot.c
 *  @brief			test bonding and HW_reboot
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 06:02:06 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCReboot);
 
 TEST_SETUP(SOCReboot)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCReboot)
 {
 
 }
 
 TEST(SOCReboot, RebootTest)
 {
	unsigned int ret;		
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_GETBONDING,(void *)&ret));

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_REBOOT,(void *)&ret));
		
 }
 
 TEST_GROUP_RUNNER(SOCReboot)
 {
	 RUN_TEST_CASE(SOCReboot, RebootTest);
 }
 
 static void run_soc_reboot()
 {
	 RUN_TEST_GROUP(SOCReboot);
 }
 
 static int run_group_soc_reboot(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_reboot);
 
	 return 0;
 }

