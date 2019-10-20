/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsmc.c
 *  @brief              ALi smart card interfaces. All applications and other
 *                      function should only access hardware or driver by
 *                      this function interface.
 *
 *  @version            1.0
 *  @date               06/04/2013 02:24:25 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

/* system headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>

/* kernel headers */
#include <ali_smc_common.h>
#include <sys/ioctl.h>

/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislsmc.h>
#include <bits_op.h>

/* local headers */
#include "internal.h"

static struct smc_device *m_dev[MAX_SMC_SLOT] = {0};
static char msg_buffer_array[MAX_SMC_SLOT][MAX_KUMSG_SIZE];
static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;

// Invoke plug event callback at active(start)
#define BEGIN_CALLBACK

static alisl_retcode alislsmc_construct(alisl_handle *handle)
{
	struct smc_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		SL_DBG("malloc memory failed!\n");
		return ERROR_NOMEM;
	}

	memset(dev, 0, sizeof(*dev));

	dev->fd = -1;
	dev->id = -1;
	dev->portid = -1;
	dev->kumsgfd = -1;
	dev->ev_handle = NULL;
	dev->status = SMC_STATUS_CONSTRUCT;

	*handle = dev;

	return 0;
}

static alisl_retcode alislsmc_destruct(alisl_handle *handle)
{
	struct smc_device *dev = (struct smc_device *)(*handle);

	free(dev);
	*handle = NULL;

	return 0;
}

