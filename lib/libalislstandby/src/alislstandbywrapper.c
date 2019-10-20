/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file			alislstandbywrapper.c
 *  @brief		    wrapper level of standby moudle
 *
 *  @version		1.0
 *  @date			07/12/2013 09:16:12 AM
 *  @revision		none
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 */
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>
#include <linux/input.h>
#include "alislstandby.h"
#include "internal.h"
#include "ali_keyset.h"
#include "error.h"
#include <ali_pmu_common.h>

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <alislevent.h>
#include <alisl_types.h>

#define SUSPEND_RESUME_KEY 0x44
#define MAX_CHAR_LINE 1024

#define PANEL_DEV_FILE ("/dev/pan_ch455")

typedef int (*pget_scancode)(char *_evdevname, struct ali_str2key_tbl_t *str2key_tbl,
		unsigned long val, unsigned long cmd);
pget_scancode m_getscancode = NULL;
extern int alislstandby_get_sys_info(unsigned int cmd, void *param);
//static int hexalpha_to_int(int c);
//static int is_hex_char(int c);
//static unsigned long ahtoi(const char *s_ptr);
static int find_int_value(char* line, unsigned long* value, const char *key);
static int get_standby_key();

alisl_retcode alislstandby_pmu_read_rtcvalue(struct mcu_rtc_time *prtctime);
alisl_retcode alislstandby_pm_set_resumeparam(struct pm_standby_param *pm_param);
alisl_retcode alislstandby_pmu_get_mcutime(struct mcu_rtc_time *curtime);

alisl_retcode alislstandby_pmu_enter_standby();
int alislstandby_open_pmu_device();
int alislstandby_close_pmu_device(int fd);

/**
 *  Function Name:	alislstandby_get_func_handle
 *  @brief			auto get wakeup key value
 *
 *  @param
 *
 *  @return         wakeup key value(0 -- failed)
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/12/2013, Created
 *
 *  @note
 */
static alisl_retcode alislstandby_get_func_handle()
{
	void *pkeyhandle = NULL;
	void *pfun = NULL;

	pkeyhandle = dlopen("libkeyset.so", RTLD_NOW);
	if (!pkeyhandle) {
		SL_DBG("open libkeyset.so failed.\n");
		return ERROR_OPEN;
	}
	pfun = dlsym(pkeyhandle, "get_code");
	if (pfun) {
		m_getscancode = (pget_scancode)pfun;
	}
	else {
		SL_DBG("get func handle failed.\n");
		return ERROR_NULLDEV;
	}

	return 0;
}

/**
 *  Function Name:	alislstandby_autoget_resume_key
 *  @brief			get standby resume key
 *
 *  @param			void
 *
 *  @return         0 -- failed   others -- resume key
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/18/2013, Created
 *
 *  @note
 */
uint32_t alislstandby_autoget_resume_key()
{
	struct ali_str2key_tbl_t str2key_tbl;
	FILE *fp = NULL;

	alislstandby_get_func_handle();
	/* get power key physical value */
	if (m_getscancode) {
		memset(&str2key_tbl, 0, sizeof(struct ali_str2key_tbl_t));
		if ((fp = fopen("/dev/event0", "r")) != NULL) {
			fclose(fp);
			(*m_getscancode)("/dev/event0", &str2key_tbl, KEY_POWER,
					GET_SCANCODE_MASK);
		} else {
			(*m_getscancode)("/dev/input/event0", &str2key_tbl, KEY_POWER,
					GET_SCANCODE_MASK);
		}
		if (str2key_tbl.key_array.used) {
			return str2key_tbl.key_array.array[0];
		} else {
			SL_DBG(" get key failed.\n");
			return 0;
		}
	} else {
		SL_DBG("get function handle failed.\n");
		return 0;
	}
}

