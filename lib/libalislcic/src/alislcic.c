/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislcic.c
 *  @brief              This file contains all functions definition
 *                      of CI controler driver.
 *
 *  @version            1.0
 *  @date               07/04/2013 04:37:11 PM
 *  @revision           none
 *
 *  @author             Adolph Liu <adolph.liu@alitech.com>
 */

/* system headers */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/epoll.h>
//#include <ali_netlink_common.h>
#include <pthread.h>

/* ali driver headers */
#include <ali_cic.h>

/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislcic.h>
#include <alislevent.h>
#include <bits_op.h>

/* local headers */
#include "internal.h"

#define ALISLCIC_MAX_MSG_LEN 1024
#define CIC_SET_PORT_ID 1 /*reserved,to avoid building error,need to delete*/
#define MAX_KUMSG_SIZE 1024

static p_aliplsl_fun_cb puser_callback;     /* user callback*/
static unsigned int i_sockaddr = 0;     /* socket communication port */
static alisl_handle event_hdl;
static struct alislevent st_event;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 *  Function Name:      alislcic_construct
 *  @brief              construct some needed data or struct
 *
 *  @param handle       pointer to struct cic_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph Liu <adolph.liu@alitech.com>
 *  @date               07/11/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_construct(alisl_handle *handle)
{
    pthread_mutex_lock(&m_mutex);
	if (NULL != *handle) {
        SL_ERR("CIC device handle have been constructed\n");
		pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }
    cic_device_t *dev = NULL;
    dev = malloc(sizeof(cic_device_t));
    if (dev == NULL) {
        SL_ERR("malloc memory failed\n");
		pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_MALLOCFAILED;
    }

    memset(dev, 0, sizeof(cic_device_t));

    dev->fd = -1;
    dev->status = ALISLCIC_STATUS_CONSTRUCT;
	
    *handle = (alisl_handle)dev;
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

static void * alislcic_callback(void  *pdata)
{
    int len = 0;
    unsigned char msg_buf[MAX_KUMSG_SIZE] = {0};	

    pthread_mutex_lock(&m_mutex);
    len = read(i_sockaddr, msg_buf, MAX_KUMSG_SIZE);   
    pthread_mutex_unlock(&m_mutex);

    int i = 0;
    char str[200], s[20];
    str[0] = 0;
    for(i = 0; i<len; i++) {
	    sprintf(s, "%02x",msg_buf[i] & 0xFF);
	    strncat(str, s, sizeof(str) - strlen(str));
    }
    SL_DBG(str);

    puser_callback(0);

    return 0;
}

alisl_retcode alislcic_register_callback(p_aliplsl_fun_cb p_callback_init)
{
    //int ret= 0;
    
    pthread_mutex_lock(&m_mutex);
    if(!p_callback_init) { /* not register callback */
        goto err_exit;
    }

    puser_callback = p_callback_init;
    st_event.fd = i_sockaddr;    
    SL_DBG("ALISLCCI:i_sockaddr = %d\n",i_sockaddr);

    st_event.events = EPOLLIN;
    st_event.cb = alislcic_callback;
    st_event.data = NULL;
    if(alislevent_open(&event_hdl) || !event_hdl) {
        SL_DBG("Open event device error!\n");
        goto err_exit;
    }
    SL_DBG("alislevent_open return success!\n");
    if(alislevent_add(event_hdl, &st_event)) {
        SL_DBG("alislevent_add error!\n");
        goto err_exit;
    }
    
    pthread_mutex_unlock(&m_mutex);
    return 0;
err_exit:
    if(event_hdl) {
        alislevent_close(event_hdl);
        event_hdl = NULL;
    }
    puser_callback = NULL;
    if(st_event.fd) {
        close(st_event.fd);
        st_event.fd = -1;
    }
    pthread_mutex_unlock(&m_mutex);
    return 1;
}

/**
 *  Function Name:      alislcic_open
 *  @brief              This function is used to open a CI controller device
 *                      identified by device structure pointer *dev. After call
 *                      this function, upper layer can operate the CI controller
 *                      and CAM.
 *
 *  @param handle       The handle's device will be opened.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph Liu <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note
 */
alisl_retcode alislcic_open(alisl_handle handle)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
//    int fd;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Try to open before construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If openned already, exit */
    if (bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s openned already!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    /* Open this device */
    dev->fd = open(ALISLCIC_DEV_NAME, O_RDWR);
    if (dev->fd <= 0) {
        SL_ERR("Open CI controller %s handle failed,errno:%d\n",ALISLCIC_DEV_NAME,errno);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_CANTOPENDEV;
    }

    //SL_DBG("\n to get ir kumsgfd with: %d\n", dev->fd);
    int flags = O_CLOEXEC;
    i_sockaddr = ioctl(dev->fd, CIC_GET_GET_KUMSGQ, &flags); 
    if (i_sockaddr == 0) {
        SL_DBG("CIC_GET_GET_KUMSGQ error!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }  

    bit_set(dev->status, ALISLCIC_STATUS_OPEN);
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

/**
 *  Function Name:      alislcic_close
 *  @brief              This function used to close a CI controller device
 *                      identified by device structure pointer *dev. After call
 *                      this function, all operation for this device, include
 *                      CAM card operation, are invalid.This function will only
 *                                              set dev->status to ALISLCIC_STATUS_CLOSE,not really close CI device
 *
 *  @param  handle      The handle's device will be closed.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph Liu <adolph.liu@alitech.com>
 *  @date               07/08/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_close(alisl_handle handle)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Try to close before construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s closed already!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    /* close socket fd */
    /*if(st_event.fd) {
        close(st_event.fd);
        st_event.fd = -1;
        pthread_mutex_unlock(&m_mutex);
    }*/
    if (i_sockaddr)
        close(i_sockaddr);

    /* Close device */
    close(dev->fd);
    dev->fd = -1;

    /* Update & clean flags */
    bit_clear(dev->status, ALISLCIC_STATUS_OPEN);

    
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

/**
 *  Function Name:      alislcic_read_io_data
 *  @brief              This function used to read "size" bytes data to *buffer
 *                      from the CI command interface data register which indicated
 *                      by *dev.
 *
 *
 *  @param  handle      CI controller device handle
 *  @param  slot        The slot index, value must between [0, MAX_CI_SLOT_NUM)
 *  @param  size        Data's size which will be readed (byte)
 *  @param  buffer      Buffer allocated by up layer used to store the read data.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph Liu <adolph.liu@alitech.com>
 *  @date               07/10/2013, Created
 *
 *  @note               It only read CI register data, but we also need detect
 *                      the other register's status include size_low. size_high
 *                      and status register. So before invoking this funciton ,up
 *                      layer must implement the CI physic layer read operation
 *                      (check status, get read data size and read data out, etc).
 *
 */
alisl_retcode alislcic_read_io_data(alisl_handle handle, uint32_t slot,
                                    size_t size, unsigned char *buffer)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }
    if(CIC_MSG_MAX_LEN < size) {
        SL_ERR("Invalid parameter!\n");
		pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }
    /* config the message which will send to cic driver */
    dev->cic_msg.index = (uint16_t)slot;
    dev->cic_msg.type = CIC_BLOCK << 16;
    dev->cic_msg.length = (uint16_t)size;

    if (ioctl(dev->fd, CIC_GET_MSG, &dev->cic_msg) != 0) {
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }
    memcpy(buffer, dev->cic_msg.msg, size);
    
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

/**
 *  Function Name:      alislcic_write_io_data
 *  @brief              This function used to write "size" bytes data from *buffer
 *                      into the CI command interface data register which indicated
 *                      by *dev.
 *
 *
 *  @param  handle      CI controller device handle
 *  @param  slot        Ioctl message index
 *  @param  size        Data's size which will be writed (byte)
 *  @param  buffer      Buffer allocated by up layer used to store the write data.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph Liu <adolph.liu@alitech.com>
 *  @date               07/10/2013, Created
 *
 *  @note               It only write CI register data, but we also need detect
 *                      the other register's status include size_low. size_high
 *                      and status register. So before invoking this funciton ,up
 *                      layer must implement the CI physic layer write operation
 *                      (check status, get read data size and read data out, etc).
 *
 */
alisl_retcode alislcic_write_io_data(alisl_handle handle, uint32_t slot,
                                     size_t size, unsigned char *buffer)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    if(CIC_MSG_MAX_LEN < size) {
        SL_DBG("Invalide parameter!");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }
    /* config the message which will send to cic driver */
    memset(&dev->cic_msg, 0, sizeof(struct cic_msg));
    dev->cic_msg.index = (uint16_t)slot;
    dev->cic_msg.type = CIC_BLOCK << 16;
    dev->cic_msg.length = (uint16_t)size;
    memcpy(dev->cic_msg.msg, buffer, size);
    if (ioctl(dev->fd, CIC_SEND_MSG, &dev->cic_msg) != 0) {
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }

    pthread_mutex_unlock(&m_mutex);
    return 0;
}


alisl_retcode alislcic_write_mem(alisl_handle handle, uint32_t slot,
                                 size_t size, unsigned short addr, unsigned char *buffer)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    if(!buffer) {
        SL_ERR("Invalid parameter\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    if(CIC_MSG_MAX_LEN < size) {
        SL_DBG("Invalide parameter!");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    dev->cic_msg.index = slot;
    dev->cic_msg.type = (CIC_MEMORY << 16) | addr;
    dev->cic_msg.length = size;
    memcpy(dev->cic_msg.msg, buffer, size);
    if (ioctl(dev->fd, CA_SEND_MSG, &dev->cic_msg) != 0) {
        SL_DBG("Write memory error!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }

    SL_DBG("ALISLCIC INFO:cic CIC_DRIVER_WRITEMEM," \
                  "slot:%d, addr:%d.\n", slot, addr);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}


alisl_retcode alislcic_read_mem(alisl_handle handle, uint32_t slot,
                                size_t size, unsigned short addr, unsigned char *buffer)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    if(!buffer) {
        SL_ERR("Invalid parameter\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    if(CIC_MSG_MAX_LEN < size) {
        SL_DBG("Invalid parameter!");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    dev->cic_msg.index = slot;
    dev->cic_msg.type = (CIC_MEMORY << 16) | addr;
    dev->cic_msg.length = size;
    if (ioctl(dev->fd, CA_GET_MSG, &dev->cic_msg) != 0) {
        SL_DBG("Read memory error!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }
    memcpy(buffer, dev->cic_msg.msg, size);

    SL_DBG("ALISLCIC INFO:cic CIC_DRIVER_READMEM," \
                  "slot:%d, addr:%d.\n", slot, addr);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}


alisl_retcode alislcic_write_io_reg(alisl_handle handle, uint32_t slot,
                                    uint32_t reg, unsigned char *buffer)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    if(!buffer) {
        SL_ERR("Invalid parameter\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    dev->cic_msg.index = slot;
    dev->cic_msg.type = reg << 16;
    dev->cic_msg.length = 1;
    dev->cic_msg.msg[0] = buffer[0];
    if (ioctl(dev->fd, CIC_SEND_MSG, &dev->cic_msg) != 0) {
        SL_DBG("Write IO error!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }

    SL_DBG("ALISLCIC INFO:cic CIC_DRIVER_WRITEIO,slot"\
                  "%d, reg:%d, data:0x%02x.\n",slot, \
                  reg, buffer[0]);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}


alisl_retcode alislcic_read_io_reg(alisl_handle handle, uint32_t slot,
                                   uint32_t reg, unsigned char *buffer)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    if(!buffer) {
        SL_ERR("Invalid parameter\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    dev->cic_msg.index = slot;
    dev->cic_msg.type = reg << 16;
    dev->cic_msg.length = 1;
    if (ioctl(dev->fd, CIC_GET_MSG, &dev->cic_msg) != 0) {
        SL_DBG("Read IO error!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }
    buffer[0] = dev->cic_msg.msg[0];
    SL_DBG("ALISLCIC INFO:cic CIC_DRIVER_READIO,slot" \
                  ":%d, reg:%d, data:0x%02x.\n",slot, \
                  reg, buffer[0]);

    pthread_mutex_unlock(&m_mutex);
    return 0;

}

/*Test a hardware PC card signal*/
alisl_retcode alislcic_test_signal(alisl_handle handle, uint32_t slot,
                                   uint32_t signal, unsigned char *status)
{
    int ret = 0;

    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    if(!status) {
        SL_ERR("Invalid parameter\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    dev->cic_slot_info.num = slot;
    dev->cic_slot_info.type = signal;
    SL_DBG("dev->cic_slot_info.num:%d,dev->cic_slot_info.type:%d\n",dev->cic_slot_info.num,\
                  dev->cic_slot_info.type);
    ret = ioctl(dev->fd, CIC_GET_SLOT_INFO, &dev->cic_slot_info);
    if (ret) {
        SL_DBG("Test signal error!,%d,ret=%d\n",errno,ret);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }
    *status = dev->cic_slot_info.flags;
    SL_DBG("ALISLCIC INFO:cic CIC_DRIVER_TSIGNAL,slot"\
                  "%d, signal:%d, status:%d.\n",slot,\
                  signal, *status);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}

/* Set or clear a PC card pin */
alisl_retcode alislcic_set_signal(alisl_handle handle, uint32_t slot,
                                  uint32_t signal, unsigned char status)
{
    //int ret = 0;
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }
    dev->cic_slot_info.num = slot;
    dev->cic_slot_info.type = signal;
    dev->cic_slot_info.flags = status;
    if (ioctl(dev->fd, CIC_SET_SLOT_INFO, &dev->cic_slot_info) != 0) {
        SL_DBG("Set signal error!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_IOACCESS;
    }
    SL_DBG("ALISLCIC INFO:cic CIC_DRIVER_SSIGNAL,slot"\
                  ":%d, signal:%d, status:%d.\n",slot,\
                  signal, status);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}


alisl_retcode alislcic_ioctl(alisl_handle handle, unsigned int cmd, unsigned long param)
{
//    int ret = 0;
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)handle;
    pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_ERR("Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDHANDLE;
    }

    /* If device not open, exit */
    if (!bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
        SL_WARN("Device %s not open yet!\n",
                      ALISLCIC_DEV_NAME);
        pthread_mutex_unlock(&m_mutex);
        return ALISLCIC_ERR_INVALIDPARAM;
    }

    switch(cmd) {
        case CIC_DRIVER_SETPORT:
            if(st_event.fd<=0 || st_event.cb == NULL) {
                SL_DBG("have not registed callback\n");
                pthread_mutex_unlock(&m_mutex);
                return 0;
            }
            param = i_sockaddr;
            SL_DBG("socket communication port: %d\n", param);
            if (ioctl(dev->fd, CIC_SET_PORT_ID, &param)) {
                SL_DBG("Set socket communication port error!\n");
                pthread_mutex_unlock(&m_mutex);
                return ALISLCIC_ERR_IOACCESS;
            }
            break;
        default:
            SL_WARN("Unkown command!\n");
            break;
    }

    pthread_mutex_unlock(&m_mutex);
    return 0;
}


alisl_retcode alislcic_destruct(alisl_handle handle)
{
    /* Transfer the handle to cic device point */
    cic_device_t *dev = (cic_device_t *)(handle);
	pthread_mutex_lock(&m_mutex);
    if (dev == NULL) {
        SL_DBG("ALISLCIC WAR:Device didn't construct!\n");
        pthread_mutex_unlock(&m_mutex);
        return 0;
    } else {
        if (bit_test_any(dev->status, ALISLCIC_STATUS_OPEN)) {
            SL_DBG("ALISLCIC WAR:CIC device not closed yet,"\
                          "should be closed befor you call this function!\n");
			pthread_mutex_unlock(&m_mutex);
			return ALISLCIC_ERR_INVALIDPARAM;
        }

        free(dev);
        handle = NULL;
    }

    pthread_mutex_unlock(&m_mutex);
    return 0;
}

