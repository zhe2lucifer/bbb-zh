/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislupginfokit.c
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
#include <alipltflog.h>

/* Upgrade header */
#include <alislupgrade.h>

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG
#define ALISLUPGKIT_DEBUG  printf
#else
#define ALISLUPGKIT_DEBUG
#endif

/*
 * Upgrade status store in flash 
 * This ugly structure is from an old usage
 * It'll be optimized later, maybe
 * It's defined and use by APP
 */
typedef struct upgstat {
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
} upgstat_t;


/* Status for normal boot */
static upgstat_t init_stat = {
	.boot_status        = ALISLUPG_RUN_OVER,
	.lowlevel_status    = ALISLUPG_RUN_OVER,
	.application_status = ALISLUPG_RUN_OVER,
};

/* Status for upgrade reboot */
static upgstat_t final_stat = {
	.bootloader_run_cnt  = 0,
	.lowlevel_run_cnt    = 0,
	.application_run_cnt = 0,
	.boot_status         = ALISLUPG_RUN_NONE,
	.lowlevel_status     = ALISLUPG_RUN_NONE,
	.application_status  = ALISLUPG_RUN_NONE,
	.lowlevel_upgrade    = ALISLUPG_UPG_OVER,
};

/* Status for none run - Mono upgrade */
static upgstat_t nonrun_stat = {
	.boot_status         = ALISLUPG_RUN_NONE,
	.lowlevel_status     = ALISLUPG_RUN_NONE,
	.application_status  = ALISLUPG_RUN_NONE,
	.bootloader_upgrade  = ALISLUPG_UPG_NONE,
	.lowlevel_upgrade    = ALISLUPG_UPG_NONE,
	.application_upgrade = ALISLUPG_UPG_NONE,
};

int main(int argc, char *argv[])
{
	unsigned char buf_rd[SYSINFO_SIZE_MAX];
	unsigned char buf_wr[SYSINFO_SIZE_MAX] = {0x00, 0x12, 0x85, 0x98, 0x96, 0x23};
	upgstat_t stat;

	/* upg info read */
	alislupg_upginfo_read((unsigned char*)&stat, sizeof(upgstat_t));
	/* upg info write */
	alislupg_upginfo_write((unsigned char*)&nonrun_stat, sizeof(upgstat_t));

	/* ext info read */
	alislupg_extinfo_read(buf_rd, SYSINFO_SIZE_MAX);
	/* ext info write */
	alislupg_extinfo_write(buf_wr, SYSINFO_SIZE_MAX);

	return 0;
}

