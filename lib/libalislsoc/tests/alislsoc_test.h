/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			alislsoc_test.h
 *  @brief			alislsoc module head
 *
 *	@Version:		1.0
 *	@date:			05/14/2014 01:08:07 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */

#ifdef	__cplusplus
  extern "C"
  {
#endif
  
struct soc_test_paras {
	unsigned char * to;
	unsigned char *from;
	int len;
}; 				   

 
 void alislsoc_test_register(struct cli_command *parent);
 
#ifdef	__cplusplus
  }
#endif
 
