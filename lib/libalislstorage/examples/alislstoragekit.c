/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislstokit.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 14:41:13
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* System header */
#include <stdio.h>
#include <string.h>
#include <limits.h>

/* Storage header */
#include <alislstorage.h>

/* kernel headers */
#include <ali_mtd_common.h>

typedef struct stokit_map
{
	alislsto_idx_t  idx;
	char            *name;
} stokit_map_t;

typedef struct pmi_info_user pmi_info_t;

/* Internal header */

/* Mapping table */
static stokit_map_t nand_map[] =
{
	{STO_PMI,        "pmi"},
	{STO_PRIVATE,    "private"},
	{STO_KERNEL,     "kernel"},
	{STO_SEE,        "see"},
	{STO_ROOTFS,     "rootfs"},
	{STO_APPDATA,    "appdata"},
	{STO_USERFS,     "userfs"},
	{STO_RESERVED1,  ""},
	{STO_RESERVED2,  ""},
};

void testNewFunc()
{
	alislsto_partinfo_list_t *partition_list = NULL;
	int i;
	int j;
	alisl_handle handle;
	char buf_tmp[512] = {0};
	char name[512] = "bootlogo";
	int ret = 0;
	int offset = 0;

	printf("Enter testNewFunc\n");

	alislsto_get_partinfo(&partition_list);
	if (partition_list == NULL) {
		printf("1. [%s] fail!\n", __FUNCTION__);
		return;
	} else {
		printf("1. [%s] Got %d partitions.\n",
				__FUNCTION__, partition_list->count);
	}

	alislsto_get_partinfo(&partition_list);
	if (partition_list == NULL) {
		printf("2. [%s] fail!\n", __FUNCTION__);
		return;
	} else {
		printf("2. [%s] Got %d partitions.\n",
				__FUNCTION__, partition_list->count);
	}

	printf("3.[%s] read %d partitions \n",
			__FUNCTION__, partition_list->count);

	alislsto_partinfo_t *list = partition_list->list;
	for (i = 0; i < partition_list->count; i++) {
		printf("\n\nindex=%d [%s] size=%#lx\n",
				list[i].index, list[i].name, list[i].size);

		alislsto_mtd_open_by_name(&handle, list[i].name, O_RDONLY);
		if (handle != NULL) {
			alislsto_read(handle, buf_tmp, 512);
			printf("\n%s [%d]=============================\n", list[i].name, i);
			for (j = 0; j < 16; j++) {
				printf("%#x, ", buf_tmp[j]);
			}
			printf("\n%s [%d]=============================\n", list[i].name, i);
		} else {
			printf("%s alislsto_open_by_name fail!\n", list[i].name);
		}
		alislsto_mtd_close(handle);
	}

	printf("4.[%s] again read %d partitions \n",
				__FUNCTION__, partition_list->count);

	for (i = 0; i < partition_list->count; i++) {
		printf("index=%d [%s] size=%#lx\n",
				list[i].index, list[i].name, list[i].size);

		alislsto_mtd_open_by_name(&handle, list[i].name, O_RDONLY);
		if (handle != NULL) {
			alislsto_read(handle, buf_tmp, 512);
			printf("\n%s [%d]=============================\n", list[i].name, i);
			for (j = 0; j < 16; j++) {
				printf("%#x, ", buf_tmp[j]);
			}
			printf("\n%s [%d]=============================\n", list[i].name, i);
		} else {
			printf("%s alislsto_open_by_name fail!\n", list[i].name);
		}
		alislsto_mtd_close(handle);
	}

	printf("5.[%s] again read OK %d partitions \n",
					__FUNCTION__, partition_list->count);
	return;

	printf("write test\n");

	ret = alislsto_mtd_open_by_name(&handle, name, O_RDWR);
	if (ret == STO_ERR_NONE) {
		printf("\n%s [ERASE] ++++++++++++++++++++\n", name);
		ret =  alislsto_erase(handle, 0x200000, 2 * 1024 * 1024);

		offset = alislsto_lseek(handle, 0x200000, SEEK_SET);
		printf("\n%s [seek] offset=%#x++++++++++++++++++++\n", name, offset);

		alislsto_read(handle, buf_tmp, 512);
		printf("\n%s =============================\n", name);
		for (j = 0; j < 16; j++) {
			printf("%#x, ", buf_tmp[j]);
		}
		printf("\n%s =============================\n", name);

		offset = alislsto_lseek(handle, 0x200000, SEEK_SET);
		printf("\n%s [seek] offset=%#x++++++++++++++++++++\n", name, offset);

		printf("\n%s [WRITE] ++++++++++++++++++++\n", name);
		memset(buf_tmp, 0xA5, 512);
		ret =  alislsto_write(handle, buf_tmp, 128);
		memset(buf_tmp, 0xA6, 512);
		ret =  alislsto_write(handle, buf_tmp, 128);
		memset(buf_tmp, 0xA7, 512);
		ret =  alislsto_write(handle, buf_tmp, 128);
		memset(buf_tmp, 0xA8, 512);
		ret =  alislsto_write(handle, buf_tmp, 128);

		alislsto_read(handle, buf_tmp, 512);
		printf("\n%s =============================\n", name);
		for (j = 0; j < 16; j++) {
			printf("%#x, ", buf_tmp[j]);
		}
		printf("\n%s =============================\n", name);

		alislsto_lseek(handle, 0x200000, SEEK_SET);

		alislsto_read(handle, buf_tmp, 512);
		printf("\n%s =============================\n", name);
		for (j = 0; j < 16; j++) {
			printf("%#x, ", buf_tmp[j]);
		}
		printf("\n%s =============================\n", name);

	} else {
		printf("%s alislsto_open_by_name fail!\n", name);
	}
	alislsto_mtd_close(handle);
}

