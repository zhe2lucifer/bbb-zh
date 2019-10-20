/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisltsi.c
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
#include <ali_tsi_common.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alipltflog.h>
#include <alisltsi.h>

/* local headers */
#include "internel.h"

typedef struct dev_tsi {
	enum tsi_id     id;
	const char      *path;
	const char      *pathfeed;      /**< when playback, we need to feed data\n
	                                     to device specified by pathfeed */
} dev_tsi_t;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;
static struct tsi_device *m_dev[TSI_NB]={0};
struct dev_tsi dev_tsi[] = {
	{TSI_ID_M3602_0, "/dev/ali_m36_tsi_0",    NULL},
};

/**
 *  Function Name:      alisltsi_construct
 *  @brief
 *
 *  @param handle       pointer to struct tsi_device
 *
 *  @return
 *
 *  @author             Franky.Liang <franky.liang@alitech.com>
 *  @date               7/23/2013, Created
 *
 *  @note
 */


static alisl_retcode alisltsi_construct(alisl_handle *handle)
{

	struct tsi_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		SL_DBG("malloc memory failed!\n");
		return ERROR_NOMEM;
	}

	memset(dev, 0, sizeof(*dev));

	flag_init(&dev->status, TSI_STATUS_CONSTRUCT);

	*handle = dev;

	return 0;

}

/**
 *  Function Name:     alisltsi_destruct
 *  @brief
 *
 *  @param  handle     pointer to struct tsi_device
 *  @param
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

static alisl_retcode alisltsi_destruct(alisl_handle *handle)
{
    struct tsi_device *dev = (struct tsi_device *)(*handle);

    if (!flag_bit_test_any(&dev->status, TSI_STATUS_CLOSE)) {
        SL_DBG("Tsi not closed yet, now close it!\n");
        alisltsi_close(dev);
    }
    free(dev);
    *handle = NULL;

	return 0;
}

/**
 *  Function Name:        alisltsi_open
 *  @brief
 *
 *  @param  handle        pointer to struct tsi_device
 *  @param  id            enum tsi_id
 *  tunner_config_handle  pointer to param
 *  @return               alisl_retcode
 *
 *  @author               Franky.Liang  <franky.liang@alitech.com>
 *  @date                 7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_open(alisl_handle *handle, enum tsi_id id, void *param)
{
	struct tsi_device *dev;
	int i;
	/*
	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	*/
	
	for (i=0; i<ARRAY_SIZE(dev_tsi); i++) {
		if (id == dev_tsi[i].id) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev_tsi)) {
		SL_DBG("Invalidate tsi id!\n");
		return ERROR_INVAL;
	}

	pthread_mutex_lock(&m_mutex);
	if (m_dev[i] == NULL) {
		if (alisltsi_construct((alisl_handle *)&dev)) {
			*handle = NULL;
			pthread_mutex_unlock(&m_mutex);
			return 1;
		}
	} else {
		dev = m_dev[i];
	}
	
	if (flag_bit_test_any(&dev->status, TSI_STATUS_OPEN)) {
		SL_DBG("Already opened!\n");
		//nb_opened_dmx ++;
		dev->open_cnt ++;
		m_dev[i] = dev;
		*handle = dev;
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	dev->id = id;
	dev->path = dev_tsi[i].path;
	dev->pathfeed = dev_tsi[i].pathfeed;

	dev->fd = open(dev->path, O_RDWR);
	if (dev->fd < 0) {
		SL_DBG("%s open failed!\n", dev->path);
		pthread_mutex_unlock(&m_mutex);
		free(dev);
		*handle=NULL;
		return ERROR_OPEN;
	}

	if (dev->pathfeed != NULL) {
		dev->feedfd = open(dev->pathfeed, O_RDWR);
		if (dev->feedfd < 0) {
			close(dev->fd);
			SL_DBG("%s open failed\n", dev->pathfeed);
			pthread_mutex_unlock(&m_mutex);
			free(dev);
			*handle=NULL;
			return ERROR_OPEN;
		}
	}
	flag_bit_set(&dev->status, TSI_STATUS_OPEN);
	flag_bit_clear(&dev->status, TSI_STATUS_CLOSE);
	flag_bit_set(&dev->status, TSI_STATUS_START);

	//nb_opened_dmx ++;
	dev->open_cnt ++;
	m_dev[i] = dev;
	*handle = dev;
	pthread_mutex_unlock(&m_mutex);
	
	return 0;

}

static alisl_retcode alisltsi_inputid_tds_to_linux(uint32_t tds_input_id,
				enum ali_tsi_input_id *linux_input_id)
{
	alisl_retcode ret;

