/**@file
 *  (c) Copyright 2013-2063  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislotp_wrapper.c
 *  @brief              wrapper layer of otp      
 *
 *  @version            1.0
 *  @date               07/06/2013 10:11:44 AM
 *  @revision           none
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Platform header */
#include <alipltfretcode.h>

/* Platformlog header */
#include <alipltflog.h>

/* OTP header */
#include <alislotp.h>

/*  otp driver header */
#include <ca_otp.h>

/* Internal header */
#include <internal.h>

alisl_retcode alislotp_open(void *handle);
alisl_retcode alislotp_close(void *handle);
alisl_retcode alislotp_destruct(void **handle);
alisl_retcode alislotp_construct(void **handle,
                                      alislotp_param_t *param);
alisl_retcode alislotp_set_param(void *handle, unsigned int cmd,
                                      unsigned long arg);
/**
 *  Function Name:      alislotp_op_read
 *  @brief              read otp data
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_op_read(unsigned long offset, unsigned char *buf, int len)
{
	alislotp_err_t ret = 0;
	void *dev = NULL;
	alislotp_param_t param;
	unsigned long enable = 0;
	
	struct otp_paras paras = {offset,buf,len};
	//SL_DBG("read otp start.\n");

	memset(&param,0,sizeof(alislotp_param_t));
	param.status = ALISLOTP_ACTIVATE;

	alislotp_construct(&dev, &param);
	alislotp_open(dev);
	enable = (unsigned long)&paras;
	
	ret = alislotp_set_param(dev, ALISLOTP_CMD_READ, enable);
	
	alislotp_close(dev);
	alislotp_destruct(&dev);
	//SL_DBG("read otp successful.\n");

	return ret;
}

/**
 *  Function Name:      alislotp_op_write
 *  @brief              write otp data
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislotp_op_write(unsigned long offset, unsigned char *buf, int len)
{
	alislotp_err_t ret = 0;
	void *dev = NULL;
	alislotp_param_t param;
	unsigned long enable = 0;
	
	struct otp_paras paras = {offset,buf,len};

	SL_DBG("write otp start.\n");

	memset(&param,0,sizeof(alislotp_param_t));
	param.status = ALISLOTP_ACTIVATE;

	SL_DBG("offset = %d,len = %d", offset, len);
	SL_DUMP("buf: ", (char *)buf, len);

	alislotp_construct(&dev, &param);
	alislotp_open(dev);
	enable = (unsigned long)&paras;
	
	ret = alislotp_set_param(dev, ALISLOTP_CMD_WRITE, enable);
	
	alislotp_close(dev);
	alislotp_destruct(&dev);
	SL_DBG("write otp successful.\n");

	return ret;
}
