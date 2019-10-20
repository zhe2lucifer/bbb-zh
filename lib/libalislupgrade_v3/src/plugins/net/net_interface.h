/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               net_interface.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               7/22/2013 15:12:24
 *  @revision           none
 *
 *  @author             evan.wu <evan.wu@alitech.com>
 */

#ifndef __UPGRADE_NET_INTERFACE__H_
#define __UPGRADE_NET_INTERFACE__H_

#include <curl/curl.h>

#include <upgrade_object.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** libcurl/other write_data each time max data len */
#define WRITER_MAX_LEN  16384   
/** buffer size for seek function */
#define SEEK_BUF_SIZE   1024    
/** buffer size for net cache */
#define CACHE_BUF_SIZE   (100*1024) 
/** default path for certificate */
#define UPG_CA_PATH "/usr/appdb/ott-ap/upg"

/** buffer for net cache */
static char cache_buf[CACHE_BUF_SIZE];  
/** size of url file*/
static int url_file_size = 0;   
/** abort flag */
static int url_abort = 0;   

/** Define cache for net */
typedef struct
{
	pthread_mutex_t  cache_mutex;
	char*            cache_buff;
	int              cache_size;
	int              unread_data_len;
	int              rd_pos;
	int              wr_pos;
	int              file_offset;
}net_cache_t;

/** Error handler of net interface */
typedef enum net_err {
	NET_ERR_NONE,
	NET_ERR_OPEN_FAIL,
	NET_ERR_CLOSE_FAIL,
	NET_ERR_TRANSFER_FAIL,
	NET_ERR_ABORT,
	NET_ERR_OTHER,
} net_err_t;

typedef struct
{
	char *pCertFile;
	char *pCACertFile;
	char *pKeyName;
}net_cainfo_t;


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
pthread_t url_open(const char *url);


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
int url_close(pthread_t transferid);


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
int url_read(char* ptr, int size);


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
int url_seek(int offset, int where);


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
int url_get_file_size();


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
void url_set_abort(int abort);


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
int url_get_abort();


#ifdef __cplusplus
}
#endif

#endif