	ret = 0;

	switch (tds_input_id)
	{
		case ALISL_TSI_SPI_0:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI_0;
		}
		break;
		case ALISL_TSI_SPI_1:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI_1;
		}
		break;
		case ALISL_TSI_SPI_TSG:
		{
			*linux_input_id = ALI_TSI_INPUT_TSG;
		}
		break;
		case ALISL_TSI_SPI_3:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI_3;
		}
		break;
		case ALISL_TSI_SSI_0:
		{
			*linux_input_id = ALI_TSI_INPUT_SSI_0;
		}
		break;
		case ALISL_TSI_SSI_1:
		{
			*linux_input_id = ALI_TSI_INPUT_SSI_1;
		}
		break;
		case ALISL_TSI_SSI_2:
		{
			*linux_input_id = ALI_TSI_INPUT_SSI_2;
		}
		break;
		case ALISL_TSI_SSI_3:
		{
			*linux_input_id = ALI_TSI_INPUT_SSI_3;
		}
		break;
		case ALISL_TSI_SSI2B_0:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI2B_0;
		}
		break;
		case ALISL_TSI_SSI2B_1:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI2B_1;
		}
		break;
		case ALISL_TSI_SSI4B_0:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI4B_0;
		}
		break;
		case ALISL_TSI_SSI4B_1:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI4B_1;
		}
		break;
		case ALISL_PARA_MODE_SRC:
		{
			*linux_input_id = ALI_TSI_INPUT_PARA;
		}
		break;
		case ALISL_TSI_SSI2B_2:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI2B_2;
		}
		break;
		case ALISL_TSI_SSI2B_3:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI2B_3;
		}
		break;
		case ALISL_TSI_SSI4B_2:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI4B_2;
		}
		break;
		case ALISL_TSI_SSI4B_3:
		{
			*linux_input_id = ALI_TSI_INPUT_SPI4B_3;
		}
		break;
		
		default:
		{
			SL_DBG("%s, %d\n", __FUNCTION__, __LINE__);
			ret = ERROR_INVAL;
		}
		break;
	}
	return(ret);
}

