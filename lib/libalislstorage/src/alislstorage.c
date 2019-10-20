/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislstorage.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 14:44:42
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#include <stdlib.h>
#include <stdio.h>

#include <alipltflog.h>
#include <alislstorage.h>

/* kernel headers */
#include <ali_mtd_common.h>

/* Internal header */
#include "internal.h"

#define SECTOR_SIZE_4K  4096  // 4k 

#undef ENABLE_DEBUG

/* Mapping table */
static storage_map_t nand_map[] =
{
	{STO_PMI,        STO_DEFAULT_OFFSET, O_RDWR, "PMI"},
	{STO_PRIVATE,    STO_DEFAULT_OFFSET, O_RDWR, "PRIVATE"},
	{STO_KERNEL,     STO_DEFAULT_OFFSET, O_RDWR, "Kernel"},
	{STO_SEE,        STO_DEFAULT_OFFSET, O_RDWR, "SEE"},
	{STO_ROOTFS,     STO_DEFAULT_OFFSET, O_RDWR, "RootFS"},
	{STO_APPDATA,    STO_DEFAULT_OFFSET, O_RDWR, "AppData"},
	{STO_USERFS,     STO_DEFAULT_OFFSET, O_RDWR, "UserFS"},
	{STO_RESERVED1,  STO_DEFAULT_OFFSET, O_RDWR},
	{STO_RESERVED2,  STO_DEFAULT_OFFSET, O_RDWR},
	{STO_RESERVED3,  STO_DEFAULT_OFFSET, O_RDWR},
	{STO_RESERVED4,  STO_DEFAULT_OFFSET, O_RDWR},
	{STO_RESERVED5,  STO_DEFAULT_OFFSET, O_RDWR},
	{STO_UBOOT,      STO_DEFAULT_OFFSET, O_RDWR},
	{STO_UBOOT_BAK,  STO_DEFAULT_OFFSET, O_RDWR},
};

static storage_map_t nor_map[] =
{
	{STO_BOOTLOADER, STO_DEFAULT_OFFSET, O_RDWR, "BootLoader"},
};

static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
static alislsto_dev_t *nor_handle = NULL;
static alislsto_dev_t *nand_handle = NULL;

alislsto_partinfo_list_t partinfo_list = {0};

typedef struct alislsto_handle_list alislsto_handle_list_t;
struct alislsto_handle_list {
	int count;
	alislsto_dev_t **list;
};

alislsto_handle_list_t handle_list = {0};

static alisl_retcode alislsto_destruct(alisl_handle *handle);
static alisl_retcode alislsto_construct(alisl_handle *handle);

#if ENABLE_DEBUG
static char *storage_err[] = {
	"No error happen",
	"Can't open required device",
	"Can't find flash layout mapping",
	"Invalid device handle",
	"Can't seek to required position",
	"Seek over flow",
};
#endif

static char* find_last_mtd_dev(const char *head,
		const char *tail, const char *key)
{
	char *pos;
	unsigned int len;
	if (tail == NULL || key == NULL)
	{
		return NULL;
	}

	len = strlen(key);
	pos = (char *)(tail - len);
	while (strncmp(pos, key, len) != 0 &&
			(pos - head >= 0))
	{
		pos--;
	}

	if (pos - head < 0)
	{
		return NULL;
	}

	return pos;
}

/*
 * Get the mtd name and check if it is the one we need. 
 * buffer@in: looks like 'see" mtd4: 0x10000 0x10000 "seebl" '
 * mtd_name@in: the mtd name we need
 */
static char * find_dev_name_str(const char *buffer,
		const char *mtd_name)
{
    char *start = NULL, *end = NULL, *tmp = NULL;
    unsigned int len;
    //char name[32];

    tmp = (char *)buffer;
    while (*tmp) {
        start = strstr(tmp, mtd_name);        
        if (NULL == start)
            return NULL;
        if (*(start - 1) != '\"') {
            //printf("attention!!! not a valid name.\n");
            tmp = start + 1;
            continue;
        }
        end = strchr(start, '\"');    
        if (NULL == end)
            return NULL;
        //printf("%s, start: %s\n", __func__, start);
        
        len = end - start;
        //strncpy(name, start, len);
        //printf("%s, name: %s, mtd_name: %s\n", __func__, name, mtd_name);
        //if (len > strlen(mtd_name))
        //    len = strlen(mtd_name);
        if (0 == memcmp(mtd_name, start, len))
            return start;
        tmp = end;
    }
    return NULL;
}


/*
 * In order to open mtd device according to mtd name
 *
 * output format of /dev/mtd:
 * device: size erasesize name
 */
static alisl_retcode find_mtd_dev_num(const char *mtd_name,
		char *mtd_dev, unsigned int max_len)
{
	char buffer[1024];
	unsigned int len = -1;
	int size = -1;
	char *start = NULL, *end = NULL;
	int fd = -1;

	if (mtd_name == NULL)
	{
		SL_ERR("Invalid input param!\n");
		return STO_ERR_INVALIDPARAM;
	}

	memset(buffer, 0x0, sizeof(buffer));
	fd = open("/proc/mtd", O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		SL_ERR("Can not open file!\n");
		return STO_ERR_DRIVER;
	}

	size = read(fd, buffer, sizeof(buffer) - 1);
	if (size < 0)
	{
		SL_ERR("Can not read file!\n");
		goto err;
	}

	//start = strstr(buffer, mtd_name);
	start = find_dev_name_str(buffer, mtd_name);
	//if (start == NULL)
	//{
	//	SL_ERR("Can not find mtd name!\n");
	//	goto err;
	//}

	start = find_last_mtd_dev(buffer, start, "mtd");
	if (start == NULL)
	{
		SL_ERR("Can not find mtd device %s!\n", mtd_name);
		goto err;
	}

	end = strchr(start, ':');
	if (end == NULL)
	{
		SL_ERR("Can not find mtd device!\n");
		goto err;
	}

	len = end - start;
	if (len >= max_len)
	{
		SL_ERR("MTD device string over max length!\n");
		len = max_len - 1;
	}

	strncpy(mtd_dev, start, len);
	mtd_dev[len] = '\0';
	SL_DBG("Find MTD device: %s\n", mtd_dev);

	close(fd);
	return STO_ERR_NONE;

err:
	close(fd);
	return STO_ERR_DRIVER;
}

static void bitarray_set_index(char *bitarray, size_t idx)
{
	if (bitarray == NULL) {
		return;
	}
	bitarray[idx / 8] |= (1 << (idx % 8));
}

//static void bitarray_clear_index(unsigned char *bitarray, size_t idx)
//{
//	if (bitarray == NULL) {
//		return;
//	}
//	bitarray[idx / 8] &= (~(1 << (idx % 8)));
//}

static int bitarray_get_index(char *bitarray, size_t idx)
{
	if (bitarray == NULL) {
		return -1;
	}
	return bitarray[idx / 8] & (1 << (idx % 8));
}

// suppose:
// block size is 0x200000, flash size is 0x800000
//
// case 1: block 0 is bad block
// offset_logic: 0x000000, offset_phy should be 0x200000
//
// case 2: block 1 is bad block
// offset_logic: 0x200000, offset_phy should be 0x400000
//
// case 3: block 2 is bad block
// offset_logic: 0x200000, offset_phy should be 0x200000
//
// case 4: block 2 is bad block
// offset_logic: 0x400000, offset_phy should be 0x600000
//
// case 5: block 2 is bad block
// offset_logic: 0x600000, return error
//
// case 6: block 1 and block 2 is bad block
// offset_logic: 0x200000, offset_phy should be 0x600000

alisl_retcode alislsto_get_phy_addr(alisl_handle handle,
		loff_t offset_logic,
		loff_t *offset_phy)
{
	unsigned long block_all = 0;
	unsigned long block_logic = 0;
	unsigned long block_idx = 0;
	unsigned long cnt = 0;
	loff_t addr = 0;/*for bug #56342, MUST use loff_t instead of off_t.*/

	alislsto_dev_t *dev = (alislsto_dev_t *)handle;
	if (dev == NULL || offset_phy == NULL || dev->info == NULL
			|| offset_logic > dev->info->size) {
		printf("[%s](%d) Input error\n", __FUNCTION__, __LINE__);
		return -1;
	}
	if (!dev->bad_cnt  // have no bad block
        || dev->info->type != MTD_NANDFLASH) { // is nor flash
		*offset_phy = offset_logic;
		return 0;
	}

	// Scan the bad block map, counts the blocks,
	// until encounter certain number good blocks.
	block_all = dev->info->size / dev->info->erasesize;
	block_logic = offset_logic / dev->info->erasesize;
	for (cnt = 0, block_idx = 0; cnt < block_all; cnt++) {
		// 1st. Must make sure the block is a good one.
		if (!bitarray_get_index(dev->bad_map, (size_t)cnt)) {
			// 2nd. To check whether it's the target.
			if (block_idx == block_logic) {
				break;
			}
			// 3rd. Increases good block counter by one.
			block_idx++;
		}
	}
	if (block_idx == block_all) {
		printf("[%s](%d) Logic addr is out of rang\n", __FUNCTION__, __LINE__);
		return -1;
	}
	addr = cnt * dev->info->erasesize;
	addr += offset_logic % dev->info->erasesize;
	*offset_phy = addr;
	return 0;
}

// suppose:
// block size is 0x200000, flash size is 0x800000
//
// case 1: block 0 is bad block
// offset_phy: 0x200000, offset_logic should be 0x000000
//
// case 2: block 1 is bad block
// offset_phy: 0x400000, offset_logic should be 0x200000
//
// case 3: block 2 is bad block
// offset_phy: 0x200000, offset_logic should be 0x200000
//
// case 4: block 2 is bad block
// offset_phy: 0x400000, offset_logic should be 0x400000
//
// case 5: block 2 is bad block
// offset_phy: 0x600000, offset_logic should be 0x400000
//
// case 6: block 1 and block 2 is bad block
// offset_phy: 0x600000, offset_logic should be 0x200000

