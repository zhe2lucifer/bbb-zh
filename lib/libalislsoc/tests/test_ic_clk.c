/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			test_ic_clk.c
 *  @brief			test cpu ram clk
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 04:09:53 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */
 TEST_GROUP(SOCClkNum);
 
 TEST_SETUP(SOCClkNum)
 {
 
 }
 
 TEST_TEAR_DOWN(SOCClkNum)
 {
 
 }
 
 TEST(SOCClkNum, ClkNumTest)
 {
	unsigned int ret;
	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_CPUCLK,(void *)&ret));
	printf("the CPU clock is 0x%x\n",ret);

	CHECK(!alislsoc_op_ioctl(ALISLSOC_CMD_DRAMCLK,(void *)&ret));
	printf("the DRAM clock is 0x%x\n",ret);
 }
 
 TEST_GROUP_RUNNER(SOCClkNum)
 {
	 RUN_TEST_CASE(SOCClkNum, ClkNumTest);
 }
 
 static void run_soc_clk()
 {
	 RUN_TEST_GROUP(SOCClkNum);
 }
 
 static int run_group_soc_clk(int argc, char *argv[])
 {
	 UnityMain(argc, argv, run_soc_clk);
 
	 return 0;
 }