/**
 *  Function Name:      alislsmc_open
 *  @brief              open actual hardware device
 *
 *  @param handle       pointer to module handle
 *  @param id           actual smart card id number that would open.
 *                      usually this id is 0, but if hardware have several
 *                      cards and driver supports different id, then this id
 *                      could be 0 or 1 or 2 or others.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_open(alisl_handle *handle, int id)
{
	struct smc_device *dev;
	char              devpath[32];
	alisl_retcode retcode;
	long flags = O_CLOEXEC;

	if (NULL == handle || id < 0 || id >= MAX_SMC_SLOT) {
		SL_DBG("Invalidate card id!\n");
		return ERROR_INVAL;
	}

	pthread_mutex_lock(&m_mutex);

	if (m_dev[id] == NULL) {
		if (alislsmc_construct((alisl_handle *)&dev)) {
			retcode = ERROR_NOMEM;
			goto exit_fail;
		}
	} else {
		dev = m_dev[id];
	}

	if (bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Already opened!\n");
		retcode = 0;
		goto exit_success;
	}

	dev->id = id;

	memset(devpath, 0, sizeof(devpath));
	snprintf(devpath, sizeof(devpath), "/dev/ali_smc_%d", id);
	SL_DBG("devpath : %s\n", devpath);
	if ((dev->fd = open(devpath, O_RDWR)) < 0) {
		SL_DBG("device %s open failed!\n", devpath);
		retcode = ERROR_OPEN;
		goto fail0;
	}

	dev->ev_handle = NULL;
	alislevent_open(&dev->ev_handle);
	if (dev->ev_handle == NULL) {
		SL_DBG("alislevent_open FAIL \n");
	}

	SL_DBG("call SMC_CMD_GET_KUMSGQ dev->fd: %d, flags: 0x%x\n",dev->fd, flags);
	if ((dev->kumsgfd = ioctl(dev->fd, SMC_CMD_GET_KUMSGQ, &flags)) < 0) {
		SL_DBG("SMC_CMD_GET_KUMSGQ FAIL \n");
	}

	bit_clear(dev->status, SMC_STATUS_START);
	bit_set(dev->status, SMC_STATUS_STOP);

	bit_clear(dev->status, SMC_STATUS_CLOSE);
	bit_set(dev->status, SMC_STATUS_OPEN);

exit_success:
	dev->open_cnt ++;
	m_dev[id] = dev;
	*handle = dev;
	pthread_mutex_unlock(&m_mutex);
	return 0;

fail0:
	free(dev);
exit_fail:
	*handle = NULL;
	pthread_mutex_unlock(&m_mutex);
	return retcode;
}

/**
 *  Function Name:      alislsmc_ioctl
 *  @brief              provide misc function method
 *
 *  @param dev          pointer to smc_device
 *  @param cmd          command, refer to enum smc_io_command
 *  @param param        parameter for related command
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_ioctl(alisl_handle handle,
							 unsigned int cmd, unsigned long param)
{
	struct smc_device *dev = (struct smc_device *)handle;
	unsigned int c;
	unsigned long p;
	unsigned char ch;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	c = cmd;
	p = param;
	switch (cmd) {
		case SMC_IOCMD_SET_IO_ONOFF:   {
			c = SMC_CMD_SET_IO_ONOFF;
			break;
		}
		case SMC_IOCMD_SET_ETU:        {
			c = SMC_CMD_SET_ETU;
			p = (unsigned long)&param;
			break;
		}
		case SMC_IOCMD_SET_WWT:        {
			c = SMC_CMD_SET_WWT;
			p = (unsigned long)&param;
			break;
		}
		case SMC_IOCMD_SET_GUARDTIME:  {
			break;
		}
		case SMC_IOCMD_SET_BAUDRATE:   {
			break;
		}
		case SMC_IOCMD_CHECK_STATUS:   {
			c = SMC_CMD_CHECK_STATUS;
			break;
		}
		case SMC_IOCMD_CLKCHG_SPECIAL: {
			break;
		}
		case SMC_IOCMD_FORCE_SETTING:  {
			break;
		}
		case SMC_IOCMD_SET_CWT:        {
			c = SMC_CMD_SET_CWT;
			p = (unsigned long)&param;
			break;
		}
		case SMC_IOCMD_GET_F:          {
			c = SMC_CMD_GET_F;
			break;
		}
		case SMC_IOCMD_GET_D:          {
			c = SMC_CMD_GET_D;
			break;
		}
		case SMC_IOCMD_GET_ATR_RESULT: {
			c = SMC_CMD_GET_ATR_RESULT;
			break;
		}
		case SMC_IOCMD_GET_HB:         {
			c = SMC_CMD_GET_HB;
			break;
		}
		case SMC_IOCMD_GET_PROTOCOL:   {
			c = SMC_CMD_GET_PROTOCOL;
			break;
		}
		case SMC_IOCMD_SET_WCLK:       {
			c = SMC_CMD_SET_WCLK;
			p = (unsigned long)&param;
			break;
		}
		case SMC_IOCMD_GET_CLASS:      {
			c = SMC_CMD_GET_CLASS;
			break;
		}
		case SMC_IOCMD_SET_CLASS:      {
			c = SMC_CMD_SET_CLASS;
			p = (unsigned long)&param;
			break;
		}
		case SMC_IOCMD_SET_PROTOCOL:   {
			c = SMC_CMD_SET_PROTOCOL;
			ch = (unsigned char)param;
			p = (unsigned long)&ch;
			return ioctl(dev->fd, c, p);
			break;
		}
		case SMC_IOCMD_GET_WCLK: {
		    c = SMC_CMD_GET_WCLK;
		    p = param;
		    break;
		}
		case SMC_IOCMD_SET_RESET_MODE: {
		    c = SMC_CMD_SET_RESET_MODE;
		    p = param;
		    break;
		}
		case SMC_IOCMD_DISABLE_PPS_ON_RESET:    {
			c = SMC_CMD_DISABLE_PPS;
			p = param;
			break;
		}
		default:
			SL_DBG("Warning!!! un-recognised IO command\n");
			SL_DBG("Maybe want to use driver layer command directly!\n");
			break;
	}
	SL_DBG("dev->fd: %x, c:%u, p:%ld", dev->fd, c, *((unsigned long *)p));
	return ioctl(dev->fd, c, p);
}

static alisl_retcode cfg_copy(struct smc_dev_cfg *drv_cfg,
							  struct smc_device_cfg *cfg)
{
	drv_cfg->init_clk_trigger          = cfg->init_clk_trigger;
	drv_cfg->def_etu_trigger           = cfg->def_etu_trigger;
	drv_cfg->sys_clk_trigger           = cfg->sys_clk_trigger;
	drv_cfg->gpio_cd_trigger           = cfg->gpio_cd_trigger;
	drv_cfg->gpio_cs_trigger           = cfg->gpio_cs_trigger;
	drv_cfg->force_tx_rx_trigger       = cfg->force_tx_rx_trigger;
	drv_cfg->parity_disable_trigger    = cfg->parity_disable_trigger;
	drv_cfg->parity_odd_trigger        = cfg->parity_odd_trigger;
	drv_cfg->apd_disable_trigger       = cfg->apd_disable_trigger;
	drv_cfg->type_chk_trigger          = cfg->type_chk_trigger;
	drv_cfg->warm_reset_trigger        = cfg->warm_reset_trigger;
	drv_cfg->gpio_vpp_trigger          = cfg->gpio_vpp_trigger;
	drv_cfg->disable_pps               = cfg->disable_pps;
	drv_cfg->invert_power              = cfg->invert_power;
	drv_cfg->invert_detect             = cfg->invert_detect;
	drv_cfg->class_selection_supported = cfg->class_selection_supported;
	drv_cfg->board_supported_class     = cfg->board_supported_class;
	drv_cfg->reserved                  = cfg->reserved;
	drv_cfg->init_clk_number           = cfg->init_clk_number;
	drv_cfg->init_clk_array            = (unsigned long *)cfg->init_clk_array;
	drv_cfg->default_etu               = cfg->default_etu;
	drv_cfg->smc_sys_clk               = cfg->smc_sys_clk;
	drv_cfg->gpio_cd_pol               = cfg->gpio_cd_pol;
	drv_cfg->gpio_cd_io                = cfg->gpio_cd_io;
	drv_cfg->gpio_cd_pos               = cfg->gpio_cd_pos;
	drv_cfg->gpio_cs_pol               = cfg->gpio_cs_pol;
	drv_cfg->gpio_cs_io                = cfg->gpio_cs_io;
	drv_cfg->gpio_cs_pos               = cfg->gpio_cs_pos;
	drv_cfg->force_tx_rx_cmd           = cfg->force_tx_rx_cmd;
	drv_cfg->force_tx_rx_cmd_len       = cfg->force_tx_rx_cmd_len;
	drv_cfg->intf_dev_type             = cfg->intf_dev_type;
	drv_cfg->reserved1                 = cfg->reserved1;
	drv_cfg->gpio_vpp_pol              = cfg->gpio_vpp_pol;
	drv_cfg->gpio_vpp_io               = cfg->gpio_vpp_io;
	drv_cfg->gpio_vpp_pos              = cfg->gpio_vpp_pos;
	drv_cfg->ext_cfg_tag               = cfg->ext_cfg_tag;
	drv_cfg->ext_cfg_pointer           = cfg->ext_cfg_pointer;
	drv_cfg->class_select              =
		(void (*)(enum class_selection))cfg->class_select;
	drv_cfg->use_default_cfg           = cfg->use_default_cfg;

	return 0;
}

/**
 *  Function Name:      alislsmc_set_cfg
 *  @brief              Set the card parameter
 *
 *  @param handle       pointer to module handle
 *  @param cfg          pointer to struct smc_device_cfg
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/05/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_set_cfg(alisl_handle handle,
							   struct smc_device_cfg *cfg)
{
	struct smc_device *dev = (struct smc_device *)handle;
	struct smc_dev_cfg drv_cfg;
	int ret;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	memset(&drv_cfg, 0, sizeof(drv_cfg));
	cfg_copy(&drv_cfg, cfg);
	SL_DBG("call SMC_CMD_CONFIG dev->fd: %d, drv_cfg.init_clk_trigger: %d, drv_cfg.def_etu_trigger: %d\n, "\
		 									"drv_cfg.force_tx_rx_trigger: %d, drv_cfg.parity_disable_trigger: %d\n, "\
											" drv_cfg.parity_odd_trigger: %d, drv_cfg.apd_disable_trigger: %d\n, "\
											"drv_cfg.warm_reset_trigger: %d, drv_cfg.disable_pps: %d\n, "\
											"drv_cfg.class_selection_supported: %d, drv_cfg.board_supported_class: %d\n, "\
											"drv_cfg.board_class: %d, drv_cfg.default_etu: %d\n, "\
											"drv_cfg.smc_sys_clk: %d, drv_cfg.gpio_cd_io: %d\n, "\
											"drv_cfg.force_tx_rx_cmd: %d, drv_cfg.force_tx_rx_cmd_len: %d\n", 
		 									dev->fd, drv_cfg.init_clk_trigger, drv_cfg.def_etu_trigger,
											drv_cfg.force_tx_rx_trigger, drv_cfg.parity_disable_trigger,
											drv_cfg.parity_odd_trigger, drv_cfg.apd_disable_trigger,
											drv_cfg.warm_reset_trigger, drv_cfg.disable_pps,
											drv_cfg.class_selection_supported, drv_cfg.board_supported_class,
											drv_cfg.board_class, drv_cfg.default_etu,
											drv_cfg.smc_sys_clk, drv_cfg.gpio_cd_io,
											drv_cfg.force_tx_rx_cmd, drv_cfg.force_tx_rx_cmd_len);
	ret = ioctl(dev->fd, SMC_CMD_CONFIG, &drv_cfg);
	if (ret) {
		SL_DBG("Set card cfg parameter error!\n");
		return  ERROR_CFG;
	}

	bit_set(dev->status, SMC_STATUS_CONFIGURE);

	return 0;
}

/**
 *  Function Name:      alislsmc_set_defcfg
 *  @brief              Set default card parameter
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/05/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_set_defcfg(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int ret;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	dev->cfg.use_default_cfg = 1;
	ret = ioctl(dev->fd, SMC_CMD_CONFIG, &dev->cfg);
	if (ret != 0) {
		SL_DBG("Set card cfg parameter error!\n");
		return  ERROR_CFG;
	}

	bit_set(dev->status, SMC_STATUS_CONFIGURE);

	return 0;
}

/**
 *  Function Name:      alislsmc_register_callback
 *  @brief              register a callback function which will be called when
 *                      monitor receive messages
 *
 *  @param handle       pointer to module handle
 *  @param p_user_data	user data from upper layer which will be transfered to upper layer by the callback.
 *  @param callback     function pointer
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/06/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_register_callback(alisl_handle handle,
										 void *p_user_data,
										 void (*callback)(void *user_data, uint32_t param))
{
	struct smc_device *dev = (struct smc_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	dev->callback = callback;
	dev->callback_user_data = p_user_data;

	return 0;
}

static void * alislsmc_monitor_pthread(void *arg)
{
	alisl_handle handle = (struct smc_device *)arg;
	struct smc_device *dev = (struct smc_device *)handle;
	smc_notification_param_t *nt = NULL;
	ssize_t size = 0;

	for (;;) {
		nt = (smc_notification_param_t *)msg_buffer_array[dev->id];
		//memset(nt, 0, sizeof(smc_notification_param_t));
		if ((size = read(dev->kumsgfd, nt, MAX_KUMSG_SIZE)) < 0) {
//			SL_DBG("ERROR reading kumsgfd SMC %ld: %s.\n", errno, strerror(errno));
//			usleep(10 * 1000);
//			continue;
			break;
		}
		SL_DBG("SMC: Notification from kernel T %d L %d V %d\n",
				 nt->smc_msg_tag,
				 nt->smc_msg_len,
				 nt->smc_msg_buf[0]);

		switch (nt->smc_msg_tag) {
			case SMC_MSG_CARD_STATUS:
				if (dev->callback) {
					dev->callback(dev->callback_user_data, nt->smc_msg_buf[0]);
				}
				break;
			default:
				break;
		}
	}
		return 0;
}

static alisl_retcode alislsmc_register_monitor(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;
	//pthread_attr_t attr;
	//int ret;

	if (dev == NULL || dev->ev_handle == NULL || dev->kumsgfd < 0) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
#if 0
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, STACKSIZE);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&dev->tid, &attr,
						 alislsmc_monitor_pthread, (void *)dev);
	if (ret != 0) {
		SL_DBG("Create monitor pthread failed: %s\n", strerror(errno));
		pthread_attr_destroy(&attr);
		return ERROR_PTCREATE;
	}

	pthread_attr_destroy(&attr);
#endif

#ifdef BEGIN_CALLBACK
// Not to Empty KUMSG, Use another method,
// invoke callback only when no card inserted.
// See smc_callback_at_active

//	// Wait for the KUMSG at least 10ms,
//	// or else 1 unexpected plug event may exist,
//	// then empty all the KUMSG
//	int empty_flag = 0;
//	int i = 0;
//	int sleep_cnt = 3; // At lease 10ms, wait 30ms now
//	int sleep_interval = 10; // MS
//	char buf[MAX_KUMSG_SIZE] = {0};
//	for (i = 1; i <= sleep_cnt; i++) {
//		SL_DBG("Wait SMC KUMSG %d / %d ms\n",
//				i * sleep_interval, sleep_cnt * sleep_interval);
//		usleep(sleep_interval * 1000);
//		// Empty message queue;
//		while (read(dev->kumsgfd, buf, MAX_KUMSG_SIZE) > 0) {
//			empty_flag++;
//		}
//		if (empty_flag) {
//			break;
//		}
//	}
//	SL_DBG("Ignore %d SMC KUMSG!!!\n", empty_flag);
#endif
	dev->slev.fd = dev->kumsgfd;
	dev->slev.events = EPOLLIN;
	dev->slev.cb = alislsmc_monitor_pthread;
	dev->slev.data = dev;
	if (alislevent_add(dev->ev_handle, &dev->slev) != 0) {
		return ERROR_NULLDEV;
	}

	bit_set(dev->status, SMC_STATUS_MONITOR);

	return 0;
}

static alisl_retcode alislsmc_unregister_monitor(alisl_handle handle)
{
	struct smc_device *dev = NULL;
//	int ret;

	dev = (struct smc_device *)handle;
#if 0
	ret = pthread_cancel(dev->tid);
	if (ret) {
		SL_DBG("Cancel monitor pthread failed: %s\n", strerror(errno));
		return ERROR_PTCANCEL;
	}
	pthread_join(dev->tid, NULL);
#endif
	if (dev->ev_handle != NULL) {
		alislevent_del(dev->ev_handle, &dev->slev);
	}
	bit_clear(dev->status, SMC_STATUS_MONITOR);

	return 0;
}

void *smc_callback_at_active(void *handle)
{
	// Do not invoke in another thread.
	//pthread_detach(pthread_self());
	struct smc_device *dev = (struct smc_device *)handle;
	int status = 0;
	if (ioctl(dev->fd, SMC_CMD_GET_CARD_STATUS, &status)) {
		SL_DBG("SMC_CMD_GET_CARD_STATUS ERROR!\n");
		return (void *)1;
	}
	/*
	 * 0 : no card
	 * 1 : card hw OK
	 * 3 : card sw OK
	 * For compatibity of the upper invocation, we do the transferation
	 */
	status = !!status;
	// In Linux, driver will emit a plug in event if card was inserted,
	// while do nothing if no card was inserted.
	// SMC homologation test require plug event no matter
	// whether the card was inserted or not.
	// AUI will invoke a plug event callback when no card was inserted.
	if (!status) {
		if (dev->callback) {
			dev->callback(dev->callback_user_data, status);
		}
	}
	return (void *)0;
}

