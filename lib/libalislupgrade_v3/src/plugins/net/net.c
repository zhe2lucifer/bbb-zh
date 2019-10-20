/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               net.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:09:37
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


#include <string.h>

/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "net.h"
#include "net_interface.h"
#include "../../internal.h"

static netupg_object_t object =
{
	.name = UPG_OBJ_NAME,
};


/*
get url from config file url
example:
http://192.168.20.238/download_setting.cfg
==>
http://192.168.20.238/
*/
static int get_url_from_configurl(const char* configurl, char* url)
{
	char *pos;
	int name_len;
	name_len = strlen(configurl);
	pos = (char*)(configurl + name_len);
	SL_DBG("configurl=%s, name_len=%d, pos=%d\n", configurl, name_len, pos);

	while (*pos != '/' && pos != configurl)
	{
		pos --;
	}

	memcpy(url, configurl, pos - configurl + 1);
	SL_DBG("url=%s, pos=%d, configurl=%d\n", url, pos, configurl);
	url[pos - configurl + 1] = 0;
	SL_DBG("url=%s, pos=%d, configurl=%d\n", url, pos, configurl);

	return ALISLUPG_ERR_NONE;
}

/* call the C library funtion to split the string */
static char *_strsep(char **stringp, const char *delim)
{
	register char *tmp = *stringp;
	register char *tmp2 = tmp;
	register const char *tmp3;

	if (!*stringp) return 0;

	for (tmp2 = tmp; *tmp2; ++tmp2)
	{
		for (tmp3 = delim; *tmp3; ++tmp3)
		{
			if (*tmp2 == *tmp3)
			{
				/* delimiter found */
				*tmp2 = 0;
				*stringp = tmp2 + 1;
				return tmp;
			}
		}
	}

	*stringp = 0;
	return tmp;
}

static int calc_version(const char *sver)
{
	int v = 0;
	int field = 0;
	int len = strlen((char *)sver);
	int i;

	//follow the "xx.xx.xx" format
	for (i = 0; i < len; i++)
	{
		if ((sver[i] >= '0') && (sver[i] <= '9'))
		{
			field = field * 10 + sver[i] - '0';

			if (field > 99)
			{
				return 0;
			}
		}
		else if (sver[i] == '.')
		{
			v = v * 100 + field;
			field = 0;
		}
		else
		{
			//illegal character
			return 0;
		}
	}

	v = v * 100 + field;
	return (v > 999999 ? 0 : v);
}

static int check_config_item(char *line, char *sver, char *sfile)
{
	int i;
	char *sitems[] = {NULL, NULL, NULL, NULL}; /* model, version, file */

	i = 0;
	char *token = _strsep(&line, ";");

	while (token)
	{
		sitems[i++] = token;

		if (i == 3)
		{
			break;
		}

		token = _strsep(&line, ";");
	}

	if (i != 3)
	{
		//not enough field, return 0
		return ALISLUPG_ERR_OTHER;
	}

	//split the STB model version
	// 1=35001-01001
	char *s = sitems[0];
	token = _strsep(&s, "=");

	if (token == 0)
	{
		//not follow the format
		return ALISLUPG_ERR_OTHER;
	}

	//check with the ver in bootloader
	/*
	CHUNK_HEADER blk_header;
	unsigned long id = 0;
	sto_chunk_goto(&id, 0, 1);
	sto_get_chunk_header(id, &blk_header);
	*/

	SL_DBG("STB module = %s\n", s);
	SL_DBG("version = %s, %d\n", sitems[1], calc_version(sitems[1]));
	SL_DBG("file = %s\n", sitems[2]);

	if (sver)
	{
		strncpy(sver, sitems[1], 256);
	}

	if (sfile)
	{
		strncpy(sfile, sitems[2], 256);
	}

	return ALISLUPG_ERR_NONE;
}

