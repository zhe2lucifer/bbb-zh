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

#define UPGVERSION  "UPG4.0"

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
#if HAVE_GETOPT_LONG || HAVE_GETOPT
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
#endif

static int parse(alislupg_param_t *param, int argc, char *argv[])
{
	int ret = -1;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
int c = 0;
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
				// Fixed cpptest bug. If there have 2 'f'.
				if (param->config == NULL) {
					param->config = strdup(optarg);
				}
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

int main(int argc, char *argv[])
{
	ALISLUPGKIT_DEBUG("Alislupgradekit Start\n");
	ALISLUPGKIT_DEBUG("[%s %s ] %s\n", __DATE__, __TIME__, UPGVERSION);

	int ret = ALISLUPG_ERR_NONE;
	alisl_handle desc;
	alislupg_param_t *param;
	int percent, error;
	char part_name[MAX_PART][MAX_PART_NAME_LEN];
	uint8_t part_cnt = 0;
	uint8_t i = 0;
	alislupg_header_t upg_info;
	alislupg_ota_dl_param_t ota_dl_param;
	
	//config upg param
	param = malloc(sizeof(alislupg_param_t));
	if (param == NULL) {
		ALISLUPGKIT_DEBUG("malloc error\n");
		return -1;
	}
	memset(param, 0x00, sizeof(alislupg_param_t));
	ret = parse(param, argc, argv);
	if (ret < 0)
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}
	param->method = PIECE_BY_PIECE;//ONE_SHOT;//
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

	if(UPG_OTA == param->source)
	{
		memset(&ota_dl_param, 0, sizeof(alislupg_ota_dl_param_t));
		ota_dl_param.oui        = 0x090E6;
		ota_dl_param.hw_model   = 0x3516;
		ota_dl_param.hw_version = 0x0000;
		ota_dl_param.sw_model   = 0x0000;
		ota_dl_param.sw_version = 0x0000;
		if (ALISLUPG_ERR_NONE != alislupg_set_ota_dl_param(desc, ota_dl_param))
		{
			ALISLUPGKIT_DEBUG("error code: %d\n", ret);
			goto over;
		}
	}

	memset(&upg_info, 0, sizeof(alislupg_header_t));
	if (ALISLUPG_ERR_NONE != (ret = alislupg_get_upgrade_info(desc, &upg_info)))
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
		goto over;
	}
	ALISLUPGKIT_DEBUG("[upg info] package_ver=0x%x, part_cnt=%d\n", upg_info.package_ver, upg_info.part_cnt);
	
	/**
	 *  OneShot: download all the data to DRAM first 
	 */
	if (param->method == ONE_SHOT)
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
    
	/* get upgrade partitions [option]*/
	ret = alislupg_get_upgrade_parts(desc, part_name, &part_cnt);
	if (ALISLUPG_ERR_NONE != ret)
	{
		ALISLUPGKIT_DEBUG("error code: %d\n", ret);
				goto over;
	}
	ALISLUPGKIT_DEBUG("upgrade counts : %d.\n", part_cnt);
	ALISLUPGKIT_DEBUG("partition name :\n");
	for (i = 0; i < part_cnt; i++)
	{
		ALISLUPGKIT_DEBUG("%s.\n", part_name[i]);
	}
	#if 0//just open this for pingpong solution
	/*****************************************************************
	* re-specify the upg partitions[optional,for pingpong solution]
	* you can change the part_name to select upgrade which mtd part, 
	* for example: kernel_1 -> kernel_2, 
	* but you can't delete/add new part name
	*****************************************************************/
	alislupg_set_upgrade_parts(desc, part_name, part_cnt);	
	ALISLUPGKIT_DEBUG("partition name :\n");
	for (i = 0; i < part_cnt; i++)
	{
		ALISLUPGKIT_DEBUG("%s.\n", part_name[i]);
	}
	#endif

	/**
	 *  OneShot:      get date from DRAM(prepared by alislupg_prestart) and burn to flash
	 *  PieceByPiece: get data from source(NET/USB/OTA) and burn to flash 
	 */
	ret = alislupg_start(desc);
	if (ret != ALISLUPG_ERR_NONE)
	{
		ALISLUPGKIT_DEBUG("LINE %d, start post fail.\n", __LINE__);
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

	if (param->method == ONE_SHOT)
	{
		alislupg_prestop(desc);
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

	if (ALISLUPG_ERR_NONE == error)
	{
		ALISLUPGKIT_DEBUG("Successful to run a upgrade test %s, mode = %s\n", argv[1],\
			((param->method == ONE_SHOT) ? ("ONE_SHOT") : ("PIECE_BY_PIECE")));
	}
	else
	{
		ALISLUPGKIT_DEBUG("Fail to run a upgrade test %s, mode = %s\n", argv[1],\
			((param->method == ONE_SHOT) ? ("ONE_SHOT") : ("PIECE_BY_PIECE")));
		upg_error_print(error);
	}

over:
	ALISLUPGKIT_DEBUG("[%s (%d)] code:%d\n", __FUNCTION__, __LINE__, ret);
	if (param->config)
		free(param->config);
	free(param);
	return ret;
}
