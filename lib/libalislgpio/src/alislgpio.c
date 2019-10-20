/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file                  	alislgpio.c
 *  @brief               	ALi gpio irq device interfaces. All applications and other
 *                      		function should only access hardware or driver by
 *                      		this function interface.
 *
 *  @version            	1.0
 *  @date               	2016.07.14  02:24:25 PM
 *  @revision           	none
 *
 *  @author             	Steven.Zhang <steven.zhang@alitech.com>
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>

/* kernel headers */
#include <ali_gpio_common.h>
#include <sys/ioctl.h>
#include <ali_audio_common.h>

/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislevent.h>

/* local headers */
#include "internal.h"

static struct alislevent sl_gpio_cb_event;

static struct gpio_sl_dev *g_gpio_sl_dev = NULL;

static pthread_mutex_t m_sl_gpio_mutex      = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_mutex_gpio_cb = PTHREAD_MUTEX_INITIALIZER;

int sl_gpio_del_poll_fd(struct gpio_sl_dev *dev);
int sl_gpio_add_poll_fd(struct gpio_sl_dev *dev);
alisl_retcode alislgpio_construct(alisl_handle *handle);
alisl_retcode alislgpio_destruct(alisl_handle *handle);

#define GPIO_DEV_PATH          "/dev/ali_gpio"

/**
 *  Function Name:      	alislgpio_open
 *  @brief             	open actual hardware device
 *
 * @param[in]     		handle:	gpio irq device pointer
 *
 *  @return             	alisl_retcode
 *
 *  @author             	Steven Zhang <steven.zhang@alitech.com>
 *  @date               	2016.07.14, Created
 *
 *  @note
 */

alisl_retcode alislgpio_open(alisl_handle *handle)
{
	struct gpio_sl_dev *p_gpio_dev = NULL;

	pthread_mutex_lock(&m_sl_gpio_mutex);
	if (NULL == g_gpio_sl_dev) {
		alislgpio_construct((alisl_handle*)&p_gpio_dev);
		if (NULL == p_gpio_dev) {
			pthread_mutex_unlock(&m_sl_gpio_mutex);
			return -1;
		}
		if ((p_gpio_dev->fd = open(GPIO_DEV_PATH, O_RDWR)) < 0) {//"dev/ali_gpio"

			SL_DBG("GPIO device open failed!\n");
                        alislgpio_destruct((alisl_handle*)&p_gpio_dev);
			pthread_mutex_unlock(&m_sl_gpio_mutex);
			return -1;
		}
		g_gpio_sl_dev = p_gpio_dev;
	}
	g_gpio_sl_dev->open_cnt++;
	*handle = g_gpio_sl_dev;
	SL_DBG("g_gpio_sl_dev:%p g_gpio_sl_dev->fd:%d\n", g_gpio_sl_dev,g_gpio_sl_dev->fd);

	pthread_mutex_unlock(&m_sl_gpio_mutex);
	return 0;
}

/**
 * @brief         gpio device close, close gpio irq device handle and dell the msg event socket fd.
 * @author        steven.zhang
 * @date          2016.07.18
 *
 * @param[in]     handle:	gpio irq device pointer
 *
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislgpio_close(alisl_handle *handle)
{
	gpio_sl_dev *priv = (gpio_sl_dev *)*handle;

	if (priv == NULL) {
		SL_DBG("NULL gpio device data structure!\n");
		return -1;
	}
	pthread_mutex_lock(&m_sl_gpio_mutex);

	if (--priv->open_cnt) {
		pthread_mutex_unlock(&m_sl_gpio_mutex);
		return 0;
	}

    if (priv->kumsg_fd!= -1) {
        SL_DBG("sl_gpio_del_poll_fd\n");
        sl_gpio_del_poll_fd(priv);
        priv->handle_msg = NULL;
    }

	close(priv->fd);
	priv->fd = -1;

	alislgpio_destruct((alisl_handle*)handle);
	g_gpio_sl_dev = NULL;

	pthread_mutex_unlock(&m_sl_gpio_mutex);
	return 0;
}

/**
 * @brief          set the gpio reg call back funtion, it will be falling or rising.
 * @author        steven.zhang
 * @date          2016.07.18
 *
 * @param[in]     hdl:				gpio device pointer
 * @param[in]     p_regcb_param:	the param of callback funtion's will be register.
 *
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */

alisl_retcode alislgpio_callback_reg(alisl_handle hdl, gpio_io_reg_callback_param *regcb_param)
{
    gpio_io_reg_callback_param *p_regcb_param= regcb_param;
    gpio_sl_dev *gpio_dev = (gpio_sl_dev *)hdl;
    alisl_retcode ret = 0;

	gpio_info gpio_info_set;

    if(!gpio_dev) {//p_regcb_param don't be check for null.
        SL_ERR("invalid arg, Maybe not Open.\n");
        return -1;
    }
	int index = p_regcb_param->gpio_index;

	pthread_mutex_lock(&m_sl_gpio_mutex);

    if (-1 == gpio_dev->kumsg_fd) {

        SL_DBG("before sl_gpio_add_poll_fd\n");
        if (sl_gpio_add_poll_fd(gpio_dev)) {

            SL_ERR("sl_gpio_add_poll_fd fail\n");
            pthread_mutex_unlock(&m_sl_gpio_mutex);
            return -1;
        }
    }

	switch (regcb_param->irq_type) {
		case  GPIO_INTERRUPT_DISABLED:
			/* tell kernel about nl_gpio_pid */
			if (0 != ioctl(gpio_dev->fd, GPIO_SET_IRQ_DISENABLE, index)) {
				SL_ERR("GPIO_SET_IRQ_ENABLE failed\n");
			}
			break;

		case  GPIO_INTERRUPT_RISING_EDGE:
		case  GPIO_INTERRUPT_FALLING_EDGE:
		case  GPIO_INTERRUPT_EDGE: //set rising and falling
			/* tell kernel about nl_gpio_pid */

			SL_DBG("before GPIO_SET_IRQ_ENABLE\n");
			if (0 != ioctl(gpio_dev->fd, GPIO_SET_IRQ_ENABLE, index)) {
				SL_ERR("GPIO_SET_IRQ_ENABLE failed\n");
				ret = -1;
			}
			gpio_info_set.gpio = index;
			gpio_info_set.status = regcb_param->irq_type;

			if (0 != ioctl(gpio_dev->fd,GPIO_SET_IRQ_TYPE,(unsigned long)&gpio_info_set)) {
				SL_ERR("GPIO_SET_IRQ_TYPE failed\n");
				ret = -1;
			} else {
				SL_ERR("GPIO_SET_IRQ_TYPE sucess!\n");
				gpio_dev->handle_msg = p_regcb_param->p_cb;
			}

			break;

		default:
			SL_ERR("It's default\n");
			break;

		}

	pthread_mutex_unlock(&m_sl_gpio_mutex);

	return ret;
}

/**
 * @brief          		unregister gpio  call back funtion
 * @author        		steven.zhang
 * @date          		2016.07.18
 *
 * @param[in]     		hdl:				gpio device pointer
 * @param[in]     		p_regcb_param:	the param of callback funtion's will be unregister.
 *
 * @param[out]
 * @return        		alisl_retcode
 * @note
 *
 */

alisl_retcode alislgpio_callback_unreg(alisl_handle hdl, unsigned int index)
{
	 struct gpio_sl_dev *gpio_dev = (struct gpio_sl_dev *)hdl;
	 alisl_retcode ret = 0;

	 if (!gpio_dev) {
		 SL_ERR("invalid arg, Maybe not open\n");
		 return -1;
	 }

	 pthread_mutex_lock(&m_sl_gpio_mutex);

	 /* tell kernel about nl_gpio_pid */
	 if (0 != ioctl(gpio_dev->fd, GPIO_SET_IRQ_DISENABLE, index)) {
		 SL_ERR("GPIO_SET_IRQ_ENABLE failed\n");
		 ret = -1;
	 }

	 pthread_mutex_unlock(&m_sl_gpio_mutex);

	 return ret;

}