static int check_url_config_file(char *confile_url, char *upgfile_url)
{
	//firstly split with "\r\n"
	int ret = ALISLUPG_ERR_NONE;
	char *line;
	int ver = 0, maxver;
	char stmpfile[256];
	char stmpver[256];
	char sw_ver[128];
	char ver_tmp[16];
	char *sfile = NULL;
	char *sver = NULL;
	char *down_buf = NULL;
	int down_data_len = 0;
	int file_len = 0;
	int ret_len = 0;
	int per_size = 10240;
	pthread_t url_handle = 0;
	int try_time = 0;

	url_handle = url_open(confile_url);

	if (0 == url_handle)
	{
		SL_DBG("url_open fail\n");
		ret = ALISLUPG_ERR_OTHER;
		goto over;
	}

	while (0 == url_get_file_size())
	{
		usleep(1000);
		try_time += 1;

		if (try_time >= 10000) //10s
		{
			SL_DBG("can not get url file size\n");
			ret = 1;
			goto over;
		}
	}

	file_len = url_get_file_size();
	SL_DBG("url file size=%d\n", file_len);

	down_buf = malloc(file_len);

	if (NULL == down_buf)
	{
		SL_DBG("malloc fail\n");
		ret = ALISLUPG_ERR_OTHER;
		goto over;
	}

	memset(down_buf, 0, file_len);

	ret_len = url_read(down_buf, file_len);

	if (ret_len < 0)
	{
		ret = ALISLUPG_ERR_CANTGETCFG;
		goto over;
	}

	memset(stmpfile, 0, sizeof(stmpfile));
	memset(stmpver, 0, sizeof(stmpver));

	/* get version in bootloader */
	memset(stmpfile, 0, sizeof(stmpfile));
	memset(stmpver, 0, sizeof(stmpver));

	maxver = 0;
	line = _strsep(&down_buf, "\n");

	while (line != NULL)
	{
		//split the items
		if (ALISLUPG_ERR_NONE ==  check_config_item(line, stmpver, stmpfile))
		{
			ver = calc_version(stmpver);
			sfile = stmpfile;

			if (0) //(maxver < ver)
			{
				maxver = ver;
				sver = stmpver;
				sfile = stmpfile;
			}
		}

		line = _strsep(&down_buf, "\n");
	}

	if (sfile && file_len)
	{
		get_url_from_configurl(confile_url, upgfile_url);
		SL_DBG("upgfile_url=%s, sfile=%s\n", upgfile_url, sfile);
		strcat(upgfile_url, sfile);
		SL_DBG("upgfile_url=%s, sfile=%s\n", upgfile_url, sfile);

	}
	else
	{
		ret = ALISLUPG_ERR_OTHER;
		goto over;
	}

over:

	if (NULL != down_buf)
	{
		free(down_buf);
		down_buf = NULL;
	}

	if (0 != url_handle)
	{
		url_close(url_handle);
		url_handle = 0;
	}

	return ret;
}

static int get_config_from_urlfile(char *upgfile_url, char *buf, int len)
{
	//firstly split with "\r\n"
	int ret_len = 0;
	int per_size = 10240;
	pthread_t url_handle = 0;
	int try_time = 0;

	url_handle = url_open(upgfile_url);

	if (0 == url_handle)
	{
		SL_DBG("url_open fail\n");
		goto over;
	}

	while (0 == url_get_file_size())
	{
		usleep(1000);
		try_time += 1;

		if (try_time >= 10000) //10s
		{
			SL_DBG("can not get url file size\n");
			goto over;
		}
	}

	ret_len = url_read(buf, len);
	SL_DBG("read config from url file complete\n");

over:

	if (0 != url_handle)
	{
		url_close(url_handle);
		url_handle = 0;
		SL_DBG("close\n");
	}

	return ret_len;

}