int main(int argc, char *argv[])
{
	alisl_handle dev;
	alisl_retcode ret;
	int i = 0, cnt = 0;
	alislsto_param_t stoparam;
	pmi_info_t *pmiinfo = NULL;
	unsigned char *buf = NULL;
	unsigned char *buf_tmp = NULL;
	int eccsz = 0;
	int shift = 0;
	off_t offset = 0;
	size_t size = 0;
	size_t imgsz = 0;
	const size_t rw_size = 0x100000;

	testNewFunc();

	printf("testNewFunc END!\n");
	return 0;

	/* alislsto_construct(dev); */
	alislsto_open(&dev, STO_TYPE_NAND, O_RDWR);

	/* alislsto_get_param */
	alislsto_get_param(dev, &stoparam);
	printf("STO param:\n");
	printf("type=%d\n", stoparam.type);
	printf("size=%d\n", stoparam.info->size);
	printf("blocksize=%d\n", stoparam.info->erasesize);
	printf("writesize=%d\n", stoparam.info->writesize);
	printf("maxpart=%d\n", stoparam.maxpart);

	pmiinfo = (pmi_info_t *)malloc(sizeof(pmi_info_t));

	while (cnt++ < 30)
	{
		/* alislsto_ioctl to get PMI */

		memset(pmiinfo->buf_start, 0x00, STO_HEADER_SIZE);
		pmiinfo->offset = 0;
		pmiinfo->length = STO_HEADER_SIZE;
		printf("[%s](%d)-----------1\n", __FUNCTION__, __LINE__);
		ret = alislsto_get_pmi(dev, pmiinfo);
		printf("[%s](%d)-----------2\n", __FUNCTION__, __LINE__);

		if (ret != 0)
		{
			printf("nand flash ioctl fail, ret = %d\n", ret);
			return 0;
		}

		buf = pmiinfo->buf_start;
		eccsz = *(unsigned long *)&buf[STO_ECCSIZE_POS];
		printf("eccsze=%d\n", eccsz);

		if (1024 == eccsz)
		{
			shift = 10;
		}
		else
		{
			shift = 9;
		}

		for (i = STO_KERNEL; i < STO_NANDMAX; i++)
		{
			offset = *(off_t *)(&buf[STO_IMAGE_BASEADDR_POS + (i - STO_KERNEL + 1) * 8]) << shift;
			imgsz = *(size_t *)(&buf[STO_IMAGELEN_BASEADDR_POS + (i - STO_KERNEL + 1) * 4]);
			size = *(size_t *)(&buf[STO_PARTLEN_BASEADDR_POS + (i - STO_KERNEL + 1) * 8]) << shift;
			printf("-----------------------------\n");
			printf("Offset.................0x%x\n", offset);
			printf("Size...................0x%x\n", size);
			printf("Image Length...........0x%x\n", imgsz);
			printf("-----------------------------\n");
		}

		printf("[%s](%d)-----------1\n", __FUNCTION__, __LINE__);

		if (buf[13] == 0x01)
		{
			buf[13] = 0x02;
		}
		else
		{
			buf[13] = 0x01;
		}

		ret = alislsto_set_pmi(dev, pmiinfo);
		printf("[%s](%d)-----------2\n", __FUNCTION__, __LINE__);

		if (ret != 0)
		{
			printf("nand flash ioctl fail, ret = %d\n", ret);
			return 0;
		}

		sleep(3);
	}

	free(pmiinfo);

#if 0
	/* alislsto_tell_ext */
	offset = alislsto_tell_ext(dev, STO_KERNEL, STO_PART_OFFSET);
	size = alislsto_tell_ext(dev, STO_KERNEL, STO_PART_SIZE);
	printf("kernel partition info: offse=0x%x, size=0x%x\n", offset, size);


	/* read/write/erase/seek */
	buf = (char *)malloc(rw_size);
	buf_tmp = (char *)malloc(rw_size);

	alislsto_lseek_ext(dev, STO_PRIVATE, offset);
	alislsto_read(dev, buf, rw_size);
	printf("read data: 0x%x 0x%x 0x%x 0x%x\n", buf[0], buf[1], buf[2], buf[3]);

	alislsto_lseek_ext(dev, STO_PRIVATE, offset);
	alislsto_erase(dev, offset, rw_size);
	alislsto_lseek_ext(dev, STO_PRIVATE, offset);
	alislsto_read(dev, buf_tmp, rw_size);
	printf("read data: 0x%x 0x%x 0x%x 0x%x\n", buf_tmp[0], buf_tmp[1], buf_tmp[2], buf_tmp[3]);

	alislsto_lseek_ext(dev, STO_PRIVATE, offset);
	alislsto_write(dev, buf, rw_size);
	alislsto_lseek_ext(dev, STO_PRIVATE, offset);
	alislsto_read(dev, buf_tmp, rw_size);
	printf("read data: 0x%x 0x%x 0x%x 0x%x\n", buf_tmp[0], buf_tmp[1], buf_tmp[2], buf_tmp[3]);
	free(buf_tmp);
	free(buf);
#endif

	alislsto_close(dev, false);
	/* alislsto_destruct(&dev); */

	return 0;
}
