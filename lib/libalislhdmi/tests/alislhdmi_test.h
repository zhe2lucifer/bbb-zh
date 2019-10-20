/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			alislhdmi_test.h
 *  @brief			unit test pattern for sharelib hdmi module.
 *
 *	@Version:		1.0
 *	@date:			04/22/2014 10:51:54 AM
 *	@revision:		none
 *
 *	@Author:		Peter Pan <peter.pan@alitech.com>
 */

#ifdef	__cplusplus
extern "C"
{
#endif

/*
 *	@brief			register hdmi unit test to framework
 *
 *	@param[in]		parent		cli parent node
 *
 *	@return			void
 *
 *	@author			Peter Pan <peter.pan@alitech.com>
 *	@date			04/22/2014 10:50:14 AM
 *
 */
void alislhdmi_test_register(struct cli_command *parent);

#ifdef	__cplusplus
}
#endif