alisl_retcode alislsto_get_logic_addr(alisl_handle handle,
		loff_t offset_phy,
		loff_t *offset_logic)
{
//	unsigned long block_all = 0;
	unsigned long block_phy = 0;
	unsigned long block_idx = 0;
	unsigned long cnt = 0;
	loff_t addr = 0;/*for bug #56342, MUST use loff_t instead of off_t.*/

	alislsto_dev_t *dev = (alislsto_dev_t *)handle;
	if (dev == NULL || offset_logic == NULL || dev->info == NULL
			|| offset_phy > dev->info->size) {
		printf("[%s](%d) Input error\n", __FUNCTION__, __LINE__);
		return -1;
	}
	if (!dev->bad_cnt  // has no bad block
        || dev->info->type != MTD_NANDFLASH) { // is nor flash
		*offset_logic = offset_phy;
		return 0;
	}

	// Scan the bad block map, counts the good block number,
	// until certain blocks.
	//block_all = dev->info->size / dev->info->erasesize;
	block_phy = offset_phy / dev->info->erasesize;
	for (cnt = 0, block_idx = 0; cnt < block_phy; cnt++) {
		if (!bitarray_get_index(dev->bad_map, (size_t)cnt)) {
			block_idx++;
		}
	}

	addr = block_idx * dev->info->erasesize;
	addr += offset_phy % dev->info->erasesize;
	*offset_logic = addr;
	return 0;
}

static void _mark_bad_block_for_ci()
{
#if 0
    // modify by fawn@20171222, do not need to mark bad block here, but via FTool
    if (access("/tmp/ali_nestor_test", F_OK)) {
        return;
    } 
    if(-1 == system("rm -rf /tmp/ali_nestor_test"))
    {
        SL_ERR("rm file error\n");
    }
    
    SL_ERR("---for AUI CI test, mark bad block\n");
    
    alislsto_partinfo_list_t *list = NULL;
	int i = 0, j = 0, bad = 0;
    unsigned long block_cnt = 0;
    int fd = 0;
    mtd_info_t info;
    loff_t offset = 0;
#define _PATH_SIZE 64
    char mtd_dev_path[_PATH_SIZE] = {0};
#ifdef USE_MTD_BLOCK
    const char mtd_path[] = "/dev/mtdblock%d";
#else
    const char mtd_path[] = "/dev/mtd%d";
#endif

	if (!partinfo_list.count) {
		alislsto_get_partinfo(&list);
	}

    SL_ERR("MTD total count: %d\n", partinfo_list.count);
	for (i = 0; i < partinfo_list.count; i++) {
        // open the nand device
        snprintf(mtd_dev_path, _PATH_SIZE, mtd_path, partinfo_list.list[i].index);
        fd = open(mtd_dev_path, O_RDWR);            
        if (fd < 0) {
            SL_ERR("open %s fail.\n", mtd_dev_path);
            continue;
        } else {
            // get information
            if (0 == ioctl(fd, MEMGETINFO, &info)) {
                // only for nand flash
                if (info.type == MTD_NANDFLASH) {       
                    // check wether there are bad blocks
                    block_cnt = info.size / info.erasesize;
                    bad = 0;
                    for (offset = 0, block_cnt = 0;
                        offset < info.size;
                        offset += info.erasesize, block_cnt++) {
                        int ret = ioctl(fd, MEMGETBADBLOCK, &offset);
                        //printf("check at: 0x%x, ret: %d.\n", offset, ret);
                        if (ret) {
                            SL_ERR("%s block 0x%x is bad block\n", \
                                    mtd_dev_path, (unsigned int)offset);
                            bad++;
                        }
                    }
                    SL_ERR("%s has bad block count: %ld\n", mtd_dev_path, bad);    

                    // do not have any bad block, mark it for ci test
                    if (bad < 1) {
                        SL_ERR("bad: %ld.\n", bad);
                        loff_t to_be_bad[3] = {
                                0, //first block
                                (block_cnt/2) * info.erasesize, // middle
                                info.size - info.erasesize }; // last block
                        if (i == 0) {
                            if ( !strcmp("aliboot", partinfo_list.list[i].name) 
                                 || !strcmp("boot_total_area", partinfo_list.list[i].name)
                                 || !strcmp("advanced_secure_boot", partinfo_list.list[i].name)) {
                                // nand boot, do not mark any bad block, 
                                // otherwise the bootrom can not load correct aliboot
                                SL_ERR("find nand boot part0: %s, do not mark bad block.\n", partinfo_list.list[i].name);
                                continue;
                            } else {
                                // the first block on nand never be bad
                                to_be_bad[0] = (loff_t)(info.erasesize); // second block
                            }
                        }
                        // for kernel parition, mark 3 bad block, other partitions 2 bad block
                        if (!strcmp("kernel", partinfo_list.list[i].name)) 
                        	bad = 3;
                        else 
                            bad = 2;
                        
                        for (j = 0; j < bad; j++) {
                            int ret = ioctl(fd, MEMGETBADBLOCK, &to_be_bad[j]);
                            SL_ERR("check at: 0x%x, ret: %d.\n", (unsigned int)to_be_bad[j], ret);
                            if (!ret) {
                                SL_ERR("for AUI CI test, mark a bad block at: 0x%x.\n", (unsigned int)to_be_bad[j]);
                                ioctl(fd, MEMSETBADBLOCK, &to_be_bad[j]);
                            }
                        }
                    }
                }
            }                
            
            close(fd);

        }
	}
    SL_ERR("<--- leave for AUI CI test, mark bad block\n");
#endif
}

static void update_bbt(alislsto_dev_t* dev, loff_t addr)
{
    /* set bbt if erase fail */
    if (ioctl(dev->handle, MEMSETBADBLOCK, &addr)) {
        SL_ERR("%s Set BBT failed!, start = 0x%llx\n",
                __FUNCTION__, addr);
    } else {
        // update dev->bad_map
        if (!bitarray_get_index(dev->bad_map, (size_t)(addr/dev->info->erasesize))) {
            bitarray_set_index(dev->bad_map, (size_t)(addr/dev->info->erasesize));
            dev->bad_cnt += 1;
            dev->info_logic->size -= dev->info->erasesize; 
        }           
    }
}
static alisl_retcode show_flash_info(alisl_handle handle)
{
	alislsto_dev_t *dev = (alislsto_dev_t *)handle;
	loff_t offset = 0;
	unsigned long block_cnt = 0;
	unsigned long bad_block_cnt = 0;
	int ret = 0;
	int mtd_handle = 0;
	int bad_map_size = 0;

#ifdef USE_MTD_BLOCK
	char *mtd_name;
	int name_size = 0;
	char *tmp;
	int i;
#endif

	if (dev == NULL)
	{
		SL_ERR("Try to use NULL handle!\n");
		goto param_err;
	}

	dev->info = malloc(sizeof(mtd_info_t));

	if (dev->info == NULL)
	{
		SL_ERR("Malloc memory failed!\n");
		goto memory_err;
	}

	memset(dev->info, 0, sizeof(mtd_info_t));

	dev->info_logic = malloc(sizeof(mtd_info_t));
	if (dev->info_logic == NULL) {
		SL_ERR("Malloc memory failed!\n");
		free(dev->info);
		goto memory_err;
	}
	memset(dev->info_logic, 0, sizeof(mtd_info_t));

#ifdef USE_MTD_BLOCK
	tmp = strstr(dev->dev_name, "block");
	if (tmp != NULL) {
		name_size = strlen(dev->dev_name);
		mtd_name = malloc(name_size + 1);
		if (mtd_name == NULL)
		{
			SL_ERR("Malloc memory failed!\n");
			free(dev->info);
			free(info_logic);
			goto memory_err;
		}
		memset(mtd_name, 0, name_size + 1);
		i = tmp - dev->dev_name;
		strncpy(mtd_name, dev->dev_name, i);
		strcat(mtd_name + i, dev->dev_name + i + sizeof("block") - 1);
		mtd_handle = open(mtd_name, O_RDWR);
	}
#endif

	mtd_handle = dev->handle;
	ret = ioctl(mtd_handle, MEMGETINFO, dev->info);

	if (ret != 0)
	{
		SL_ERR("Get flash info failed!, ret:%d\n", ret);
		goto driver_err;
	}
	memcpy(dev->info_logic, dev->info, sizeof(mtd_info_t));

	SL_DBG("In %s(%d) NVRAM information %s\n", \
	               __FUNCTION__, __LINE__, dev->dev_name);
	SL_DBG("Type............%d\n", dev->info->type);
	SL_DBG("Flag............%d\n", dev->info->flags);
	/* compatible with int64 */
//SL_DBG will do nothing if disable DEBUG in Mini build,thus the warning that 'size set but not used' occur
#ifdef ENABLE_DEBUG
	loff_t size = 0;
	size = dev->info->size;
	SL_DBG("Size............0x%llx\n", size);
	SL_DBG("Block Size......0x%x\n", dev->info->erasesize);
	SL_DBG("Write Size......0x%x\n", dev->info->writesize);
#endif
	// bad block scan
	if (dev->info->type == MTD_NANDFLASH) {
		block_cnt = dev->info->size / dev->info->erasesize;
		bad_map_size = (block_cnt / 8) + (block_cnt % 8 ? 1 : 0);
		dev->bad_map = malloc(bad_map_size);
		if (dev->bad_map == NULL) {
			printf("[%s](%d) bad_map malloc fail\n", __FUNCTION__, __LINE__);
			goto driver_err;
		}
		memset(dev->bad_map, 0, bad_map_size);

        // --> add for #80670, for ci test on LINUX, mark some bad blcok.
        _mark_bad_block_for_ci();
        
		for (offset = 0, block_cnt = 0;
				offset < dev->info->size;
				offset += dev->info->erasesize, block_cnt++) {
			if (ioctl(dev->handle, MEMGETBADBLOCK, &offset)) {
				printf("%s block %ld is bad block\n", \
						dev->dev_name, block_cnt);
				bitarray_set_index(dev->bad_map, (size_t)block_cnt);
				bad_block_cnt++;
			}
		}
		if (bad_block_cnt) {
			dev->info_logic->size -= bad_block_cnt * dev->info->erasesize;
			dev->bad_cnt = bad_block_cnt;
			printf("%s has %ld bad blocks\n", dev->dev_name, bad_block_cnt);
		}
	}

#ifdef USE_MTD_BLOCK
	if (mtd_handle != dev->handle) {
		close(mtd_handle);
	}
#endif

	return STO_ERR_NONE;
param_err:
	return STO_ERR_INVALIDPARAM;
memory_err:
	return STO_ERR_OVERFLOW;
driver_err:
	free(dev->info);
	dev->info = NULL;
	free(dev->info_logic);
	dev->info_logic = NULL;
	return STO_ERR_DRIVER;
}
/**
 *  Function Name:  alislsto_construct
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode alislsto_construct(alisl_handle *handle)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv = NULL;

	//if (handle == NULL)
	//{
	//	SL_ERR("Invalid handle!\n");
	//	return STO_ERR_INVALIDHANDLE;
	//}

	dev = malloc(sizeof(*dev));

	if (dev == NULL)
	{
		SL_ERR("Malloc memory failed!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	priv = malloc(sizeof(storage_priv_t));
	if (NULL == priv) {
	    free(dev);
	    dev = NULL;
	    SL_ERR("Malloc memory failed!\n");
        return STO_ERR_INVALIDHANDLE;
	}
	memset(priv, 0, sizeof(*priv));
	memset(dev, 0, sizeof(*dev));
	*handle = dev;

	dev->handle = STO_INVALID_HANDLE;
	pthread_mutex_init(&priv->rdmutex, NULL);
	pthread_mutex_init(&priv->wrmutex, NULL);
	pthread_mutex_init(&priv->seekmutex, NULL);
	pthread_mutex_init(&priv->devlock, NULL);
	dev->priv = priv;

	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_get_offset
 *  @brief          offset of upgrade writter and system info share the
 *                  same variable. so this variable should be saved before
 *                  read/write system info, and restored after read/write
 *                  system info. otherwise offset of upgrade writter will be
 *                  confuse. remember that alislsto_get_offset and
 *                  alislsto_set_offset should be used in pairs.
 *
 *  @param          handle  point to descriptor of device
 *  @param          offset
 *
 *  @return         alisl_retcode
 *
 *  @author         demon.yan <demon.yan@alitech.com>
 *  @date           2013-11-18
 *
 */