/**
 *  Function Name:	alislstandby_enter_pm_standby()
 *  @brief			call this function to enter pm standby
 *
 *  @param
 *
 *  @return         0 -- success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/12/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_enter_pm_standby()
{
	struct pm_standby_key key;
	//struct pm_standby_param param;
	FILE *fp = NULL;
	FILE *fp_state = NULL;
	static const char *sys_state = "/sys/power/state";
	static const char *mem_state = "mem\n";
	int ret = 0;
	int fd = 0;
	unsigned long pwr_flag = 0;

	memset(&key , 0, sizeof(struct pm_standby_key));
	//memset(&param , 0, sizeof(struct pm_standby_param));

	/* set pm resume key */
	key.standbykey = SUSPEND_RESUME_KEY;
	/* get power key physical value */
	if ((fp = fopen("/etc/lirc/lircd.conf", "r")) != NULL) {
		fclose(fp);
		//key.powerkey = get_standby_key();
	} else {
		key.powerkey = alislstandby_autoget_resume_key();
	}
	if (!key.powerkey) {
		SL_DBG("get power key failed.\n");
		//return ERROR_READ;
	} else {
		alislstandby_pm_set_resumekey(&key);
	}

	fd = alislstandby_open_pmu_device();
	if (fd < 0) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}

	pwr_flag = ALISL_PWR_STANDBY;
	if (ioctl(fd, ALI_PMU_SAVE_WDT_REBOOT_FLAG, &pwr_flag)) {
		SL_ERR("ALISL_PWR_STANDBY flag setting error.\n");
	}
	alislstandby_close_pmu_device(fd);

	/* set pm resume param */
	//param.board_power_gpio = 0;
	//param.timeout = 0;
	//alislstandby_pm_set_resumeparam(&param);
	/**
	 *  make the platform goes into the STR (Suspend-to-RAM) mode.
	 *  refer: //depot/Documents/STU1000/Feature/PMU/Suspend_To_RAM/M3527_VMX/Buildroot_master(PDK1.11)_M3527_VMX_STR_User_Guide.pdf
	 *  The STR command: echo "mem" >/sys/power/state
	 *
	 */
	if ((fp_state = fopen(sys_state, "w")) != NULL) {
		if (fwrite(mem_state, sizeof(mem_state), 1, fp_state) != 1) {
			SL_ERR("write %s error\n", sys_state);
			ret = ERROR_FAILED;
		}
		fclose(fp_state);
	} else {
		SL_ERR("open %s error\n", sys_state);
		ret = ERROR_OPEN;
	}

	//if (system("echo \"mem\" >/sys/power/state") == -1)
	//	return ERROR_FAILED;

	return ret;
}

/**
 *  Function Name:	alislstandby_enter_pmu_standby()
 *  @brief			call this function to enter pmu standby
 *
 *  @param
 *
 *  @return         0 -- success
 *
 *  @author			Alan Zhang <Alan.Zhang@alitech.com>
 *  @date			07/12/2013, Created
 *
 *  @note
 */
alisl_retcode alislstandby_enter_pmu_standby()
{
	//int ihandle = 0;  /* /dev/pmu handle */
	uint32_t uwakeup_key = 0;
	struct mcu_rtc_time *cur_time = NULL;
//	time_t timep;
//	struct tm *pcurtime = NULL;
	enum MCU_SHOW_PANNEL mcu_show_flag;
	FILE *fp = NULL;
	int fd = 0;
	//int rev_id = -1;
	//int chip_id = -1;
	alisl_retcode rtc_ret = 0;

	/* malloc time space */
	cur_time = (struct mcu_rtc_time*)malloc(sizeof(struct mcu_rtc_time));
	if (!cur_time) {
		SL_ERR("malloc cur_time failed.\n");
		return ERROR_INVAL;
	}

	/* open device */
	//ihandle = alislstandby_open_pmu_device();
	//if (ihandle < 0) {
	//	SL_ERR("open pmu device failed.\n");
	//	return ERROR_NULLDEV;
	//}

	if ((fp = fopen("/etc/lirc/lircd.conf", "r")) != NULL) {
		fclose(fp);
		//uwakeup_key = get_standby_key();
	} else {
		uwakeup_key = alislstandby_autoget_resume_key();
	}
	if (0 == uwakeup_key) {
		SL_ERR("failed to get pmu resume key.\n");
		//return ERROR_READ;
	} else if (0 != alislstandby_pmu_set_resumekey(uwakeup_key)) {
		SL_ERR("set pmu resume key failed.\n");
		free(cur_time);
		return ERROR_WRITE;
	}

	//time(&timep);
	//pcurtime = gmtime(&timep); /* get local time */

	rtc_ret = alislstandby_pmu_read_rtcvalue(cur_time);
	if (rtc_ret) {
		SL_ERR("Get RTC TIME error.\n");
		free(cur_time);
		return ERROR_INVAL;
	}

	//rev_id = alislstandby_get_sys_info(ALI_SOC_REV_ID, NULL);
	//chip_id = alislstandby_get_sys_info(ALI_SOC_CHIP_ID, NULL);
	//cur_time->year = 1900 + pcurtime->tm_year;
	alislstandby_pmu_set_mcutime(cur_time);
	mcu_show_flag = MCU_SHOW_BANK;
	alislstandby_pmu_enable_showtime(&mcu_show_flag);

	/* Enable panel LED and standby key */
    fd = open(PANEL_DEV_FILE, O_RDWR | O_NONBLOCK);

	/* enter standby */
	alislstandby_pmu_enter_standby();

	/* close pmu handle */
	//alislstandby_close_pmu_device(ihandle);

	/* SOC enter standby */

	//alislstandby_soc_enter_standby(0, cur_time); //Not used any more.
	free(cur_time);
	close(fd);//release the fd.
	cur_time = NULL;

	return rtc_ret;
}

