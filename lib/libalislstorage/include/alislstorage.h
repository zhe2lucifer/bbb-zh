/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislstorage.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 14:41:28
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#ifndef __STORAGE_INTERFACE__H_
#define __STORAGE_INTERFACE__H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

/* share library headers */
#include <alipltfretcode.h>

/* system headers */
#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Macro definition */
#define STO_MAX_STRLEN            64
#define STO_INVALID_HANDLE        -1
#define STO_DEFAULT_OFFSET        0XFFFFFFFF
#define STO_HEADER_SIZE           512

/* Index of part position in PMI */
#define STO_UBOOTADDR_POS               192
#define STO_IMAGE_BASEADDR_POS          260
#define STO_IMAGELEN_BASEADDR_POS       420
#define STO_PARTLEN_BASEADDR_POS        264
#define STO_PRIVATE_BASEADDR_POS        160
#define STO_ECCSIZE_POS                 28

/* Error handler of library */
typedef enum alislsto_err {
	STO_ERR_NONE,
	STO_ERR_CANTOPEN,
	STO_ERR_NOLAYOUTMAP,
	STO_ERR_INVALIDHANDLE,
	STO_ERR_CANTSEEK,
	STO_ERR_OVERFLOW,
	STO_ERR_INVALIDSIZE,
	STO_ERR_INVALIDPARAM,
	STO_ERR_DRIVER,
	STO_ERR_MEM,
} alislsto_err_t;

/* Flash index of mapping */
typedef enum alislsto_idx {
	STO_PMI,
	STO_PRIVATE,
	STO_KERNEL,
	STO_SEE,
	STO_ROOTFS,
	STO_APPDATA,
	STO_USERFS,
	STO_RESERVED1,
	STO_RESERVED2,
	STO_RESERVED3,
	STO_RESERVED4,
	STO_RESERVED5,
	STO_UBOOT,
	STO_UBOOT_BAK,
	STO_NANDMAX,

	STO_NOR = 64,
	STO_BOOTLOADER,
} alislsto_idx_t;

/* From where the data to be got */
typedef enum alislsto_type {
	STO_TYPE_NOR,
	STO_TYPE_NAND,
} alislsto_type_t;

/* Attribute of NAND partition */
typedef enum alislsto_partattr {
	STO_PART_OFFSET,
	STO_PART_SIZE,
} alislsto_partattr_t;

/* Parameters of the devices initialization */
typedef struct alislsto_param {
	alislsto_type_t  type;
	mtd_info_t       *info;
	int              maxpart;
} alislsto_param_t;

typedef struct alislsto_rw_param {
	alisl_handle handle; // point to descriptor of device
	size_t size; // read/write size
	long long offset; // address offset
	int whenence; // address from where
	alislsto_idx_t idx; // -1 or PMI partition index
	bool flag; // set offset or not(true:set, false:not set)
} alislsto_rw_param_t;

/* Partition info */
typedef struct alislsto_partinfo alislsto_partinfo_t;
struct alislsto_partinfo {
	char	name[NAME_MAX];
	int		index;
	size_t	size;
};

typedef struct alislsto_partinfo_list alislsto_partinfo_list_t;
struct alislsto_partinfo_list {
	int count;
	alislsto_partinfo_t *list;
};

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
alisl_retcode alislsto_get_pmi(alisl_handle handle, void *pmi);


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
alisl_retcode alislsto_set_pmi(alisl_handle handle, void *pmi);

/**
 *  Function Name:  alislsto_open
 *  @brief
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
alisl_retcode alislsto_open(alisl_handle *handle, alislsto_type_t sto_type, int flags);

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
 *  Notes: You can't believe the read position
 *         changed after a successful read, every time
 *         you read the mtd device, use alislsto_lseek_ext
 *         firstly
 */
 size_t alislsto_read(alisl_handle handle, unsigned char *buf, size_t len);

/**
 *  Function Name:  alislsto_write
 *  @brief
 *
 *  @param          handle  descriptor of device
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
 *         you write the mtd device, use alislsto_lseek_ext
 *         firstly
 */

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
size_t alislsto_write_no_erase(alisl_handle handle, unsigned char *buf, size_t len);

size_t alislsto_write(alisl_handle handle, unsigned char *buf, size_t len);

/**
 *  Function Name:  alislsto_erase
 *  @brief
 *
 *  @param          handle  point to descriptor of device
 *  @param          start   erase from where
 *  @param          length  derase length
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 */
alisl_retcode alislsto_erase(alisl_handle handle, loff_t start, size_t length);

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
int alislsto_lseek(alisl_handle handle, int offset, int whenence);

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
alisl_retcode alislsto_lseek_ext(alisl_handle handle, alislsto_idx_t idx, loff_t offset);/*for bug #56342, MUST use loff_t instead of off_t.*/

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
size_t alislsto_tell_ext(alisl_handle handle, alislsto_idx_t idx, alislsto_partattr_t attr);

/**
 *  Function Name:  alislsto_ioctl
 *  @brief
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
alisl_retcode alislsto_ioctl(alisl_handle handle, unsigned int cmd, void *param);

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
alisl_retcode alislsto_get_param(alisl_handle handle, alislsto_param_t *param);

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
alisl_retcode alislsto_close(alisl_handle handle, bool sync);

/**
 *  Function Name:  alislsto_get_partinfo
 *  @brief          Get the MTD partition from /proc/mtd
 *  @param          info  point to alislsto_partinfo_list_t
 *  @return         alisl_retcode
 */
alisl_retcode alislsto_get_partinfo(alislsto_partinfo_list_t **info);

/**
 *  Function Name:  alislsto_get_partsize
 *  @brief          Get the MTD partition size by name
 */
alisl_retcode alislsto_get_partsize(char* name, size_t * size);

/**
 *  Function Name:  alislsto_open_by_name
 *  @brief          Open MTD partition by name
 */
alisl_retcode alislsto_mtd_open_by_name(alisl_handle *handle, char* name, int flags);

/**
 *  Function Name:  alislsto_mtd_open
 *  @brief          Open MTD partition by index
 *  @erase_type: 0 - erase by block, 1 - erase by sector(4K)
 */
alisl_retcode alislsto_mtd_open(alisl_handle *handle, int index, int flags, int erase_type);

/**
 *  Function Name:  alislsto_mtd_close
 *  @brief          Close MTD partition
 */
alisl_retcode alislsto_mtd_close(alisl_handle handle);

alisl_retcode alislsto_lock_read(alislsto_rw_param_t para,
		unsigned char *buf, size_t *actual_size);

alisl_retcode alislsto_lock_write(alislsto_rw_param_t para,
		unsigned char *buf, size_t *actual_size);

alisl_retcode alislsto_get_phy_addr(alisl_handle handle,
		loff_t offset_logic,
		loff_t *offset_phy);

alisl_retcode alislsto_get_logic_addr(alisl_handle handle,
		loff_t offset_phy,
		loff_t *offset_logic);

int alislsto_lseek_logic(alisl_handle handle, int offset, int whenence);

alisl_retcode alislsto_get_offset(alisl_handle handle, loff_t *offset);/*for bug #56342, MUST use loff_t instead of off_t.*/

alisl_retcode alislsto_set_offset(alisl_handle handle, loff_t offset, bool flag);/*for bug #56342, MUST use loff_t instead of off_t.*/

alisl_retcode alislsto_islock_nor(alisl_handle handle, loff_t start, size_t length, unsigned long* lock);

alisl_retcode alislsto_lock_nor(alisl_handle handle, int lock, loff_t start, size_t length);


#ifdef __cplusplus
}
#endif

#endif
