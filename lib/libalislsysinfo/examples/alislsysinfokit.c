/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsysinfokit.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 14:49:07
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* System header */
#include <stdio.h>
#include <string.h>

#include <alislsysinfo.h>

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
	unsigned int cur_uboot;
	unsigned int reserved[4];
} upgstat_t;

int main(int argc, char *argv[])
{
	unsigned char buf_rd[SYSINFO_SIZE_MAX];
	unsigned char buf_wr[SYSINFO_SIZE_MAX] = {0x00, 0x12, 0x85, 0x98, 0x96, 0x23};
	upgstat_t stat;

	/* mac read */
	alislsysinfo_mac_read(buf_rd);
	/* mac write */
	alislsysinfo_mac_write(buf_wr);

	/* upg info read */
	alislsysinfo_upginfo_read((unsigned char *)&stat, sizeof(upgstat_t));
	/* upg info write */
	stat.nand_mono_upgrade = 10;
	stat.nor_reserved_upgrade = 15;
	alislsysinfo_upginfo_write((unsigned char *)&stat, sizeof(upgstat_t));

	/* ext info read */
	alislsysinfo_extinfo_read(buf_rd, SYSINFO_SIZE_MAX);
	/* ext info write */
	alislsysinfo_extinfo_write(buf_wr, SYSINFO_SIZE_MAX);

	return 0;
}