alisl_retcode alislstandby_standby_init()
{
	SL_DBG("alislstandby_standby_init");
	SL_DBG("alislstandby_standby_init");
	return 0;
}

alisl_retcode alislstandby_set_current_timer(struct tm *pcurtime)
{
	alisl_retcode ret = 0;
	SL_DBG("alislstandby_set_current_timer");
	//SL_DBG(asctime(pcurtime));
	//SL_DBG("Set current time: %d-%d-%d %d:%d:%d" ,
	//	pcurtime->tm_year, pcurtime->tm_mon + 1, pcurtime->tm_mday,
	//	pcurtime->tm_hour, pcurtime->tm_min, pcurtime->tm_sec);
	if (pcurtime == NULL) {
		SL_ERR("current time is NULL\n");
		return ERROR_INVAL;
	}
	struct mcu_rtc_time *cur_time = NULL;
	enum MCU_SHOW_PANNEL mcu_show_flag;
	//int rev_id = -1;
	//int chip_id = -1;

	/* malloc time space */
	cur_time = (struct mcu_rtc_time*)malloc(sizeof(struct mcu_rtc_time));
	if (!cur_time) {
		SL_ERR("malloc cur_time failed.\n");
		return ERROR_INVAL;
	}
	// Cause: struct tm tm_mon is 0~11,
	// while aui_clock month and aui_min_alarm month is 1~12.
	// struct tm tm_year is years since 1900
	// while aui_clock year is normal year.
	cur_time->month = pcurtime->tm_mon + 1;
	cur_time->date = pcurtime->tm_mday;
	cur_time->hour = pcurtime->tm_hour;
	cur_time->min = pcurtime->tm_min;
	cur_time->sec = pcurtime->tm_sec;

	//rev_id = alislstandby_get_sys_info(ALI_SOC_REV_ID, NULL);
	//chip_id = alislstandby_get_sys_info(ALI_SOC_CHIP_ID, NULL);
	cur_time->year = 1900 + pcurtime->tm_year;
	ret = alislstandby_pmu_set_mcutime(cur_time);
	mcu_show_flag = MCU_SHOW_BANK;
	alislstandby_pmu_enable_showtime(&mcu_show_flag);
	free(cur_time);
	return ret;
}

alisl_retcode alislstandby_get_current_timer(struct tm *pcurtime)
{
	alisl_retcode ret = 0;
	SL_DBG("alislstandby_set_current_timer");
	//SL_DBG(asctime(pcurtime));
	//SL_DBG("Set current time: %d-%d-%d %d:%d:%d" ,
	//	pcurtime->tm_year, pcurtime->tm_mon + 1, pcurtime->tm_mday,
	//	pcurtime->tm_hour, pcurtime->tm_min, pcurtime->tm_sec);
	if (pcurtime == NULL) {
		SL_ERR("current time is NULL\n");
		return ERROR_INVAL;
	}
	struct mcu_rtc_time *cur_time = NULL;
//	enum MCU_SHOW_PANNEL mcu_show_flag;
	//int rev_id = -1;
	//int chip_id = -1;

	/* malloc time space */
	cur_time = (struct mcu_rtc_time*)malloc(sizeof(struct mcu_rtc_time));
	if (!cur_time) {
		SL_ERR("malloc cur_time failed.\n");
		return ERROR_INVAL;
	}

	//rev_id = alislstandby_get_sys_info(ALI_SOC_REV_ID, NULL);
	//chip_id = alislstandby_get_sys_info(ALI_SOC_CHIP_ID, NULL);
	ret = alislstandby_pmu_get_mcutime(cur_time);

	// Cause: struct tm tm_mon is 0~11,
	// while aui_clock month and aui_min_alarm month is 1~12.
	// struct tm tm_year is years since 1900
	// while aui_clock year is normal year.
	pcurtime->tm_year = cur_time->year - 1900;
	pcurtime->tm_mon = cur_time->month - 1;
	pcurtime->tm_mday = cur_time->date;
	pcurtime->tm_hour = cur_time->hour;
	pcurtime->tm_min = cur_time->min;
	pcurtime->tm_sec = cur_time->sec;
	free(cur_time);
	return ret;
}

