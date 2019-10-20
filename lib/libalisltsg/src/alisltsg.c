/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisltsg.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/23/2013 15:58:21
 *  @revision           none
 *
 *  @author             Franky.Liang <franky.liang@alitech.com>
 */


/* system headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

/* kernel headers */

#include <linux/ioctl.h>
#include <ali_tsg_common.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alipltflog.h>
#include <alisltsg.h>

/* local headers */
#include "internel.h"

typedef struct dev_tsg {
	enum tsg_id     id;
	const char      *path;
	const char      *pathfeed;      /**< when playback, we need to feed data
	                                     to device specified by pathfeed */
} dev_tsg_t;

static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;
static struct tsg_device *m_dev[TSG_NB]={0};
struct dev_tsg dev_tsg[] = {
	{TSG_ID_M36_0, "/dev/ali_m36_tsg_0",    NULL},
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 *  Function Name:      alisltsg_construct
 *  @brief
 *
 *  @param handle       pointer to struct tsg_device
 *
 *  @return
 *
 *  @author             Franky.Liang <franky.liang@alitech.com>
 *  @date               7/23/2013, Created
 *
 *  @note
 */


static alisl_retcode alisltsg_construct(alisl_handle *handle)
{

	struct tsg_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		SL_DBG("malloc memory failed!\n");
		return ERROR_NOMEM;
	}

	memset(dev, 0, sizeof(*dev));

	flag_init(&dev->status, TSG_STATUS_CONSTRUCT);

	*handle = dev;

	return 0;

}


/**
 *  Function Name:     alisltsg_destruct
 *  @brief
 *
 *  @param  handle     pointer to struct tsg_device
 *  @param
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

static alisl_retcode alisltsg_destruct(alisl_handle *handle)
{
    struct tsg_device *dev = NULL;

    dev = (struct tsg_device *)(*handle);

/*    if (!flag_bit_test_any(&dev->status, TSG_STATUS_CLOSE)) {
        SL_DBG("Tsg not closed yet, now close it!\n");
        alisltsg_close(dev);
    }
*/
    free(dev);
    *handle = NULL;

    return 0;
}



/**
 *  Function Name:        alisltsg_open
 *  @brief
 *
 *  @param  handle        pointer to struct tsg_device
 *  @param  id            enum tsg_id
 *  tunner_config_handle  pointer to param
 *  @return               alisl_retcode
 *
 *  @author               Franky.Liang  <franky.liang@alitech.com>
 *  @date                 7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_open(alisl_handle *handle, enum tsg_id id, void *param)
{
	struct tsg_device *dev = (struct tsg_device *)handle;
	int i;
	/*
	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	*/
	for (i=0; i<ARRAY_SIZE(dev_tsg); i++) {
		if (id == dev_tsg[i].id) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev_tsg)) {
		SL_DBG("Invalidate tsg id!\n");
		return ERROR_INVAL;
	}


	pthread_mutex_lock(&m_mutex);
	if (m_dev[i] == NULL) {
		if (alisltsg_construct((alisl_handle *)&dev)) {
			*handle = NULL;
			pthread_mutex_unlock(&m_mutex);
			return 1;
		}
	} else {
		dev = m_dev[i];
	}

	if (flag_bit_test_any(&dev->status, TSG_STATUS_OPEN)) {
		SL_DBG("Already opened!\n");
		//nb_opened_dmx ++;
		dev->open_cnt ++;
		m_dev[i] = dev;
		*handle = dev;
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	dev->id = id;
	dev->path = dev_tsg[i].path;
	dev->pathfeed = dev_tsg[i].pathfeed;

	dev->fd = open(dev->path, O_RDWR);
	if (dev->fd < 0) {
		SL_DBG("%s open failed!\n", dev->path);
		free(dev);
		pthread_mutex_unlock(&m_mutex);
		return ERROR_OPEN;
	}

	if (dev->pathfeed != NULL) {
		dev->feedfd = open(dev->pathfeed, O_RDWR);
		if (dev->feedfd < 0) {
			close(dev->fd);
			SL_DBG("%s open failed\n", dev->pathfeed);
			free(dev);
			pthread_mutex_unlock(&m_mutex);
			return ERROR_OPEN;
		}
	}
	flag_bit_set(&dev->status, TSG_STATUS_OPEN);
	flag_bit_clear(&dev->status, TSG_STATUS_CLOSE);
	flag_bit_set(&dev->status, TSG_STATUS_START);

	dev->open_cnt ++;
	m_dev[i] = dev;
	*handle = dev;
	pthread_mutex_unlock(&m_mutex);
	return 0;

}

