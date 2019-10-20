/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               aliota.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:09:37
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#include <string.h>
#include <alipltfretcode.h>
#include <alislupgrade.h>
#include <upgrade_object.h>

#include "lib_ota_dbc.h"
#include "lib_ota.h"
#include "aliota.h"
#include "../../internal.h"

static otaupg_object_t object =
{
	.name = UPG_OBJ_NAME,
};

static unsigned int ota_pid = 0;
static struct dl_info ota_dl_info;
static void* ota_buf = NULL; /* global buf for ota data download */
static char* p_ota_buf = NULL; /* global buf point to ota buf for upgrade process */
alislupg_desc_t* p_desc = NULL; /* point to desc */

static void ota_save_buff_2_usb()
{
	SL_DBG("start\n");

	char file_name[128] = {0};
	FILE *file;
	int  ret;

	strcpy(file_name, "/mnt/usb/sda1/ota_buff.bin");
	remove(file_name);
	sync();

	file = fopen(file_name, "w");

	if (NULL == file)
	{
		SL_DBG("open file fail\n");
		return;
	}

	ret = fwrite(ota_buf, ota_dl_info.sw_size, 1, file);

	if (!ret)
	{
		fclose(file);
		remove(file_name);
		sync();
		SL_DBG("save ota buff fail\n");
		return;
	}

	sync();
	fclose(file);

	SL_DBG("end\n");
}

static void ota_file_set_desc(alislupg_desc_t* desc)
{
	p_desc = desc;
	return;
}

static alislupg_desc_t* ota_file_get_desc()
{
	return p_desc;
}

static void ota_file_progress_callback(unsigned long percent)
{
	static unsigned long percent_old = 0;
	alislupg_desc_t* desc;

	if (percent > 100)
		percent = 100;

	if (percent_old != percent)
	{
		percent_old = percent;

		desc = ota_file_get_desc();
		desc->percent = percent;
		pthread_mutex_lock(&desc->msg_mutex);
		pthread_cond_signal(&desc->msg_cond);
		pthread_mutex_unlock(&desc->msg_mutex);
	}

	return;
}

static int ota_file_open()
{
	if (NULL == ota_buf)
	{
		return ALISLUPG_ERR_OTHER;
	}

	p_ota_buf = ota_buf;

	return ALISLUPG_ERR_NONE;
}

static int ota_file_close()
{
	return ALISLUPG_ERR_NONE;
}

static int ota_file_seek(int offset, int where)
{
	int cur_offset = 0;
	int ret_len = 0;

	switch (where)
	{
		case SEEK_SET:
			p_ota_buf = ota_buf + offset;
			break;

		case SEEK_CUR:
			p_ota_buf += offset;
			break;

		default:
			SL_ERR("Not support seek mode\n", __FUNCTION__, __LINE__);
			break;
	}

	return ALISLUPG_ERR_NONE;
}

static int ota_file_read(char* ptr, int size)
{
	memcpy(ptr, p_ota_buf, size);

	return size;
}

static int ota_file_get_size()
{
	return ota_dl_info.sw_size;
}

static int ota_file_download(int ota_pid)
{
	unsigned long mapmem_len = 0;

	SL_DBG("ota_pid=%d\n", ota_pid);

	if (OTA_SUCCESS != alislupg_ota_get_download_info(ota_pid, &ota_dl_info))
	{
		SL_ERR("[%s(%d)] ota get downloade info fail\n", __FUNCTION__, __LINE__);
		return ALISLUPG_ERR_OTHER;
	}

	SL_DBG("get download info:\nhw_version=%d, sw_version=%d, sw_size=%d\n",
	               ota_dl_info.hw_version, ota_dl_info.sw_version, ota_dl_info.sw_size);

	/* 128K align */
	mapmem_len = DATA_ALIGN(ota_dl_info.sw_size, 0x20000);
	ota_buf = NULL;
	SL_DBG("malloc  mapmem_len=0x%x\n", mapmem_len);
	ota_buf = malloc(mapmem_len);

	if (ota_buf == NULL)
	{
		SL_ERR("malloc 0x%x failed!\n", mapmem_len);
		return ALISLUPG_ERR_OTHER;
	}

	memset(ota_buf, 0xFF, mapmem_len);
	alislupg_ota_mem_config(ota_buf, mapmem_len);

	if (OTA_SUCCESS != alislupg_ota_start_download(ota_pid, ota_file_progress_callback))
	{
		SL_ERR("[%s(%d)] ota downloade fail\n", __FUNCTION__, __LINE__);
		return ALISLUPG_ERR_OTHER;
	}

	/* save ota buff to usb for test */
	ota_save_buff_2_usb();

	return ALISLUPG_ERR_NONE;
}

