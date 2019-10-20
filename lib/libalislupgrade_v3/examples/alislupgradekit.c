/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislupgkit.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:01:03
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <string.h>

/* share library headers */
#include <alipltfretcode.h>
#include <config.h>

/* Upgrade headers */
#include <alislupgrade.h>

/* kernel headers */
#include <ali_mtd_common.h>

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG
#define ALISLUPGKIT_DEBUG  printf
#else
#define ALISLUPGKIT_DEBUG
#endif

#define VERSION_UPDATE  "UPG3.0_20130822"

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
static const struct option longopts[] =
{
	{"source",  1,  NULL,  's'},
	{"file",    1,  NULL,  'f'},
	{"freq",    1,  NULL,  'F'},
	{"verbose", 0,  NULL,  'v'},
	{"version", 0,  NULL,  'V'},
	{"help",    0,  NULL,  '?'},
	{NULL,      0,  NULL,  0  },
};
#endif
#if HAVE_GETOPT || HAVE_GETOPT_LONG
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#define UPG_OPT_STRING  "s:f:F:vV?"

/*
 * Upgrade status store in flash
 * This ugly structure is from an old usage
 * It'll be optimized later, maybe
 * It's defined and use by APP
 */
typedef struct upgstat
{
	unsigned int boot_status;
	unsigned int lowlevel_status;
	unsigned int application_status;
	unsigned int bootloader_upgrade;
	unsigned int lowlevel_upgrade;
	unsigned int application_upgrade;

	unsigned int bootloader_run_cnt;
	unsigned int lowlevel_run_cnt;
	unsigned int application_run_cnt;

	unsigned int reserved1;
	unsigned int reserved2;

	unsigned int need_upgrade;
	unsigned int backup_exist;
	unsigned int lowlevel_backup_exist;
	unsigned int boot_backup_exist;
	unsigned int nor_upgrade;
	unsigned int nor_reserved;
	unsigned int nor_reserved_upgrade;
	unsigned int nand_reserved;
	unsigned int nand_reserved_upgrade;
	unsigned int nand_mono_upgrade;
	unsigned int cur_uboot;
	unsigned int reserved[4];
} upgstat_t;

/* Status for normal boot */
static upgstat_t init_stat =
{
	.boot_status        = ALISLUPG_RUN_OVER,
	.lowlevel_status    = ALISLUPG_RUN_OVER,
	.application_status = ALISLUPG_RUN_OVER,
};

/* Status for upgrade reboot */
static upgstat_t final_stat =
{
	.bootloader_run_cnt  = 0,
	.lowlevel_run_cnt    = 0,
	.application_run_cnt = 0,
	.boot_status         = ALISLUPG_RUN_NONE,
	.lowlevel_status     = ALISLUPG_RUN_NONE,
	.application_status  = ALISLUPG_RUN_NONE,
	.lowlevel_upgrade    = ALISLUPG_UPG_OVER,
};

/* Status for none run - Mono upgrade */
static upgstat_t nonrun_stat =
{
	.boot_status         = ALISLUPG_RUN_NONE,
	.lowlevel_status     = ALISLUPG_RUN_NONE,
	.application_status  = ALISLUPG_RUN_NONE,
	.bootloader_upgrade  = ALISLUPG_UPG_NONE,
	.lowlevel_upgrade    = ALISLUPG_UPG_NONE,
	.application_upgrade = ALISLUPG_UPG_NONE,
};


/* Internal header */

