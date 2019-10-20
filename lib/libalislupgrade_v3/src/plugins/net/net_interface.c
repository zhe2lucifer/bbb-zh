/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               net_interface.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/22/2013 15:10:59
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */


/* System header */
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>

/* share library headers */
#include <alipltfretcode.h>

/* Upgrade header */
#include <alislupgrade.h>
#include <upgrade_object.h>

/* Internal header */
#include "net_interface.h"


static net_cache_t net_cache;

static net_cainfo_t net_cainfo;

#ifndef MAX_PATH
#define	MAX_PATH	(4096)
#endif

/**
 *  Function Name:  url_get_file_size
 *  @brief          get url file size
 *
 *  @param          none
 *
 *  @return         url file size
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  just support seek forword from cur/set
 */
int url_get_file_size()
{
	return url_file_size;
}

static void url_set_file_size(double len)
{
	url_file_size = (int)len;

	return;
}

static void url_set_file_size_zero()
{
	url_file_size = 0;

	return;
}

/**
 *  Function Name:  url_set_abort
 *  @brief          set abort flag
 *
 *  @param          abort flag
 *
 *  @return         none
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  support seek forword from cur/set
 */
void url_set_abort(int abort)
{
	url_abort = abort;

	return;
}


/**
 *  Function Name:  url_get_abort
 *  @brief          get abort flag
 *
 *  @param          none
 *
 *  @return         abort flag
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  support seek forword from cur/set
 */
int url_get_abort()
{
	return url_abort;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int data_size = size * nmemb;
	int len_to_end;

	net_cache_t* nci = &net_cache;

	if (url_get_abort())
	{
		SL_DBG("abort! \n");
		return NET_ERR_ABORT;
	}

	while (1)
	{
		pthread_mutex_lock(&(nci->cache_mutex));

		if (data_size > (nci->cache_size - nci->unread_data_len))
		{
			pthread_mutex_unlock(&(nci->cache_mutex));
			usleep(1000);

			if (url_get_abort())
			{
				SL_DBG("sleep, abort! \n");
				return NET_ERR_ABORT;
			}
		}
		else
		{
			break;
		}
	}

	if (nci->wr_pos >= nci->rd_pos)
	{
		len_to_end = nci->cache_size - nci->wr_pos;

		if (data_size <= len_to_end)
		{
			memcpy(nci->cache_buff + nci->wr_pos, ptr, data_size);
			nci->wr_pos = nci->wr_pos + data_size;
		}
		else// data_size > (nci->cache_size-nci->wr_pos)
		{
			memcpy(nci->cache_buff + nci->wr_pos, ptr, len_to_end);
			memcpy(nci->cache_buff, ptr + len_to_end, data_size - len_to_end);
			nci->wr_pos = data_size - len_to_end;
		}
	}
	else// (nci->wr_pos < nci->rd_pos)
	{
		memcpy(nci->cache_buff + nci->wr_pos, ptr, data_size);
		nci->wr_pos = nci->wr_pos + data_size;
	}

	nci->unread_data_len += data_size;

	pthread_mutex_unlock(&(nci->cache_mutex));

	return data_size;
}

