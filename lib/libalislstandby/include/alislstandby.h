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
#include <alipltfretcode.h>
#include <time.h>
#include <alislevent.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* exit standby mode */
typedef enum alisl_pmu_exit {
	ALISLS_PMU_POWER_EXIT,
	/*exit standby by timer*/
	ALISLS_PMU_TIMER_EXIT,
	/*exit standby by panel*/
	ALISLS_PMU_PANEL_EXIT,
	/*exit standby by IR*/
	ALISLS_PMU_IR_EXIT,
	/* Reboot by watch dog*/
	ALISLS_PMU_WDT_EXIT,
    /* Reboot by reboot command, also by watch dog*/
    ALISLS_PMU_REBOOT_EXIT
} alisl_pmu_exit_t;

typedef enum alisl_pmu_show_mode
{
	ALISLS_PMU_SHOW_OFF,
	ALISLS_PMU_SHOW_TIME,
	ALISLS_PMU_SHOW_BLANK,
	ALISLS_PMU_SHOW_DEFAULT,
	ALISLS_PMU_SHOW_NO_CHANGE,
} alisl_pmu_show_mode_t;

/**
 *  Function Name:	alislstandby_enter_pm_standby
 *  @brief			call this function to enter pm standby
 *
 *  @param          void
 *
 *  @return         0 -- success     others -- failed
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/12/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_enter_pm_standby(void);

/**
 *  Function Name:	alislstandby_enter_pmu_standby
 *  @brief			call this function to enter pmu standby
 *
 *  @param          void
 *
 *  @return         0 -- success     others -- failed
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/12/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_enter_pmu_standby(void);

alisl_retcode alislstandby_standby_init();

alisl_retcode alislstandby_set_current_timer(struct tm *pcurtime);

alisl_retcode alislstandby_get_current_timer(struct tm *pcurtime);

alisl_retcode alislstandby_set_wakeup_timer(struct tm *p_time);

alisl_retcode alislstandby_set_ir(unsigned int firstkey,
		unsigned int firstkey_type,
		unsigned int secondkey,
		unsigned int secondkey_type);

int alislstandby_set_panel_display_mode(int mode);

alisl_retcode alislstandby_pmu_enable_alarm();

alisl_retcode alislstandby_standby_enter();

alisl_retcode alislstandby_get_powerup_status(unsigned char *mode);

struct alislrtc_timer
{
	struct itimerspec its;
	struct alislevent slev;
	int config;
	/*! // 0:disable 1:enable 2:on */
	int state;
	struct tm tm;
	int ms;
	/*! 0 minute timer, 1 ms timer */
	int type;
	void *cb;
};

int alislrtc_ioctl(int filedes, int command, void *data);

unsigned long alislrtc_setalarm(int rtc_fd, alisl_handle handle,
		struct alislrtc_timer *timer, int en);


#ifdef __cplusplus
}
#endif