alisl_retcode alislstandby_set_wakeup_timer(struct tm *p_time)
{
	SL_DBG("alislstandby_set_wakeup_timer\n");

	//SL_DBG(asctime(p_time));
	//SL_DBG("Set wakeup time: %d-%d-%d %d:%d:%d" ,
	//	p_time->tm_year, p_time->tm_mon + 1, p_time->tm_mday,
	//	p_time->tm_hour, p_time->tm_min, p_time->tm_sec);

	if (p_time == NULL) {
		SL_ERR("alarm time is NULL\n");
		return ERROR_INVAL;
	}
	struct min_alarm_num alarmtime;

	// Cause: struct tm tm_mon is 0~11,
	// while aui_clock month and aui_min_alarm month is 1~12.
	alarmtime.num = 0;
	alarmtime.min_alm.en_month = 1;
	alarmtime.min_alm.en_date  = 1;
	alarmtime.min_alm.en_sun = 0;
	alarmtime.min_alm.en_mon = 0;
	alarmtime.min_alm.en_tue = 0;
	alarmtime.min_alm.en_wed = 0;
	alarmtime.min_alm.en_thr = 0;
	alarmtime.min_alm.en_fri = 0;
	alarmtime.min_alm.en_sat = 0;
	alarmtime.min_alm.month = (unsigned char)p_time->tm_mon + 1;
	alarmtime.min_alm.date = (unsigned char)p_time->tm_mday;
	alarmtime.min_alm.hour = (unsigned char)p_time->tm_hour;
	alarmtime.min_alm.min = (unsigned char)p_time->tm_min;
	alarmtime.min_alm.second = (unsigned char)p_time->tm_sec;

	alislstandby_pmu_set_minalarm(&alarmtime);
	return 0;
}

alisl_retcode alislstandby_standby_enter()
{
	SL_DBG("alislstandby_standby_enter");
	int fd = 0;
	//alislstandby_pmu_enable_alarm();

	/* Enable panel LED and standby key */
	fd = open(PANEL_DEV_FILE, O_RDWR | O_NONBLOCK);

	/* enter standby */
	alislstandby_pmu_enter_standby();

	/* close pmu handle */
	//alislstandby_close_pmu_device();

	//SL_DBG("cur_time=%d,wakeup_time=%d\n", cur_time, wakeup_time);
	/* SOC enter standby */
	//alislstandby_soc_enter_standby(&wakeup_time, &cur_time);

	close(fd);
	return 0;
}