static int get_cainfo(void)
{
	int ret = -1;
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	FILE *fp3 = NULL;
	char *upgCAPath = getenv("UPG_CA_PATH");

	if (!upgCAPath)  /* default path */
	{
		if ((net_cainfo.pCertFile = (char*)calloc(strlen(UPG_CA_PATH)+16, sizeof(char))))
			strcpy(net_cainfo.pCertFile, UPG_CA_PATH);
		else
		{
			SL_DBG("UPG_CAINFO CertFile alloc FAIL !\n");
			return ret;
		}

		if ((net_cainfo.pCACertFile = (char*)calloc(strlen(UPG_CA_PATH)+16, sizeof(char))))
			strcpy(net_cainfo.pCACertFile, UPG_CA_PATH);
		else
		{
			SL_DBG("UPG_CAINFO CACertFile alloc FAIL !\n");
			return ret;
		}

		if ((net_cainfo.pKeyName = (char*)calloc(strlen(UPG_CA_PATH)+16, sizeof(char))))
			strcpy(net_cainfo.pKeyName, UPG_CA_PATH);
		else
		{
			SL_DBG("UPG_CAINFO KeyName alloc FAIL !\n");
			return ret;
		}
	}
	else if (strlen(upgCAPath) > (MAX_PATH - 16))
	{
		SL_DBG("upgCAPath is too long %d ! MAX_PATH= %d\n", \
			__FUNCTION__, __LINE__, (int)strlen(upgCAPath), MAX_PATH);
		return ret;
	}
	else  /* export path */
	{
		if (net_cainfo.pCertFile = (char*)calloc(strlen(upgCAPath)+16, sizeof(char)))
			strcpy(net_cainfo.pCertFile, upgCAPath);
		else
		{
			SL_DBG("UPG_CAINFO CertFile alloc FAIL !\n");
			return ret;
		}

		if (net_cainfo.pCACertFile = (char*)calloc(strlen(upgCAPath)+16, sizeof(char)))
			strcpy(net_cainfo.pCACertFile, upgCAPath);
		else
		{
			SL_DBG("UPG_CAINFO CACertFile alloc FAIL !\n");
			return ret;
		}

		if (net_cainfo.pKeyName = (char*)calloc(strlen(upgCAPath)+16, sizeof(char)))
			strcpy(net_cainfo.pKeyName, upgCAPath);
		else
		{
			SL_DBG("UPG_CAINFO KeyName alloc FAIL !\n");
			return ret;
		}
	}

	strcat(net_cainfo.pCertFile, "/OTT_CA.pem");
	strcat(net_cainfo.pCACertFile, "/SERVER_CA.pem");
	strcat(net_cainfo.pKeyName, "/OTT_CA.key");

	if ((fp1 = fopen(net_cainfo.pCertFile, "r")))
		fclose(fp1);
	else
		return ret;
	if ((fp2 = fopen(net_cainfo.pCACertFile, "r")))
		fclose(fp2);
	else
		return ret;
	if ((fp3 = fopen(net_cainfo.pKeyName, "r")))
		fclose(fp3);
	else
		return ret;

	SL_DBG("UPG_CAINFO CertFile = %s\n", net_cainfo.pCertFile);
	SL_DBG("UPG_CAINFO CACertFile = %s\n", net_cainfo.pCACertFile);
	SL_DBG("UPG_CAINFO KeyName = %s\n", net_cainfo.pKeyName);

	return 0;
}

static void free_cainfo(void)
{
	if (net_cainfo.pCertFile)
		free(net_cainfo.pCertFile);
	if (net_cainfo.pCACertFile)
		free(net_cainfo.pCACertFile);
	if (net_cainfo.pKeyName)
		free(net_cainfo.pKeyName);
}

static void *transfer(void *url_in)
{
	char *url = (char *)url_in;
	CURL *curl;
	int ret = NET_ERR_NONE;
	double length = 0;
	//static const char *pCertFile = "/usr/appdb/upg/OTT_CA.pem";
	//static const char *pCACertFile = "/usr/appdb/upg/SERVER_CA.pem";
	//static const char *pKeyName = "/usr/appdb/upg/OTT_CA.key";


	curl = curl_easy_init();
	SL_DBG("curl init\n");

	curl_easy_reset(curl);
	SL_DBG("reset url ret:%x\n",ret);

	ret = curl_easy_setopt(curl, CURLOPT_URL, url);
	SL_DBG("set url ret:%x\n", ret);

	/* Set curl connection timeout with 10s instead of curl transfer timeout */
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);

	/* Support https download */
	memset(&net_cainfo, 0, sizeof(net_cainfo_t));
	if (strstr(url, "https://") && !get_cainfo())
	{
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLCERT, net_cainfo.pCertFile);
		curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_SSLKEY, net_cainfo.pKeyName);
		curl_easy_setopt(curl, CURLOPT_CAINFO, net_cainfo.pCACertFile);
		curl_easy_setopt(curl, CURLOPT_FTP_SSL, CURLFTPSSL_ALL);
		curl_easy_setopt(curl, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_SSL);
		/* OTT should support TLS 1.X protocol */
		//curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		/* Skip verify host (Common Name), default value 2 need verify */
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

		/* Peer Verification */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	}
	free_cainfo();

	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

	if (curl_easy_perform(curl) == CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
		url_set_file_size(length);
		SL_DBG("url_file_size:%d, length:%lf\n", url_get_file_size(), length);
	}
	else
	{
		url_set_file_size_zero();
		SL_DBG("can not get down file len\n");
		ret = NET_ERR_OTHER;
		goto END;
	}

	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 0);

	ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	SL_DBG("set wd ret:%x\n", ret);

	ret = curl_easy_perform(curl);
	SL_DBG("perform ret:%x\n", ret);

END:
	curl_easy_cleanup(curl);
	SL_DBG("end\n");

	return;
}

