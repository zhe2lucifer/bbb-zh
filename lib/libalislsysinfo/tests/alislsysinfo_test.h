/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			alislsysinfo_test.h
 *  @brief			unit test entry for sysinfo
 *
 *	@Version:		1.0
 *	@date:			04/23/2014 11:03:33 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

#ifdef	__cplusplus
extern "C"
{
#endif

/*
 *  @brief          register sysinfo unit test to framework
 *
 *  @param[in]      parent      cli parent node
 *
 *  @return         void
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           04/22/2014 10:50:14 AM
 *
 *  @note
 */
void alislsysinfo_test_register(struct cli_command *parent);

#ifdef	__cplusplus
}
#endif