static void *upgrade_obj_loop(void *desc_in)
{
	alislupg_desc_t *desc = (alislupg_desc_t *)desc_in;
	upg_object_t *object = desc->object;
	netupg_object_t *obj = object->plugin;
	upg_tune_t *tune = desc->tune;
	upg_header_t *hdr = desc->header;
	upg_elem_t *elem = hdr->elem;
	FILE *upgpkt = NULL;
	size_t pktsz = 0;

	size_t totalread = obj->upgdata_offset;
	size_t perread = UPG_BUF_SIZE;
	size_t elemread = 0;
	pthread_t url_handle = 0;
	int try_time = 0;
	int ret = 0;
	int writersem_val = 0;
	int ret_len = 0;
#if UPGRADE_BUILD_CRYPTO
	void *buf = NULL;
#endif

	if (NULL == elem)
		return;


	url_handle = url_open(obj->pktname);

	if (0 == url_handle)
	{
		SL_DBG("url_open fail\n");
		desc->error = ALISLUPG_ERR_OTHER;
		sem_post(&tune->sourcesem);
		goto over;
	}

	while (0 == url_get_file_size())
	{
		usleep(1000);
		try_time += 1;

		if (try_time >= 10000) //10s
		{
			SL_DBG("can not get url file size\n");
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}
	}

	pktsz = url_get_file_size();
	SL_DBG("pktsz=%d, readoffset=0x%x\n", pktsz, totalread);

	if (NET_ERR_NONE != url_seek(obj->upgdata_offset, SEEK_SET))
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
		if (NET_ERR_NONE != url_seek(elem->offset + elemread, SEEK_SET))
		{
			SL_DBG("seek data fail\n");
			desc->error = ALISLUPG_ERR_OTHER;
			sem_post(&tune->sourcesem);
			goto over;
		}

		ret_len = url_read(obj->buf, perread);

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

		SL_DBG("Source read percent %d\n", desc->percent);

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

	if (0 != url_handle)
	{
		url_close(url_handle);
		url_handle = 0;
	}

	SL_DBG("End\n");
	return;

}

static alisl_retcode upgrade_obj_prestart(alislupg_desc_t *desc)
{
	/* not do anything for net */

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_prestop(alislupg_desc_t *desc)
{
	/* not do anything for net */

	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_start(alislupg_desc_t *desc)
{
	upg_object_t *object = desc->object;
	netupg_object_t *obj = object->plugin;
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

	pthread_create(&obj->source, NULL, upgrade_obj_loop, desc);

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
	netupg_object_t *object = (netupg_object_t *)obj->plugin;
	int ret = 0;
	char upgfile_url[1024];
	int upgfile_len = 0;

	/* step1: get download setting cfg */
	if (0 == strlen(object->config))
	{
		return 0;
	}

	memset(upgfile_url, 0, 1024);
	ret = check_url_config_file(object->config, upgfile_url);

	if (ALISLUPG_ERR_NONE != ret)
	{
		return -EINVAL;
	}

	strcpy(desc->config, upgfile_url);//change dest->config to be real upgrade file

	size = UPG_CONFIGV1_SIZE;

	size = get_config_from_urlfile(upgfile_url, buf, size);

	return size;
}

static alisl_retcode upgrade_obj_setdataoffset(alislupg_desc_t *desc, off_t offset)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	netupg_object_t *obj = object->plugin;

	obj->upgdata_offset = offset;
	SL_DBG("data offset = 0x%x\n", obj->upgdata_offset);
	return ALISLUPG_ERR_NONE;
}

static alisl_retcode upgrade_obj_getdataoffset(alislupg_desc_t *desc)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	netupg_object_t *obj = object->plugin;

	return obj->upgdata_offset;
}


static alisl_retcode upgrade_obj_setpktname(alislupg_desc_t *desc, char *name)
{
	upg_object_t *object = (upg_object_t *)desc->object;
	netupg_object_t *obj = object->plugin;

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
 *
 *  For a NET upgrade, we need the configuration file
 *  firstly. Supposely, the upgrade image will be in
 *  the same directory as the configuration file, and
 *  the packet name is recorded in the configuration
 *  file
 */
alisl_retcode upgrade_obj_register(alislupg_desc_t *desc)
{
	upg_object_t *obj = (upg_object_t *)desc->object;

	if (UPG_NET == desc->source)
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

	SL_DBG("Object %s got\n",object.name);

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

	if (UPG_NET == desc->source)
	{
		desc->object = NULL;
	}

	return ALISLUPG_ERR_NONE;
}
