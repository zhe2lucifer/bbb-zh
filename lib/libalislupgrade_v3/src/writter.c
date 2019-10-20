/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               writter.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:21:45
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "internal.h"

/*
 * The loop to write data to flash
 */
static void *upgrade_writter_loop(void *desc_in)
{
	alislupg_desc_t *desc = (alislupg_desc_t *)desc_in;
	upg_tune_t *tune = desc->tune;
	upgrade_writter_t *writter = desc->writter;
	upg_header_t *hdr = desc->header;
	upg_elem_t *elem = NULL;
	static upg_elem_t *elem_old = NULL;
	int  percent = 0;
	alislupg_elemlevel_t level = UPG_LEVEL_APPLICATION;
	upgstat_t stat;
	int ret = 0;
	int sourcesem_val = 0;
	off_t orig_offset = 0;
	unsigned char *buf = NULL;
	alisl_retcode err = STO_ERR_NONE;
	size_t writed_size = 0;
	alislsto_rw_param_t para;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	/* initial upgrade percentage */
	desc->percent = 0;
	alislsto_open(&desc->nand, STO_TYPE_NAND, O_RDWR);

	while (1)
	{
		while (1)
		{
			ret = sem_getvalue(&tune->sourcesem, &sourcesem_val);

			if (sourcesem_val > 0)
			{
				sem_wait(&tune->sourcesem);
				break;
			}

			usleep(1000);
		}

		if (ALISLUPG_ERR_NONE != desc->error)
		{
			SL_DBG("[%s(%d)]desc->error=%d\n", __FUNCTION__, __LINE__, desc->error);

			/*
			* Notes:wakes up thread that is waiting on msg_cond,
			* otherwise wait thread will hang up
			*/
			pthread_mutex_lock(&desc->msg_mutex);
			pthread_cond_signal(&desc->msg_cond);
			pthread_mutex_unlock(&desc->msg_mutex);
			break;
		}

		/*SL_DBG("part id %d "
				 "current offset 0x%x\n", \
				  elem->part_id, tune->curoffset); */

		writter->buf = tune->dest;
		elem = tune->elem;
#if UPGRADE_BUILD_CRYPTO
		/* TODO: Verify signature for elem signed by image */
#endif
		SL_DBG("Writter perwrite 0x%x "
		               "total 0x%x percent %d part id %d "
		               "current offset 0x%x\n", \
		               tune->size, \
		               tune->total, desc->percent, \
		               elem->part_id, tune->curoffset);

		/* lock seekmutex until alislsto_write finish */
		//alislsto_get_offset(desc->nand, &orig_offset);

		/* erase partition first */
		if (elem != elem_old)
		{
			SL_DBG("erase part id %d from:0x%x, len:0x%x\n", \
			               elem->part_id, elem->dstoffset, elem->partsz);
			alislsto_erase(desc->nand, elem->dstoffset, elem->partsz);

			usleep(1000);

			buf = writter->buf;
			if (!hdr->mono)
			{
				if (elem->imgsz < alislsto_tell_ext(desc->nand, elem->part_id, STO_PART_SIZE))
				{
					//alislsto_lseek_ext(desc->nand, elem->part_id, tune->curoffset);
					para.handle = desc->nand;
					para.size = tune->size;
					para.offset = tune->curoffset;
					para.whenence = -1;
					para.idx = elem->part_id;
					para.flag = false;
					alislsto_lock_write(para, writter->buf, &writed_size);
				}
			}
			else
			{
				/* It's a mono upgrade */
				SL_DBG("seek to part id %d start:0x%x\n", elem->part_id, elem->dstoffset);
				//alislsto_lseek_ext(desc->nand, STO_PRIVATE, elem->dstoffset);
				para.handle = desc->nand;
				para.size = tune->size;
				para.offset = elem->dstoffset;
				para.whenence = -1;
				para.idx = STO_PRIVATE;
				para.flag = false;
				alislsto_lock_write(para, writter->buf, &writed_size);
			}

			elem_old = elem;
		} else {
			para.handle = desc->nand;
			para.size = tune->size;
			para.offset = 0;
			para.whenence = SEEK_CUR;
			para.idx = -1;
			para.flag = false;
			alislsto_lock_write(para, writter->buf, &writed_size);
		}

		//buf = writter->buf;
		//alislsto_write(desc->nand, writter->buf, tune->size);

		/*
		 * because offset of system info will be assigned,
		 * so restore original offset is unnecessary before
		 * read/write system info.
		 */
		//alislsto_set_offset(desc->nand, orig_offset, false);

		usleep(1000);

#if UPGRADE_BUILD_CRYPTO
		/* TODO: Verify signature for elem signed by file */
#endif

		if ((desc->percent - percent >= desc->progress_scale)
		    || (100 == desc->percent) || (ALISLUPG_ERR_NONE != desc->error))
		{
			pthread_mutex_lock(&desc->msg_mutex);
			pthread_cond_signal(&desc->msg_cond);
			pthread_mutex_unlock(&desc->msg_mutex);
			percent = desc->percent;
		}

		level = elem->level;
		sem_post(&tune->writersem);

		if (!tune->writting || (100 == desc->percent))
		{
			break;
		}
	}

	/*
	 * For a mono upgrade, the partition table
	 * should be updated to the new one
	 */
	if (ALISLUPG_ERR_NONE == desc->error)
	{
		/* Currently, we write pmi data into first block */
		upgrade_parse_updatepmi(desc);
	}

	/*
	 * modify state machine when upgrade uboot
	 * if current uboot is 0, set cur_boot to 1;
	 * if current uboot is 1, set cur_boot to 0;
	 */
	if (hdr->ubootfile_imgsz > 0)
	{
		SL_DBG("modify state machine for uboot boot\n");
		alislupg_upginfo_read((unsigned char *)&stat, sizeof(upgstat_t));
		stat.cur_uboot = stat.cur_uboot ? 0 : 1;
		alislupg_upginfo_write((unsigned char *)&stat, sizeof(upgstat_t));
	}

	alislsto_close(desc->nand, true);

	sem_post(&desc->oversem);

	return;
}


/**
 *  Function Name:  upgrade_writter_start
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_writter_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	upgrade_writter_t *writter = desc->writter;

	pthread_create(&writter->writter, NULL, upgrade_writter_loop, desc);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_writter_stop
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 *
 */
alisl_retcode upgrade_writter_stop(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;

	return ALISLUPG_ERR_NONE;
}