/**
 *  Function Name:     alisltsg_set_clkasync
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param clk_sel     tsg clk to be set
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_set_clkasync(alisl_handle handle, uint32_t clk_sel)
{

	struct tsg_device *dev = (struct tsg_device *)handle;


	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, TSG_STATUS_START)) {
		SL_DBG("Tsg have not start\n");
		return ERROR_STATUS;
	}

	int ret = ioctl(dev->fd, ALI_TSG_OUTPUT_CLK_SET, &clk_sel);
	if (ret) {
		SL_DBG("Tsg ioctl cmd 0x%x errno %d\n",
			 ALI_TSG_OUTPUT_CLK_SET, errno);
		return ERROR_INVAL;
	}
	return 0;
}


/**
 *  Function Name:     alisltsg_insertionstart
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param bitrate     ts bitrate
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_insertionstart(alisl_handle handle, uint32_t bitrate)
{

	struct tsg_device *dev = (struct tsg_device *)handle;


	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, TSG_STATUS_START)) {
		SL_DBG("Tsg have not start\n");
		return ERROR_STATUS;
	}

	//ALI_TSG_NULL_PKT_INSERTION_START command is not implemented. It just returns an error.
	//But it's ok. It works normally! Just keep it.
#if 0
	int ret = ioctl(dev->fd, ALI_TSG_NULL_PKT_INSERTION_START, &bitrate);
	if (ret) {
		SL_DBG("Tsg ioctl cmd 0x%x errno %d\n",
			 ALI_TSG_NULL_PKT_INSERTION_START, errno);
		return ERROR_INVAL;
	}
#endif
	return 0;
}

/**
 *  Function Name:     alisltsg_insertionstop
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_insertionstop(alisl_handle handle)
{

	struct tsg_device *dev = (struct tsg_device *)handle;


	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, TSG_STATUS_START)) {
		SL_DBG("Tsg have not start\n");
		return ERROR_STATUS;
	}

#if 0
	//ALI_TSG_NULL_PKT_INSERTION_STOP command is not implemented. It just returns an error.
	//But it's ok. It works normally! Just keep it
	int ret = ioctl(dev->fd, ALI_TSG_NULL_PKT_INSERTION_STOP);
	if (ret) {
		SL_DBG("Tsg ioctl cmd 0x%x errno %d\n",
			 ALI_TSG_NULL_PKT_INSERTION_STOP, errno);
		return ERROR_INVAL;
	}
#endif
	return 0;
}


/**
 *  Function Name:     alisltsg_copydata
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param addr        point to transfer data addr
 *  @param addr        transfer data packet number
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_copydata(alisl_handle handle, void *addr,uint32_t byte_cnt)
{

	struct tsg_device *dev = (struct tsg_device *)handle;

#ifdef CONFIG_FAST_COPY
	struct tsg_fast_copy_param    param;
#endif

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, TSG_STATUS_START)) {
		SL_DBG("Tsg have not start\n");
		return ERROR_STATUS;
	}

#ifdef CONFIG_FAST_COPY
	param.data = addr;
	param.len = byte_cnt;
	int ret = ioctl(dev->fd, ALI_TSG_WRITE_FSTCPY, &param);
	if (ret) {
		SL_DBG("Tsg ioctl cmd 0x%x errno %d\n",
			 ALI_TSG_WRITE_FSTCPY, errno);
		return ERROR_INVAL;
	}
#else
	if(write(dev->fd, addr, byte_cnt) == -1)
	{
		SL_DBG("write failed\n");
	}
#endif

	return 0;
}



/**
 *  Function Name:     alisltsg_check_remain_buf
 *  @brief
 *
 *  @param handle      point to struct tsg_device
 *  @param byte_cnt     remain ts pkt cnt
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note  Acturally means getting total ts pkt cnt stored in TSG buffer which
 *   has not been sent out by hw yet.
 */

alisl_retcode alisltsg_check_remain_buf(alisl_handle handle, uint32_t *byte_cnt)
{

	struct tsg_device *dev = (struct tsg_device *)handle;
	uint32_t cur_data_len;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, TSG_STATUS_START)) {
		SL_DBG("Tsg have not start\n");
		return ERROR_STATUS;
	}

	int ret = ioctl(dev->fd, ALI_TSG_GET_CUR_DATA_LEN, &cur_data_len);
	if (ret) {
		SL_DBG("Tsg ioctl cmd 0x%x errno %d\n",
			 ALI_TSG_GET_CUR_DATA_LEN, errno);
		return ERROR_INVAL;
	}

	*byte_cnt = cur_data_len;

	return 0;
}

/**
 *  Function Name:      alisltsg_close
 *  @brief
 *
 *  @param handle       point to struct tsg_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsg_close(alisl_handle *handle)
{
	struct tsg_device *dev = (struct tsg_device *)*handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
	pthread_mutex_lock(&m_mutex);

	if (--dev->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}
	if (!flag_bit_test_any(&dev->status, TSG_STATUS_OPEN)) {
		SL_DBG("Tsg device not opened!\n");
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	if (flag_bit_test_any(&dev->status, TSG_STATUS_CLOSE)) {
		SL_DBG("TSG device already closed!\n");
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	if (dev->feedfd > 0) {
		close(dev->feedfd);
		dev->feedfd = -1;
	}

	close(dev->fd);
	dev->fd = -1;

	flag_bit_clear(&dev->status, TSG_STATUS_OPEN | TSG_STATUS_START);
	flag_bit_set(&dev->status, TSG_STATUS_CLOSE);
	m_dev[dev->id]=NULL;
	alisltsg_destruct((alisl_handle *)handle);
	pthread_mutex_unlock(&m_mutex);
	return 0;
}
