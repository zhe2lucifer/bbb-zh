/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file			alislstandby.c
 *  @brief			Ali standby API
 *
 *  @version		1.0
 *  @date			06/19/2013 02:33:44 PM
 *  @revision		none
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include "internal.h"
#include "error.h"
#include <alisl_types.h>

static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
static int m_cnt = 0;   /* device opened count by other moudle */
static int m_pm_fd = 0;


static pthread_mutex_t m_pmu_mutex = PTHREAD_MUTEX_INITIALIZER;
static int m_pmu_cnt = 0;
static int m_pmu_fd = 0;

/**
 *  Function Name:	alislstandby_open_pm_device
 *  @brief		    get /dev/ali_pm handle
 *
 *  @param          NULL
 *
 *  @return         -1 --- error    >=0  --- success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
int alislstandby_open_pm_device()
{
	int fd = 0; /* device handle */

	pthread_mutex_lock(&m_mutex);
	if(m_cnt <= 0) {
		SL_DBG("pmu open,dev_name: %s, fd: %d\n","/dev/ali_pm",fd);
		fd = open("/dev/ali_pm", O_RDWR);

		if ( fd < 0 ) {
			SL_ERR("open ali_pm failed.\n");
			pthread_mutex_unlock(&m_mutex);
			return -1;
		}
		else {
			SL_DBG("open ali_pm success.\n");
			m_pm_fd = fd;
			m_cnt = 1;
			pthread_mutex_unlock(&m_mutex);
			return fd;
		}
	}

	m_cnt++;
	pthread_mutex_unlock(&m_mutex);
	return m_pm_fd;
}

/**
 *  Function Name:	alislstandby_close_pm_device
 *  @brief		    close pm device
 *
 *  @param          NULL
 *
 *  @return         -1 --- error    >=0  --- success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
int alislstandby_close_pm_device(int fd)
{
	int ret = -1;

	pthread_mutex_lock(&m_mutex);

	if (fd != m_pm_fd) {
		SL_ERR("Input unknown fd %d\n", fd);
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}

	if(--m_cnt) {
		goto quit;
	}

	SL_DBG("m_pm_fd: %d\n", m_pm_fd);
	ret = close(m_pm_fd);
	m_pm_fd = 0;

	if (ret < 0) {
		SL_ERR("close pm device failed.\n");
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}
	else {
		SL_DBG("close pm device success.\n");
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

quit:
	SL_DBG("device still be used.\n");
	pthread_mutex_unlock(&m_mutex);
	return 0;
}

/**
 *  Function Name:	alislstandby_pm_set_resumekey
 *  @brief			set pm standby resume key
 *
 *  @param          [in]pm_resumekey
 *
 *  @return         -1 for failed, 0 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pm_set_resumekey(struct pm_standby_key *pm_resumekey)
{
	int fd = 0;
	//pm_key_t *pkey = NULL;
	pm_key_t pm_key;


	if (NULL == pm_resumekey) {
		SL_ERR("input param pm_resumekey is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pm_device();
	if ( fd < 0 ) {
		SL_ERR("get pm handle failed.\n");
		return ERROR_NULLDEV;
	}

	//pkey = (pm_key_t*)pm_resumekey;
	pm_key.standby_key = pm_resumekey->standbykey;
	pm_key.ir_power[0] = pm_resumekey->powerkey;
	SL_DBG("call PM_CMD_SET_RESUME_KEY fd: %d, standby_key: %d, ir_power: %ld\n",
												fd, pm_key.standby_key, pm_key.ir_power[0]);
	if ( 0 == ioctl(fd, PM_CMD_SET_RESUME_KEY, &pm_key) ) {
		alislstandby_close_pm_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pm resume key failed.\n");
		alislstandby_close_pm_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pm_set_resumeparam
 *  @brief			set pm standby resume param
 *
 *  @param          pm_param
 *
 *  @return         -1, 1
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pm_set_resumeparam(struct pm_standby_param *pm_param)
{
	int fd = 0;
	pm_param_t *pparam = NULL;


	if (NULL == pm_param) {
		SL_ERR("input param pm_param is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pm_device();
	if ( fd < 0 ) {
		SL_ERR("get pm handle failed.\n");
		return ERROR_NULLDEV;
	}

	pparam = (pm_param_t*)pm_param;
	SL_DBG("call PM_CMD_SET_STANDBY_PARAM fd: %d, board_power_gpio: %d, timeout: %ld, reboot: %ld\n",
											fd, pparam->board_power_gpio, pparam->timeout, pparam->reboot);
	if ( 0 == ioctl(fd, PM_CMD_SET_STANDBY_PARAM, pparam) ) {
		alislstandby_close_pm_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pm resume time failed.\n");
		alislstandby_close_pm_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_open_pmu_device
 *  @brief		    get /dev/ali_pmu handle
 *
 *  @param          NULL
 *
 *  @return         -1 --- error    >=0  --- success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
int alislstandby_open_pmu_device()
{
    // Use reference count to avoid open/close too much
    int fd = 0; /* device handle */

    pthread_mutex_lock(&m_pmu_mutex);
    if (m_pmu_cnt <= 0) {
        SL_DBG("pmu open,dev_name: %s, fd: %d\n", "/dev/ali_pmu", fd);
        fd = open("/dev/ali_pmu", O_RDWR | O_NONBLOCK);

        if (fd < 0) {
            SL_ERR("open ali_pmu failed.\n");
            pthread_mutex_unlock(&m_pmu_mutex);
            return -1;
        } else {
            SL_DBG("open ali_pmu success.\n");
            m_pmu_fd = fd;
            m_pmu_cnt = 1;
            pthread_mutex_unlock(&m_pmu_mutex);
            return fd;
        }
    }

    m_pmu_cnt++;
    pthread_mutex_unlock(&m_pmu_mutex);
    return m_pmu_fd;
}

