/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2010 Copyright (C)
 *  (C)
 *  File: rfk.c
 *  (I)
 *  Description: receive message from kenel
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2010.06.18			Sam			Create
 ****************************************************************************/
//#include <osal/osal.h>
//#include <api/libc/alloc.h>
//#include <api/libc/printf.h>
//#include <api/libc/string.h>
//#include <sys_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include <alipltflog.h>
#include <alislinput_rfk.h>
#include <ali_netlink_common.h>

#define PRF  SL_DBG

#define MUTEX_LOCK	pthread_mutex_lock(&m_mutex)
#define MUTEX_UNLOCK pthread_mutex_unlock(&m_mutex)

static int m_rfk_init = 0;
static struct alislinput_rfk_dev *m_dev[MAX_RFK_PORT_NUM];
static pthread_mutex_t m_mutex;

unsigned char *alislinput_rfk_receive_msg(int port_id)
{
	struct alislinput_rfk_dev *dev = NULL;
	int ret = 0;

	dev = m_dev[port_id - 1];
	memset(dev->nlmsghdr, 0, NLMSG_SPACE(MAX_RFK_MSG_SIZE));
	dev->iov.iov_base = (void *)dev->nlmsghdr;
	dev->iov.iov_len = NLMSG_SPACE(MAX_RFK_MSG_SIZE);
	dev->msg.msg_name = (void *)&(dev->daddr);
	dev->msg.msg_namelen = sizeof(dev->daddr);
	dev->msg.msg_iov = &(dev->iov);
	dev->msg.msg_iovlen = 1;

	ret = recvmsg(dev->sock_id, &(dev->msg), 0);
	if(ret == -1)
	{
		PRF("Receive netlink data fail %d\n", port_id);
		return NULL;
	}

	return (NLMSG_DATA(dev->nlmsghdr));
}

int alislinput_rfk_get_socket(int port_id)
{
	if (m_dev[port_id - 1])
	{
		return m_dev[port_id - 1]->sock_id;
	}
	else
	{
		PRF("rfk dev is NULL, port_id = %d\n", port_id);
	}

	return -1;
}

int alislinput_rfk_get_port(void)
{
	struct alislinput_rfk_dev *dev = NULL;
	int port_id = -1;
	int i = 0;
	int ret = -1;

	MUTEX_LOCK;

	for(i = 0;i < MAX_RFK_PORT_NUM;i++)
		if(m_dev[i] == NULL)
			break;

	if(i >= MAX_RFK_PORT_NUM)
	{
		PRF("can't get the free rfk port\n");
		goto EXIT;
	}

	dev = (struct alislinput_rfk_dev *)calloc(1, sizeof(struct alislinput_rfk_dev));
	if(dev == NULL)
	{
		PRF("malloc rfk dev fail\n");
		goto EXIT;
	}
	//memset(dev, 0, sizeof(struct alislinput_rfk_dev));

	dev->sock_id = socket(AF_NETLINK, SOCK_RAW, NETLINK_ALITRANSPORT);
	if(dev->sock_id < 0)
	{
		PRF("create sock fail\n");
		goto EXIT;
	}

	dev->nlmsghdr = (struct nlmsghdr *)calloc(1, NLMSG_SPACE(MAX_RFK_MSG_SIZE));
	if(dev->nlmsghdr == NULL)
	{
		PRF("malloc rfk msg hdr fail\n");
		goto EXIT;
	}

	for (i=i; i<MAX_RFK_PORT_NUM; i++)
	{
		port_id = i + 1;
		dev->port_id = port_id;
		dev->saddr.nl_family = AF_NETLINK;
		dev->saddr.nl_pid = port_id;
		dev->saddr.nl_groups = 0;
		ret = bind(dev->sock_id, (struct sockaddr *)&dev->saddr, sizeof(dev->saddr));
		if (0 != ret)
		{
			PRF("[ %s %d ], ret = %d, port_id = %d\n", __FUNCTION__, __LINE__, ret, i);
		}
		else
		{
			break;
		}
	}

	if(i >= MAX_RFK_PORT_NUM)
	{
		PRF("[ %s %d ], can't get the free rfk port\n", __FUNCTION__, __LINE__);
		goto EXIT;
	}
	m_dev[i] = dev;
	dev->busy = 1;

EXIT:
	if((dev != NULL))
	{
		if (dev->busy == 0) {
			if (dev->sock_id >= 0)
			{
				close(dev->sock_id);
			}
			if (dev->nlmsghdr)
			{
				free(dev->nlmsghdr);
				dev->nlmsghdr = NULL;
			}
			free((void *)dev);
			dev = NULL;
		} else {
			// Fixed cpptest error.
			if(i < MAX_RFK_PORT_NUM)
			{
				m_dev[i] = dev;
			}
		}
	}

	MUTEX_UNLOCK;

	PRF("get the rfk port %d\n", port_id);
	return port_id;
}

void alislinput_rfk_free_port(int port_id)
{
	struct alislinput_rfk_dev *dev = NULL;

	if(port_id <= 0 || port_id > MAX_RFK_PORT_NUM)
	{
		PRF("free port_id fail %d\n", port_id);
		return;
	}

	if(m_dev[port_id - 1] == NULL)
	{
		PRF("free empty port_id %d\n", port_id);
		return;
	}

	MUTEX_LOCK;

	dev = m_dev[port_id - 1];
	if(dev->busy)
	{
		dev->busy = 0;
		close(dev->sock_id);
		free(dev->nlmsghdr);
		free(dev);
		dev = NULL;
	}

	MUTEX_UNLOCK;

	PRF("rfk free port done %d\n", port_id);
}

void alislinput_rfk_init(void)
{
	int i = 0;

	for(i = 0;i < MAX_RFK_PORT_NUM;i++)
		m_dev[i] = NULL;

	//m_mutex = osal_mutex_create();
	if(pthread_mutex_init(&m_mutex, NULL))
	{
		PRF("create rfk mutex fail\n");
		goto FAIL;
	}

	m_rfk_init = 1;
	PRF("init rfk done\n");

	return;

FAIL:
//	if(m_mutex != OSAL_INVALID_ID)
//		osal_mutex_delete(m_mutex);
	return;
}

void alislinput_rfk_exit(void)
{
	if(m_rfk_init)
	{
		m_rfk_init = 0;

//		if(m_mutex != OSAL_INVALID_ID)
//			osal_mutex_delete(m_mutex);
		pthread_mutex_destroy(&m_mutex);
	}
}

