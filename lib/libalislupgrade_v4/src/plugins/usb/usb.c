/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               usb.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:13:36
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* share library headers */
#include <alipltflog.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "usb.h"
#include "../../internal.h"

static usbupg_object_t object = {
	.name = UPG_OBJ_NAME,
};

// In M3733, static link alisldmxkit cause libalislupgradeusb.so dlopen fail.
// dlopen fail: alipltflog_printf undefined
// alipltflog_printf just do the log printing.
// It's not critical function, so use printf instead.

void * usb_buf = NULL;
void * usb_buf_file = NULL;


static int usb_file_open()
{
	if (usb_buf == NULL)
	{
		return ALISLUPG_ERR_OTHER;
	}

	usb_buf_file = usb_buf;

	return ALISLUPG_ERR_NONE;
}

static int usb_file_close()
{
	return ALISLUPG_ERR_NONE;
}

static int usb_file_seek(int offset, int where)
{
//	int cur_offset = 0;
//	int ret_len = 0;

	switch (where)
	{
		case SEEK_SET:
			usb_buf_file = usb_buf + offset;
			break;

		case SEEK_CUR:
			usb_buf_file += offset;
			break;

		default:
			SL_ERR("Not support seek mode\n");
			break;
	}

	return ALISLUPG_ERR_NONE;
}