static void sl_gpio_handle_msg(void *msg)
{
	pthread_mutex_lock(&m_mutex_gpio_cb);

    if(!g_gpio_sl_dev) {
		SL_ERR("device has been destroyed\n");
        pthread_mutex_unlock(&m_mutex_gpio_cb);
		return ;
	}
	//msg don't be check for null, because it has been referenced.

	if (g_gpio_sl_dev->handle_msg) {
		gpio_info* p_info = (gpio_info*)msg;
		if (!p_info) {
			SL_ERR("p_info is null!\n");
			pthread_mutex_unlock(&m_mutex_gpio_cb);
			return;
		}

		g_gpio_sl_dev->handle_msg(p_info->gpio, p_info->status);
	}

	pthread_mutex_unlock(&m_mutex_gpio_cb);
}

static unsigned char *sl_gpio_receive_msg(int fd)
{
    unsigned char msg[MAX_KUMSG_SIZE] = {0};
	int ret = 0;
    //g_gpio_sl_dev don't be check for null.

	ret = read(fd, msg, MAX_KUMSG_SIZE);

	if (ret > 0)
    {
    	sl_gpio_handle_msg(msg);
    }

    return NULL;

}

static void* sl_gpio_poll_cb(void *param)
{
	void *msg=NULL;

	SL_DBG("enter\n");
	if (!g_gpio_sl_dev || (0 == g_gpio_sl_dev->kumsg_fd)) {
		SL_ERR("device has been destroyed\n");
		return NULL;
	}

	do {
		msg = sl_gpio_receive_msg(g_gpio_sl_dev->kumsg_fd);

		sl_gpio_handle_msg(msg);
	}while(1);
	return NULL;
}

int sl_gpio_add_poll_fd(struct gpio_sl_dev *dev)
{
	int flags = O_CLOEXEC;

        SL_DBG("before GPIO_GET_KUMSGQ\n");
	if((dev->kumsg_fd = ioctl(dev->fd, GPIO_GET_KUMSGQ, &flags)) < 0) {
            SL_ERR("GPIO_GET_KUMSGQ fail(%d)\n", dev->kumsg_fd);
            return -1;
	}

	//create_fd
	sl_gpio_cb_event.cb = sl_gpio_poll_cb;
	sl_gpio_cb_event.data = (void *)dev;
	sl_gpio_cb_event.events = EPOLLIN;
	sl_gpio_cb_event.fd = dev->kumsg_fd;

        if(alislevent_open(&dev->gpio_event_handle))
        {
            SL_ERR("alislevent_open fail\n");
            return -1;
        }

        if(alislevent_add(dev->gpio_event_handle, &sl_gpio_cb_event))
        {
            SL_ERR("alislevent_add fail\n");
            alislevent_close(dev->gpio_event_handle);
            return -1;
        }
        return 0;
}

int sl_gpio_del_poll_fd(struct gpio_sl_dev *dev)
{
    alislevent_del(dev->gpio_event_handle, &sl_gpio_cb_event);
    memset(&sl_gpio_cb_event, 0, sizeof(sl_gpio_cb_event));
    alislevent_close(dev->gpio_event_handle);
    dev->gpio_event_handle = NULL;
	return 0;
}

alisl_retcode alislgpio_construct(alisl_handle *handle)
{
	struct gpio_sl_dev *dev = malloc(sizeof(gpio_sl_dev));
	if (NULL == dev) {
		SL_DBG("malloc memory failed!\n");
		return ERROR_NOMEM;
	}

	memset(dev, 0, sizeof(*dev));

	dev->fd = -1;
	dev->portid = -1;
	dev->kumsg_fd = -1;
	dev->status = GPIO_STATUS_CONSTRUCT;

	*handle = dev;

	return 0;
}

alisl_retcode alislgpio_destruct(alisl_handle *handle)
{
	struct gpio_sl_dev *dev = (struct gpio_sl_dev *)(*handle);

	free(dev);
	*handle = NULL;

	return 0;
}