/**
*    Function Name:	alislstandby_set_ir
*    @brief             set wakeup key,if the key is pressed,STB will exit standby.
*    @author            raynard.wang
*    @date              2014-6.24
*    @param[in]         firstkey		the first wakeup key
*    @param[in]         firstkey_type	type of the first wakeup key.
*    @param[in]         secondkey		the second wakeup key.
*    @param[in]         firstkey_type	type of the second wakeup key
*    @return            0 success
*    @return            others failure
*    @note              If success, standby device initialize successfully!\n
*						type of wakeup key is list here:\n
*							NEC 	= 0,\n
*							LAB 	= 1,\n
*							50560 	= 2,\n
*							KF		= 3,\n
*							LOGIC	= 4,\n
*							SRC		= 5,\n
*							NSE		= 6,\n
*							RC5		= 7,\n
*							RC5_X	= 8,\n
*							RC6		= 9,\n
*						To make sure which type of IR,please refer to datasheet.
*
*/
alisl_retcode alislstandby_set_ir(unsigned int firstkey,
		unsigned int firstkey_type,
		unsigned int secondkey,
		unsigned int secondkey_type)
{
	int fd = 0;
	unsigned long keys[8] = {0};
	FILE *fp = NULL;
	(void)firstkey_type;
	(void)secondkey_type;
	unsigned int power_key;
	fd = alislstandby_open_pmu_device();

	keys[0]= firstkey;

	if  (!keys[0]) {
		keys[0] = secondkey;
	} else {
		keys[1] = secondkey;
	}

	if ((!firstkey) || (!secondkey)) {
		if (fd < 0) {
			SL_ERR("get pmu handle failed.\n");
			return ERROR_NULLDEV;
		}
		if ((fp = fopen("/etc/lirc/lircd.conf", "r")) != NULL) {
			fclose(fp);
			power_key = get_standby_key();
		} else {
			power_key = alislstandby_autoget_resume_key();
		}
		if (!keys[0]) {
			keys[0]= power_key;
		} else {
			keys[1]= power_key;
		}
	}

	SL_DBG("set pmu resume key %#x / %#x\n", keys[0], keys[1]);
	if (0 == ioctl(fd, ALI_PMU_IR_PROTOL_NEC, keys)) {
	    alislstandby_close_pmu_device(fd);
		return 0;
	} else {
		SL_ERR("set pmu resume key failed.\n");
		alislstandby_close_pmu_device(fd);
		return ERROR_WRITE;
	}
}
/*
static int hexalpha_to_int(int c)
{
	const char *alpha = "aAbBcCdDeEfF";
	int i;
	int ret = 0;
	for (i=0; alpha[i]!='\0'; i++) {
		if (alpha[i] == c) {
			ret = 10 + (i/2);
			break;
		}
	}
	return ret;
}
	if ((c >= '0' && c <= '9') || \
			(c >= 'a' && c <= 'f') || \
			(c >= 'A' && c <= 'F')) {
		return 1;
	} else {
		return 0;
	}
}
static unsigned long ahtoi(const char *s_ptr) {
	unsigned long ret = 0;
	int i = 0;
	int hexint;

	//remove 0x or 0X
	if ('0' == s_ptr[i]) {
		i++;
		if ('x' == s_ptr[i] || 'X' == s_ptr[i]) {
			i++;
		}
	}

	while (is_hex_char(s_ptr[i])) {
		ret = ret*16;
		if(s_ptr[i]>='0' && s_ptr[i]<='9') {
			ret += s_ptr[i] - '0';
		} else {
			hexint = hexalpha_to_int(s_ptr[i]);
			ret += hexint;
		}
		i++;
	}

	return ret;
}
*/

static int find_int_value(char* line, unsigned long* value, const char *key) {
	char *start = NULL;
	char *pEnd = NULL;

	start = strstr(line, key);
	if (NULL != start) {
		start += strlen(key);
		*value = strtol (start, &pEnd, 0);
		return 0;
	} else {
		return -1;
	}
}

/**
 *  Function Name:	get_standby_key()
 *  @brief			In linux system, Find the (NEC)code of the first KEY_POWER
 *  				in /etc/lirc/lircd.conf
 *
 *  @return         -1 -- failed   others -- code of KEY_POWER
 *
 *  @author			Oscar shi <oscar.shi@alitech.com>
 *  @date			08/18/2014, Created
 *
 *  @note
 */
static int get_standby_key(void)
{
	const char *LIRCD_PATH = "/etc/lirc/lircd.conf";
	unsigned long power_key[8];
	unsigned long bits = 0;
	unsigned long pre_data = 0;
	unsigned long key_home = 0;
	int i = 0;
	char cur_line[MAX_CHAR_LINE] = {0};
	FILE *file_handle = NULL;

	memset(power_key, 0, sizeof(power_key));
	file_handle = fopen(LIRCD_PATH, "r");

	if (file_handle) {
		while (fgets(cur_line, MAX_CHAR_LINE, file_handle)) {
			cur_line[MAX_CHAR_LINE - 1] = 0;
			if (16 == bits) {
				if (pre_data) {
					if (0 == find_int_value(cur_line, &key_home, "KEY_POWER")) {
						key_home += (pre_data << 16);
						if (i < 7) {
							power_key[i++] = key_home;
							bits = 0;
							pre_data = 0;
							SL_DBG("get short KEY_HOME:0x%lx\n", key_home);
						} else {
							SL_DBG("KEY_HOME Full for 8\n");
							break;
						}
					}
				} else {
					if (0 == find_int_value(cur_line, &pre_data, "pre_data") && \
							0 == find_int_value(cur_line, &pre_data, "pre_data_bits")) {
						pre_data = 0;
					}
				}
			} else if (32 == bits) {
				if (0 == find_int_value(cur_line, &key_home, "KEY_POWER")) {
					if (i < 7) {
						power_key[i++] = key_home;
						bits = 0;
						pre_data = 0;
						SL_DBG("get long KEY_HOME:0x%lx\n", key_home);
					} else {
						SL_DBG("KEY_HOME Full for 8\n");
						break;
					}
				}
			} else {
				if (0 == find_int_value(cur_line, &bits, "bits") && \
						0 == find_int_value(cur_line, &bits, "pre_data_bits")) {
					bits = 0;
				}
			}
			memset(cur_line, 0, MAX_CHAR_LINE);
		}
	} else {
		SL_DBG("open lird.conf fail!\n");
		return power_key[0];
		//    return -1;
	}

	fclose(file_handle);

	return power_key[0];
	//  if (power_key[0]) {
	//    return 0;
	//  } else {
	//    return -1;
	//  }
}

