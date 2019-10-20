/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file			alislstandby.h
 *  @brief			Ali share library of standby
 *
 *  @version		1.0
 *  @date			06/19/2013 02:12:08 PM
 *  @revision		none
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 */
#include <ali_pmu_common.h>
#include <ali_pm_common.h>
//#include <ali_soc_common.h>
#include <alipltfretcode.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <alipltflog.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct pm_standby_key {
    unsigned char standbykey;  /* standby physical key value */
    unsigned long powerkey;    /* power physical key value */
	unsigned char rev[3];      /* reserved param */
}pm_standby_key_t;

typedef struct pm_standby_param {
    unsigned int board_power_gpio;  /* pm standby board power gpio */
    unsigned long timeout;          /* pm standby auto resume time */
}pm_standby_param_t;

struct mcu_min_alarm
{
	unsigned char en_month;
	unsigned char en_date;
	unsigned char en_sun;
	unsigned char en_mon;
	unsigned char en_tue;
	unsigned char en_wed;
	unsigned char en_thr;
	unsigned char en_fri;
	unsigned char en_sat;
	unsigned char month;
	unsigned char date;
	unsigned char hour;
	unsigned char min;
};

struct mcu_min_alarm_num
{
    struct mcu_min_alarm min_alarm;
	unsigned char num;    /*  rtc alarm number,from 0~7 */
};

struct mcu_ms_alarm
{
	unsigned char en_hour;
	unsigned char en_min;
	unsigned char en_sec;
	unsigned char en_ms;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
    unsigned char ms;
};

struct mcu_ms_alarm_num
{
    struct mcu_ms_alarm ms_alarm;
    unsigned char num;        /* rtc alarm number,from 8~9 */
};

struct mcu_rtc_time
{
	unsigned int year;
	unsigned int month;
	unsigned int date;
	unsigned int day;
	unsigned int hour;
	unsigned int min;
    unsigned int sec;
};

/**
 *  Function Name:  alislstandby_open_pm_device
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
int alislstandby_open_pm_device(void);

/**
 *  Function Name:	alislstandby_pm_set_resumekey
 *  @brief			set pm standby resume key
 *
 *  @param          [in]pm_resumekey
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pm_set_resumekey(struct pm_standby_key *pm_resumekey);

/**
 *  Function Name:	alislstandby_pm_set_resumetime
 *  @brief			set pm standby resume time
 *
 *  @param          [in]pm_param
 *
 *  @return         ALISLSTANDBY_ERR, ALISLSTANDBY_OK
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pm_set_resumetime(struct pm_standby_param *pm_param);

/**
 *  Function Name:	alislstandby_pmu_set_resumekey
 *  @brief			set pmu resume key
 *
 *  @param          resumekey
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_resumekey(uint32_t resumekey);

/**
 *  Function Name:	alislstandby_pmu_set_rtctime
 *  @brief			set current time to pmu rtc
 *
 *  @param          current time
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_rtctime(struct mcu_rtc_time* curtime);

/**
 *  Function Name:	alislstandby_pmu_set_mcutime
 *  @brief			set current time to pmu mcu
 *
 *  @param          current time
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_mcutime(struct mcu_rtc_time* curtime);

/**
 *  Function Name:	alislstandby_pmu_enable_showtime
 *  @brief			enable show pmu time
 *
 *  @param          pannelflag
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/20/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_enable_showtime(enum MCU_SHOW_PANNEL *pannelflag);

/**
 *  Function Name:	alislstandby_pmu_set_minalarm
 *  @brief			set min alarm time
 *
 *  @param          alarmtime
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_minalarm(struct min_alarm_num *alarmtime);

/**
 *  Function Name:	alislstandby_pmu_enable_alarm
 *  @brief		    enable pmu alarm
 *
 *  @param          void
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_enable_alarm(void);

/**
 *  Function Name:	alislstandby_pmu_enable_mcustandby
 *  @brief			enable pmu mcu standby
 *
 *  @param          flag
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_enable_mcustandby(int flag);

/**
 *  Function Name:	alislstandby_pmu_set_msalarm
 *  @brief		    set pmu ms alarm
 *
 *  @param          ms_num
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_msalarm(struct mcu_ms_alarm_num* ms_num);

/**
 *  Function Name:	alislstandby_pmu_read_rtcmsvalue
 *  @brief			read rtc ms time value
 *
 *  @param          pmstime
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_read_rtcmsvalue(unsigned short *pmstime);

/**
 *  Function Name:	alislstandby_pmu_read_exitstatus
 *  @brief			read pmu exit standby status
 *
 *  @param          uint8_t status
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_read_exitstatus(uint8_t *pstatus);

/**
 *  Function Name:	alislstandby_pmu_set_mcuwakeuptime
 *  @brief			set mcu wakeup time
 *
 *  @param          wakeuptime
 *
 *  @return         ALISLSTANDBY_ERR for failed, ALISLSTANDBY_OK for success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_pmu_set_mcuwakeuptime(struct mcu_min_alarm_num *wakeuptime);
#ifdef __cplusplus
}
#endif

