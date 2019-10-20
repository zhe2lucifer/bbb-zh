/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			otp_main.c
 *  @brief
 *
 *	@Version:		1.0
 *	@date:			04/25/2014 03:17:49 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <alislotp.h>

int main(int argc, char *argv[])
{
	unsigned char *buf=NULL,*tem_buf=NULL,*buf_write=NULL;
	unsigned long offset=0;
	unsigned int len=0,i;

	if(3 != argc)
	{
		printf("Usage:\n");
		printf("  %s offset len\n",argv[0]);
		printf(" offset+len <= 0x400\n");
		return -1;
	}

	offset = strtoul(argv[1],NULL,16);
	len = strtoul(argv[2],NULL,16);
	buf = (unsigned char *)malloc(sizeof(unsigned char)*len);
	if(NULL == buf)
	{
		printf("Malloc memory failed!\n");
		exit(-1);
	}
	memset(buf,0x77,len);

	buf_write = (unsigned char *)malloc(sizeof(unsigned char)*len);
	if(NULL == buf_write)
	{
		printf("buf_write Malloc memory failed!\n");
		exit(-1);
	}
	memset(buf_write,0x22,len);


	printf(" offset=0x%x, len=0x%x\n",offset,len);
	tem_buf = buf;
	for(i=0;i<len;i++)
	{
		printf("0x%.2x\n",*tem_buf++);
	}

	alislotp_op_write(offset,buf_write,len);

	alislotp_op_read(offset, buf, len);


	printf(" second ======offset=0x%x, len=0x%x\n",offset,len);
	tem_buf = buf;
	for(i=0;i<len;i++)
	{
		printf("0x%x\n",*tem_buf++);
	}
	free(buf);
	free(buf_write);
	return 0;
}