/**
 * Function Name:  alislstandby_set_panel_display_mode
 * @brief          PMU standby set panel display mode
 * @param[in]      mode
 * @return         0 for success ,other for failed
 * @author         Oscar.Shi <oscar.shi@alitech.com>
 * @date           08/27/2014, Created
 * @note
 */
int alislstandby_set_panel_display_mode(int mode)
{
	int fd = 0;
	int ret = 0;
	enum MCU_SHOW_PANNEL flag = MCU_SHOW_DEFAULT;

	fd = alislstandby_open_pmu_device();
	if (fd < 0) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}
	switch (mode)
	{
	case ALISLS_PMU_SHOW_OFF:
		flag = MCU_SHOW_OFF;
		break;
	case ALISLS_PMU_SHOW_TIME:
		flag = MCU_SHOW_TIME;
		break;
	case ALISLS_PMU_SHOW_BLANK:
		flag = MCU_SHOW_BANK;
		break;
	case ALISLS_PMU_SHOW_DEFAULT:
		flag = MCU_SHOW_DEFAULT;
		break;
	case ALISLS_PMU_SHOW_NO_CHANGE:
		flag = MCU_NO_CHANGE;
		break;
	default:
		flag = MCU_SHOW_DEFAULT;
	}

	if ((ret = ioctl(fd, ALI_PMU_SET_SHOW_PANEL_TYPE, &flag)) != 0) {
		SL_ERR("set PMU panel display mode failed.\n");
		alislstandby_close_pmu_device(fd);
	} else {
		SL_DBG("set PMU panel display mode: %d.\n", mode);
		alislstandby_close_pmu_device(fd);
	}
	return ret;
}

// WDT_REBOOT_FLAG identify some different wakeup reasons about WDT.
alisl_retcode alislstandby_get_powerup_status(unsigned char *mode)
{
	int fd = 0;
	alisl_retcode ret = 0;
	unsigned char exit_type = 0;
	unsigned long pwr_flag = 0;

	fd = alislstandby_open_pmu_device();
	if (fd < 0) {
		SL_ERR("get pmu handle failed.\n");
		return ERROR_NULLDEV;
	}
	ret = ioctl(fd, ALI_PMU_REPORT_EXIT_TYPE, &exit_type);
	if (ret) {
	    alislstandby_close_pmu_device(fd);
	    SL_ERR("Get ALI_PMU_REPORT_EXIT_TYPE failed.\n");
	    return -1;
	}
	//SL_DBG("[pwr_on_reason]REPORT: %d\n", (unsigned int)exit_type);

	// NOTE: read ALI_PMU_GET_WDT_REBOOT_FLAG
	ret |= ioctl(fd, ALI_PMU_GET_WDT_REBOOT_FLAG, &pwr_flag);
	if (ret) {
	    SL_ERR("Get ALI_PMU_GET_WDT_REBOOT_FLAG failed.\n");
	    alislstandby_close_pmu_device(fd);
	    return -1;
	}
	alislstandby_close_pmu_device(fd);
	//SL_DBG("[pwr_on_reason]pwr_flag: %lx\n", pwr_flag);

	switch (pwr_flag) {
	    case ALISL_PWR_WDT:
	        // NOTE: WDT reboot
	        *mode = ALISLS_PMU_WDT_EXIT;
	        break;
	    case ALISL_PWR_REBOOT:
	        // NOTE: reboot
	        *mode = ALISLS_PMU_REBOOT_EXIT;
	        break;
	    default:
	        // NOTE: wake up from standby
	        switch (exit_type) {
	            case E_PMU_KEY_EXIT:
	                *mode = ALISLS_PMU_PANEL_EXIT;
	                break;
	            case E_PMU_IR_EXIT:
	                *mode = ALISLS_PMU_IR_EXIT;
	                break;
	            case E_PMU_RTC_EXIT:
	                *mode = ALISLS_PMU_TIMER_EXIT;
	                break;
//	            case E_PMU_COLD_BOOT:
//	                *mode = ALISLS_PMU_POWER_EXIT;
//	                break;
//	            default:
//	                SL_ERR("Unknown standby wake up type.\n");
//	                return -1;
	            default:
	                // PMU driver will not provide E_PMU_COLD_BOOT anymore.
	                // If it's not ALISL_PWR_WDT and ALISL_PWR_REBOOT,
	                // and PMU can't confirm the boot reason, the boot reason
	                // is cold boot.
	                *mode = ALISLS_PMU_POWER_EXIT;
	                break;
	        }
	        break;
	}

	return ret;
}