int smc_active_misc(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;
//	pthread_t tid = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
#if 0
	// Nagra AS test performed by Tom Xie
	// AS test need a plug event in at active.
	// The test will reset the card in the plug event call back function.
	// And the callback function must be invoked at another thread.(Wrong)
	if (pthread_create(&tid, NULL, smc_callback_at_active, (void *)handle)) {
		SL_DBG("pthread_create fail! smc callback fail at active!\n");
		return -1;
	}
	return 0;
#else
	return (int)smc_callback_at_active((void *)handle);
#endif
}

/**
 *  Function Name:      alislsmc_start
 *  @brief              make hardware or driver start to work
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_start(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;
	//int status = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	if (bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Already started!\n");
		return 0;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_CONFIGURE)) {
		SL_DBG("Card not configured, use default configure!\n");
		alislsmc_set_defcfg(dev);
	}

	if (dev->callback == NULL) {
		SL_DBG("Warning!!! No callback function registered!\n");
	}

	if (alislsmc_register_monitor(dev))
		SL_DBG("Warning!!! register monitor failed!\n");

	bit_clear(dev->status, SMC_STATUS_STOP);
	bit_set(dev->status, SMC_STATUS_START);

#ifdef BEGIN_CALLBACK
	// Do some misc work when active
	// The callback will be invoke.
	// The invoking should behind the status setting.
	// Because the APP will do something in the callback,
	// and the APP need the correct status
    smc_active_misc(handle);
#endif

	return 0;
}

/**
 *  Function Name:      alislsmc_card_exist
 *  @brief              check if there is card exist
 *
 *  @param handle       pointer to module handle
 *
 *  @retval false       no card
 *  @retval true        card exist
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
bool alislsmc_card_exist(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int status = false;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	
	ioctl(dev->fd, SMC_CMD_GET_CARD_STATUS, &status);

	return !!status;
}

/**
 *  Function Name:      alislsmc_reset
 *  @brief              reset smart card
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_reset(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int ret;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	
	ret = ioctl(dev->fd, SMC_CMD_RESET, &dev->atr);
	if (ret < 0) {
		SL_DBG("Reset smart card failed!\n");
		dev->atr.atr_size = 0;
		return ERROR_RESET;
	}

	return 0;
}

/**
 *  Function Name:      alislsmc_get_atr
 *  @brief              get answer-to-reset info
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer to store answer-to-reset information
 *  @param size         buffer size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_get_atr(alisl_handle handle,
							   void *buf, unsigned short *size)
{
	struct smc_device *dev = (struct smc_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (NULL == buf || NULL == size) {
		SL_DBG("Invalid parameter!\n");
		return ERROR_INVAL;
	}

	*size = dev->atr.atr_size;
	if (0 != dev->atr.atr_size) {
		memcpy(buf, dev->atr.atr_buf, dev->atr.atr_size);
	}

	return 0;
}

/**
 *  Function Name:      alislsmc_deactive
 *  @brief              deactive smart card
 *
 *  @param handle       pointer to module handle
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_deactive(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;

	int ret = 0;
	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	
	SL_DBG("call SMC_CMD_DEACTIVE dev->fd: %d, \n", dev->fd);
	ret = ioctl(dev->fd, SMC_CMD_DEACTIVE, NULL); /* usually this will success */

	return ret;
}