static int ota_file_download_thread(alislupg_desc_t *desc)
{
	int pid = 0;
	int ret = 0;
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;

	/**
	 * download ota file
	 * for ota, config is the PID in Decimal string format
	 * eg: config = "256" means pid = 256
	 */
	if (0 == strlen(obj->config))
	{
		return 0;
	}

	ota_file_set_desc(desc);
	pid = atoi(obj->config);
	ret = ota_file_download(pid);

	if (ALISLUPG_ERR_NONE != ret)
	{
		return -EINVAL;;
	}

	return ALISLUPG_ERR_NONE;

}

static int ota_file_download_abort(alislupg_desc_t *desc)
{
	return alislupg_ota_download_abort();
}

static int ota_file_get_config(char *buf, int len)
{
	int ret_len = 0;
	int per_size = 10240;
	pthread_t url_handle = 0;
	int try_time = 0;

	if (ALISLUPG_ERR_NONE != ota_file_open())
	{
		SL_ERR("[%s](%d) ota file open fail\n", __FUNCTION__, __LINE__);
		return;
	}

	ret_len = ota_file_read(buf, len);

	return ret_len;
}

static void upgrade_obj_loop(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;
	upg_header_t *hdr = desc->header;
	upg_elem_t *elem = hdr->elem;
	FILE *upgpkt = NULL;
	size_t pktsz = 0;
	size_t totalread = obj->upgdata_offset;
	size_t perread = UPG_BUF_SIZE;
	size_t elemread = 0;
	int try_time = 0;
	int ret = 0;
	int writersem_val = 0;
	int ret_len = 0;
#if UPGRADE_BUILD_CRYPTO
	void *buf = NULL;
#endif

	if (NULL == elem)
		return;

	if (ALISLUPG_ERR_NONE != ota_file_open())
	{
		SL_ERR("[%s](%d) ota file open fail\n", __FUNCTION__, __LINE__);
		desc->error = ALISLUPG_ERR_OTHER;
		sem_post(&tune->sourcesem);
		return;
	}

	pktsz = ota_file_get_size();
	SL_DBG("pktsz=%d, readoffset=0x%x\n", pktsz, totalread);

	if (ALISLUPG_ERR_NONE != ota_file_seek(obj->upgdata_offset, SEEK_SET))
	{
		SL_DBG("can not seek to upgdata offset\n");
		desc->error = ALISLUPG_ERR_OTHER;
		sem_post(&tune->sourcesem);
		goto over;
	}

	tune->total = pktsz;
	tune->writting = true;

	while (totalread < pktsz && NULL != elem)
	{
		if (0 == elem->imgsz)
		{
			totalread += elem->size;
			elem = elem->next;
			continue;
		}

		while (1)
		{
			ret = sem_getvalue(&tune->writersem, &writersem_val);

			if (writersem_val > 0)
			{
				sem_wait(&tune->writersem);
				break;
			}

			usleep(1000);
		}

		/**
		 * It safe to quit, due to the error is set by writter
		 * If the error hanppen, the writter should be already
		 * finish at that time
		 */

		if (ALISLUPG_ERR_NONE != desc->error)
			break;

		tune->elem = elem;
		tune->curoffset = elemread;

		if (elem->imgsz < UPG_BUF_SIZE)
			perread = elem->imgsz;

#if UPGRADE_BUILD_CRYPTO

		if (NULL != buf)
		{
			free(tune->dest);
			tune->dest = buf;
			buf = NULL;
		}

		if (HWCRYPTO_SIGN_BYIMG == elem->sign_method)
		{
			perread = elem->imgsz;

			if (NULL == buf)
			{
				buf = tune->dest;
			}

			tune->dest = malloc(perread);
			memset(tune->dest, 0, perread);
		}

#endif
		obj->buf = tune->dest;

		/* Get a segment from upgrade server */
		if (ALISLUPG_ERR_NONE != ota_file_seek(elem->offset + elemread, SEEK_SET))
		{
			SL_DBG("seek data fail\n");
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}

		ret_len = ota_file_read(obj->buf, perread);

		if (ret_len <= 0)
		{
			SL_DBG("read data fail\n");
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}

		elemread += perread;
		tune->size = perread;
		totalread += perread;
		SL_DBG("Source perread 0x%x "
		               "total 0x%x 0x%x 0x%x elem size 0x%x 0x%x "
		               "elemread 0x%x, elem offset 0x%x part id %d, "
		               "current offset 0x%x\n", \
		               perread, \
		               totalread, pktsz, \
		               tune->total, elem->size, \
		               elem->imgsz, elemread, \
		               elem->offset, elem->part_id, \
		               tune->curoffset);

		if (elem->imgsz - elemread < UPG_BUF_SIZE)
		{
			perread = elem->imgsz - elemread;
		}

		if (0 == perread)
		{
			totalread += elem->size - elem->imgsz;
			elem = elem->next;
			elemread = 0;
			perread = UPG_BUF_SIZE;
		}

		desc->percent = (totalread) / (tune->total / 100);

		if (totalread == tune->total)
		{
			//in case (totalread) / (tune->total/100) not equal 100
			desc->percent = 100;
		}

		SL_DBG("Source read percent %d\n",desc->percent);

		sem_post(&tune->sourcesem);
	}

over:

#if UPGRADE_BUILD_CRYPTO

	if (NULL != buf)
	{
		free(tune->dest);
		tune->dest = buf;
	}

#endif

	tune->writting = false;

	ota_file_close();

	SL_DBG("End\n");
	return;
}