/**
 *  Function Name:	alislrtc_ioctl()
 *  @brief			Wrapper functions of AUI rtc ioctl.
 *
 *    @param[in]    filedes		an open rtc file descriptor
 *    @param[in]    command		rtc command code
 *    @param[in]    data		the data struct poiner.
 *
 *  @return         -1 -- failed   success 0
 *
 *  @author			Oscar shi <oscar.shi@alitech.com>
 *  @date			08/18/2014, Created
 *
 *  @note
 */
int alislrtc_ioctl(int filedes, int command, void *data)
{
	int ret = -1;

	if (!filedes) {
		return -1;
	}
	switch (command) {
	case ALI_PMU_RTC_RD_VAL:
	case ALI_PMU_RTC_SET_VAL:
	case ALI_PMU_MCU_READ_TIME:
	case ALI_PMU_MCU_SET_TIME:
		break;
	default:
		return -1;
	}
	ret = ioctl(filedes, command, data);
	return ret;
}

int create_timerfd(struct itimerspec *its)
{
	struct timespec nw;
	unsigned long sum = 0;
	unsigned long ONE_SECOND_NS = 1000 * 1000 * 1000;
	if (clock_gettime(CLOCK_MONOTONIC, &nw) != 0){
	//	SL_ERR("clock_gettime error");
		return -1;
	}
	its->it_value.tv_sec = nw.tv_sec + its->it_value.tv_sec;
	sum = nw.tv_nsec + its->it_value.tv_nsec;
	if (sum > ONE_SECOND_NS) {
		sum -= ONE_SECOND_NS;
		its->it_value.tv_sec += 1;
	}
	its->it_value.tv_nsec = sum;
	return 0;
}

int rtc_to_tm(struct rtc_time_pmu *rtc, struct tm* tm)
{
	tm->tm_year = rtc->year - 1900;
	tm->tm_mon = rtc->month - 1;
	tm->tm_mday = rtc->date;
	tm->tm_hour = rtc->hour;
	tm->tm_min = rtc->min;
	tm->tm_sec = rtc->sec;
	return 0;
}

int tm_to_rtc(struct tm *tm, struct rtc_time_pmu *rtc)
{
	rtc->year = tm->tm_year + 1900;
	rtc->month = tm->tm_mon + 1;
	rtc->date = tm->tm_mday;
	rtc->day = tm->tm_wday;
	rtc->hour = tm->tm_hour;
	rtc->min = tm->tm_min;
	rtc->sec = tm->tm_sec;
	return 0;
}