/**
 *  Function Name:  url_open
 *  @brief          open a url
 *
 *  @param          url  url address
 *
 *  @return         url thread id
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
pthread_t url_open(const char *url)
{
	int ret = 0;
	pthread_t transferid;

	url_set_abort(0);
	url_set_file_size_zero();
	pthread_mutex_init(&(net_cache.cache_mutex), NULL);
	memset(&net_cache, 0, sizeof(net_cache_t));
	net_cache.cache_buff = cache_buf;
	net_cache.cache_size = CACHE_BUF_SIZE;

	if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
	{
		SL_DBG("curl_global_init() failed\n");
		return 0;
	}

	ret = pthread_create(&transferid, NULL, transfer, (void *)url);

	if (0 != ret)
	{
		SL_DBG("create download thread failed\n");
		return 0;
	}

	return transferid;
}

/**
 *  Function Name:  url_close
 *  @brief          close a url
 *
 *  @param          transferid  url thread id
 *
 *  @return         url thread id
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
int url_close(pthread_t transferid)
{
	url_set_abort(1);
	pthread_join(transferid, NULL);
	curl_global_cleanup();
	pthread_mutex_destroy(&(net_cache.cache_mutex));

	SL_DBG("end\n");

	return NET_ERR_NONE;
}

/* size should be less than CACHE_BUF_SIZE-WRITER_MAX_LEN */
static int url_read_cache(char* ptr, int size)
{
	net_cache_t* nci = &net_cache;
	int tm = 10000; /* 5s */
	int len_to_end = 0;

	while (1)
	{
		pthread_mutex_lock(&(nci->cache_mutex));

		if (size > nci->unread_data_len)
		{
			pthread_mutex_unlock(&(nci->cache_mutex));
			usleep(1000);
			tm--;

			if (tm <= 0)
			{
				return -1;
			}
		}
		else
		{
			break;
		}
	}

	if (nci->wr_pos >= nci->rd_pos)
	{
		memcpy(ptr, nci->cache_buff + nci->rd_pos, size);
		nci->rd_pos = nci->rd_pos + size;
	}
	else// (nci->wr_pos < nci->rd_pos)
	{
		len_to_end = nci->cache_size - nci->rd_pos;

		if (size <= len_to_end)
		{
			memcpy(ptr, nci->cache_buff + nci->rd_pos, size);
			nci->rd_pos = nci->rd_pos + size;
		}
		else// size > (nci->cache_size-nci->rd_pos)
		{
			memcpy(ptr, nci->cache_buff + nci->rd_pos, len_to_end);
			memcpy(ptr + len_to_end, nci->cache_buff, size - len_to_end);
			nci->rd_pos = size - len_to_end;
		}
	}

	nci->unread_data_len -= size;
	nci->file_offset += size;

	pthread_mutex_unlock(&(nci->cache_mutex));

	return size;
}

/**
 *  Function Name:  url_read
 *  @brief          read url data to buffer
 *
 *  @param          ptr   buffer to save the data
 *  @param          size  data size to be read
 *
 *  @return         read data len
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 */
int url_read(char* ptr, int size)
{
	int down_data_len = 0;
	int per_size = 10240;//per_size should be less than CACHE_BUF_SIZE-WRITER_MAX_LEN
	int ret_len = 0;

	while (down_data_len < size)
	{
		if (size < per_size)
		{
			per_size = size;
		}

		if (per_size <= (size - down_data_len))
		{
			ret_len = url_read_cache(ptr + down_data_len, per_size);
		}
		else
		{
			ret_len = url_read_cache(ptr + down_data_len, (size - down_data_len));
		}

		if (ret_len < 0)
		{
			return -1;
		}

		down_data_len += ret_len;
	}

	return down_data_len;

}

/**
 *  Function Name:  url_seek
 *  @brief          seek offset of url
 *
 *  @param          offset  offset from where
 *  @param          where   where of url
 *
 *  @return         0:success  other:fail
 *
 *  @author         evan.wu <evan.wu@alitech.com>
 *  @date           2013-6-27
 *
 *  just support seek forword from cur/set
 */
int url_seek(int offset, int where)
{
	int cur_offset = 0;
	int per_size = SEEK_BUF_SIZE;
	char seek_buf[SEEK_BUF_SIZE];
	int ret_len = 0;
	net_cache_t* nci = &net_cache;

	if (SEEK_CUR != where && SEEK_SET != where)
	{
		SL_DBG("wrong\n");
		return NET_ERR_OTHER;
	}

	if (SEEK_SET == where)
	{
		pthread_mutex_lock(&(nci->cache_mutex));
		cur_offset = nci->file_offset;
		pthread_mutex_unlock(&(nci->cache_mutex));
	}

	while (cur_offset < offset)
	{
		if (per_size <= (offset - cur_offset))
		{
			ret_len = url_read(seek_buf, per_size);
		}
		else
		{
			ret_len = url_read(seek_buf, (offset - cur_offset));
		}

		if (ret_len < 0)
		{
			return NET_ERR_TRANSFER_FAIL;
		}

		cur_offset += ret_len;
	}

	return NET_ERR_NONE;
}