alisl_retcode alislsto_get_offset(alisl_handle handle, loff_t *offset)
{
	alislsto_dev_t *dev;
//	storage_priv_t *priv;

	if (offset == NULL || handle == NULL)
	{
		SL_ERR("Invalid input param!\n");
		return STO_ERR_INVALIDPARAM;
	}

	dev = (alislsto_dev_t *)handle;
//	priv = (storage_priv_t *)dev->priv;

	//pthread_mutex_lock(&priv->seekmutex);
	*offset = dev->offset;
	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_set_offset
 *  @brief          remember that alislsto_get_offset and
 *                  alislsto_set_offset should be used in pairs.
 *
 *  @param          handle  point to descriptor of device
 *  @param          offset
 *  @param          flag set offset or not(true:set, false:not set)
 *
 *  @return         alisl_retcode
 *
 *  @author         demon.yan <demon.yan@alitech.com>
 *  @date           2013-11-18
 *
 */
alisl_retcode alislsto_set_offset(alisl_handle handle, loff_t offset, bool flag)
{
	alislsto_dev_t *dev;
	//storage_priv_t *priv;

	if (handle == NULL)
	{
		SL_ERR("Invalid handle!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	dev = (alislsto_dev_t *)handle;
//	priv = (storage_priv_t *)dev->priv;

	if (flag)
		dev->offset = offset;

	//pthread_mutex_unlock(&priv->seekmutex);
	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_lock_read
 *  @brief          This function lock seekmutex when reading.
 *
 *  @param          para alislsto_rw_param_t some para
 *  @param          buf read buffer
 *  @param          actual_size read actual size
 *
 *  @return         alisl_retcode
 *
 *  @author         oscar.shi <oscar.shi@alitech.com>
 *  @date           2014-08-06
 *
 */
alisl_retcode alislsto_lock_read(alislsto_rw_param_t para,
		unsigned char *buf, size_t *actual_size)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;
	alisl_retcode err = STO_ERR_NONE;
	loff_t orig_offset = 0;/*for bug #56342, MUST use loff_t instead of off_t.*/
	int cur_offset = 0;

//	printf("[size test]\n");
//	printf("size_t size=%d\n",sizeof(size_t));
//	printf("off_t size=%d\n",sizeof(off_t));
//	printf("loff_t size=%d\n",sizeof(loff_t));
//	printf("int size=%d\n",sizeof(int));
//	printf("long size=%d\n",sizeof(long));
//	printf("long long size=%d\n",sizeof(long long));

#ifdef ENABLE_DEBUG
	SL_DBG("[%s %d]\n", __FUNCTION__, __LINE__);
	SL_DBG("handle=%#x\n", para.handle);
	SL_DBG("buf=%#x\n", &buf);
	SL_DBG("size=%#x\n", para.size);
	SL_DBG("offset=%#llx\n", para.offset);
	SL_DBG("whenence=%d\n", para.whenence);
	SL_DBG("idx=%d\n", para.idx);
	SL_DBG("read=%#x\n", *actual_size);
	SL_DBG("flag=%d\n", para.flag);
#endif

	if (para.handle == NULL || buf == NULL || para.size < 0) {
		SL_ERR("Invalid handle!\n");
		return STO_ERR_INVALIDHANDLE;
	}
	dev = (alislsto_dev_t *)para.handle;
	priv = (storage_priv_t *)dev->priv;
	pthread_mutex_lock(&priv->seekmutex);
	alislsto_get_offset(dev, &orig_offset);
	if (para.idx == -1) {
		cur_offset = alislsto_lseek(dev, para.offset, para.whenence);
	} else {
		//err = alislsto_lseek_ext(dev, STO_PRIVATE, offset);
		if (!(para.whenence == SEEK_CUR && para.offset == 0)) {
			err = alislsto_lseek_ext(dev, para.idx, para.offset);
		}
	}
	if (err != STO_ERR_NONE || cur_offset == -1) {
		alislsto_set_offset(dev, orig_offset, para.flag);
		SL_ERR("alislsto_lock_read error ");
		pthread_mutex_unlock(&priv->seekmutex);
		return err;
	}
	if (para.size != 0) {
		*actual_size = alislsto_read(dev, buf, para.size);
	} else {
		*actual_size = 0;
	}
	alislsto_set_offset(dev, orig_offset, para.flag);
	pthread_mutex_unlock(&priv->seekmutex);
	if (*actual_size != para.size) {
		return STO_ERR_MEM;
	}
	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_lock_write
 *  @brief          This function lock seekmutex when writing.
 *
 *  @param          para alislsto_rw_param_t some para
 *  @param          buf write buffer
 *  @param          actual_size write actual size
 *
 *  @return         alisl_retcode
 *
 *  @author         oscar.shi <oscar.shi@alitech.com>
 *  @date           2014-08-06
 *
 */
alisl_retcode alislsto_lock_write(alislsto_rw_param_t para,
		unsigned char *buf, size_t *actual_size)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;
	alisl_retcode err = STO_ERR_NONE;
	loff_t orig_offset = 0;/*for bug #56342, MUST use loff_t instead of off_t.*/
	int cur_offset = 0;

#ifdef ENABLE_DEBUG
	SL_DBG("[%s %d]\n", __FUNCTION__, __LINE__);
	SL_DBG("handle=%#x\n", para.handle);
	SL_DBG("buf=%#x\n", &buf);
	SL_DBG("size=%#x\n", para.size);
	SL_DBG("offset=%#llx\n", para.offset);
	SL_DBG("whenence=%d\n", para.whenence);
	SL_DBG("idx=%d\n", para.idx);
	SL_DBG("write=%#x\n", *actual_size);
	SL_DBG("flag=%d\n", para.flag);
#endif

	if (para.handle == NULL || buf == NULL || para.size < 0) {
		SL_ERR("Invalid handle!\n");
		return STO_ERR_INVALIDHANDLE;
	}
	dev = (alislsto_dev_t *)para.handle;
	priv = (storage_priv_t *)dev->priv;
	pthread_mutex_lock(&priv->seekmutex);
	alislsto_get_offset(dev, &orig_offset);
	if (para.idx == -1) {
		cur_offset = alislsto_lseek(dev, para.offset, para.whenence);
	} else {
		//err = alislsto_lseek_ext(dev, STO_PRIVATE, offset);
		if (!(para.whenence == SEEK_CUR && para.offset == 0)) {
			err = alislsto_lseek_ext(dev, para.idx, para.offset);
		}
	}
	if (err != STO_ERR_NONE || cur_offset == -1) {
		alislsto_set_offset(dev, orig_offset, para.flag);
		SL_ERR("alislsto_lock_write error ");
		pthread_mutex_unlock(&priv->seekmutex);
		return err;
	}
	if (para.size != 0) {
//		*actual_size = alislsto_write(dev, buf, para.size);
		*actual_size = alislsto_write_no_erase(dev, buf, para.size);
	} else {
		*actual_size = 0;
	}
	alislsto_set_offset(dev, orig_offset, para.flag);
	pthread_mutex_unlock(&priv->seekmutex);
	if (*actual_size != para.size) {
		return STO_ERR_MEM;
	}
	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_get_pmi
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          pmi_info
 *
 *  @return         alisl_retcode
 *
 *  @author         demon.yan <demon.yan@alitech.com>
 *  @date           2014-2-24
 *
 */
alisl_retcode alislsto_get_pmi(alisl_handle handle, void *pmi)
{
	alislsto_dev_t *dev;
	pmi_info_t *pmi_info;
	storage_priv_t *priv;
	unsigned char *buffer;
	loff_t start_offset = 0;
	loff_t temp_offset = 0;
	int i = 0;

	SL_DBG("Try to get PMI!\n");

	dev = (alislsto_dev_t *)handle;
	pmi_info = (pmi_info_t *)pmi;

	if (dev == NULL || pmi_info == NULL)
	{
		SL_ERR("Try to operate null pointer!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	if (dev->info == NULL)
	{
		SL_ERR("Try to operate null pointer!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	priv = dev->priv;

	pthread_mutex_lock(&priv->rdmutex);

	buffer = pmi_info->buf_start;

	for (i = 0; i < 4; i++)
	{
		start_offset = dev->info->writesize * i * 256;
		temp_offset = start_offset + dev->info->erasesize;

		/* check ddr&miniboot block is bad or not */
		if (ioctl(dev->handle, MEMGETBADBLOCK, &temp_offset))
		{
			continue;
		}

		if (ioctl(dev->handle, MEMGETBADBLOCK, &start_offset))
		{
			SL_DBG("Find a bad block at 0x%llx!\n", start_offset);
			continue;
		}

		if (lseek(dev->handle, start_offset, SEEK_SET) < 0)
		{
			break;
		}

		if(read(dev->handle, buffer, pmi_info->length) == -1)
		{
			 SL_DBG("read failed\n");
		}

		SL_DBG("0x%x 0x%x 0x%x 0x%x!\n",
		       buffer[12], buffer[13], buffer[14], buffer[15]);

		/* check PMI tag */
		if (buffer[14] == 0x55 && buffer[15] == 0xAA)
		{
			pmi_info->offset = start_offset;
			break;
		}
	}

	pthread_mutex_unlock(&priv->rdmutex);

	if (i >= 4)
	{
		SL_DBG("Can not get pmi!\n");
		dev->pmi = false;
		return STO_ERR_DRIVER;
	}
	else
		dev->pmi = true;

	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_set_pmi
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          pmi_info
 *
 *  @return         alisl_retcode
 *
 *  @author         demon.yan <demon.yan@alitech.com>
 *  @date           2014-2-24
 *
 */
alisl_retcode alislsto_set_pmi(alisl_handle handle, void *pmi)
{
	alislsto_dev_t *dev;
	pmi_info_t *pmi_info;
	storage_priv_t *priv;
	unsigned char *wrbuf;
	size_t size = 0;

	erase_info_t eraseinfo;

	dev = (alislsto_dev_t *)handle;
	pmi_info = (pmi_info_t *)pmi;

	if (dev == NULL || pmi_info == NULL)
	{
		SL_ERR("Try to operate null pointer!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	if (dev->pmi == false)
	{
		SL_DBG("Not suppport PMI\n");
		return STO_ERR_DRIVER;
	}

	size = dev->info->erasesize;
	wrbuf = (unsigned char *)malloc(size);
	if (NULL == wrbuf) {
		SL_DBG("Malloc failed\n");
        return STO_ERR_MEM;
	}
	memset(wrbuf, 0, size);

	priv = dev->priv;
	dev->offset = pmi_info->offset;

	pthread_mutex_lock(&priv->wrmutex);
	SL_ERR("PMI info offset: 0x%llx!\n", dev->offset);

	if (ioctl(dev->handle, MEMGETBADBLOCK, &dev->offset))
	{
		SL_ERR("PMI block at 0x%llx is a bad block!\n", dev->offset);
		goto driver_err;
	}

	if (lseek(dev->handle, dev->offset, SEEK_SET) < 0)
	{
		goto driver_err;
	}

	if(read(dev->handle, wrbuf, size) == -1)
	{
		 SL_DBG("read failed\n");
	}
	memcpy(wrbuf, pmi_info->buf_start, pmi_info->length);

	eraseinfo.start = dev->offset;
	eraseinfo.length = dev->info->erasesize;

	if (ioctl(dev->handle, MEMERASE, &eraseinfo) < 0)
	{
		SL_ERR("In %s Erase fail 0x%llx %d\n", \
		               __FUNCTION__, \
		               eraseinfo.start, eraseinfo.length);
		goto driver_err;
	}

	if (lseek(dev->handle, dev->offset, SEEK_SET) < 0)
	{
		goto driver_err;
	}

	SL_ERR("PMI info offset: 0x%llx!\n", dev->offset);

	if(write(dev->handle, wrbuf, size) == -1)
	{

		SL_DBG("write failed\n");
	}
	//if (NULL != wrbuf) {
        free(wrbuf);
        wrbuf = NULL;
	//}
	pthread_mutex_unlock(&priv->wrmutex);
	return STO_ERR_NONE;

driver_err:
//	if (NULL != wrbuf) {
        free(wrbuf);
        wrbuf = NULL;
//	}
	pthread_mutex_unlock(&priv->wrmutex);
	return STO_ERR_DRIVER;
}

/**
 *  Function Name:  alislsto_open
 *  @brief          open nor flash/nand flash device
 *
 *  @param          handle    point to descriptor of device
 *  @param          sto_type  storage type(STO_TYPE_NAND,STO_TYPE_NOR)
 *  @param          flags     open mode(O_RDONLY, O_WRONLY, O_RDWR)
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislsto_open(alisl_handle *handle, alislsto_type_t sto_type, int flags)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;
	char mtd_dev[32];
	char *mtd_dev_path = NULL;
	int mtd_dev_path_size = 64;
	alisl_retcode ret;
	alisl_handle new_handle = NULL;
	int mtd_index = -1;
#ifdef USE_MTD_BLOCK
	const char mtd_path[] = "/dev/mtdblock%d";
#else
	const char mtd_path[] = "/dev/mtd%d";
#endif

	pthread_mutex_lock(&m_mutex);

	if (handle == NULL)
	{
		SL_ERR("invalid handle\n");
		goto handle_err;
	}

	/*
	* if device has been opened, we just increment counter and then
	* return the exist handle. so, multiple threads may share this handle.
	*/
	if (sto_type == STO_TYPE_NOR || sto_type == STO_TYPE_NAND)
	{
		if (nor_handle != NULL && sto_type == STO_TYPE_NOR)
		{
			nor_handle->open_cnt++;
			*handle = (alisl_handle *)nor_handle;

			SL_DBG("Open exist nor flash, cnt: %d\n",
			               nor_handle->open_cnt);
			goto no_err;
		}
		else if (nand_handle != NULL && sto_type == STO_TYPE_NAND)
		{
			nand_handle->open_cnt++;
			*handle = (alisl_handle *)nand_handle;

			SL_DBG("Open exist nand flash, cnt: %d\n",
			               nand_handle->open_cnt);
			goto no_err;
		}
	}
	else
	{
		SL_ERR("Try to open error sto type!\n");
		goto param_err;
	}

    mtd_dev_path = (char *)malloc(mtd_dev_path_size);
	if (NULL == mtd_dev_path) {
		SL_ERR("malloc failed\n");
        goto mem_err;
	}

	/*
	* if device has not been opened, we need construct a new handle and then
	* open this device
	*/
	alislsto_construct(&new_handle);
	dev = (alislsto_dev_t *)new_handle;

	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		goto handle_err;
	}

	priv = dev->priv;
	dev->type = sto_type;
	dev->dev_name = NULL;
	dev->handle = STO_INVALID_HANDLE;

	if (STO_TYPE_NOR == dev->type)
	{
		/* get mtd dev by mtd name */
		ret = find_mtd_dev_num("sflash", mtd_dev, 32);

		if (ret != STO_ERR_NONE)
		{
		    alislsto_destruct(&new_handle);
			goto handle_err;
		}
		//sprintf(mtd_dev_path, "/dev/%s", mtd_dev);
		mtd_index = atoi(mtd_dev + 3);
		snprintf(mtd_dev_path, mtd_dev_path_size, mtd_path, mtd_index);
		dev->dev_name = mtd_dev_path;
		dev->attr = nor_map;
	}
	else if (STO_TYPE_NAND == dev->type)
	{
		/* get mtd dev by mtd name */
		//ret = find_mtd_dev_num("ali_nand", mtd_dev, 32);
		ret = find_mtd_dev_num("ALI_Nand_Whole", mtd_dev, 32);

		if (ret != STO_ERR_NONE)
		{
			strncpy(mtd_dev, "mtd1", sizeof(mtd_dev)-1);
			mtd_dev[sizeof(mtd_dev)-1] = '\0';
		}
		//sprintf(mtd_dev_path, "/dev/%s", mtd_dev);
		mtd_index = atoi(mtd_dev + 3);
		snprintf(mtd_dev_path, mtd_dev_path_size, mtd_path, mtd_index);
		dev->dev_name = mtd_dev_path;
		dev->attr = nand_map;
	}

	pthread_mutex_lock(&priv->devlock);
	if (NULL != dev->dev_name) {
		//printf("Open mtd file: %s\n", mtd_dev_path);
	    dev->handle = open(dev->dev_name, flags);
	}

	if (STO_INVALID_HANDLE == dev->handle)
	{
		SL_ERR("In %s Can't open device %s\n", \
		               __FUNCTION__, dev->dev_name);
		alislsto_destruct(&new_handle);
		goto handle_err;
	}

	if (STO_TYPE_NOR == dev->type)
	{
		nor_handle = (alislsto_dev_t *)new_handle;
		nor_handle->open_cnt = 1;
	}
	else if (STO_TYPE_NAND == dev->type)
	{
		nand_handle = (alislsto_dev_t *)new_handle;
		nand_handle->open_cnt = 1;
	}

	show_flash_info(dev);

	lseek(dev->handle, 0, SEEK_SET);
	if (STO_TYPE_NAND == dev->type) {
		storage_parse(dev);
	}

	dev->offset = 0;
	*handle = new_handle;

no_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_NONE;
param_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_INVALIDPARAM;
mem_err:
    pthread_mutex_unlock(&m_mutex);
    return STO_ERR_MEM;
handle_err:
    if (NULL != mtd_dev_path) {
        free(mtd_dev_path);
        mtd_dev_path = NULL;
    }
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_INVALIDHANDLE;
}


/**
 *  Function Name:  alislsto_read
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          buf     buf to store result data
 *  @param          len     data length to be read
 *
 *  @return         read size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
size_t alislsto_read(alisl_handle handle, unsigned char *buf, size_t len)
{
	bool    bad = 0;
	size_t  readsz = 0;
	size_t  size = 0;
	unsigned char *buffer = buf;
	alislsto_dev_t *dev;
	//storage_priv_t *priv;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL || buffer == NULL)
	{
		SL_ERR("Invalid input param!\n");
		return STO_ERR_INVALIDPARAM;
	}

//	priv = dev->priv;

	SL_DBG("dev->type = %d\n", dev->type);


	/*
	 * Risk: Some guy seek, but the read action won't atomic
	 *       in any case, the the offset may be wrong
	 */

	/* pthread_mutex_lock(&priv->seekmutex); */
	do
	{
		if (dev->offset >= dev->info->size)
		{
			SL_DBG("Block offset overflow! offset: 0x%llx\n", dev->offset);
			break;
		}

		if (STO_TYPE_NAND == dev->type)
		{
		    bad = ioctl(dev->handle, MEMGETBADBLOCK, &dev->offset);
			if (bad)
			{
				SL_DBG("In %s A bad block 0x%llx\n", \
				               __FUNCTION__, dev->offset);
				dev->offset += dev->info->erasesize;
				continue;
			}
		}

		if (lseek(dev->handle, dev->offset, SEEK_SET) < 0)
		{
			break;
		}

		size = len - readsz;

		if (size >= dev->info->erasesize - (dev->offset % dev->info->erasesize))
		{
			size = dev->info->erasesize - (dev->offset % dev->info->erasesize);
		}

		if(read(dev->handle, buffer, size) == -1)
		{
			SL_DBG("read failed\n");
		}
		readsz += size;

		dev->offset += size;
		buffer += size;
	}
	while (readsz < len);

	/* pthread_mutex_unlock(&priv->seekmutex); */
	return readsz;
}

/**
 *  Function Name:  alislsto_write_no_erase
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          buf     buf to store written data
 *  @param          len     data length to be written
 *
 *  @return         write size
 *
 *  @author         demon.yan <demon.yan@alitech.com>
 *  @date           2014-2-18
 *
 *  Notes:  Upper application should set the written offset, which
 *          should be page align; For flash, most important thing is
 *          the writting block have been erased before.
 *
 */
size_t alislsto_write_no_erase(alisl_handle handle, unsigned char *buf, size_t len)
{
	bool           bad = 0;
	size_t         total_written_size = 0;
	size_t         size = 0;
	size_t         wrsize = 0;
	unsigned char  *buffer = buf;
	alislsto_dev_t *dev;
	storage_priv_t *priv;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL || buffer == NULL)
	{
		SL_ERR("Invalid input param!\n");
		return STO_ERR_INVALIDPARAM;
	}

	priv = dev->priv;

	/*
	 * Risk: Some guy seek, but the write action won't atomic
	 *       in any case, the the offset may be wrong
	 */
	pthread_mutex_lock(&priv->wrmutex);

	do
	{
		if (dev->offset >= dev->info->size)
		{
			SL_DBG("Block offset overflow! offset: 0x%llx\n", dev->offset);
			break;
		}

		if (STO_TYPE_NAND == dev->type)
		{
		    bad = ioctl(dev->handle, MEMGETBADBLOCK, &dev->offset);
			while (bad)
			{
				SL_DBG("In %s A bad block 0x%x\n", \
				               __FUNCTION__, dev->offset);
				dev->offset += dev->info->erasesize;
                bad = ioctl(dev->handle, MEMGETBADBLOCK, &dev->offset);
			}
		}

		/* Seek to a block aligned position */
		if (dev->offset % dev->info->writesize)
		{
			SL_DBG("In %s Offset 0x%x if not page align\n", \
			               __FUNCTION__, dev->offset);
			break;
		}

		size = len - total_written_size;

		if (size >= dev->info->erasesize - (dev->offset % dev->info->erasesize))
		{
			size = dev->info->erasesize - (dev->offset % dev->info->erasesize);
		}

		SL_DBG("In %s Write to offset:0x%llx size:0x%x\n", \
		               __FUNCTION__, dev->offset, size);

		/* Seek to a block aligned position */
		if (lseek(dev->handle, \
		          dev->offset, \
		          SEEK_SET) < 0)
		{
		    SL_DBG("%s lseek fail! offset: 0x%11x.\n", __FUNCTION__, dev->offset);
			break;
		}

		wrsize = write(dev->handle, buffer, size);

		if (wrsize != size)
		{
			SL_DBG("In %s Write error, offset:0x%llx wrsize:0x%x\n", \
			               __FUNCTION__, dev->offset, wrsize);
			break;
		}

		total_written_size += wrsize;
		dev->offset += wrsize;
		buffer += wrsize;
	}
	while (total_written_size < len);

	pthread_mutex_unlock(&priv->wrmutex);

	return total_written_size;
}

/**
 *  Function Name:  alislsto_write
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          buf     buf to store written data
 *  @param          len     data length to be written
 *
 *  @return         write size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  Notes: You can't believe the write position
 *         changed after a successful write, every time
 *         you write the mtd device, use alislsto_lseek
 *         firstly
 */
size_t alislsto_write(alisl_handle handle, unsigned char *buf, size_t len)
{
	bool          bad = 0;
	size_t        total_written_size = 0;
	size_t        size = 0;
	size_t        wrsize = 0;
	size_t        dstsize = 0;
    size_t        ret_size = 0;
	unsigned char *buffer = buf;
	unsigned char *wrbuf = NULL;
#if ENABLE_DEBUG
	unsigned char *rdbuf = NULL;
#endif
	unsigned char  *dstbuf = NULL;
	erase_info_t   eraseinfo;
	alislsto_dev_t *dev;
	storage_priv_t *priv;
    size_t         unit_size = 0;
    int ret = 0;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL || buffer == NULL)
	{
		SL_ERR("Invalid input param!\n");
		return STO_ERR_INVALIDPARAM;
	}

	priv = dev->priv;
    if (dev->erase_type == STO_ERASE_TYPE_SECTOR) {
        unit_size = SECTOR_SIZE_4K;
    } else {
        unit_size = dev->info->erasesize;
    }
	wrbuf = malloc(unit_size);
	if (NULL == wrbuf) {
		SL_ERR("Malloc failed!\n");
		return STO_ERR_MEM;
	}
	memset(wrbuf, 0, unit_size);
#if ENABLE_DEBUG
	rdbuf = malloc(unit_size);
	if (NULL == rdbuf) {
	    free(wrbuf);
	    wrbuf = NULL;
		SL_ERR("Malloc failed!\n");
		return STO_ERR_MEM;
	}
	memset(rdbuf, 0, unit_size);
#endif

	/*
	 * Risk: Some guy seek, but the write action won't atomic
	 *       in any case, the the offset may be wrong
	 */
	/* pthread_mutex_lock(&priv->seekmutex); */
	pthread_mutex_lock(&priv->wrmutex);

	do
	{
		/* To avoid death loop */
		if (dev->offset >= dev->info->size)
		{
			SL_DBG("Block offset overflow! offset: 0x%llx\n", dev->offset);
			break;
		}

		if (STO_TYPE_NAND == dev->type)
		{
		    bad = ioctl(dev->handle, MEMGETBADBLOCK, &dev->offset);
			if (bad)
			{
				SL_DBG("A bad block 0x%llx\n", dev->offset);
				dev->offset += dev->info->erasesize;

				continue;
			}
		}

		/* Seek to a block/sector aligned position */
		if (lseek(dev->handle, \
		          dev->offset - dev->offset % unit_size, \
		          SEEK_SET) < 0)
		{
			break;
		}

		dstbuf = buffer;
		size = unit_size;
		wrsize = unit_size;

		/* Read a whole block/sector firstly */
		if (0 != (dev->offset % unit_size))
		{
			/* If we're here, it must be the first write */
			ret_size = read(dev->handle, wrbuf, unit_size);
            if (ret_size != unit_size) {
                break;
            }
			dstsize = unit_size - dev->offset % unit_size;

			if (len < dstsize)
			{
				dstsize = len;
			}

			memcpy(wrbuf + dev->offset % unit_size, buffer, dstsize);
			wrsize = dstsize;
			dstbuf = wrbuf;
		}
		else if (len - total_written_size < unit_size)
		{
			/* If we're here, it must be the last write */
			ret_size = read(dev->handle, wrbuf, unit_size);
            if (ret_size != unit_size) {
                break;
            }
			dstsize = len - total_written_size;
			memcpy(wrbuf, buffer, dstsize);
			wrsize = dstsize;
			dstbuf = wrbuf;
		}

		eraseinfo.start = dev->offset - dev->offset % unit_size;
		eraseinfo.length = unit_size;

        if (dev->erase_type == STO_ERASE_TYPE_SECTOR)
            ret = ioctl(dev->handle, MEMERASE4K, &eraseinfo);
        else
            ret = ioctl(dev->handle, MEMERASE, &eraseinfo);
        if (ret < 0) {
            SL_ERR("In %s Erase fail 0x%llx %d\n", \
                    __FUNCTION__, \
                    eraseinfo.start, eraseinfo.length);
            /* set bbt if erase fail */
            update_bbt(dev, eraseinfo.start); 
            dev->offset += unit_size;

            continue;
        }

		/* Seek to a block aligned position */
		if (lseek(dev->handle, \
		          dev->offset - dev->offset % unit_size, \
		          SEEK_SET) < 0)
		{
			break;
		}

#if 0
		SL_DBG("In %s Write to offset:0x%llx wrsize:0x%x, dstbuf[0~3]:0x%x %x %x %x\n", \
		               __FUNCTION__, dev->offset, wrsize, dstbuf[0], dstbuf[1], dstbuf[2], dstbuf[3]);
#else
		SL_DBG("In %s Write to offset:0x%llx wrsize:0x%x\n", \
		               __FUNCTION__, dev->offset, wrsize);
#endif

		ret_size = write(dev->handle, dstbuf, size);
        if (ret_size != size) {
            SL_DBG("In %s Write error, offset:0x%llx wrsize:0x%x\n", \
			               __FUNCTION__, dev->offset, wrsize);
            break;
        }
#if ENABLE_DEBUG

		if (lseek(dev->handle, \
		          dev->offset - dev->offset % unit_size, \
		          SEEK_SET) < 0)
		{
			break;
		}

		read(dev->handle, rdbuf, size);

		if (0 != memcmp(rdbuf, dstbuf, size))
		{
			SL_ERR("In %s Write error 0x%x\n", \
			               __FUNCTION__, \
			               dev->offset - dev->offset % unit_size);
		}

#if 0
		SL_DBG("In %s Read back offset:0x%llx size:0x%x, rdbuf[0~3]:0x%x %x %x %x\n", \
		               __FUNCTION__, dev->offset, wrsize, rdbuf[0], rdbuf[1], rdbuf[2], rdbuf[3]);
#endif

#endif
		total_written_size += wrsize;

		dev->offset += wrsize;
		buffer += wrsize;
	}
	while (total_written_size < len);

	pthread_mutex_unlock(&priv->wrmutex);
	/* pthread_mutex_unlock(&priv->seekmutex); */

#if ENABLE_DEBUG
	free(rdbuf);
#endif
	free(wrbuf);
	return total_written_size;
}

/**
 *  Function Name:  alislsto_erase
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          start   erase from where, physical address
 *  @param          length  derase length
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 */
alisl_retcode alislsto_erase(alisl_handle handle, loff_t start, size_t length)
{
	int ret = 0;
	erase_info_t fls_erase;
	int block_size = 0;
	int i = 0;
	int bad_block = 0;
	int erase_num = 0;
	loff_t erase_start = 0;
	loff_t block_start = 0;
    loff_t addr_logic = 0;
	alislsto_dev_t *dev;
	//storage_priv_t *priv;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL) {
		SL_ERR("Try to open before construct!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	//priv = dev->priv;
	block_size = dev->info->erasesize;

	if (-1 == block_size || block_size == 0) {
		SL_ERR("Get block info error! block_size=0x%x\n", block_size);
		return STO_ERR_INVALIDSIZE;
	}

    if ((start % block_size) || (length % block_size)) {
        SL_ERR("start address and erase size should be block align!!\n");
        return STO_ERR_INVALIDPARAM;
    }   

    if (alislsto_get_logic_addr(dev, start, &addr_logic)) {
        SL_ERR("pls check the start address!!\n");
        return STO_ERR_INVALIDPARAM;
    }
    
	memset(&fls_erase, 0, sizeof(erase_info_t));

    if (dev->erase_type == STO_ERASE_TYPE_SECTOR) {
        block_size = SECTOR_SIZE_4K;
    } 
	erase_num = length / block_size;
	erase_start = start;
	fls_erase.length = block_size;
    
	SL_DBG("%s size of off_t: %d\n", __FUNCTION__, sizeof (off_t));
	SL_DBG("%s start = 0x%llx length = 0x%x erase_num = %d\n", \
	               __FUNCTION__ , start, length, erase_num);

	for (i = 0; i < erase_num; i++) {
		fls_erase.start  = erase_start;
		block_start = erase_start;
        /* To avoid death loop */
        if (erase_start >= dev->info->size){
            SL_DBG("%s addr_logic = 0x%llx length = 0x%x logic flash_size = %d\n", \
                   __FUNCTION__ , addr_logic, length, dev->info_logic->size);
            if ((addr_logic == 0) 
                && (length >= dev->info_logic->size)) {
                SL_DBG("erase whole partition done.\n");
                break;
            }
            SL_WARN("Block offset overflow! offset: 0x%x\n", (unsigned int)erase_start);
            return STO_ERR_OVERFLOW;
        }

		if (STO_TYPE_NAND == dev->type) {
			bad_block = ioctl(dev->handle, MEMGETBADBLOCK, &block_start);

			if (bad_block) {
                SL_WARN("%s, 0x%x is bad block.\n", __func__, (unsigned int)(block_start));
				erase_start += block_size;
                i--;                
				continue;
			}
		}

        if (dev->erase_type == STO_ERASE_TYPE_SECTOR)
            ret = ioctl(dev->handle, MEMERASE4K, &fls_erase);
        else
            ret = ioctl(dev->handle, MEMERASE, &fls_erase);

        if (ret) {
            SL_DBG("%s Erase erea at 0x%llx failed! length = 0x%x\n",
                __FUNCTION__ , fls_erase.start, fls_erase.length);
            /* set bbt if erase fail */
            update_bbt(dev, block_start); 
            erase_start += block_size;
            i--;                
            continue;
        }

	    erase_start += block_size;
	}

	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_lseek
 *  @brief
 *
 *  @param          handle   point to descriptor of device
 *  @param          offset   seek offset
 *  @param          whenence seek whenence
 *
 *  @return         success: current offset; fail: -1
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
int alislsto_lseek(alisl_handle handle, int offset, int whenence)
{
	alislsto_dev_t *dev;
	//storage_priv_t *priv;
	int ret = 0;
	int new_addr;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	//priv = dev->priv;

	if (STO_INVALID_HANDLE == dev->handle)
	{
		SL_ERR("In %s Invalid handle of %s\n", \
		               __FUNCTION__, dev->dev_name);
		return STO_ERR_INVALIDHANDLE;
	}

/*	if (NULL == dev->attr)
	{
		SL_ERR("In %s No layout mapping found for %s\n", \
		               __FUNCTION__, dev->dev_name);
		return STO_ERR_NOLAYOUTMAP;
	}
*/
	/* pthread_mutex_lock(&priv->seekmutex); */

	switch (whenence)
	{
		case SEEK_SET:

			/* Great than totol size, seek to end */
			if (offset >= dev->info->size)
			{
				dev->offset = dev->info->size - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
				dev->offset = offset;
			}

			break;

		case SEEK_CUR:
			new_addr = dev->offset + offset;

			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				dev->offset = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= dev->info->size)
			{
				dev->offset = dev->info->size - 1;
			}
			/* Common seek */
			else
			{
				dev->offset = new_addr;
			}

			break;

		case SEEK_END:
			new_addr = (int)dev->info->size + offset - 1;

			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				dev->offset = 0;
			}
			/* Common seek */
			else if (offset <= 0)
			{
				dev->offset = new_addr;
			}

			break;

		default:
			SL_ERR("please check your whenence parameter!\n");
			ret = -1;
	}

	/* pthread_mutex_unlock(&priv->seekmutex); */

	if (ret >= 0)
	{
		return dev->offset;
	}
	else
	{
		return ret;
	}
}

int alislsto_lseek_logic(alisl_handle handle, int offset, int whenence)
{
	alislsto_dev_t *dev;
	//storage_priv_t *priv;
	int ret = 0;
	int new_addr;
	loff_t addr_logic = 0;
	loff_t addr_phy = 0;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	//priv = dev->priv;

	if (STO_INVALID_HANDLE == dev->handle)
	{
		SL_ERR("In %s Invalid handle of %s\n", \
		               __FUNCTION__, dev->dev_name);
		return STO_ERR_INVALIDHANDLE;
	}

/*	if (NULL == dev->attr)
	{
		SL_ERR("In %s No layout mapping found for %s\n", \
		               __FUNCTION__, dev->dev_name);
		return STO_ERR_NOLAYOUTMAP;
	}
*/
	/* pthread_mutex_lock(&priv->seekmutex); */

	// dev->offset is the current logic addr
	// Need the current logic addr to calculate the new logic addr
	if (alislsto_get_logic_addr(dev, dev->offset, &addr_logic)) {
		return -1;
	}

	switch (whenence)
	{
		case SEEK_SET:

			/* Great than totol size, seek to end */
			if (offset >= dev->info_logic->size)
			{
				addr_logic = dev->info_logic->size - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
				addr_logic = offset;
			}

			break;

		case SEEK_CUR:
			new_addr = addr_logic + offset;

			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				addr_logic = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= dev->info_logic->size)
			{
				addr_logic = dev->info_logic->size - 1;
			}
			/* Common seek */
			else
			{
				addr_logic = new_addr;
			}

			break;

		case SEEK_END:
			new_addr = (int)dev->info_logic->size + offset - 1;

			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				addr_logic = 0;
			}
			/* Common seek */
			else if (offset <= 0)
			{
				addr_logic = new_addr;
			}

			break;

		default:
			SL_ERR("please check your whenence parameter!\n");
			ret = -1;
	}

	/* pthread_mutex_unlock(&priv->seekmutex); */

	if (ret >= 0)
	{
		// addr_logic is the new logic addr
		// Assign dev->offset with the new physical addr
		if (alislsto_get_phy_addr(dev, addr_logic, &addr_phy)) {
			return -1;
		} else {
			dev->offset = addr_phy;
		}
		return addr_logic;
	}
	else
	{
		return ret;
	}
}

/**
 *  Function Name:  alislsto_lseek_ext
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          idx     which area to seek
 *  @param          offset  seek to where
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  Notes: alislsto_lseek_ext will seek to offset based on partition,
 *         while the alislsto_lseek is based on the sto device
 */
alisl_retcode alislsto_lseek_ext(alisl_handle handle, alislsto_idx_t idx, loff_t offset)
{
	storage_map_t *map = NULL;
	alislsto_dev_t *dev;
	storage_priv_t *priv;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	if (dev->pmi == false)
	{
		SL_DBG("Not suppport PMI\n");
		return STO_ERR_DRIVER;
	}

	priv = dev->priv;

	if (STO_INVALID_HANDLE == dev->handle)
	{
		SL_ERR("In %s Invalid handle of %s\n", \
		               __FUNCTION__, dev->dev_name);
		return STO_ERR_INVALIDHANDLE;
	}

	if (NULL == dev->attr)
	{
		SL_ERR("In %s No layout mapping found for %s\n", \
		               __FUNCTION__, dev->dev_name);
		return STO_ERR_NOLAYOUTMAP;
	}

	map = (storage_map_t *)dev->attr;

	if (((0 != map[idx].size) && (offset > map[idx].size)) || \
	    (map[idx].offset + offset > dev->info->size))
	{
		SL_ERR("In %s Seek %s overflow size 0x%x 0x%x offset 0x%x 0x%x 0x%x\n", \
		               __FUNCTION__, dev->dev_name, \
		               map[idx].size, dev->info->size, \
		               map[idx].offset, offset, \
		               map[idx].offset + offset);
		return STO_ERR_OVERFLOW;
	}

	/* pthread_mutex_lock(&priv->seekmutex); */
	dev->offset = map[idx].offset + offset;

	if (STO_PMI == idx)
	{
		priv->margin = dev->info->size;
	}
	else
	{
		priv->margin = map[idx].offset + map[idx].size;
	}

	/* pthread_mutex_unlock(&priv->seekmutex); */

	return STO_ERR_NONE;
}


/**
 *  Function Name:  alislsto_tell_ext
 *  @brief          tell the caller of the partition size
 *
 *  @param          handle  point to descriptor of device
 *  @param          idx     which area to seek
 *  @param          attr    storage partattr
 *
 *  @return         partition size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  Notes: alislsto_tell_ext is based on the partition,
 *         which will tell the partition offset and size
 */
size_t alislsto_tell_ext(alisl_handle handle, alislsto_idx_t idx, alislsto_partattr_t attr)
{
	storage_map_t *map;
	alislsto_dev_t *dev;
//	storage_priv_t *priv;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	if (dev->pmi == false)
	{
		SL_DBG("Not suppport PMI\n");
		return STO_ERR_DRIVER;
	}

	//priv = dev->priv;
	map = (storage_map_t *)dev->attr;

	if (attr == STO_PART_SIZE)
	{
		return map[idx].size;
	} else 
	//if (attr == STO_PART_OFFSET)
	{
		return map[idx].offset;
	}
}

/**
 *  Function Name:  alislsto_ioctl
 *  @brief          lock write/read before call kernel ioctl
 *                  to prevent kernel panic
 *
 *  @param          handle  point to alislsto device
 *  @param          cmd     ioctl cmd
 *  @param          param   point to ioctl param
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-7-16
 *
 */
alisl_retcode alislsto_ioctl(alisl_handle handle, unsigned int cmd, void *param)
{
	alisl_retcode ret = 0;
	alislsto_dev_t *dev;
	storage_priv_t *priv;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL || param == NULL)
	{
		SL_DBG("param invalid!\n");
		return STO_ERR_INVALIDHANDLE;
	}

	priv = dev->priv;

	pthread_mutex_lock(&priv->rdmutex);
	pthread_mutex_lock(&priv->wrmutex);

	ret = ioctl(dev->handle, cmd, param);

	pthread_mutex_unlock(&priv->wrmutex);
	pthread_mutex_unlock(&priv->rdmutex);

	return ret;
}

/**
 *  Function Name:  alislsto_get_param
 *  @brief
 *
 *  @param          handle  point to alislsto device
 *  @param          param   point to param buff
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-7-16
 *
 */
alisl_retcode alislsto_get_param(alisl_handle handle, alislsto_param_t *param)
{
	//alisl_retcode ret = 0;
	alislsto_dev_t *dev;

	dev = (alislsto_dev_t *)handle;

	if (dev == NULL || param == NULL)
	{
		SL_DBG("Invalid input param!\n");
		return STO_ERR_INVALIDPARAM;
	}

	param->type = dev->type;
	//param->info = dev->info;
	param->info = dev->info_logic;
	param->maxpart = dev->maxpart;

	return STO_ERR_NONE;
}

/**
 *  Function Name:  alislsto_close
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          sync    not use right now
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode alislsto_close(alisl_handle handle, bool sync)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;

	pthread_mutex_lock(&m_mutex);
	dev = (alislsto_dev_t *)handle;

	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		goto param_err;
	}

	if (dev->type == STO_TYPE_NOR)
	{
		if (--dev->open_cnt)
		{
			SL_DBG("Close nor flash, cnt: %d\n",
			               dev->open_cnt);
			goto no_err;
		}

		nor_handle = NULL;
	}

	if (dev->type == STO_TYPE_NAND)
	{
		if (--dev->open_cnt)
		{
			SL_DBG("Close nand flash, cnt: %d\n",
			               dev->open_cnt);
			goto no_err;
		}

		nand_handle = NULL;
	}

	if (STO_INVALID_HANDLE != dev->handle)
	{
		close(dev->handle);
		dev->handle = STO_INVALID_HANDLE;
	}

	if (NULL != dev->info)
	{
		free(dev->info);
		dev->info = NULL;
	}
	if (NULL != dev->info_logic)
	{
		free(dev->info_logic);
		dev->info_logic = NULL;
	}

	dev->offset = 0;
	if (NULL != dev->dev_name) {
	    free(dev->dev_name);
	    dev->dev_name = NULL;
	}
	dev->attr = NULL;

	priv = dev->priv;
	pthread_mutex_unlock(&priv->devlock);

	alislsto_destruct(&handle);

no_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_NONE;
param_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_INVALIDPARAM;
}


/**
 *  Function Name:  alislsto_destruct
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode alislsto_destruct(alisl_handle *handle)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;

	dev = (alislsto_dev_t *)(*handle);

	if (dev == NULL)
	{
		SL_ERR("STO private is NULL before destruct!");
		return 0;
	}

	priv = dev->priv;

	pthread_mutex_destroy(&priv->devlock);
	pthread_mutex_destroy(&priv->rdmutex);
	pthread_mutex_destroy(&priv->wrmutex);
	pthread_mutex_destroy(&priv->seekmutex);

	free(dev->priv);
	dev->priv = NULL;

	free(dev);
	*handle = NULL;

	return STO_ERR_NONE;
}

alisl_retcode alislsto_get_partinfo(alislsto_partinfo_list_t **partition_list)
{
	char buffer[256] = {0};
	//unsigned int len = -1;
	//size_t size = -1;
	char *start = NULL;
	char *end = NULL;
	FILE* fp = NULL;
	int index = 0;
	alislsto_partinfo_t *info = NULL;
//	alisl_handle new_handle = NULL;
	alislsto_partinfo_t *new_list = NULL;
	alislsto_dev_t **new_dev_list = NULL;

	*partition_list = (alislsto_partinfo_list_t *)NULL;

	if ((fp = fopen("/proc/mtd", "r")) == NULL) {
		SL_ERR("Open /proc/mtd fail!\n");
		return STO_ERR_DRIVER;
	}

	index = 0;
	partinfo_list.count = 0;
	//printf("\n==========================================\n");
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		//printf("[count=%d] %s\n", partinfo_list.count, buffer);
		if ((start = strstr(buffer, "mtd")) != NULL) {
			index = partinfo_list.count;
			if (index % 10 == 0) {
				new_list = realloc(partinfo_list.list,
						(index + 10) * sizeof(alislsto_partinfo_t));
				if (new_list) {
					partinfo_list.list = new_list;
				} else {
					fclose(fp);
					return STO_ERR_MEM;
				}
			}
			memset(&partinfo_list.list[index], 0, sizeof(alislsto_partinfo_t));
			info = &partinfo_list.list[index];
			start += 3;
			info->index = atoi(start);
			start = strchr(start, ':');
			start += 2;
			info->size = strtol(start, NULL, 16);
			start = strchr(start, '\"');
			strncpy(info->name, start + 1, sizeof(info->name)-1);
			info->name[sizeof(info->name)-1] = '\0';
			end = strchr(info->name, '\"');
			if (end != NULL) {
				*end = 0;
			}
			partinfo_list.count++;
		}
	}
	//printf("\n==========================================\n");

	if (partinfo_list.count == 0) {
		SL_ERR("Not MTD partition found!\n");
		fclose(fp);
		return STO_ERR_DRIVER;
	}

	if (handle_list.count < partinfo_list.count && partinfo_list.count > 0) {
		new_dev_list = realloc(handle_list.list,
				(partinfo_list.count) * sizeof(void *));
		if (new_dev_list) {
			handle_list.list = new_dev_list;
		} else {
			fclose(fp);
			return STO_ERR_MEM;
		}

		for (index = handle_list.count; index < partinfo_list.count; index++) {
			//alislsto_construct(&new_handle);
			//handle_list.list[index] = (alislsto_dev_t *)new_handle;
			handle_list.list[index] = NULL;
		}
		handle_list.count = partinfo_list.count;
		printf("MTD DEVICES TOTAL NUMBER: %d\n", handle_list.count);
	} else {
		// TODO
	}

	SL_DBG("MTD device count=%d\n",partinfo_list.count);
	for (index = 0; index < partinfo_list.count; index++) {
		info = &partinfo_list.list[index];
		if (info) {
			SL_DBG("index=%d [%s] size=%#lx\n",
					info->index, info->name, info->size);
		} else {
			SL_DBG("partinfo_list.list error! index=%d\n",
					index);
		}
	}

	*partition_list = &partinfo_list;

	fclose(fp);
	return 0;
}

alisl_retcode alislsto_get_partsize(char* name, size_t *size)
{
	alislsto_partinfo_list_t *list = NULL;
	int i = 0;

	if (!partinfo_list.count) {
		alislsto_get_partinfo(&list);
	}
	for (i = 0; i < partinfo_list.count; i++) {
		if (!strcmp(name, partinfo_list.list[i].name)) {
			*size = partinfo_list.list[i].size;
			return 0;
		}
	}
	*size = 0;
	return STO_ERR_DRIVER;
}

alisl_retcode alislsto_mtd_open_by_name(alisl_handle *handle, char* name, int flags)
{
	alislsto_partinfo_list_t *list = NULL;
	int i = 0;

	if (!partinfo_list.count) {
		alislsto_get_partinfo(&list);
	}

	for (i = 0; i < partinfo_list.count; i++) {
		if (!strcmp(name, partinfo_list.list[i].name)) {
			alislsto_mtd_open(handle, i, flags, STO_ERASE_TYPE_BLOCK);
			return 0;
		}
	}

	return STO_ERR_DRIVER;
}

alisl_retcode alislsto_mtd_open(alisl_handle *handle, int index, int flags, int erase_type)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;
//	char mtd_dev[32];
	char *mtd_dev_path = NULL;
	int mtd_dev_path_size = 64;
	//alisl_retcode ret;
	alisl_handle new_handle = NULL;
	//int mtd_index = -1;
	alislsto_partinfo_list_t *partition_list = NULL;

#ifdef USE_MTD_BLOCK
	const char mtd_path[] = "/dev/mtdblock%d";
#else
	const char mtd_path[] = "/dev/mtd%d";
#endif

	pthread_mutex_lock(&m_mutex);

	if (handle == NULL)
	{
		SL_ERR("invalid handle\n");
		goto handle_err;
	}

	/*
	* if device has been opened, we just increment counter and then
	* return the exist handle. so, multiple threads may share this handle.
	*/
	if (!handle_list.count || index > handle_list.count) {
		alislsto_get_partinfo(&partition_list);
	}
	if (!handle_list.count || index > handle_list.count) {
		SL_ERR("mtd%d not found, handle_list.count=%d!\n", index, handle_list.count);
		goto param_err;
	} else {
		SL_DBG("alislsto_mtd_open mtd%d [%d]!\n", index, handle_list.count);
	}

	if (handle_list.list[index] !=NULL && handle_list.list[index]->open_cnt) {
		*handle = (alisl_handle *)(handle_list.list[index]);
		(handle_list.list[index]->open_cnt)++;
		SL_DBG("Open exist flash index: %d, cnt: %d\n", index,
			handle_list.list[index]->open_cnt);
		goto no_err;
	}

    mtd_dev_path = (char *)malloc(mtd_dev_path_size);
	if (NULL == mtd_dev_path) {
		SL_ERR("malloc failed\n");
        goto mem_err;
	}

	/*
	* if device has not been opened, we need construct a new handle and then
	* open this device
	*/
	alislsto_construct(&new_handle);
	dev = (alislsto_dev_t *)new_handle;

	if (dev == NULL) {
		SL_ERR("Try to open before construct!\n");
		goto handle_err;
	}

	priv = dev->priv;
	if (strcmp(partinfo_list.list[index].name, "sflash") == 0) {
		dev->type = STO_TYPE_NOR;
	} else {
		dev->type = STO_TYPE_NAND;
	}
	dev->dev_name = NULL;
	dev->handle = STO_INVALID_HANDLE;
    dev->erase_type = erase_type;

	if (index != -1) {
		snprintf(mtd_dev_path, mtd_dev_path_size, mtd_path, index);
		dev->dev_name = mtd_dev_path;
		dev->attr = NULL;
	}

	pthread_mutex_lock(&priv->devlock);
	if (NULL != dev->dev_name) {
		//printf("Open mtd file: %s\n", mtd_dev_path);
	    dev->handle = open(dev->dev_name, flags);
	}

	if (STO_INVALID_HANDLE == dev->handle)
	{
		SL_ERR("In %s Can't open device %s\n", \
		               __FUNCTION__, dev->dev_name);
		alislsto_destruct(&new_handle);
		goto handle_err;
	}

	if (index != -1) {
		handle_list.list[index] = (alislsto_dev_t *)new_handle;
		handle_list.list[index]->open_cnt = 1;
	}

	// Fill alislsto_dev_t info
	show_flash_info(dev);

	lseek(dev->handle, 0, SEEK_SET);
//	storage_parse(dev);

	dev->offset = 0;
	*handle = new_handle;

no_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_NONE;
param_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_INVALIDPARAM;
mem_err:
    pthread_mutex_unlock(&m_mutex);
    return STO_ERR_MEM;
handle_err:
    if (NULL != mtd_dev_path) {
        free(mtd_dev_path);
        mtd_dev_path = NULL;
    }
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_INVALIDHANDLE;
}

alisl_retcode alislsto_mtd_close(alisl_handle handle)
{
	alislsto_dev_t *dev;
	storage_priv_t *priv;

	pthread_mutex_lock(&m_mutex);
	dev = (alislsto_dev_t *)handle;


	if (dev == NULL)
	{
		SL_ERR("Try to open before construct!\n");
		goto param_err;
	}

	if (!dev->open_cnt) {
		goto no_err;
	} else {
		--(dev->open_cnt);
	}

	if (dev->open_cnt) {
		goto no_err;
	}

	if (STO_INVALID_HANDLE != dev->handle)
	{
		close(dev->handle);
		dev->handle = STO_INVALID_HANDLE;
	}

	if (NULL != dev->info)
	{
		free(dev->info);
		dev->info = NULL;
	}
	if (NULL != dev->info_logic)
	{
		free(dev->info_logic);
		dev->info_logic = NULL;
	}

	dev->offset = 0;
	if (NULL != dev->dev_name) {
	    free(dev->dev_name);
	    dev->dev_name = NULL;
	}
	dev->attr = NULL;

	priv = dev->priv;
	pthread_mutex_unlock(&priv->devlock);

	int i = 0;
	for (i = 0; i < handle_list.count; i++) {
		if (handle == handle_list.list[i]) {
			handle_list.list[i] = NULL;
		}
	}
	alislsto_destruct(&handle);

no_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_NONE;
param_err:
	pthread_mutex_unlock(&m_mutex);
	return STO_ERR_INVALIDPARAM;
}

alisl_retcode alislsto_lock_nor(alisl_handle handle, int lock, loff_t start, size_t length)
{
    int ret = 0;
    alislsto_dev_t *dev;

    dev = (alislsto_dev_t *)handle;
    if (dev == NULL) {
        SL_ERR("Try to open before construct!\n");
        return STO_ERR_INVALIDHANDLE;
    }
    /* Do not check the flash size.
    Because the user need to do lock/unlock according to the nand flash datasheet,
    for some nand flash, only support lock/unlock 0-N m, 
    and we have more than one nor partition, maybe: part0(128K), part1(1M), part2(2M),
    If need to lock 0-2M by handler of part0, that would fail.
    if (start + length > dev->info->size) {
        SL_ERR("Erase length overflow! start: 0x%llx, len: 0x%x, size: 0x%x\n",
           start, length, dev->info->size);
        return STO_ERR_INVALIDHANDLE;
    }*/

    erase_info_t s_lock;
    memset(&s_lock, 0, sizeof(erase_info_t));
    s_lock.start = start;
    s_lock.length = length;

    SL_DBG("%s start(0x%x), len: 0x%x\n", __FUNCTION__, s_lock.start, s_lock.length);

    if (lock == 1) {
        SL_DBG("%s call MEMLOCK ---- \n", __FUNCTION__);
        ret = ioctl(dev->handle, MEMLOCK, &s_lock);
    } else {
        SL_DBG("%s call MEMUNLOCK ---- \n", __FUNCTION__);
        ret = ioctl(dev->handle, MEMUNLOCK, &s_lock);
    }
    if (ret != 0) {
        SL_ERR("%s, %s return fail: %d!\n", __func__, lock? "MEMLOCK":"MEMUNLOCK", ret);
    }

    return ret;
}

alisl_retcode alislsto_islock_nor(alisl_handle handle, loff_t start, size_t length, unsigned long* lock)
{
    int ret = 0;
    alislsto_dev_t *dev;

    dev = (alislsto_dev_t *)handle;
    if (dev == NULL) {
        SL_ERR("Try to open before construct!\n");
        return STO_ERR_INVALIDHANDLE;
    }

    /* Do not check the flash size.
    Because the user need to do lock/unlock according to the nand flash datasheet,
    for some nand flash, only support lock/unlock 0-N m, 
    and we have more than one nor partition, maybe: part0(128K), part1(1M), part2(2M),
    If need to lock 0-2M by handler of part0, that would fail.
    if (start + length > dev->info->size) {
        SL_ERR("Erase length overflow! start: 0x%llx, len: 0x%x, size: 0x%x\n",
               start, length, dev->info->size);
        return STO_ERR_INVALIDHANDLE;
    }*/

    erase_info_t s_lock;
    memset(&s_lock, 0, sizeof(erase_info_t));
    s_lock.start = start;
    s_lock.length = length;

    SL_DBG("%s start(0x%x), len: 0x%x\n", __FUNCTION__, s_lock.start, s_lock.length);

    ret = ioctl(dev->handle, MEMISLOCKED, &s_lock);
    SL_DBG("%s, MEMISLOCKED return : %d.\n", __func__, ret);
    if (ret < 0) {
        SL_ERR("MEMISLOCKED return fail: %d\n", ret);
    } else {
        *lock = ret;
        SL_DBG("MEMISLOCKED return: %d\n", ret);
        ret = 0;
    }

    return ret;
}