static int usb_file_read(unsigned char * ptr, int size)
{
	if (ptr == NULL)
	{
		SL_ERR("invalid parameters.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	memcpy(ptr, usb_buf_file, size);

	return size;
}


static int usb_file_download(alislupg_desc_t * desc)
{
	size_t mal_size = 0;
	struct stat stat_temp;
	FILE * file_stream = NULL;
	size_t len = 0;
	int cur_percent= 0;
//	static int total_read = 0;

//	if (desc == NULL) //steven@20170324 for cpp test@ Condition "desc == 0" always evaluates to false
//	{
//		SL_ERR("invalid parameter.\n");
//		return ALISLUPG_ERR_INVALIDPARAM;
//	}

	memset(&stat_temp, 0, sizeof(struct stat));
	lstat(desc->config, &stat_temp);
	mal_size = UPG_ALIGN_128K(stat_temp.st_size);
	SL_DBG("file name : %s,file size : 0x%x\n",
		desc->config, stat_temp.st_size);
	SL_DBG("file align size :0x%x\n",
		mal_size);

	usb_buf = malloc(mal_size);
	if (usb_buf == NULL)
	{
		SL_ERR("not enough memory\n");
		desc->error = ALISLUPG_ERR_NOTENOUGHSPACE;
		pthread_mutex_lock(&desc->msg_mutex);
		pthread_cond_signal(&desc->msg_cond);
		pthread_mutex_unlock(&desc->msg_mutex);

		return ALISLUPG_ERR_NOTENOUGHSPACE;
	}

	file_stream = fopen(desc->config, "rb");
	if (file_stream == NULL)
	{
		SL_ERR("open file \"%s\" fail\n", desc->config);
		return ALISLUPG_ERR_FILEOPERATEFAIL;
	}

	len = fread(usb_buf, stat_temp.st_size, 1, file_stream);
	if (len == 0)
	{
		SL_ERR("read file fail\n");
		fclose(file_stream);
		return ALISLUPG_ERR_FILEOPERATEFAIL;
	}
	else
	{
		SL_DBG("read success\n");

		cur_percent = 100;
		desc->percent = cur_percent;
		pthread_mutex_lock(&desc->msg_mutex);
		pthread_cond_signal(&desc->msg_cond);
		pthread_mutex_unlock(&desc->msg_mutex);
		SL_DBG("percent %d\n", desc->percent);
	}

	fclose(file_stream);
	return ALISLUPG_ERR_NONE;
}


static void *usb_file_download_thread(void * description)
{
	int ret = 0;

	alislupg_desc_t * desc = (alislupg_desc_t *)description;
	if (desc == NULL)
	{
		SL_ERR("invalid parameters\n");
		return (void *)ALISLUPG_ERR_INVALIDPARAM;
	}

	ret = usb_file_download(desc);
	if (ret != ALISLUPG_ERR_NONE)
	{
		SL_ERR("download file by usb fail\n");
	}

	return (void *)ret;
}

static void *upgrade_obj_loop(void *desc_para)
{
	alislupg_desc_t *desc = NULL;
	upg_object_t *object = NULL;
	usbupg_object_t *obj = NULL;
	upg_tune_t *tune = NULL;
	upg_elem_t *elem = NULL;
	FILE *upgpkt = NULL;
//	size_t pktsz = 0;
	struct stat pktstat;
	size_t totalread = 0;
	size_t perread = UPG_BUF_SIZE;
	size_t elemread = 0;
	int writersem_val = 0;

	if (NULL == desc_para)
	{
		return NULL;
	}

    desc = (alislupg_desc_t *)desc_para;
	object = desc->object;
	obj = object->plugin;
	tune = desc->tune;
	elem = desc->elem;

	stat(obj->pktname, &pktstat);
	if (desc->method == PIECE_BY_PIECE)
	{
		upgpkt = fopen(obj->pktname, "rb");
		if (NULL == upgpkt)
		{
			SL_DBG("open usb file %s fail\n", obj->pktname);
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}

		if(0 != fseek(upgpkt, sizeof(alislupg_header_t), SEEK_SET))
		{
			SL_DBG("can not seek to upgdata offset\n");
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}
	}
	else
	{
		/* one shot mode */
		usb_file_open();
		usb_file_seek(sizeof(alislupg_header_t), SEEK_SET);
	}

	tune->total = pktstat.st_size - sizeof(alislupg_header_t);
	SL_DBG("pktsz=0x%x, image_size=0x%x\n", pktstat.st_size, tune->total);

	while(totalread < tune->total && NULL != elem)
	{
		while(1)
		{
			sem_getvalue(&tune->writersem, &writersem_val);
			if(writersem_val>0)
			{
				sem_wait(&tune->writersem);
				break;
			}
			usleep(1000);
		}

		/*
		 * It safe to quit, due to the error is set by writter
		 * If the error hanppen, the writter should be already
		 * finish at that time
		 */

		if (ALISLUPG_ERR_NONE != desc->error)
		{
			break;
		}
		tune->elem = elem;
		if (elem->img_size < UPG_BUF_SIZE)
		{
			perread = elem->img_size;
		}
		obj->buf = tune->buf;
		if (desc->method == PIECE_BY_PIECE)
		{
			fseek(upgpkt, elem->img_offset + elemread, SEEK_SET);
			fread(obj->buf, perread, 1, upgpkt);
		}
		else
		{
			/* one shot mode */
			usb_file_seek(elem->img_offset + elemread, SEEK_SET);
			usb_file_read(obj->buf, (int)perread);
		}
		elemread += perread;
		tune->size = perread;
		totalread += perread;
		SL_DBG("elemsize(0x%x) elemread(0x%x) perread(0x%x)\n",
		        elem->img_size, elemread, perread);
		SL_DBG("part name : %s\n", elem->part_name);

		if ((elem->img_size >= elemread) && (elem->img_size - elemread < UPG_BUF_SIZE))
		{
			perread = elem->img_size - elemread;
		}

		if (0 == perread)
		{
			elem = elem->next;
			elemread = 0;
			perread = UPG_BUF_SIZE;
		}

		SL_DBG("Source read percent %d\n", (totalread) / (tune->total/100));

		sem_post(&tune->sourcesem);
	}

over:

	if (desc->method == PIECE_BY_PIECE)
	{
		if (upgpkt != NULL)
		{
			fclose(upgpkt);
		}
	}
	else
	{
		usb_file_close();
	}

	SL_DBG("End\n");
	return NULL;
}

static alisl_retcode upgrade_obj_prestart(alislupg_desc_t *desc)
{
	int ret = ALISLUPG_ERR_NONE;
	pthread_t pthread_id;

	if (desc == NULL)
	{
		SL_ERR("invalid parameters.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	if (desc->method == ONE_SHOT)
	{
		ret = pthread_create(&pthread_id, NULL, usb_file_download_thread, desc);
		if (ret != 0)
		{
			SL_ERR("create pthread fail.\n");
		}
	}

	return ret;
}

static alisl_retcode upgrade_obj_prestop(alislupg_desc_t *desc)
{
	if (desc == NULL)
	{
		SL_ERR("invalid param.\n");
		return ALISLUPG_ERR_INVALIDPARAM;
	}

	if (desc->method == ONE_SHOT)
	{
		if (usb_buf != NULL)
		{
			free(usb_buf);
			usb_buf = usb_buf_file = NULL;
		}
	}

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	usbupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;

	if (NULL == tune->buf)
	{
		tune->buf = malloc(UPG_BUF_SIZE);
		if (tune->buf == NULL)
		{
			return ALISLUPG_ERR_NOTENOUGHSPACE;
		}
		memset(tune->buf, 0, UPG_BUF_SIZE);
	}

	pthread_create(&obj->source, NULL, upgrade_obj_loop, desc);

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

/**
 *  Function Name:  upgrade_obj_getcfg
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *  @param          buf   store the configuration
 *  @param          max   max byte which could be store in the buf
 *
 *  @return
 *                  Actual byte of configuration
 *                  If the return value > max, then the buf should be
 *                  reallocate the call this function again
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static size_t upgrade_obj_getcfg(alislupg_desc_t *desc, \
								 unsigned char *buf, \
								 size_t max)
{
	int             size = 0;
	upg_object_t    *obj = (upg_object_t *)desc->object;
	usbupg_object_t *object = (usbupg_object_t *)obj->plugin;
	FILE            *cfg = NULL;
	struct stat     stat;

	if (0 == strlen(object->config))
	{
		return 0;
	}

	lstat(object->config, &stat);

	if (0 == stat.st_size)
	{
		return 0;
	}

	/*
	 * The parser need binary content
	 */
	if (NULL == (cfg = fopen(object->config, "rb")))
	{
		return -EINVAL;
	}

	size = fread(buf, sizeof(alislupg_header_t), 1, cfg);
	
	 //steven add@20170324 for cpp test:File not closed: cfg 
	fclose(cfg);

	return (unsigned long)size;

}

/**
 *  Function Name:  upgrade_obj_setpktname
 *  @brief
 *
 *  @param          desc  descriptor with upgrade parameters
 *  @param          name  package name
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
static alisl_retcode upgrade_obj_setpktname(alislupg_desc_t *desc, char *name)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	usbupg_object_t *obj = object->plugin;

	strcpy(obj->pktname, name);

	return ALISLUPG_ERR_NONE;
}


/**
 *  Function Name:  upgrade_obj_register
 *  @brief          Common register function of upgrade plugin
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          param  param of upgrade
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  For a USB upgrade, we need the configuration file
 *  firstly. Supposely, the upgrade image will be in
 *  the same directory as the configuration file.
 */
alisl_retcode upgrade_obj_register(alislupg_desc_t *desc)
{
	upg_object_t *obj = (upg_object_t *)desc->object;

	if (UPG_USB == desc->source)
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
 *  @brief          Common unregister function of upgrade plugin
 *
 *  @param          desc   descriptor with upgrade parameters
 *  @param          param  param of upgrade
 *
 *  @return         alisl_retcode
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
alisl_retcode upgrade_obj_unregister(alislupg_desc_t *desc, alislupg_param_t *param)
{

	if (UPG_USB == desc->source)
	{
		desc->object = NULL;
	}

	return ALISLUPG_ERR_NONE;
}