/**
 *  Function Name:	alislstandby_close_pmu_device
 *  @brief		    close pmu device
 *
 *  @param          NULL
 *
 *  @return         -1 --- error    >=0  --- success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
int alislstandby_close_pmu_device(int fd)
{
    int ret = -1;

    pthread_mutex_lock(&m_pmu_mutex);

    if (fd != m_pmu_fd) {
        SL_ERR("Input unknown fd %d\n", fd);
        pthread_mutex_unlock(&m_pmu_mutex);
        return -1;
    }

    if (--m_pmu_cnt) {
        goto quit;
    }

    SL_DBG("m_pmu_fd: %d\n", m_pmu_fd);
    ret = close(m_pmu_fd);
    m_pmu_fd = 0;

    if (ret < 0) {
        SL_ERR("close pmu device failed.\n");
        pthread_mutex_unlock(&m_pmu_mutex);
        return -1;
    } else {
        SL_DBG("close pmu device success.\n");
        pthread_mutex_unlock(&m_pmu_mutex);
        return 0;
    }

quit:
    pthread_mutex_unlock(&m_pmu_mutex);
    return 0;
}

/**
 *  Function Name:	alislstandby_pmu_set_resumekey
 *  @brief			set pmu resume key
 *
 *  @param          resumekey
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_resumekey(uint32_t resumekey)
{
	int fd = 0;

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	SL_DBG("call ALI_PMU_IR_PROTOL_NEC fd: %d, resumekey: %d\n", fd, resumekey);
	if ( 0 == ioctl(fd, ALI_PMU_IR_PROTOL_NEC, &resumekey) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pmu resume key failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_set_rtctime
 *  @brief			set current time to rtc
 *
 *  @param          current time
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_rtctime(struct mcu_rtc_time *curtime)
{
	int fd = 0;
	struct rtc_time_pmu rtc_time;

	if (!curtime) {
		SL_ERR("param curtime is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_OPEN;
	}

	rtc_time.year = curtime->year;
	rtc_time.month = (unsigned char)curtime->month;
	rtc_time.date = (unsigned char)curtime->date;
	rtc_time.day = (unsigned char)curtime->day;
	rtc_time.hour = (unsigned char)curtime->hour;
	rtc_time.min = (unsigned char)curtime->min;
	rtc_time.sec = (unsigned char)curtime->sec;

	SL_DBG("call ALI_PMU_RTC_SET_VAL [%04d-%02d-%02d %02d:%02d:%02d]\n",
			rtc_time.year, rtc_time.month, rtc_time.date,
			rtc_time.hour, rtc_time.min, rtc_time.sec);
	if ( 0 == ioctl(fd, ALI_PMU_RTC_SET_VAL, &rtc_time) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pmu rtc time failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_set_mcutime
 *  @brief			set current time to pmu mcu
 *
 *  @param          current time
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_mcutime(struct mcu_rtc_time *curtime)
{
	int fd = 0;
	struct rtc_time_pmu rtc_time;

	if (!curtime) {
		SL_ERR("param curtime is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	rtc_time.year = curtime->year;
	rtc_time.month = (unsigned char)curtime->month;
	rtc_time.date = (unsigned char)curtime->date;
	rtc_time.day = (unsigned char)curtime->day;
	rtc_time.hour = (unsigned char)curtime->hour;
	rtc_time.min = (unsigned char)curtime->min;
	rtc_time.sec = (unsigned char)curtime->sec;

	SL_DBG("call ALI_PMU_MCU_SET_TIME %d-%d-%d %d %d:%d:%d\n",
			rtc_time.year, rtc_time.month, rtc_time.date,
			rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);
	if ( 0 == ioctl(fd, ALI_PMU_MCU_SET_TIME, &rtc_time) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pmu mcu time failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_get_mcutime
 *  @brief			get current time from pmu mcu
 *
 *  @param          current time
 *
 *  @return         0 for success, other for failed,
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_get_mcutime(struct mcu_rtc_time *curtime)
{
	int fd = 0;
	struct rtc_time_pmu rtc_time;

	if (!curtime) {
		SL_ERR("param curtime is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	if ( 0 == ioctl(fd, ALI_PMU_MCU_READ_TIME, &rtc_time) ) {
		curtime->year = rtc_time.year;
		curtime->month = rtc_time.month;
		curtime->date = rtc_time.date;
		curtime->day = rtc_time.day;
		curtime->hour = rtc_time.hour;
		curtime->min = rtc_time.min;
		curtime->sec = rtc_time.sec;
		SL_DBG("call ALI_PMU_MCU_READ_TIME %d-%d-%d %d %d:%d:%d\n",
			rtc_time.year, rtc_time.month, rtc_time.date,
			rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);
//		SL_DBG("ALI_PMU_MCU_READ_TIME %d-%d-%d %d %d:%d:%d\n",
//				curtime->year, curtime->month, curtime->date,
//				curtime->day, curtime->hour, curtime->min, curtime->sec);
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("get pmu mcu time failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_enable_showtime
 *  @brief			enable show pmu time
 *
 *  @param          pannelflag
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_enable_showtime(enum MCU_SHOW_PANNEL *pannelflag)
{
	int fd = 0;

	if (!pannelflag) {
		SL_ERR("param pannelflag is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	SL_DBG("call ALI_PMU_SET_SHOW_PANEL_TYPE  fd: %d, pannelflag: %d\n", fd, pannelflag);
	if ( 0 == ioctl(fd, ALI_PMU_SET_SHOW_PANEL_TYPE, pannelflag) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pmu enable show time failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_set_minalarm
 *  @brief			set min alarm time
 *
 *  @param          alarmtime
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_minalarm(struct min_alarm_num* alarmtime)
{
	int fd = 0;
	struct min_alarm_num *pminalarmtime = NULL;

	if (!alarmtime) {
		SL_ERR("param alarmtime is NULL.\n");
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	pminalarmtime = (struct min_alarm_num *)alarmtime;
	SL_DBG("\nALI_PMU_MCU_WAKEUP_TIME num=%d %02d-%02d %02d:%02d:%02d\n",
			pminalarmtime->num, pminalarmtime->min_alm.month, pminalarmtime->min_alm.date,
			pminalarmtime->min_alm.hour, pminalarmtime->min_alm.min, pminalarmtime->min_alm.second);
	SL_DBG("Enable: %d-%d sun-sat:%d %d %d %d %d %d %d\n",
		pminalarmtime->min_alm.en_month, pminalarmtime->min_alm.en_date,
		pminalarmtime->min_alm.en_sun, pminalarmtime->min_alm.en_mon,
		pminalarmtime->min_alm.en_tue, pminalarmtime->min_alm.en_wed,
		pminalarmtime->min_alm.en_thr, pminalarmtime->min_alm.en_fri,
		pminalarmtime->min_alm.en_sat);
	if ( 0 == ioctl(fd, ALI_PMU_MCU_WAKEUP_TIME, pminalarmtime) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pmu min alarm failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_enable_alarm
 *  @brief		    enable pmu alarm
 *
 *  @param          void
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_enable_alarm()
{
	int fd = 0;
	uint8_t num[2] = {1, 0};  /* num[0]: 1--enable  0--disable  num[1]--GPIO addr */

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}
	SL_DBG("ALI_PMU_RTC_EN_ALARM enable: %d, GPIO addr: %d", num[0], num[1]);
	if ( 0 == ioctl(fd, ALI_PMU_RTC_EN_ALARM, num) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("pmu enable alarm failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_get_sys_info
 *  @brief			get system information
 *
 *  @param          cmd: command[in]   param: other param[in]
 *
 *  @return         -1:error   others:chip ID
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
int alislstandby_get_sys_info(unsigned int cmd, void *param)
{
	int fd = -1;
	int ret = -1;

	SL_DBG("soc open,dev_name: %s, fd: %d\n","/dev/ali_soc", fd);
	fd = open("/dev/ali_soc", O_RDONLY);
	if(fd < 0) {
		SL_ERR("open ali_soc failed.\n");
		return -1;
	}
	SL_DBG("fd: %d, cmd: %d, param: %p\n", fd, cmd, param);
	ret = ioctl(fd, cmd, param);
	close(fd);

	return ret;
}

/**
 *  Function Name:	alislstandby_pmu_enter_standby
 *  @brief			enter pmu standby
 *
 *  @param          void
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_enter_standby()
{
	int fd = 0;
	//int rev_id = -1;
	//int chip_id = -1;
	unsigned long pwr_flag = 0;

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	pwr_flag = ALISL_PWR_STANDBY;
	SL_DBG("call ALI_PMU_SAVE_WDT_REBOOT_FLAG fd: %d, pwr_flag: 0x%x\n", fd, pwr_flag);
	if (ioctl(fd, ALI_PMU_SAVE_WDT_REBOOT_FLAG, &pwr_flag)) {
		SL_ERR("ALISL_PWR_STANDBY flag setting error.\n");
	}

	//rev_id = alislstandby_get_sys_info(ALI_SOC_REV_ID, NULL);
	//chip_id = alislstandby_get_sys_info(ALI_SOC_CHIP_ID, NULL);

	SL_DBG("call ALI_PMU_MCU_ENTER_STANDBY fd: %d\n", fd);
	if ( 0 == ioctl(fd, ALI_PMU_MCU_ENTER_STANDBY, 0) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	} else {
		SL_ERR("enable pmu mcu standby failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}


/**
 *  Function Name:	alislstandby_pmu_set_msalarm
 *  @brief		    set pmu ms alarm
 *
 *  @param          ms_num
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_msalarm(struct mcu_ms_alarm_num* mcu_ms_num)
{
	int fd = 0;
	struct ms_alarm_num *ms_num = NULL;

	if (!mcu_ms_num) {
		SL_ERR("param mcu_ms_num is NULL.\n");
		return ERROR_INVAL;
	}
	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	ms_num = (struct ms_alarm_num *)mcu_ms_num;
	SL_DBG("call ALI_PMU_MCU_ENTER_STANDBY fd: %d, %02d:%02d:%02d:%02d, %02d:%02d:%02d:%02d\n",
											fd, ms_num->ms_alm.en_hour,ms_num->ms_alm.en_min,
											ms_num->ms_alm.en_sec,ms_num->ms_alm.en_ms,
											ms_num->ms_alm.hour,ms_num->ms_alm.min,
											ms_num->ms_alm.sec,ms_num->ms_alm.ms);
	if ( 0 == ioctl(fd, ALI_PMU_RTC_SET_MS_ALARM, ms_num) ) {
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("pmu set ms alarm failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_pmu_read_rtcvalue
 *  @brief			read rtc time value
 *
 *  @param          [out]prtctime:rtc time get from pmu
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_read_rtcvalue(struct mcu_rtc_time *prtctime)
{
	int fd = 0;
	struct rtc_time_pmu *ptime = NULL;

	ptime = (struct rtc_time_pmu*)malloc(sizeof(struct rtc_time_pmu));
	if (!ptime) {
		SL_ERR("pmu malloc rtc time failed.\n");
		prtctime = NULL;
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		free(ptime);
		prtctime = NULL;
		return ERROR_NULLDEV;
	}

	if ( 0 == ioctl(fd, ALI_PMU_RTC_RD_VAL, ptime) ) {
		//prtctime = (struct mcu_rtc_time*)ptime;
		prtctime->year = ptime->year;
		prtctime->month = (unsigned char)ptime->month;
		prtctime->date = (unsigned char)ptime->date;
		prtctime->day = (unsigned char)ptime->day;
		prtctime->hour = (unsigned char)ptime->hour;
		prtctime->min = (unsigned char)ptime->min;
		prtctime->sec = (unsigned char)ptime->sec;
		SL_DBG("call ALI_PMU_RTC_RD_VAL %d-%d-%d %d %d:%d:%d\n",
				        ptime->year, ptime->month, ptime->date,
				        ptime->day, ptime->hour, ptime->min, ptime->sec);
		free(ptime);
		alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("pmu read rtc value failed.\n");
		free(ptime);
		prtctime = NULL;
		alislstandby_close_pmu_device(fd);
		return ERROR_READ;
	}
}

alisl_retcode alislstandby_pmu_read_mcuvalue(struct mcu_rtc_time *prtctime)
{
	int fd = 0;
	struct rtc_time_pmu *ptime = NULL;

	ptime = (struct rtc_time_pmu*)malloc(sizeof(struct rtc_time_pmu));
	if (!ptime) {
		SL_ERR("pmu malloc rtc time failed.\n");
		prtctime = NULL;
		return ERROR_INVAL;
	}

	fd = alislstandby_open_pmu_device();
	if (fd < 0) {
		SL_ERR("get pmu handle failed.\n");
		free(ptime);
		prtctime = NULL;
		return ERROR_NULLDEV;
	}

	if ( 0 == ioctl(fd, ALI_PMU_MCU_READ_TIME, ptime) ) {
		//prtctime = (struct mcu_rtc_time*)ptime;
		prtctime->year = ptime->year;
		prtctime->month = (unsigned char)ptime->month;
		prtctime->date = (unsigned char)ptime->date;
		prtctime->day = (unsigned char)ptime->day;
		prtctime->hour = (unsigned char)ptime->hour;
		prtctime->min = (unsigned char)ptime->min;
		prtctime->sec = (unsigned char)ptime->sec;
		SL_DBG("\r ALI_PMU_MCU_READ_TIME [%04d-%02d-%02d %02d:%02d:%02d].\n",
				ptime->year, ptime->month, ptime->date,
				ptime->hour, ptime->min, ptime->sec);
		free(ptime);
		alislstandby_close_pmu_device(fd);
		return 0;
	} else {
		SL_ERR("pmu read mcu value failed.\n");
		free(ptime);
		prtctime = NULL;
		alislstandby_close_pmu_device(fd);
		return ERROR_READ;
	}
}

/**
 *  Function Name:	alislstandby_pmu_set_mcuwakeuptime
 *  @brief			set mcu wakeup time
 *
 *  @param          wakeuptime
 *
 *  @return         -1 for failed, 1 for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_mcuwakeuptime(struct mcu_min_alarm_num *wakeuptime)
{
	int fd = 0;
	struct min_alarm_num *wakeup_time = NULL;

	if (!wakeuptime) {
		 SL_ERR("param wakeuptime is NULL.\n");
	     return ERROR_INVAL;
	}

    fd = alislstandby_open_pmu_device();
	if ( fd < 0 ) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_READ;
	}

	wakeup_time = (struct min_alarm_num *)wakeuptime;
	SL_DBG("\nALI_PMU_MCU_WAKEUP_TIME num=%d %02d-%02d %02d:%02d:%02d\n",
			wakeup_time->num, wakeup_time->min_alm.month, wakeup_time->min_alm.date,
			wakeup_time->min_alm.hour, wakeup_time->min_alm.min, wakeup_time->min_alm.second);
	SL_DBG("Enable: %d-%d sun-sat:%d %d %d %d %d %d %d\n",
		wakeup_time->min_alm.en_month, wakeup_time->min_alm.en_date,
		wakeup_time->min_alm.en_sun, wakeup_time->min_alm.en_mon,
		wakeup_time->min_alm.en_tue, wakeup_time->min_alm.en_wed,
		wakeup_time->min_alm.en_thr, wakeup_time->min_alm.en_fri,
		wakeup_time->min_alm.en_sat);
	if ( 0 == ioctl(fd, ALI_PMU_MCU_WAKEUP_TIME, wakeup_time) ) {
	    alislstandby_close_pmu_device(fd);
		return 0;
	}
	else {
		SL_ERR("set pmu mcu wakeup time failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}

/**
 *  Function Name:	alislstandby_soc_enter_standby
 *  @brief
 *
 *  @param          [in]exp_time:expired time
 *  @param          [in]cur_time:current time
 *
 *  @return         0 -- successed      -1 -- failed
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/15/2013, Created
 *
 *  @note This function is Not used any more.
 */
//alisl_retcode alislstandby_soc_enter_standby(unsigned int exp_time, unsigned int cur_time)
//{
//    unsigned int param[2] = {0};
//	int fd = 0;
//
//	param[0] = exp_time;
//	param[1] = cur_time;
//	fd = open("/dev/ali_soc", O_RDONLY);
//	if (fd < 0) {
//		SL_ERR("open ali soc failed.\n");
//		return ERROR_OPEN;
//	}
//
//	return ioctl(fd, ALI_SOC_ENTER_STANDBY, param);
//}
