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

/* system headers */
//#include <ali_soc_common.h>

/* share library headers */
#include <alipltfretcode.h>
//#include <alislsoc.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "internal.h"

static bool is_pmi_bootrom()
{
#if 0
	bool ret = FALSE;
	unsigned int chipid, revid, is_as;
	alislsoc_op_ioctl(ALISLSOC_CMD_CHIPID,(void *)&chipid);
	alislsoc_op_ioctl(ALISLSOC_CMD_REVID,(void *)&revid);
	alislsoc_op_ioctl(ALISLSOC_CMD_SPLENABLE,(void *)&is_as);

	SL_DBG("chipid=0x%x,revid=0x%x,is_as=%d\n",
	       __FUNCTION__, __LINE__, chipid, revid, is_as);

	switch(chipid)
	{
		case ALI_C3701:
			ret = TRUE;
			break;

		case ALI_S3503:
			if (revid < IC_REV_2)	//C3503A
			ret = TRUE;
			break;

		case ALI_S3821:
			if (!is_as)		//C3821 non-AS
				ret = TRUE;
			break;

		default:
			ret = FALSE;
			break;
	}

	SL_DBG("ret=%d\n", ret);
#endif
	return 0;
}

/*
 * The loop to write data to flash
 */
static void * upgrade_writter_loop(void * desc_in)
{
	alislupg_desc_t *desc = (alislupg_desc_t *)desc_in;
	upg_tune_t *tune = desc->tune;
	upgrade_writter_t *writter = desc->writter;
	upg_elem_t *elem = NULL;
	static upg_elem_t *elem_old = NULL;
	int  percent = 0;
	int ret = 0;
	int sourcesem_val = 0;
//	off_t orig_offset = 0;
	alisl_handle mtd_handle = NULL;
	unsigned char *boot_param = NULL;
	int i = 0;
	size_t part_size;
	size_t totalwrite = 0;
	bool pmi_bootrom = 0;
	alislsto_param_t stoparam;
	size_t page_size = 0;
	unsigned char *pmi_tmp_buf = NULL;
	bool updating_pmi_data = 0;

	memset(&stoparam, 0, sizeof(stoparam));
	pmi_bootrom = is_pmi_bootrom();

	/* initial upgrade percentage */
	desc->percent = 0;

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
			SL_DBG("desc->error=%d\n", desc->error);

			/*
			* Notes:wakes up thread that is waiting on msg_cond,
			* otherwise wait thread will hang up
			*/
			pthread_mutex_lock(&desc->msg_mutex);
			pthread_cond_signal(&desc->msg_cond);
			pthread_mutex_unlock(&desc->msg_mutex);
			break;
		}

		writter->buf = tune->buf;
		elem = tune->elem;
		SL_DBG("Writter perwrite(0x%x) total(0x%x) percent(%d)\n", \
		         tune->size, tune->total, desc->percent);
		SL_DBG("part name : %s\n", elem->part_name);

		/* erase partition first */
		if (elem != elem_old)
		{
			SL_DBG("erase part name %s\n", elem->part_name);
			/* Need to open and erase the specify partition here */
			ret = alislsto_mtd_open_by_name(&mtd_handle, elem->part_name, O_RDWR);
			if (ret != 0)
			{
				SL_ERR("%s open mtd device fail, ret = %d\n", __FUNCTION__, ret);
				desc->error = ALISLUPG_ERR_OTHER;
				pthread_mutex_lock(&desc->msg_mutex);
				pthread_cond_signal(&desc->msg_cond);
				pthread_mutex_unlock(&desc->msg_mutex);

				break;
			}

			/*
			 * boot partition(with PMI): backup PMI data to buffer before write
			 *                           note: there are 4 PMIs, each PMI have it's data to prevent bad data
			 * boot partition(w/o  PMI): backup the boot param to the buffer before write
			 * other partition:          erase the whole partiton to rewirte all the data
			 */
			if ((strcmp(elem->part_name, BOOT_NAME) == 0) ||
				(strcmp(elem->part_name,BOOT_NAME_BAK) == 0))
			{
				if(pmi_bootrom)
				{
					alislsto_get_param(mtd_handle, &stoparam);
					page_size = stoparam.info->writesize;
					SL_DBG("page_size=0x%x\n", page_size);

					if(tune->size > (PAGES_OF_EACH_PMI*page_size - PMI_DATA_SIZE))
					{
						SL_ERR("%s tune->size too large for boot partition with PMI\n", __FUNCTION__);
						desc->error = ALISLUPG_ERR_OTHER;
						pthread_mutex_lock(&desc->msg_mutex);
						pthread_cond_signal(&desc->msg_cond);
						pthread_mutex_unlock(&desc->msg_mutex);
						break;
					}

					pmi_tmp_buf = malloc(PMI_DATA_SIZE*TOTAL_PMI_NUM);
					if (NULL == pmi_tmp_buf)
					{
						SL_ERR("%s malloc buffer fail\n", __FUNCTION__);
						desc->error = ALISLUPG_ERR_OTHER;
						pthread_mutex_lock(&desc->msg_mutex);
						pthread_cond_signal(&desc->msg_cond);
						pthread_mutex_unlock(&desc->msg_mutex);
						break;
					}
					memset(pmi_tmp_buf, 0, PMI_DATA_SIZE*TOTAL_PMI_NUM);
					for(i=0; i<TOTAL_PMI_NUM; i++)
					{
						alislsto_lseek(mtd_handle, PAGES_OF_EACH_PMI*page_size*i, SEEK_SET);
						alislsto_read(mtd_handle, pmi_tmp_buf+PMI_DATA_SIZE*i, PMI_DATA_SIZE);
					}
					updating_pmi_data = 1;
				}
				else
				{
					/*
					 * tune data should larger than SCS_HASH_LEVEL1_LEN+SCS_SIG_LEN+BOOT_PARAM_TOTAL_SIZE,
					 * so that it can backup the BOOT_PARAM_TOTAL_SIZE to the tune data
					 */
					if(tune->size < (SCS_HASH_LEVEL1_LEN+SCS_SIG_LEN+BOOT_PARAM_TOTAL_SIZE))
					{
						SL_ERR("%s malloc buffer fail\n", __FUNCTION__);
						desc->error = ALISLUPG_ERR_OTHER;
						pthread_mutex_lock(&desc->msg_mutex);
						pthread_cond_signal(&desc->msg_cond);
						pthread_mutex_unlock(&desc->msg_mutex);
						break;
					}
					boot_param = malloc(BOOT_PARAM_TOTAL_SIZE);
					if(NULL == boot_param)
					{
						SL_ERR("%s malloc buffer fail\n", __FUNCTION__);
						desc->error = ALISLUPG_ERR_OTHER;
						pthread_mutex_lock(&desc->msg_mutex);
						pthread_cond_signal(&desc->msg_cond);
						pthread_mutex_unlock(&desc->msg_mutex);
						break;
					}
					memset(boot_param, 0, BOOT_PARAM_TOTAL_SIZE);
					alislsto_lseek(mtd_handle, SCS_HASH_LEVEL1_LEN + SCS_SIG_LEN, SEEK_SET);
					alislsto_read(mtd_handle, boot_param, BOOT_PARAM_TOTAL_SIZE);
					memcpy(&(((uint8_t *)writter->buf)[SCS_HASH_LEVEL1_LEN + SCS_SIG_LEN]), boot_param, BOOT_PARAM_TOTAL_SIZE);
					free(boot_param);
					boot_param = NULL;
					alislsto_lseek(mtd_handle, 0, SEEK_SET);
				}
			}

			/* 
			 * erase the partition.
			 * sflash(nor) should not erase all data of the partition, because 
			 * evn is stored in the back of the sflash for some solutions.
			 */
			if(strcmp(elem->part_name, SFLASH_NAME) == 0)
			{	
				alislsto_erase(mtd_handle, 0, elem->img_size);
				alislsto_lseek(mtd_handle, 0, SEEK_SET);
			}
			else
			{
				alislsto_get_partsize(elem->part_name, &part_size);
				alislsto_erase(mtd_handle, 0, part_size);
				alislsto_lseek(mtd_handle, 0, SEEK_SET);
			}

			elem_old = elem;
		}

		if(updating_pmi_data)
		{
			for(i=0; i<TOTAL_PMI_NUM; i++)
			{
				alislsto_lseek(mtd_handle, PAGES_OF_EACH_PMI*page_size*i, SEEK_SET);
				alislsto_write(mtd_handle, pmi_tmp_buf+PMI_DATA_SIZE*i, PMI_DATA_SIZE);
				alislsto_write(mtd_handle, writter->buf, tune->size);
			}
			free(pmi_tmp_buf);
			pmi_tmp_buf = NULL;
			updating_pmi_data = 0;
		}
		else
		{
			alislsto_write(mtd_handle, writter->buf, tune->size);
		}

		totalwrite += tune->size;
		if (totalwrite >= tune->total)
		{
			desc->percent = 100;
		}
		else
		{
			desc->percent = (totalwrite) / (tune->total/100);
		}
		SL_DBG("Source write percent %d\n", desc->percent);

		if ((desc->percent - percent >= desc->progress_scale)
		    || (100 == desc->percent) || (ALISLUPG_ERR_NONE != desc->error))
		{
			pthread_mutex_lock(&desc->msg_mutex);
			pthread_cond_signal(&desc->msg_cond);
			pthread_mutex_unlock(&desc->msg_mutex);
			percent = desc->percent;
		}

		sem_post(&tune->writersem);

		if ((100 == desc->percent) || (ALISLUPG_ERR_NONE != desc->error))
		{
			break;
		}
	}

	alislsto_mtd_close(mtd_handle);

	sem_post(&desc->oversem);

	if(NULL != boot_param)
	{
		free(boot_param);
		boot_param = NULL;
	}
	if(NULL != pmi_tmp_buf)
	{
		free(pmi_tmp_buf);
		pmi_tmp_buf = NULL;
	}

	return 0;
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
	//upg_object_t *object = desc->object;
	upgrade_writter_t *writter = desc->writter;

	pthread_create(&writter->tid, NULL, upgrade_writter_loop, desc);

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
	//upg_object_t *object = desc->object;

	return ALISLUPG_ERR_NONE;
}