/**
 *  Function Name:     alisltsi_setinput
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param input_id    tds_input_id
 *  @param attrib      attribute of tsi setting
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_setinput(alisl_handle handle, uint32_t input_id, uint32_t attrib)
{

	struct tsi_device *dev = (struct tsi_device *)handle;
	struct ali_tsi_input_set_param input;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
		SL_DBG("Tsi have not start\n");
		return ERROR_STATUS;
	}
	if (alisltsi_inputid_tds_to_linux(input_id, &input.id))
	{

		SL_DBG("change Tsi input id to linux fail\n");
		return ERROR_INVAL;
	}
	input.attribute = (uint8_t)attrib;

	ioctl(dev->fd, ALI_TSI_INPUT_SET, &input);

	return 0;
}

/**
 *  Function Name:     alisltsi_setchannel
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param input_id    tds_input_id
 *  @param channel_id  channel_id for setting
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_setchannel(alisl_handle handle, uint32_t input_id, int32_t channel_id)
{

	struct tsi_device *dev = (struct tsi_device *)handle;
	struct ali_tsi_channel_set_param channel;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
		SL_DBG("Tsi have not start\n");
		return ERROR_STATUS;
	}

	if (alisltsi_inputid_tds_to_linux(input_id, &channel.input_id))
	{

		SL_DBG("change Tsi input id to linux fail\n");
		return ERROR_INVAL;
	}

	if (channel_id == ALISL_TSI_TS_A)
		channel.channel_id = ALI_TSI_CHANNEL_0;
	else 	if (channel_id == ALISL_TSI_TS_B)
		channel.channel_id = ALI_TSI_CHANNEL_1;
	else 	if (channel_id == ALISL_TSI_TS_C)
		channel.channel_id = ALI_TSI_CHANNEL_2;
	else
		channel.channel_id = ALI_TSI_CHANNEL_3;

	ioctl(dev->fd, ALI_TSI_CHANNEL_SET, &channel);

	return 0;
}

/**
 *  Function Name:     alisltsi_setoutput
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param dmx_id      dmx_id
 *  @param channel_id  channel_id for setting
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_setoutput(alisl_handle handle, int32_t dmx_id, int32_t channel_id)
{

	struct tsi_device *dev = (struct tsi_device *)handle;
	struct ali_tsi_output_set_param output;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
		SL_DBG("Tsi have not start\n");
		return ERROR_STATUS;
	}

	if (ALISL_TSI_TS_A == channel_id)
		output.channel_id = ALI_TSI_CHANNEL_0;
	else if (ALISL_TSI_TS_B == channel_id)
		output.channel_id = ALI_TSI_CHANNEL_1;
	else if (ALISL_TSI_TS_C == channel_id)
		output.channel_id = ALI_TSI_CHANNEL_2;
	else if (ALISL_TSI_TS_D == channel_id)
		output.channel_id = ALI_TSI_CHANNEL_3;
	else {
		SL_DBG("channel_id  error\n");
		return ERROR_INVAL;
	}

	if (ALISL_TSI_DMX_0 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_0;
	else if (ALISL_TSI_DMX_1 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_1;
	else if (ALISL_TSI_DMX_2 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_2;
	else if (ALISL_TSI_DMX_3 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_3;
	else {
		SL_DBG("dmx_id  error\n");
		return ERROR_INVAL;
	}

	ioctl(dev->fd, ALI_TSI_OUTPUT_SET, &output);

	return 0;
}

/**
 *  Function Name:     alisltsi_set_parallel_mode
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param para        tsi ci link mode
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_set_parallel_mode(alisl_handle handle, int32_t para)
{
    struct tsi_device *dev = (struct tsi_device *)handle;
    int ci_link_mode;
    if (dev == NULL) {
        SL_DBG("Try to open before construct!\n");
        return ERROR_NULLDEV;
    }
    if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
        SL_DBG("Tsi have not start\n");
        return ERROR_STATUS;
    }

    if(ALISL_MODE_PARALLEL == para) {
        ci_link_mode = ALI_TSI_CI_LINK_PARALLEL;
    }
    else if(ALISL_MODE_CHAIN == para) {
        ci_link_mode = ALI_TSI_CI_LINK_CHAIN;
    } else {
        SL_DBG("Tsi para error\n");
        return ERROR_INVAL;
    } 

	ioctl(dev->fd, ALI_TSI_CI_LINK_MODE_SET, ci_link_mode);

	return 0;
}

/**
 *  Function Name:     alisltsi_set_cibypass
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param ci_id       ci id
 *  @param chg_en      enable bypass
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_set_cibypass(alisl_handle handle, int32_t ci_id, uint32_t chg_en)
{

	struct tsi_device *dev = (struct tsi_device *)handle;
	struct ali_tsi_ci_bypass_set_param  input;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
		SL_DBG("Tsi have not start\n");
		return ERROR_STATUS;
	}

	input.ci_id = ci_id;
	input.is_bypass = chg_en;

	ioctl(dev->fd, ALI_TSI_CI_BYPASS_SET, &input);

	return 0;
}

/**
 *  Function Name:     alisltsi_change_tsiid
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param chg_en      tsi spi
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_change_tsiid(alisl_handle handle, int32_t chg_en)
{

	struct tsi_device *dev = (struct tsi_device *)handle;
//	struct ali_tsi_ci_bypass_set_param  input;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
		SL_DBG("Tsi have not start\n");
		return ERROR_STATUS;
	}

	ioctl(dev->fd, ALI_TSI_CI_SPI_0_1_SWAP, &chg_en);

	return 0;
}

static alisl_retcode tsi_inputid_linux_to_tds(enum ali_tsi_input_id  linux_input_id,
											uint8_t *tds_input_id)
{
	alisl_retcode ret;

	ret = 0;
	switch (linux_input_id)
	{
		case ALI_TSI_INPUT_SPI_0:
		{
			*tds_input_id = ALISL_TSI_SPI_0;
		}
		break;

		case ALI_TSI_INPUT_SPI_1:
		{
			*tds_input_id = ALISL_TSI_SPI_1;
		}
		break;

		case ALI_TSI_INPUT_TSG:
		{
			*tds_input_id = ALISL_TSI_SPI_TSG;
		}
		break;

		case ALI_TSI_INPUT_SSI_0:
		{
			*tds_input_id = ALISL_TSI_SSI_0;
		}
		break;

		case ALI_TSI_INPUT_SSI_1:
		{
			*tds_input_id = ALISL_TSI_SSI_1;
		}
		break;

		case ALI_TSI_INPUT_SSI_2:
		{
			*tds_input_id = ALISL_TSI_SSI_2;
		}
		break;

		case ALI_TSI_INPUT_SSI_3:
		{
			*tds_input_id = ALISL_TSI_SSI_3;
		}
		break;

		case ALI_TSI_INPUT_SPI2B_0:
		{
			*tds_input_id = ALISL_TSI_SSI2B_0;
		}
		break;

		case ALI_TSI_INPUT_SPI2B_1:
		{
			*tds_input_id = ALISL_TSI_SSI2B_1;
		}
		break;

		case ALI_TSI_INPUT_SPI4B_0:
		{
			*tds_input_id = ALISL_TSI_SSI4B_0;
		}
		break;

		case ALI_TSI_INPUT_SPI4B_1:
		{
			*tds_input_id = ALISL_TSI_SSI4B_1;
		}
		break;

		case ALI_TSI_INPUT_PARA:
		{
			*tds_input_id = ALISL_PARA_MODE_SRC;
		}
		break;

		default:
		{
			SL_DBG("%s, %d\n", __FUNCTION__, __LINE__);
			ret = ERROR_INVAL;
		}
		break;
	}

	return(ret);
}


/**
 *  Function Name:     alisltsi_check_tsi_info
 *  @brief
 *
 *  @param handle      point to struct tsi_device
 *  @param dmx_id      dmx_id
 *  @param tsi_info    point to struct alisl_tsiroute_info_t
 *  @return            alisl_retcode
 *
 *  @author            Franky.Liang  <franky.liang@alitech.com>
 *  @date              7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_check_tsi_info(alisl_handle handle, int32_t dmx_id,alisl_tsiroute_info_t* tsi_info)
{

	struct ali_tsi_input_set_param   input;
	struct ali_tsi_channel_set_param channel;
	struct ali_tsi_output_set_param  output;
	enum ali_tsi_ci_link_mode        ci_link_mode;

	struct tsi_device *dev = (struct tsi_device *)handle;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, TSI_STATUS_START)) {
		SL_DBG("Tsi have not start\n");
		return ERROR_STATUS;
	}

	if (ALISL_TSI_DMX_0 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_0;
	else if (ALISL_TSI_DMX_1 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_1;
	else if (ALISL_TSI_DMX_2 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_2;
	else if (ALISL_TSI_DMX_3 == dmx_id)
		output.output_id = ALI_TSI_OUTPUT_DMX_3;
	else {
		SL_DBG("dmx_id  error\n");
		return ERROR_INVAL;
	}

	ioctl(dev->fd, ALI_TSI_OUTPUT_GET, &output);

	channel.channel_id = output.channel_id;

	ioctl(dev->fd, ALI_TSI_CHANNEL_GET, &channel);

	input.id = channel.input_id;

	ioctl(dev->fd, ALI_TSI_INPUT_GET, &input);

	ioctl(dev->fd, ALI_TSI_CI_LINK_MODE_GET, &ci_link_mode);

	if (ALI_TSI_CHANNEL_0 == output.channel_id)
		tsi_info->channel_id = ALISL_TSI_TS_A;
	else if (ALI_TSI_CHANNEL_1 == output.channel_id)
		tsi_info->channel_id = ALISL_TSI_TS_B;
	else if (ALI_TSI_CHANNEL_2 == output.channel_id)
		tsi_info->channel_id = ALISL_TSI_TS_C;
	else
		tsi_info->channel_id = ALISL_TSI_TS_D;

	if (ALI_TSI_CI_LINK_CHAIN == ci_link_mode)
	{
		tsi_info->ci_mode = ALISL_MODE_CHAIN;
	}
	else
	{
		tsi_info->ci_mode = ALISL_MODE_PARALLEL;
	}

	tsi_inputid_linux_to_tds(channel.input_id, &(tsi_info->input_id));

	return 0;

}


/**
 *  Function Name:      alisltsi_close
 *  @brief
 *
 *  @param handle       point to struct tsi_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alisltsi_close(alisl_handle handle)
{
	struct tsi_device *dev = (struct tsi_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	pthread_mutex_lock(&m_mutex);

	if (--dev->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}
	
	if (!flag_bit_test_any(&dev->status, TSI_STATUS_OPEN)) {
		SL_DBG("Tsi device not opened!\n");
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	if (flag_bit_test_any(&dev->status, TSI_STATUS_CLOSE)) {
		SL_DBG("TSI device already closed!\n");
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}
	
	if (dev->feedfd > 0) {
		close(dev->feedfd);
		dev->feedfd = -1;
	}

	close(dev->fd);
	dev->fd = -1;

	flag_bit_clear(&dev->status, TSI_STATUS_OPEN | TSI_STATUS_START);
	flag_bit_set(&dev->status, TSI_STATUS_CLOSE);
	m_dev[dev->id] = NULL;
	alisltsi_destruct((alisl_handle *)&dev);
	pthread_mutex_unlock(&m_mutex);
	return 0;
}





