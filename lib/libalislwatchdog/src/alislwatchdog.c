/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislwatchdog.c
 *  @brief              Implementation of watchdog function
 *
 *  @version            1.0
 *  @date               07/06/2013 10:10:45 AM
 *  @revision           none
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/* ALI Platform */
#include <alipltfretcode.h>

/* Platformlog header */
#include <alipltflog.h>

/* WATCHDOG driver header */
//#include <ali_watchdog.h>
#include <linux/watchdog.h>
#include <ali_watchdog_common.h>
/* WATCHDOG library header */
#include <alislwatchdog.h>

/* Internal header */
#include <internal.h>


static alislwatchdog_dev_t *m_alidog = NULL;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 *  Function Name:      alislwatchdog_open
 *  @brief              open watchdog device
 *
 *  @param              handle point to struct wathcdog device
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alislwatchdog_open(alisl_handle **handle)
{
	alislwatchdog_dev_t *dev = NULL;

	if(NULL == handle) {
		SL_DBG("test start.\n");
		return 0;
	}
	pthread_mutex_lock(&m_mutex);
	if(NULL == m_alidog) {
		dev = (alislwatchdog_dev_t*)malloc(sizeof(alislwatchdog_dev_t));
		if(NULL == dev) {
			SL_ERR("malloc memory failed.\n");
			goto err;
		}

		dev->dev_name = (unsigned char*)ALISLWATCHDOG_DEV_NAME;
		dev->open_cnt = 0;
		dev->handle = open((const char*)dev->dev_name, O_RDWR);
		if (dev->handle < 0) {
			SL_ERR("open watchdog dev failed.\n");
			goto err;
		}
		
		m_alidog = dev;
	}

	m_alidog->open_cnt++;
	*handle = (alisl_handle*)m_alidog;
	pthread_mutex_unlock(&m_mutex);
	return 0;

err:
	free(dev);
	pthread_mutex_unlock(&m_mutex);
	return ALISLWATCHDOG_ERR_CANTOPEN;
}

/**
 *  Function Name:      alislwatchdog_close
 *  @brief              close watchdog device
 *
 *  @param              handle point to struct watchdog device
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alislwatchdog_close(alisl_handle *handle)
{
	if(NULL == handle) {
		SL_ERR("In %s,"
						"handle is NULL.\n", __func__);
		return -1;
	}
	alislwatchdog_dev_t *dev = (alislwatchdog_dev_t *)handle;

	pthread_mutex_lock(&m_mutex);
	if(--m_alidog->open_cnt) {
		goto dog_exit;
	}
	close(dev->handle);
	free(dev);
	m_alidog = NULL;
	pthread_mutex_unlock(&m_mutex);
	return 0;

dog_exit:
	SL_ERR("device still be used.\n");
	pthread_mutex_unlock(&m_mutex);
	return 0;
}

/**
 *  Function Name:      alislwatchdog_ioctl
 *  @brief              choose function by command
 *
 *  @param              dev descirption of wathdog device
 *  @param              cmd command use to choose function of whatchdog
 *  @param              arg point to parameter address
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
static alisl_retcode alislwatchdog_ioctl(alislwatchdog_dev_t *dev,
                                          unsigned int cmd, unsigned long arg)
{
	if (dev->handle < 0) {
		SL_ERR("The dev's handle is invalid!\n");
		return ALISLWATCHDOG_ERR_INVALID_HANDLE;
	}

	SL_DBG("command is %d, The arg is %d\n", cmd, arg);
	
	if (ioctl(dev->handle, cmd, arg) < 0) {
		SL_ERR("IO access is error!\n");
		return ALISLWATCHDOG_ERR_IOACCESS;
	}
	
	return 0;
}

/**
 *  Function Name:      alislwatchdog_set_param
 *  @brief              set watchdog function by parameters
 *
 *  @param              handle  point to struct watchdog device
 *  @param              cmd command use to choose function of whatchdog
 *  @param              arg point to parameter
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislwatchdog_set_param(alisl_handle *handle, unsigned int cmd,
                                      unsigned long arg)
{
	if(NULL == handle) {
		SL_ERR("handle is NULL.\n");
		return -1;
	}
	alislwatchdog_dev_t *dev = (alislwatchdog_dev_t *)handle;
	alislwatchdog_err_t ret  = 0;
	unsigned int c;
	unsigned long p;

	switch (cmd) {
	case ALISLWATCHDOG_CMD_REBOOT_DOG:  { c = WDIOC_WDT_REBOOT;
                                              p = arg; break; }
	case ALISLWATCHDOG_CMD_START_DOG:   { c = WDIOC_SETOPTIONS;
                                              p = arg; break; }
	case ALISLWATCHDOG_CMD_FEED_DOG:    { c = WDIOC_KEEPALIVE;
                                              p = arg; break; }
	case ALISLWATCHDOG_CMD_STOP_DOG:    { c = WDIOC_SETOPTIONS;
                                              p = arg; break; }
	case ALISLWATCHDOG_CMD_SET_TIMEOUT: { c = WDIOC_SETTIMEOUT; 
                                              p = arg; break; }
	case ALISLWATCHDOG_CMD_GET_TIMELEFT: { c = WDIOC_GETTIMELEFT;
                                              p = arg; break; }
	default:
		SL_ERR("Input command %d is over the cover!\n", cmd);
        return ALISLWATCHDOG_ERR_INPUT_PARAM;     
	}

	ret = alislwatchdog_ioctl(dev, c, (unsigned long)&p);

	return ret;
}

/**
 *  Function Name:      alislwatchdog_get_status
 *  @brief              get watchdog'status by command
 * 
 *  @param              handle point to struct watchdog device
 *  @param              cmd command use to choose function of whatchdog
 *  @param              arg parameter address
 *
 *  @return             alisl_retcode
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 *  @date               07/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislwatchdog_get_status(alisl_handle *handle,
                                       unsigned int cmd, unsigned long arg)
{
	if(NULL == handle) {
		SL_ERR("handle is NULL.\n");
		return -1;
	}
	alislwatchdog_dev_t *dev = (alislwatchdog_dev_t *)handle;
	alislwatchdog_err_t ret = 0;
	unsigned int c;

	switch (cmd) {
	case ALISLWATCHDOG_CMD_GET_TIMELEFT: { c = WDIOC_GETTIMELEFT; break; }
	default:
		SL_ERR("command %d is over the cover!", cmd);
        return ALISLWATCHDOG_ERR_INPUT_PARAM;     
	}

	ret = alislwatchdog_ioctl(dev, c, arg);
	
	return ret;
}