/**
 *  Function Name:      alislsmc_raw_read
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer to store raw data
 *  @param size         size to read
 *  @param actlen       actual size that have really read
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_raw_read(alisl_handle handle,
								void *buf, size_t size, size_t *actlen)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int ret = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (NULL == buf || 0 == size || NULL == actlen) {
		SL_DBG("Invalid parameter!\n");
		return ERROR_INVAL;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	*actlen = 0;
	
	ret = read(dev->fd, buf, size);
	SL_DUMP("buf: ", buf, size);
	if (-1 == ret && (EINTR == errno || EAGAIN == errno)) {
		/* Failure, maybe need to try one more time */
		ret = read(dev->fd, buf, size);
		SL_DUMP("buf: ", buf, size);
	}

	if (-1 == ret || 0 == ret) {
		return ERROR_READ;
	}

	*actlen += ret;

	// Conax test issue
	// Should not do check the response, this is APP's job
//	if (ret < size) {
//		size -= ret;
//		ret = read(dev->fd, ((char *)buf) + ret, size);
//		if (-1 == ret) {
//			ret = 0;
//		}
//
//		*actlen += ret;
//	}

	return 0;
}

/**
 *  Function Name:      alislsmc_raw_write
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer stored raw data to be written
 *  @param size         size of buffer
 *  @param actlen       actual size that have written
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_raw_write(alisl_handle handle,
								 void *buf, size_t size, size_t *actlen)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int ret = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (NULL == buf || NULL == actlen || 0 == size) {
		SL_DBG("Invalid parameter!\n");
		return ERROR_INVAL;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	*actlen = 0;
	SL_DUMP("buf: ", buf, size);
	ret = write(dev->fd, buf, size);
	if (-1 == ret && EINTR == errno) {
		/* Failure, maybe need to try one more time */
		ret = write(dev->fd, buf, size);
	}

	if (-1 == ret) {
		return ERROR_WRITE;
	}

	*actlen += ret;

	// Conax test issue
	// Should not do check the response, this is APP's job
