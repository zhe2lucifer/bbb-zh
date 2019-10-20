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
#include <alipltflog.h>
#include <alislupgrade.h>
#include <upgrade_object.h>

#include "lib_ota_dbc.h"
#include "lib_ota.h"
#include "aliota.h"
#include "../../internal.h"
#include "../../fastCRC.h"

//#define OTA_DEBUG


static otaupg_object_t object =
{
	.name = UPG_OBJ_NAME,
};

//static unsigned int ota_pid = 0;
static struct dl_info ota_dl_info;
static void* ota_buf = NULL; /* global buf for ota data download */
static char* p_ota_buf = NULL; /* global buf point to ota buf for upgrade process */
static char* p_ota_valid = NULL; /* global buf point, using in PIECE BY PIECE mode */
static sem_t piece_read_sem; /* global sem, using in PIECE BY PIECE mode */
static sem_t piece_source_sem; /* global sem, using in PIECE BY PIECE mode */
alislupg_desc_t* p_desc = NULL; /* point to desc */
static unsigned long g_piece_size = 0; /* global param for save cuurent piece size */

#ifdef OTA_DEBUG
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

static void ota_save_buff_2_file(int size)
{
	SL_DBG("start\n");

	char file_name[128] = {0};
	FILE *file;
	int  ret;

	strcpy(file_name, "/mnt/ota_buff.bin");
	//remove(file_name);
	sync();

	file = fopen(file_name, "a+");

	if (NULL == file)
	{
		SL_DBG("open file fail\n");
		return;
	}

	ret = fwrite(ota_buf, size, 1, file);

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
#endif

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



//static int ota_file_seek(int offset, int where)
//{
//	int cur_offset = 0;
//	int ret_len = 0;
//
//	SL_DBG("p_ota_valid = %p offset = %d\n", p_ota_valid, offset);
//
//	if (p_ota_valid != NULL)	
//	{
//		return ALISLUPG_ERR_NONE;
//	}
//
//	switch (where)
//	{
//		case SEEK_SET:
//			p_ota_buf = ota_buf + offset;
//			break;
//
//		case SEEK_CUR:
//			p_ota_buf += offset;
//			break;
//
//		default:
//			SL_ERR("Not support seek mode\n");
//			break;
//	}
//
//	return ALISLUPG_ERR_NONE;
//}

static int ota_file_read(char* ptr, int size)
{
	int remain_buf_size = 0;
	int remain_read_size = 0;
	int piece_source_sem_val = 0;
	char* r_ptr = ptr;

	if (p_ota_valid != NULL)	
	{
		remain_buf_size = p_ota_valid - p_ota_buf;
		remain_read_size = size;

		SL_DBG("ota_buf = %p, p_ota_buf = %p p_ota_valid = %p remain_buf_size=%d size=%d\n",
			ota_buf, p_ota_buf, p_ota_valid, remain_buf_size, size);

		if ((remain_buf_size < 0) || (remain_buf_size > g_piece_size))
		{
			SL_ERR("Invald p_ota_valid value, please check it! \n");
			return 0;	
		}

		while(remain_read_size > 0 && p_ota_buf != NULL)
		{	
			if (remain_buf_size >= remain_read_size)
			{
				memcpy(r_ptr, p_ota_buf, remain_read_size);
				p_ota_buf += remain_read_size;
				remain_buf_size -= remain_read_size;
				break;
			}
			
			memcpy(r_ptr, p_ota_buf, remain_buf_size);
			remain_read_size -= remain_buf_size;
			r_ptr += remain_buf_size;

			sem_post(&piece_read_sem);
			while (1)
			{
				sem_getvalue(&piece_source_sem, &piece_source_sem_val);

				if (piece_source_sem_val > 0)
				{
					sem_wait(&piece_source_sem);
					break;
				}

				usleep(1000);
			}

			if( NULL == p_ota_valid )
			{
				SL_ERR("ota read download data fail\n");
				return -1;
			}

			p_ota_buf = ota_buf;
			remain_buf_size = p_ota_valid - p_ota_buf;
			

		}

		SL_DBG("ota_buf = %p, p_ota_buf = %p p_ota_valid = %p remain_buf_size=%d size=%d\n",
			ota_buf, p_ota_buf, p_ota_valid, remain_buf_size, size);
			
	}
	else
	{
		SL_DBG("size = %d\n", size);
		memcpy(ptr, p_ota_buf, size);
	}

	return size;
}

static void	ota_file_skip_head()
{
	int piece_source_sem_val = 0;
	alislupg_header_t head;

	SL_DBG("ota_buf = %p, p_ota_buf = %p p_ota_valid = %p\n",
		ota_buf, p_ota_buf, p_ota_valid);
	while (1)
	{
		sem_getvalue(&piece_source_sem, &piece_source_sem_val);

		if (piece_source_sem_val > 0)
		{
			sem_wait(&piece_source_sem);
			break;
		}

		usleep(1000);
	}

	ota_file_open();
	SL_DBG("ota_buf = %p, p_ota_buf = %p p_ota_valid = %p\n",
		ota_buf, p_ota_buf, p_ota_valid);
	ota_file_read((char*)&head, sizeof(head));
	SL_DBG("ota_buf = %p, p_ota_buf = %p p_ota_valid = %p\n",
	       ota_buf, p_ota_buf, p_ota_valid);
}

static int ota_file_get_size(alislupg_desc_t *desc)
{
	unsigned long size;

	unsigned long mapmem_len = 0;
	unsigned short piece_num = 0;
	struct modules_range mdl_range;
	void* buf = NULL;

	if(g_piece_size == 0)
	{
		g_piece_size = desc->package_header.ota_down_buf_size * 1024 * 1024;
	}
	
	/* 128K align */
	mapmem_len = DATA_ALIGN(g_piece_size, 0x20000);
	buf = malloc(mapmem_len);

	if (buf == NULL)
	{
		SL_ERR("malloc 0x%x failed!\n", mapmem_len);
		return ALISLUPG_ERR_OTHER;
	}

	memset(buf, 0xFF, mapmem_len);

	alislupg_ota_mem_config_by_piece(buf, mapmem_len, &piece_num, &mdl_range);

	SL_ERR("ota_dl_info.sw_size %d, piece_num %d\n", ota_dl_info.sw_size, piece_num);
	/* We need remove the CRC size for every piece */
	size = ota_dl_info.sw_size - (4 * piece_num);
	SL_ERR("size=0x%x \n", size);

	free(buf);

	return size;
}
#if 0
static int ota_file_download_one_shot(int ota_pid)
{
	unsigned long mapmem_len = 0;
	struct modules_range mdl_range;

	SL_DBG("ota_pid=%d\n", ota_pid);

	if (OTA_SUCCESS != alislupg_ota_get_download_info(ota_pid, &ota_dl_info))
	{
		SL_ERR("ota get downloade info fail\n");
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
	alislupg_ota_mem_config_one_shot(ota_buf, mapmem_len, &mdl_range);
	
	if (OTA_SUCCESS != alislupg_ota_start_download_one_shot(ota_pid, &mdl_range, ota_file_progress_callback))
	{
		SL_ERR("ota downloade fail\n");
		return ALISLUPG_ERR_OTHER;
	}

#ifdef OTA_DEBUG
	/* save ota buff to usb for test */
	ota_save_buff_2_usb();
#endif

	return ALISLUPG_ERR_NONE;
}
#endif
static int ota_file_get_config(alislupg_desc_t *desc, char *buf, int len)
{
	unsigned long mapmem_len = 0;
	unsigned short piece_num = 0;
	struct modules_range mdl_range;
	int ota_pid;
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
	ota_pid = atoi(obj->config);

	memset(&mdl_range, 0, sizeof(mdl_range));

	SL_DBG("ota_pid=%d\n", ota_pid);

	if (OTA_SUCCESS != alislupg_ota_get_download_info(ota_pid, &ota_dl_info))
	{
		SL_ERR("ota get downloade info fail\n");
		return ALISLUPG_ERR_OTHER;
	}

	SL_DBG("get download info:\nhw_version=%d, sw_version=%d, sw_size=%d\n",
			ota_dl_info.hw_version, ota_dl_info.sw_version, ota_dl_info.sw_size);

	/* 128K align */
	mapmem_len = DATA_ALIGN(FIRST_PIECE_SIZE, 0x20000);
	ota_buf = NULL;
	SL_DBG("malloc  mapmem_len=0x%x\n", mapmem_len);
	ota_buf = malloc(mapmem_len);
	if (ota_buf == NULL)
	{
		SL_ERR("malloc 0x%x failed!\n", mapmem_len);
		return ALISLUPG_ERR_OTHER;
	}
	memset(ota_buf, 0xFF, mapmem_len);

	alislupg_ota_mem_config_by_piece(ota_buf, mapmem_len, &piece_num, &mdl_range);

	mdl_range.start = 0;
	mdl_range.end = mdl_range.len - 1;
	SL_DBG("OTA Debug mdl_range.start=%d, mdl_range.end=%d, mdl_range.len=%d\n",
	       mdl_range.start, mdl_range.end, mdl_range.len);

	if (OTA_SUCCESS != alislupg_ota_start_download_by_piece(ota_pid, &mdl_range, NULL))
	{
		SL_ERR("ota downloade fail\n");
		return ALISLUPG_ERR_OTHER;
	}

	memcpy(buf, ota_buf, len);

	SL_DBG("Get ota upg info Finished !\n");
	free(ota_buf);
	ota_buf = NULL;
	p_ota_buf = NULL;
	return len;
}

static int ota_file_download_by_piece(int ota_pid)
{
	unsigned long mapmem_len = 0;
	unsigned short piece_num = 0;
	int piece_read_sem_val = 0;
	int i;
	struct modules_range mdl_range;
	unsigned int crc = 0;
	unsigned int src_crc = 0;
		

	memset(&mdl_range, 0, sizeof(mdl_range));

	SL_DBG("ota_pid=%d\n", ota_pid);

	if (OTA_SUCCESS != alislupg_ota_get_download_info(ota_pid, &ota_dl_info))
	{
		SL_ERR("ota get downloade info fail\n");
		return ALISLUPG_ERR_OTHER;
	}

	SL_DBG("get download info:\nhw_version=%d, sw_version=%d, sw_size=%d\n",
	               ota_dl_info.hw_version, ota_dl_info.sw_version, ota_dl_info.sw_size);

	/* 128K align */
	mapmem_len = DATA_ALIGN(g_piece_size, 0x20000);
	ota_buf = NULL;
	SL_DBG("malloc  mapmem_len=0x%x\n", mapmem_len);
	ota_buf = malloc(mapmem_len);

	if (ota_buf == NULL)
	{
		SL_ERR("malloc 0x%x failed!\n", mapmem_len);
		return ALISLUPG_ERR_OTHER;
	}

	memset(ota_buf, 0xFF, mapmem_len);

	alislupg_ota_mem_config_by_piece(ota_buf, mapmem_len, &piece_num, &mdl_range);

	for(i = 0; i < piece_num; i++)
	{
		mdl_range.start = i * mdl_range.len ;
		mdl_range.end = ( i + 1 ) * mdl_range.len - 1;

		if(mdl_range.end >= mdl_range.total)
		{
			mdl_range.end = mdl_range.total - 1;
		}
		
		SL_DBG("OTA Debug mdl_range.start=%d, mdl_range.end=%d, mdl_range.len=%d\n",
		       mdl_range.start, mdl_range.end, mdl_range.len);
		
		while (1)
		{

			if (OTA_SUCCESS != alislupg_ota_start_download_by_piece(ota_pid, &mdl_range, ota_file_progress_callback))
			{
				SL_ERR("ota download fail\n");
				p_ota_valid = NULL;
				sem_post(&piece_source_sem);
				return ALISLUPG_ERR_OTHER;
			}

			mdl_range.valid_size -= sizeof(crc);
			crc = MG_Table_Driven_CRC(0xFFFFFFFF, ota_buf, mdl_range.valid_size);
			memcpy(&src_crc, ota_buf + mdl_range.valid_size, sizeof(crc));

			SL_DBG("crc = 0x%x, src_crc = 0x%x mdl_range.valid_size=%d \n",
			       crc, src_crc, mdl_range.valid_size);

			//ota_save_buff_2_file(mdl_range.valid_size);
			
			if (crc != src_crc)
			{
				SL_DBG("CRC check failed, we need download this piece again! \n");
				continue;
			}

			break;
		}
		
		p_ota_valid = ota_buf + mdl_range.valid_size;

		SL_DBG("OTA Debug mdl_range.valid_size=%d \n", mdl_range.valid_size);

#ifdef OTA_DEBUG
		/* save ota buff to mount fs for test */
		ota_save_buff_2_file(mdl_range.valid_size);
#endif

		sem_post(&piece_source_sem);
		
		if (i == (piece_num -1))
		{
			SL_DBG("No need wait singal in last piece!\n");
			break;		
		}
		
		while (1)
		{
			sem_getvalue(&piece_read_sem, &piece_read_sem_val);

			if (piece_read_sem_val > 0)
			{
				sem_wait(&piece_read_sem);
				break;
			}

			usleep(1000);
		}

	}

	SL_DBG("Download Finished !\n");

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

	g_piece_size = desc->package_header.ota_down_buf_size * 1024 * 1024;
	ret = ota_file_download_by_piece(pid);
	SL_DBG("OTA Debug g_piece_size = %d\n", g_piece_size);

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

static alisl_retcode upgrade_obj_prestart(alislupg_desc_t *desc);


static void ota_download_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;

	sem_init(&piece_read_sem, 0, 0);
	sem_init(&piece_source_sem, 0, 0);

	pthread_create(&obj->download, NULL, (void*)ota_file_download_thread, desc);
}

static void upgrade_obj_loop(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	otaupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;
	upg_elem_t *elem = desc->elem;
//	FILE *upgpkt = NULL;
	size_t pktsz = 0;
	size_t totalread = 0;
	size_t perread = UPG_BUF_SIZE;
	size_t elemread = 0;
//	int try_time = 0;
//	int ret = 0;
	int writersem_val = 0;
	int ret_len = 0;

	if (NULL == elem)
		return;

	ota_file_open();

	pktsz = ota_file_get_size(desc);
	SL_DBG("pktsz=%d, readoffset=0x%x\n", pktsz, totalread);

	//if (ALISLUPG_ERR_NONE != ota_file_seek(sizeof(alislupg_header_t), SEEK_SET))
	//{
	//	SL_DBG("can not seek to upgdata offset\n");
	//	desc->error = ALISLUPG_ERR_OTHER;
	//	sem_post(&tune->sourcesem);
	//	goto over;
	//}
	ota_file_skip_head();
	tune->total = pktsz - sizeof(alislupg_header_t);

	while (totalread < tune->total && NULL != elem)
	{
		while (1)
		{
			sem_getvalue(&tune->writersem, &writersem_val);

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

		if (elem->img_size < UPG_BUF_SIZE)
			perread = elem->img_size;

		obj->buf = tune->buf;

		///* Get a segment from upgrade server */
		//if (ALISLUPG_ERR_NONE != ota_file_seek(elem->img_offset + elemread, SEEK_SET))
		//{
		//	SL_DBG("seek data fail\n");
		//	desc->error = ALISLUPG_ERR_OTHER;
		//	sem_post(&tune->sourcesem);
		//	goto over;
		//}

		SL_DBG("OTA Debug elem->img_offset=0x%x, elemread=0x%x, sizeof(alislupg_header_t)=0x%x \n",
		       elem->img_offset, elemread, sizeof(alislupg_header_t));

		ret_len = ota_file_read((char*)obj->buf, perread);
		if (ret_len < 0)
		{
			SL_DBG("read data fail\n");
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}

		elemread += perread;
		tune->size = perread;
		totalread += perread;
		SL_DBG("elemsize(0x%x) elemread(0x%x) perread(0x%x)\n",
		         elem->img_size, elemread, perread);
		SL_DBG("part name : %s\n", elem->part_name);

		if (elem->img_size - elemread < UPG_BUF_SIZE)
		{
			perread = elem->img_size - elemread;
		}

		if (0 == perread)
		{
			elem = elem->next;
			elemread = 0;
			perread = UPG_BUF_SIZE;
		}

		SL_DBG("Source read totalread %d percent %d \n", totalread, (totalread) / (tune->total / 100));

		sem_post(&tune->sourcesem);
	}

over:

	ota_file_close();
	sem_destroy(&piece_read_sem);
	sem_destroy(&piece_source_sem);
	SL_DBG("End\n");
	return;
}

static alisl_retcode upgrade_obj_prestart(alislupg_desc_t *desc)
{
	pthread_t download_tid;
//	upg_object_t *object = (upg_object_t *)desc->object;
//	otaupg_object_t *obj = object->plugin;

	pthread_create(&download_tid, NULL, (void*)ota_file_download_thread, desc);

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_prestop(alislupg_desc_t *desc)
{
//	upg_object_t *object = desc->object;
//	otaupg_object_t *obj = object->plugin;

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

	if (NULL == tune->buf)
	{
		tune->buf = malloc(UPG_BUF_SIZE);
		if (tune->buf) {
			memset(tune->buf, 0, UPG_BUF_SIZE);
		} else {
			return ALISLUPG_ERR_OTHER;
		}
	}

	ota_download_start(desc);

	pthread_create(&obj->source, NULL, (void*)upgrade_obj_loop, desc);

	return ALISLUPG_ERR_NONE;
}


static alisl_retcode upgrade_obj_stop(alislupg_desc_t *desc)
{
	upg_tune_t *tune = desc->tune;

	if (NULL != tune->buf)
	{
		free(tune->buf);
	}


	tune->buf = NULL;

	return ALISLUPG_ERR_NONE;
}

static size_t upgrade_obj_getcfg(alislupg_desc_t *desc, unsigned char *buf, size_t max)
{
	int             size = 0;
//	upg_object_t    *obj = (upg_object_t *)desc->object;
//	otaupg_object_t *object = (otaupg_object_t *)obj->plugin;
//	int ret = 0;

	/* get config from ota file */
	size = ota_file_get_config(desc, (char*)buf, sizeof(alislupg_header_t));

	return size;
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

