/**@file
 *  (c) Copyright 2013-2063  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislsoc_wrapper.c
 *  @brief              wrapper layer of soc      
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

/* SOC header */
#include <alislsoc.h>

/*  soc driver header */
#include <ali_soc_common.h>

/* Internal header */
#include <internal.h>

alisl_retcode alislsoc_open(void **handle);
alisl_retcode alislsoc_close(void **handle);
alisl_retcode alislsoc_set_param(void *handle, unsigned int cmd,
                                 unsigned long arg);
/**
 *  Function Name:      alislsoc_op_read
 *  @brief              read soc data
 *
 *  @return             alisl_retcode 0:right  
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_read(unsigned char *dst, unsigned char *buf, int len)
{
	alislsoc_err_t ret = 0;
	void *dev = NULL;
	unsigned long *enable = NULL;
	
	struct soc_op_paras paras = {dst,buf,len};
	SL_DBG("read soc start.\n");

	enable = (unsigned long*)&paras;

	alislsoc_open(&dev);
	
	ret = alislsoc_set_param(dev, ALISLSOC_CMD_READ,(unsigned long) enable);
	
	alislsoc_close(&dev);
	SL_DBG("read soc successful.\n",
		                    __func__);

	return ret;
}

/**
 *  Function Name:      alislsoc_op_write
 *  @brief              write soc data
 *
 *  @return             alisl_retcode 0:right
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_write(unsigned char *dst, unsigned char *buf, int len)
{
	alislsoc_err_t ret = 0;
	void *dev = NULL;
	unsigned long *enable = NULL;
	
	struct soc_op_paras paras = {dst,buf,len};
	SL_DBG("write soc start.\n");

	alislsoc_open(&dev);
	enable = (unsigned long*)&paras;
	
	ret = alislsoc_set_param(dev, ALISLSOC_CMD_WRITE, (unsigned long)enable);
	
	alislsoc_close(&dev);
	SL_DBG("write soc successful.\n");
	printf("alislsoc_op_write return val %d\n",ret);
	return ret;
}

/**
 *  Function Name:      alislsoc_op_reboot
 *  @brief              hardware watchdog reboot system
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_op_reboot()
{
	alislsoc_err_t ret = 0;
	void *dev = NULL;
	unsigned long *enable = NULL;
	
	SL_DBG("write soc start.\n");

	alislsoc_open(&dev);
	
	ret = alislsoc_set_param(dev, ALISLSOC_CMD_REBOOT,(unsigned long) enable);
	
	alislsoc_close(&dev);
	SL_DBG("soc reboot successful.\n");

	return ret;
}

/**
 *  Function Name:      alislsoc_op_ioctl
 *  @brief              uniform interface 
 *
 *  @return             alisl_retcode 0:right
 *
 *  @author             Vedic Fu <vedic.fu@alitech.com>
 *  @date               05/14/2014, Created
 *
 *  @note
 */

alisl_retcode alislsoc_op_ioctl(alislsoc_cmd_t cmd, void *para)
{
	alislsoc_err_t ret = 0;
	void *dev = NULL;
	unsigned long *enable = NULL;
	
	SL_DBG("alislsoc_op_ioctl operation soc start.\n");

	if(cmd >= ALISLSOC_CMD_NULL)
	{
		printf("the cmd is error!\n");
		return -1;
	}

	alislsoc_open(&dev);

	if((cmd == ALISLSOC_CMD_READ) || (cmd == ALISLSOC_CMD_WRITE) || (cmd == ALISLSOC_CMD_USBPORT)
		|| (cmd == ALISLSOC_CMD_RBGT) || (cmd == ALISLSOC_CMD_ETSTANDBY))
	{
		if(alislsoc_set_param(dev, cmd,(unsigned long) para) != 0 )
		{
			printf("alislsoc_set_param return error");
			return -1;
		}
	}
	else
	{
		*(int *)para = alislsoc_set_param(dev, cmd,(unsigned long) enable);
		if(*(int *)para <0) return -1;
	}
	
		
	alislsoc_close(&dev);
	
	SL_DBG("alislsoc_op_ioctl operation soc successful.\n");

	return ret;
}

