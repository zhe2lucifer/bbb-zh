/**@file
 *  (c) Copyright 2013-2063  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsoc.c
 *  @brief              Implementation of soc function
 *
 *  @version            1.0
 *  @date               07/06/2013 10:10:45 AM
 *  @revision           none
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pthread.h>

/* ALI Platform */
#include <alipltfretcode.h>

/* Platformlog header */
#include <alipltflog.h>

/* soc driver header */
#include <ali_soc_common.h>

/* WATCHDOG library header */
#include <alislsoc.h>

/* Internal header */
#include <internal.h>

static alislsoc_dev_t *m_dev = NULL;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 *  Function Name:      alislsoc_open
 *  @brief              open soc device
 *
 *  @param              handle point to struct soc device
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_open(void **handle)
{
    alislsoc_dev_t *dev = NULL;

    pthread_mutex_lock(&m_mutex);
    if(NULL == m_dev)
    {
        dev = (alislsoc_dev_t *)malloc(sizeof(alislsoc_dev_t));
        if (NULL == dev)
        {
            SL_DBG("Malloc memory failed!\n");
            pthread_mutex_unlock(&m_mutex);
            return ALISLSOC_ERR_MALLOC;
        }
        dev->dev_name = (unsigned char*)ALISLSOC_DEV_NAME;
        dev->open_cnt = 0;

        dev->handle = open((const char*)dev->dev_name, O_RDWR);
        if (dev->handle < 0)
        {
            goto err;
        }
		m_dev = dev;
    }
    *handle = m_dev;
    m_dev->open_cnt++;
    pthread_mutex_unlock(&m_mutex);
    return 0;

err:
    free(dev);
    m_dev = NULL;
    SL_DBG("The soc dev is not open!\n");
    pthread_mutex_unlock(&m_mutex);
    return ALISLSOC_ERR_CANTOPEN;
}



