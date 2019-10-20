/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			alislsdec_test.h
 *  @brief			unit test pattern for sharelib sdec module.
 *
 *	@Version:		1.0
 *	@date:			05/20/2014 10:51:54 AM
 *	@revision:		none
 *
 *	@Author:		Lisa Liu
 */

#ifdef	__cplusplus
extern "C"
{
#endif

/*
 *	@brief			register sdec unit test to framework
 *
 *	@param[in]		parent		cli parent node
 *
 *	@return			void
 *
 *	@author			Lisa Liu
 *	@date			05/20/2014 10:50:14 AM
 *
 */
void alislsdec_test_register(struct cli_command *parent);

#ifdef	__cplusplus
}
#endif