//	if (ret < size) {
//		size -= ret;
//		ret = write(dev->fd, buf + ret, size);
//		if (-1 == ret) {
//			ret = 0;
//		}
//
//		*actlen += ret;
//	}

	return 0;
}

/**
 *  Function Name:      alislsmc_raw_fifo_write
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param buf          buffer stored raw data to be written
 *  @param size         size of buffer
 *  @param actlen       actual size that have written
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_raw_fifo_write(alisl_handle handle,
									  void *buf, size_t size, size_t *actlen)
{
	return alislsmc_raw_write(handle, buf, size, actlen);
}

static alisl_retcode iso_transfer(int fd, uint32_t iocmd,
								  void *command, size_t nwrite,
								  void *response, size_t nread,
								  size_t *actlen)
{
	smc_iso_transfer_t iso_transfer;
	int ret = 0;

	*actlen = 0;

	iso_transfer.actual_size = 0;
	iso_transfer.command = command;
	iso_transfer.num_to_read = nread;
	iso_transfer.num_to_write = nwrite;
	iso_transfer.response = response;
	iso_transfer.transfer_err = SMART_NO_ERROR;
	
	SL_DBG("fd: %d, iocmd: %d, command: %p, num_to_write: %d,"\
							  "response: %p, num_to_read: %d,"\
							  "actual_size: %d, transfer_err: %d\n", 
                               fd, iocmd, iso_transfer.command, iso_transfer.num_to_write,
							   iso_transfer.response, iso_transfer.num_to_read,
							   iso_transfer.actual_size, iso_transfer.transfer_err);
	ret = ioctl(fd, iocmd, &iso_transfer);
	if (ret < 0) {
		SL_DBG("Warning!!! iso transfer error\n");
	}

	*actlen = iso_transfer.actual_size;

	return ret;
}

/**
 *  Function Name:      alislsmc_iso_transfer
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param command      iso transfer command
 *  @param nwrite       iso transfer command size
 *  @param response     iso transfer response
 *  @param nread        iso transfer response size
 *  @param actlen       actual response size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_iso_transfer(alisl_handle handle,
									void *command, size_t nwrite,
									void *response, size_t nread,
									size_t *actlen)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int ret = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	ret = iso_transfer(dev->fd, SMC_CMD_ISO_TRANS,
				 command, nwrite,
				 response, nread,
				 actlen);

	return ret;
}

/**
 *  Function Name:      alislsmc_iso_transfer_t1
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param command      iso transfer t1 command
 *  @param nwrite       iso transfer t1 command size
 *  @param response     iso transfer t1 response
 *  @param nread        iso transfer t1 response size
 *  @param actlen       actual response size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_iso_transfer_t1(alisl_handle handle,
									   void *command, size_t nwrite,
									   void *response, size_t nread,
									   size_t *actlen)
{
	struct smc_device *dev = (struct smc_device *)handle;
	int ret = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	ret = iso_transfer(dev->fd, SMC_CMD_ISO_TRANS_T1,
				 command, nwrite,
				 response, nread,
				 actlen);

	return ret;
}

/**
 *  Function Name:      alislsmc_iso_transfer_t14
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param command      iso transfer t14 command
 *  @param nwrite       iso transfer t14 command size
 *  @param response     iso transfer t14 response
 *  @param nread        iso transfer t14 response size
 *  @param actlen       actual response size
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_iso_transfer_t14(alisl_handle handle,
										void *command, size_t nwrite,
										void *response, size_t nread,
										size_t *actlen)
{
	return alislsmc_iso_transfer_t1(handle,
									command, nwrite,
									response, nread,
									actlen);
}

/**
 *  Function Name:      alislsmc_t1_transfer
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param dad
 *  @param sendbuf
 *  @param sendlen
 *  @param recvbuf
 *  @param recvlen
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_t1_transfer(alisl_handle handle,
								   uint8_t dad,
								   const void *sendbuf, size_t sendlen,
								   void *recvbuf, size_t recvlen)
{
	struct smc_device *dev = (struct smc_device *)handle;
	smc_t1_trans_t t1_trans;
	size_t actlen = 0;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	t1_trans.dad = dad;
	t1_trans.rcv_buf = recvbuf;
	t1_trans.rcv_len = recvlen;
	t1_trans.send_buf = (void *)sendbuf;
	t1_trans.send_len = sendlen;
	
	SL_DUMP("call SMC_CMD_T1_TRANS: ", t1_trans.send_buf, t1_trans.send_len);
	actlen = ioctl(dev->fd, SMC_CMD_T1_TRANS, &t1_trans);
	SL_DUMP("call SMC_CMD_T1_TRANS: ", t1_trans.rcv_buf, t1_trans.rcv_len);

	return actlen;
}

/**
 *  Function Name:      alislsmc_t1_xcv
 *  @brief

 *  @param handle       pointer to module handle
 *  @param sblock
 *  @param slen
 *  @param rblock
 *  @param rmax
 *  @param actlen
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_t1_xcv(alisl_handle handle,
							  void *sblock, size_t slen,
							  void *rblock, size_t rmax,
							  size_t *actlen)
{
	struct smc_device *dev = (struct smc_device *)handle;
	smc_t1_xcv_t t1_xcv;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	t1_xcv.actual_size = 0;
	t1_xcv.rblock = rblock;
	t1_xcv.rmax = rmax;
	t1_xcv.sblock = sblock;
	t1_xcv.slen = slen;

	ioctl(dev->fd, SMC_CMD_T1_XCV, &t1_xcv);

	*actlen = t1_xcv.actual_size;

	return 0;
}

/**
 *  Function Name:      alislsmc_t1_negociate_ifsd
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param dad
 *  @param ifsd
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/07/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_t1_negociate_ifsd(alisl_handle handle,
										 uint32_t dad, uint32_t ifsd)
{
	struct smc_device *dev = (struct smc_device *)handle;
	smc_t1_nego_ifsd_t t1_nego_ifsd;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("Not started!\n");
		return ERROR_NOSTART;
	}

	t1_nego_ifsd.dad = dad;
	t1_nego_ifsd.ifsd = ifsd;
	
	SL_DBG("call SMC_CMD_T1_NEGO_IFSD dev->fd: %d, iocmd: %d, dad: %d, ifsd: %d\n", dev->fd, t1_nego_ifsd.dad, t1_nego_ifsd.ifsd);
	ioctl(dev->fd, SMC_CMD_T1_NEGO_IFSD, &t1_nego_ifsd);

	return 0;
}

/**
 *  Function Name:      alislsmc_abort
 *  @brief              make hardware or driver exit work mode
 *                      by unblock way
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_abort(alisl_handle handle)
{
	/* TODO */
	return 0;
}