/**
 *  Function Name:      alislsoc_close
 *  @brief              close soc device
 *
 *  @param              handle point to struct soc device
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_close(void **handle)
{
	if(NULL == *handle) return ALISLSOC_ERR_INPUT_PARAM;
	if(NULL == m_dev) return ALISLSOC_ERR_MALLOC;
	alislsoc_dev_t *dev = (alislsoc_dev_t *)(*handle);

    pthread_mutex_lock(&m_mutex);
    if(--m_dev->open_cnt)
    {
        goto quit;
    }

    close(dev->handle);
    free(dev);
    m_dev = NULL;
    pthread_mutex_unlock(&m_mutex);
    return 0;

quit:
    SL_DBG("device still be used.\n");
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

/**
 *  Function Name:      alislsoc_ioctl
 *  @brief              choose function by command
 *
 *  @param              dev descirption of soc device
 *  @param              cmd command use to choose function of soc
 *  @param              arg point to parameter address
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
static alisl_retcode alislsoc_ioctl(alislsoc_dev_t *dev,
                                    unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	if(NULL == dev) return ALISLSOC_ERR_INPUT_PARAM;
	if (dev->handle < 0)
    {
        SL_DBG("The dev's handle is invalid!\n");
        return ALISLSOC_ERR_INVALID_HANDLE;
    }

    SL_DBG("The command is %d, The arg is %d\n", cmd, arg);

	ret = ioctl(dev->handle, cmd, (void *)arg);
    if ( ret < 0)
    {
	    SL_DBG("The IO acess is error!\n");
        return ALISLSOC_ERR_IOACCESS;
    }

    return ret;
}

/**
 *  Function Name:      alislsoc_set_param
 *  @brief              set soc function by parameters
 *
 *  @param              handle  point to struct soc device
 *  @param              cmd command use to choose function of soc
 *  @param              arg point to parameter
 *
 *  @return             alisl_retcode
 *
 *  @author             Alan Zhang <alan.zhang@alitech.com>
 *  @date               07/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislsoc_set_param(void *handle, unsigned int cmd,
                                 unsigned long arg)
{
    alislsoc_dev_t *dev = (alislsoc_dev_t *)handle;
    alislsoc_err_t ret  = 0;
    unsigned int c = 0;
    unsigned long p = 0;

    switch (cmd)
    {
        case ALISLSOC_CMD_READ:
        {
            c =  ALI_SOC_READ;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_WRITE:
        {
            c = ALI_SOC_WRITE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_CHIPID:
        {
            c = ALI_SOC_CHIP_ID;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_PRODUCTID:
        {
            c = ALI_SOC_PRODUCT_ID;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_GETBONDING:
        {
            c = ALI_SOC_GET_BONDING;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_REVID:
        {
            c = ALI_SOC_REV_ID;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_CPUCLK:
        {
            c = ALI_SOC_CPU_CLOCK;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_DRAMCLK:
        {
            c = ALI_SOC_DRAM_CLOCK;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_HDENABLE:
        {
            c = ALI_SOC_HD_ENABLED;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_C3603PDC:
        {
            c = ALI_SOC_C3603_PRODUCT;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_USBNUM:
        {
            c = ALI_SOC_USB_NUM;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_USBPORT:
        {
            c = ALI_SOC_USB_PORT_ENABLED;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_NIM3501:
        {
            c = ALI_SOC_NIM_M3501_SUPPORT;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_NIM:
        {
            c = ALI_SOC_NIM_SUPPORT;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_CINUM:
        {
            c = ALI_SOC_CI_NUM;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_MACNUM:
        {
            c = ALI_SOC_MAC_NUM;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_TUNERNUM:
        {
            c = ALI_SOC_TUNER_NUM;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_ISHDENABLE:
        {
            c = ALI_SOC_HD_IS_ENABLED;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_SATAENABLE:
        {
            c = ALI_SOC_SATA_EANBLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_SCRAMENABLE:
        {
            c = ALI_SOC_SCRAM_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_SECUENABLE:
        {
            c = ALI_SOC_SECU_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_SPLENABLE:
        {
            c = ALI_SOC_SPL_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_UARTENABLE:
        {
            c = ALI_SOC_UART_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_ETJTENABLE:
        {
            c = ALI_SOC_ETJT_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_MGENABLE:
        {
            c = ALI_SOC_MG_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_AC3ENABLE:
        {
            c = ALI_SOC_AC3_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_DDPENABLE:
        {
            c = ALI_SOC_DDP_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_XDENABLE:
        {
            c = ALI_SOC_XD_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_XDPENABLE:
        {
            c = ALI_SOC_XDP_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_AACENABLE:
        {
            c = ALI_SOC_AAC_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_H264ENABLE:
        {
            c = ALI_SOC_H264_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_MP4ENABLE:
        {
            c = ALI_SOC_MP4_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_MS10ENABLE:
        {
            c = ALI_SOC_MS10_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_MS11ENABLE:
        {
            c = ALI_SOC_MS11_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_RMVBENABLE:
        {
            c = ALI_SOC_RMVB_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_VC1ENABLE:
        {
            c = ALI_SOC_VC1_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_AVSENABLE:
        {
            c = ALI_SOC_AVS_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_VP8ENABLE:
        {
            c = ALI_SOC_VP8_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_FLVENABLE:
        {
            c = ALI_SOC_FLV_ENABLE;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_RBGT:
        {
            c = ALI_SOC_REBOOT_GET_TIMER;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_ETSTANDBY:
        {
            c = ALI_SOC_ENTER_STANDBY;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_REBOOT:
        {
            c = ALI_SOC_REBOOT;
            p = arg;
            break;
        }
        case ALISLSOC_CMD_DACD:
        {
            c = ALI_DSC_ACC_CE_DIS;
            p = arg;
            break;
        }
        default:
            SL_DBG("Input command %d is over the cover!\n", cmd);
            return ALISLSOC_ERR_INPUT_PARAM;
    }

    ret = alislsoc_ioctl(dev, c, p);

    return ret;
}