int create_itimer(int rtc_fd, struct itimerspec *its, struct tm *tm, int ms,
		int type)
{
	struct rtc_time_pmu rtc_time;
	struct tm cur_tm;
	struct tm alarm_tm;
	time_t cur_time;
	time_t alarm_time;
	struct tm *tmp_tm;

	memset(&rtc_time,0x00,sizeof(struct rtc_time_pmu));
	if (alislrtc_ioctl(rtc_fd,  ALI_PMU_RTC_RD_VAL, &rtc_time) == -1)
		return -1;
	memset(&cur_tm, 0, sizeof(struct tm));
	rtc_to_tm(&rtc_time, &cur_tm);
	cur_time = mktime(&cur_tm);
//	SL_DBG("Current RTC: %s %s (%d)\n",
//			asctime(&cur_tm), ctime(&cur_time), cur_time);

	alarm_tm = cur_tm;

//	SL_DBG("type: %d\n", type);
//	SL_DBG("[RTC] %4d-%02d-%02d %02d:%02d:%02d yday:%d wday:%d,\n",
//			alarm_tm.tm_year + 1900, alarm_tm.tm_mon, alarm_tm.tm_mday,
//			alarm_tm.tm_hour, alarm_tm.tm_min, alarm_tm.tm_sec,
//			alarm_tm.tm_yday, alarm_tm.tm_wday);
	if (type == 0) { // min
		alarm_tm.tm_mon = tm->tm_mon;
		alarm_tm.tm_mday = tm->tm_mday;
		alarm_tm.tm_hour = tm->tm_hour;
		alarm_tm.tm_min = tm->tm_min;
		alarm_tm.tm_sec = 0;
		alarm_tm.tm_wday = 0;
		alarm_tm.tm_yday = 0;
	} else { // ms
		alarm_tm.tm_hour = tm->tm_hour;
		alarm_tm.tm_min = tm->tm_min;
		alarm_tm.tm_sec = tm->tm_sec;
	}
//	SL_DBG("[RTC-alarm] %4d-%02d-%02d %02d:%02d:%02d yday:%d wday:%d,\n",
//				alarm_tm.tm_year + 1900, alarm_tm.tm_mon, alarm_tm.tm_mday,
//				alarm_tm.tm_hour, alarm_tm.tm_min, alarm_tm.tm_sec,
//				alarm_tm.tm_yday, alarm_tm.tm_wday);
	alarm_time = mktime(&alarm_tm);
//	SL_DBG("alarm_time: %ld\n", alarm_time);
	if (alarm_time <= cur_time) {
		if (type == 0) {
			alarm_tm.tm_year += 1;
			alarm_time = mktime(&alarm_tm);
		} else {
			alarm_time += 24 * 3600;
			tmp_tm = gmtime(&alarm_time);
			alarm_tm = *tmp_tm;
		}
	}
//	SL_DBG("[alarm] %4d-%02d-%02d %02d:%02d:%02d yday:%d wday:%d,\n",
//			alarm_tm.tm_year + 1900, alarm_tm.tm_mon, alarm_tm.tm_mday,
//			alarm_tm.tm_hour, alarm_tm.tm_min, alarm_tm.tm_sec,
//			alarm_tm.tm_yday, alarm_tm.tm_wday);
//	SL_DBG("RTC alarm: %s %s (%d)\n", asctime(&alarm_tm), ctime(&alarm_time),
//			alarm_time);

	its->it_value.tv_sec = alarm_time - cur_time;
	its->it_value.tv_nsec = ms * 1000000;
	its->it_interval.tv_sec = 0;
	its->it_interval.tv_nsec = 0;
//	SL_DBG("Delta time: %lds %ldns\n", its->it_value.tv_sec,
//			its->it_value.tv_nsec);
	create_timerfd(its);
	return 0;
}

unsigned long alislrtc_setalarm(int rtc_fd, alisl_handle handle,
		struct alislrtc_timer *timer, int en)
{
	unsigned long err = 0;
	struct itimerspec *its;
	struct alislevent *slev;
	int tfd = -1;
	int mod = 0;

	if (handle == NULL || timer == NULL) {
		return -1;
	}
	slev = &timer->slev;
	its = &timer->its;
	// disable
	if (!en) {
		if (timer->state != 0) {
			if (slev->fd >= 0 && timer->state == 1) {
				err = alislevent_del(handle, slev); // del alarm
			}
			if (!err) {
				timer->state = 0;
			}
//			SL_DBG("RTC alarm disable %s.\n", err ? "fail" : "OK");
			return err;
		} else {
			SL_DBG("RTC alarm is already disable.\n");
			return err;
		}
	}

	// enable
	if (create_itimer(rtc_fd, its, &timer->tm, timer->ms, timer->type)) {
		return -1;
	}
	if (slev->fd <= 0) {
		tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
		if (tfd < 0){
			//SL_ERR("timerfd_create error");
			return -1;
		}
		slev->fd = tfd;
//		SL_DBG("timerfd_create ok, tfd: %d\n", tfd);
	} else {
		tfd = slev->fd;
		mod = 1;
	}
//	SL_DBG("timerfd_settime tfd: %d\n", tfd);
	if (timerfd_settime(tfd, TFD_TIMER_ABSTIME, its, NULL) != 0) {
		SL_DBG("%lds %ldns\n", its->it_value.tv_sec, its->it_value.tv_nsec);
		SL_DBG("%lds %ldns\n", its->it_interval.tv_sec,
				its->it_interval.tv_nsec);
		//SL_ERR("timerfd_settime error");
		if (!mod) {
			close(slev->fd);
			slev->fd = -1;
		}
		return -1;
	}
	slev->events = EPOLLIN | EPOLLET; // event

	if (mod) {
		err = alislevent_mod(handle, slev); // modify alarm
	} else {
		err = alislevent_add(handle, slev); // add alarm
	}
//	SL_DBG("alislrtc_setalarm err:%d mod:%d fd:%d en:%d\n",
//			err, mod, slev->fd, en);
	if (!err) {
		timer->config = 1;
		timer->state = 1;
	}
	return err;
}