static alisl_retcode upgrade_obj_prestart(alislupg_desc_t *desc)
{
	pthread_t download_tid;
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;

	pthread_create(&download_tid, NULL, (void*)ota_file_download_thread, desc);

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_prestop(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;

	SL_DBG("desc->error=%d\n", desc->error);

	ota_file_download_abort(desc);

	desc->error = ALISLUPG_ERR_OTHER;
	pthread_mutex_lock(&desc->msg_mutex);
	pthread_cond_signal(&desc->msg_cond);
	pthread_mutex_unlock(&desc->msg_mutex);

	return ALISLUPG_ERR_NONE;
}


static alisl_retcode upgrade_obj_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;

	if (NULL == tune->dest)
	{
		tune->dest = malloc(UPG_BUF_SIZE);
		if (tune->dest) {
			memset(tune->dest, 0, UPG_BUF_SIZE);
		} else {
			return ALISLUPG_ERR_OTHER;
		}
	}

	pthread_create(&obj->source, NULL, (void*)upgrade_obj_loop, desc);

	return ALISLUPG_ERR_NONE;
}


static alisl_retcode upgrade_obj_stop(alislupg_desc_t *desc)
{
	upg_tune_t *tune = desc->tune;

	if (NULL != tune->dest)
	{
		free(tune->dest);
	}

	tune->dest = NULL;

	return ALISLUPG_ERR_NONE;
}

static size_t upgrade_obj_getcfg(alislupg_desc_t *desc, unsigned char *buf, size_t max)
{
	int             size = 0;
	upg_object_t    *obj = (upg_object_t *)desc->object;
	otaupg_object_t *object = (otaupg_object_t *)obj->plugin;
	int ret = 0;
	int upgfile_len = 0;

	/* get config from ota file */
	size = UPG_CONFIGV1_SIZE;
	size = ota_file_get_config(buf, size);

	return size;
}

static alisl_retcode upgrade_obj_setdataoffset(alislupg_desc_t *desc, off_t offset)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	otaupg_object_t *obj = object->plugin;

	obj->upgdata_offset = offset;

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_getdataoffset(alislupg_desc_t *desc)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	otaupg_object_t *obj = object->plugin;

	return obj->upgdata_offset;
}


static alisl_retcode upgrade_obj_setpktname(alislupg_desc_t *desc, char *name)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	otaupg_object_t *obj = object->plugin;

	strcpy(obj->pktname, name);

	return ALISLUPG_ERR_NONE;
}

/**
 *  Function Name:  upgrade_obj_register
 *  @brief
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          param  upgrade parameters
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-26
 */
alisl_retcode upgrade_obj_register(alislupg_desc_t *desc)
{
	upg_object_t *obj = (upg_object_t *)desc->object;

	if (UPG_OTA == desc->source)
	{
		strcpy(object.config, desc->config);
		obj->plugin = &object;
		obj->f_getcfg = upgrade_obj_getcfg;
		obj->f_prestart = upgrade_obj_prestart;
		obj->f_prestop = upgrade_obj_prestop;
		obj->f_start = upgrade_obj_start;
		obj->f_stop = upgrade_obj_stop;
		obj->f_setpktname = upgrade_obj_setpktname;
		obj->f_setupgdata_offset = upgrade_obj_setdataoffset;
		obj->f_getupgdata_offset = upgrade_obj_getdataoffset;
	}

	SL_DBG("Object %s got\n", object.name);

	desc->percent = 0;

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_obj_unregister
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
alisl_retcode upgrade_obj_unregister(alislupg_desc_t *desc, alislupg_param_t *param)
{
	if (UPG_OTA == desc->source)
	{
		desc->object = NULL;
	}

	return ALISLUPG_ERR_NONE;
}