/**
 *  Function Name:      alislsmc_stop
 *  @brief              make hardware or driver exit work mode
 *                      by block way
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_stop(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_START)) {
		SL_DBG("SMC not started!\n");
		return 0;
	}

	if (bit_test_any(dev->status, SMC_STATUS_MONITOR)) {
		SL_DBG("un-register monitor pthread!\n");
		alislsmc_unregister_monitor(dev);
	}
	
	SL_DBG("call SMC_CMD_DECONFIG dev->fd: %d", dev->fd);
	ioctl(dev->fd, SMC_CMD_DECONFIG, NULL);

	bit_clear(dev->status, SMC_STATUS_START | SMC_STATUS_CONFIGURE);
	bit_set(dev->status, SMC_STATUS_STOP);

	return 0;
}

/**
 *  Function Name:      alislsmc_close
 *  @brief              close hardware device
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alislsmc_close(alisl_handle handle)
{
	struct smc_device *dev = (struct smc_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	pthread_mutex_lock(&m_mutex);

	if (--dev->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	if (!bit_test_any(dev->status, SMC_STATUS_STOP)) {
		SL_DBG("SMC not stoped yet, now stop it!\n");
		alislsmc_stop(dev);
	}

	SL_DBG("dev->fd: %d", dev->fd);
	close(dev->fd);
	dev->fd = -1;

	if (dev->ev_handle != NULL) {
	    SL_DBG("alislevent_close handel: %x\n", dev->ev_handle);
		alislevent_close(dev->ev_handle);
		dev->ev_handle = NULL;
	}

	SL_DBG("dev->kumsgfd: %d", dev->kumsgfd);
	if (dev->kumsgfd > 0) {
		close(dev->kumsgfd);
		dev->kumsgfd = -1;
	}

	bit_clear(dev->status, SMC_STATUS_STOP | SMC_STATUS_OPEN);
	bit_set(dev->status, SMC_STATUS_CLOSE);

	m_dev[dev->id] = NULL;
	alislsmc_destruct((alisl_handle *)&dev);
	pthread_mutex_unlock(&m_mutex);

	return 0;
}