static void usage(char *program)
{
#if HAVE_GETOPT_LONG
	fprintf (stderr, "usage: %s [-sfFvV?] [--source=ota|usb|net] "
	         "[--file=usb_file_directory] "
	         "[--freq=frontend_freq ] "
	         "[--verbose] [--version] [--help]\n", program);
#else
	fprintf (stderr, "usage: %s [-sfFvV?]\n", program);
#endif
	fprintf (stderr, "upgrade kit for function test\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "  -s, --source         source of upgrade data[default=ota]\n");
	fprintf (stderr, "  -f, --file           file directory used for usb upgrade\n");
	fprintf (stderr, "  -F, --freq           frequency of frontend for upgrade\n");
	fprintf (stderr, "  -v, --verbose        display status information\n");
	fprintf (stderr, "  -V, --version        display upgkit version and exit\n");
	fprintf (stderr, "  -?, --help           display this help and exit\n");
}


static int parse(alislupg_param_t *param, int argc, char *argv[])
{
	int ret = -1;
	int c = 0;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
#if HAVE_GETOPT_LONG

	while ((c = getopt_long(argc, argv, UPG_OPT_STRING, longopts, NULL)) != -1)
#else
	while ((c = getopt(argc, argv, UPG_OPT_STRING)) != -1)
#endif
	{
		ret = 0;

		switch (c)
		{
			case 's':
				param->source = UPG_OTA;

				if (0 == strcmp("usb", optarg))
				{
					param->source = UPG_USB;
				}
				else if (0 == strcmp("net", optarg))
				{
					param->source = UPG_NET;
				}

				break;

			case 'f':
				param->config = strdup(optarg);
				break;

			case 'F':
				break;

			case 'v':
				return -1;

			case 'V':
				return -1;

			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}

#endif
	return ret;
}


static int upg_progress_callback(int percent)
{
	if (percent > 100)
		percent = 100;

	ALISLUPGKIT_DEBUG("[%s(%d)] Progress ........ %d\n", __FUNCTION__, __LINE__, percent);

	return 0;
}

static int upg_error_print(int error)
{
	if (ALISLUPG_ERR_NONE == error)
	{
		ALISLUPGKIT_DEBUG("Upg no error!\n");
	}
	else
	{
		ALISLUPGKIT_DEBUG("Upg error:%d!\n", error);
	}

	return 0;
}

/* test */
#define WIFI_CONNECT_TIMEOUT 20
#define SOURCE_URI_LENGTH	1024
#define WIFI_SSID_LENGTH	32
#define WIFI_PASSWD_LENGTH	128
#define CURL_CONNECT_TIMEOUT 10

typedef struct _NMPSYSTEM_UPG_DATA_
{
	char sourceURI[SOURCE_URI_LENGTH];
	char wifi_ssid[WIFI_SSID_LENGTH];
	char wifi_passwd[WIFI_PASSWD_LENGTH];
	int wifi_enc;
	int tv_format;
	int dirty;
} sNMPSYSTEM_UPG_DATA;

static int set_dirty_flag(bool flag)
{
	sNMPSYSTEM_UPG_DATA data;
	alislupg_extinfo_read((unsigned char*)&data, sizeof(sNMPSYSTEM_UPG_DATA));
	data.dirty = flag;
	alislupg_extinfo_write((unsigned char*)&data, sizeof(sNMPSYSTEM_UPG_DATA));
	ALISLUPGKIT_DEBUG("set_dirty_flag : %d\n", flag);
	return 0;
}
/*~test */

int main(int argc, char *argv[])
{
	ALISLUPGKIT_DEBUG("Alislupgradekit Start\n");
	ALISLUPGKIT_DEBUG("[%s %s ] %s\n", __DATE__, __TIME__, VERSION_UPDATE);

	int ret = ALISLUPG_ERR_NONE;
	alisl_handle desc;
	alislupg_param_t *param;
	int percent, error;

	//config upg param
	param = malloc(sizeof(alislupg_param_t));
	memset(param, 0x00, sizeof(alislupg_param_t));
	ret = parse(param, argc, argv);

	if (ret < 0)
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	param->progress_scale = 1;

	if (ALISLUPG_ERR_NONE != (ret = alislupg_construct(&desc)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	if (ALISLUPG_ERR_NONE != (ret = alislupg_open(desc, param)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	if (UPG_OTA == param->source) /* OTA need download data first */
	{
		if (ALISLUPG_ERR_NONE != (ret = alislupg_prestart(desc)))
		{
			ALISLUPGKIT_DEBUG("error code: %d\n", ret);
			goto over;
		}

		/* progress update */
		percent = 0;
		error = ALISLUPG_ERR_NONE;

		while ((percent < 100) && (ALISLUPG_ERR_NONE == error))
		{
			if (ALISLUPG_ERR_NONE != (ret = alislupg_wait_status(desc, &percent, &error)))
			{
				ALISLUPGKIT_DEBUG("error code: %d\n", ret);
				goto over;
			}

			upg_progress_callback(percent);
		}
	}
    

	if (ALISLUPG_ERR_NONE != (ret = alislupg_start(desc)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	/* progress update */
	percent = 0;
	error = ALISLUPG_ERR_NONE;

	while ((percent < 100) && (ALISLUPG_ERR_NONE == error))
	{
		if (ALISLUPG_ERR_NONE != (ret = alislupg_wait_status(desc, &percent, &error)))
		{
			ALISLUPGKIT_DEBUG("error code: %d\n", ret);
			goto over;
		}

		upg_progress_callback(percent);
	}

	if (ALISLUPG_ERR_NONE != (ret = alislupg_stop(desc)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	if (ALISLUPG_ERR_NONE != (ret = alislupg_close(desc)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	if (ALISLUPG_ERR_NONE != (ret = alislupg_destruct(&desc)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}

	/* To be simpler, we update the status here */
	/* if (ALISLUPG_ERR_NONE != (ret = alislupg_upginfo_write(&nonrun_stat, sizeof(upgstat_t))))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	} */

	if (ALISLUPG_ERR_NONE == error)
	{
		ALISLUPGKIT_DEBUG("Successful to run a upgrade test %s\n", argv[1]);
		set_dirty_flag(0);
	}
	else
	{
		ALISLUPGKIT_DEBUG("Fail to run a upgrade test %s\n", argv[1]);
		upg_error_print(error);
	}

over:
	ALISLUPGKIT_DEBUG("[%s (%d)] code:%d\n", __FUNCTION__, __LINE__, ret);
	free(param);
	return ret;
}
