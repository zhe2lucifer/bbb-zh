/** @file     alislavsync.c
 *  @brief    avsync device function wrapping 
 *  @author   wendy.he
 *  @date     2014-8-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2014-2999 Copyright (C)
 */

/* system headers */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>

/* kernel headers */
#include <ali_avsync_common.h>

/* share library headers */
#include <alipltflog.h>
#include <alislavsync.h>

/* local headers */
#include "internal.h"

static struct avsync_private *m_avsync_dev = NULL;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief         avsync device construct, malloc memory and init params
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislavsync_construct(void **handle)
{
	struct avsync_private *priv ;
		
	priv = malloc(sizeof(*priv));
	if (priv == NULL) {
		SL_ERR("avsync malloc memory failed!\n");
		return AVSYNC_ERR_NOMEM;
	}
	
	memset(priv, 0, sizeof(*priv)); 
	priv->fd = -1;
	priv->video_id = AVSYNC_VIDEO_NB;
	*handle = priv;
	
	return AVSYNC_ERR_NOERR;
}


/**
 * @brief         avsync device destruct, free memory, close device handle.
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
static alisl_retcode alislavsync_destruct(void **handle)
{
	free(*handle);
	*handle = NULL;
	
	return AVSYNC_ERR_NOERR;
}

/**  
 * @brief        Open ali_avsync0 for common usage. 
 *               It should be opend before opening demux, audio and video devices.
 * @author       wendy.he
 * @date         2014-8-8
 * @param[in]    handle is avsync device pointer
 * @param[out]   
 * @return       alisl_retcode
 * @note
 *
 */
alisl_retcode alislavsync_open(alisl_handle *handle)
{
	struct avsync_private *priv = NULL;

	pthread_mutex_lock(&m_mutex);
	if (m_avsync_dev == NULL) {
		alislavsync_construct((void **)&priv);
		if(NULL == priv) {
			pthread_mutex_unlock(&m_mutex);
			return AVSYNC_ERR_NOMEM;
		}
		if ((priv->fd = open(ALISLAVSYNC_DEV_NAME, O_RDWR)) < 0) {
			SL_ERR("avsync device open failed!\n");
			pthread_mutex_unlock(&m_mutex);
			return AVSYNC_ERR_FILEOPENFAILED;
		}
		m_avsync_dev = priv;
	}
	m_avsync_dev->open_cnt++;
	*handle = m_avsync_dev;
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d, open_cnt: %d\n",
	    m_avsync_dev, m_avsync_dev->fd, m_avsync_dev->open_cnt);
	
	pthread_mutex_unlock(&m_mutex);
	return AVSYNC_ERR_NOERR;
}

/**
 * @brief         close avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislavsync_close(alisl_handle handle)
{
	struct avsync_private *priv = (struct avsync_private *)handle;
	SL_DBG("0 avsync_dev:%p avsync_dev->fd:%d, open_cnt:%d\n",
	       m_avsync_dev, m_avsync_dev->fd, m_avsync_dev->open_cnt);
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);
	
	if(--priv->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return AVSYNC_ERR_NOERR;
	}
	SL_DBG("1 avsync_dev:%p avsync_dev->fd:%d, open_cnt:%d\n",
	       m_avsync_dev, m_avsync_dev->fd, m_avsync_dev->open_cnt);
	close(priv->fd);
	priv->fd = -1;
	alislavsync_destruct((void **)&priv);
	m_avsync_dev = NULL;
	pthread_mutex_unlock(&m_mutex);
	
	return AVSYNC_ERR_NOERR;
}

/**
 * @brief         start avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer, 
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislavsync_start(alisl_handle handle)
{
	struct avsync_private *priv = (struct avsync_private *)handle;
	alisl_retcode ret = AVSYNC_ERR_NOERR;	
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}

	SL_DBG("avsync_dev:%p avsync_dev->fd:%d, start_cnt: %d\n",
	       m_avsync_dev, m_avsync_dev->fd, priv->start_cnt);
	pthread_mutex_lock(&m_mutex);
	if (priv->start_cnt == 0) {
		SL_DBG("IOCTL_NAME:ALI_AVSYNC_START\n");
		if (ioctl(priv->fd, ALI_AVSYNC_START)) {
            SL_ERR("ioctl ALI_AVSYNC_START failed\n");
			ret = AVSYNC_ERR_IOCTLFAILED;
		}
	}
	/* 
	for PIP, two streams were playing, when one(audio not playing) stopped, the other
	one(audio playing) continued playing. In this case, when the stopped one called
	avsync stop, the avsync can't not stop immediately, we use start_cnt to control 
	it, when all the streams stopped playing, stop avsync
	*/
	priv->start_cnt++;	
	pthread_mutex_unlock(&m_mutex);
	return ret;	
}

/**
 * @brief         stop avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislavsync_stop(alisl_handle handle)
{
	struct avsync_private *priv = (struct avsync_private *)handle;
	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d, start_cnt: %d\n",
	       m_avsync_dev, m_avsync_dev->fd, priv->start_cnt);
	pthread_mutex_lock(&m_mutex);
	if (priv->start_cnt == 0) {
		//avsync not started, just return
		pthread_mutex_unlock(&m_mutex);
		return ret;
	}
	/* 
	for PIP, two streams were playing, when one(audio not playing) stopped, the other
	one(audio playing) continued playing. In this case, when the stopped one called
	avsync stop, the avsync can't not stop immediately, we use start_cnt to control 
	it, when all the streams stopped playing, stop avsync
	*/
    if(--priv->start_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return ret;
	}
	SL_DBG("IOCTL_NAME: ALI_AVSYNC_STOP\n");
	if (ioctl(priv->fd, ALI_AVSYNC_STOP)) {
        SL_ERR("ioctl ALI_AVSYNC_STOP failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	priv->video_id = AVSYNC_VIDEO_NB;
	pthread_mutex_unlock(&m_mutex);
	return ret;	
}

/**
 * @brief         reset avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    
 * @return        alisl_retcode
 * @note          
 *
 */
alisl_retcode alislavsync_reset(alisl_handle handle)
{   
	struct avsync_private *priv = (struct avsync_private *)handle;
	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);
        SL_DBG("IOCTL_NAME: ALI_AVSYNC_RESET\n");
	if (ioctl(priv->fd, ALI_AVSYNC_RESET)) {
        SL_ERR("ioctl ALI_AVSYNC_RESET failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

/**
 * @brief         set av sync mode
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[in]     sync_mode, default: AVSYNC_PCR  
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislavsync_set_av_sync_mode(alisl_handle handle, 
    enum avsync_sync_mode sync_mode)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    AVSYNC_MODE_E avsync_mode;
    alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
    SL_DBG("avsync_dev:%p avsync_dev->fd:%d, sync_mode: %d\n",
	    m_avsync_dev, m_avsync_dev->fd, sync_mode);
	pthread_mutex_lock(&m_mutex);    
	switch(sync_mode){
	    case AVSYNC_PCR:
	        avsync_mode = AVSYNC_MODE_PCR;
	        break;
	    case AVSYNC_AUDIO:
	        avsync_mode = AVSYNC_MODE_AUDIO;
	        break;
	    case AVSYNC_AV_FREERUN:    
	        avsync_mode = AVSYNC_MODE_AV_FREERUN;
	        break;
	    default:
	        avsync_mode = AVSYNC_MODE_PCR;
	        break;
	}
	SL_DBG("IOCTL_NAME:ALI_AVSYNC_SET_SYNC_MODE, priv->fd: %d,avsync_mode: %d\n",priv->fd,avsync_mode);
	if (ioctl(priv->fd, ALI_AVSYNC_SET_SYNC_MODE, avsync_mode)) {
        SL_ERR("ioctl ALI_AVSYNC_SET_SYNC_MODE failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret; 
}

/**
 * @brief         get av sync mode
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    sync_mode
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislavsync_get_av_sync_mode(alisl_handle handle, 
    enum avsync_sync_mode *sync_mode)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    AVSYNC_MODE_E avsync_mode;
    
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);
	if (ioctl(priv->fd, ALI_AVSYNC_GET_SYNC_MODE, &avsync_mode)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_GET_SYNC_MODE failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}
    SL_DBG("IOCTL_NAME: ALI_AVSYNC_GET_SYNC_MODE, priv->fd: %d,avsync_mode: %d\n",priv->fd,avsync_mode);
	pthread_mutex_unlock(&m_mutex);
	switch(avsync_mode){
    case AVSYNC_MODE_PCR:
        *sync_mode = AVSYNC_PCR;
        break;
    case AVSYNC_MODE_AUDIO:
        *sync_mode = AVSYNC_AUDIO;
        break;
    case AVSYNC_MODE_V_FREERUN:
    case AVSYNC_MODE_A_FREERUN:
    case AVSYNC_MODE_AV_FREERUN:    
        *sync_mode = AVSYNC_AV_FREERUN;
        break;
    default:
        *sync_mode = AVSYNC_INVALID;
        break;
	}
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d, sync_mode: %d\n",
	       m_avsync_dev, m_avsync_dev->fd, *sync_mode);
	return AVSYNC_ERR_NOERR; 
}

/**
 * @brief         set source type
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    src
 * @return        alisl_retcode
 * @note           
 *
 */
alisl_retcode alislavsync_set_sourcetype(alisl_handle handle,
    enum avsync_srctype srctype)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	SL_DBG("IOCTL_NAME: ALI_AVSYNC_SET_SOURCE_TYPE, priv->fd:%d, srctype: %d\n",
	       priv->fd, srctype);
	pthread_mutex_lock(&m_mutex);    
	if (ioctl(priv->fd, ALI_AVSYNC_SET_SOURCE_TYPE, srctype)) {
        SL_ERR("ioctl ALI_AVSYNC_SET_SOURCE_TYPE failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

/**
 * @brief         get source type
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    src
 * @return        alisl_retcode
 * @note           
 *
 */    
alisl_retcode alislavsync_get_sourcetype(alisl_handle handle,
    enum avsync_srctype *srctype)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;

	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);    
	if (ioctl(priv->fd, ALI_AVSYNC_GET_SOURCE_TYPE, srctype)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_GET_SOURCE_TYPE failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	SL_DBG("IOCTL_NAME:ALI_AVSYNC_GET_SOURCE_TYPE, priv->fd: %d,srctype: %d\n",
	    priv->fd, *srctype);	
	return AVSYNC_ERR_NOERR;
}

/**
 * @brief         get current pts
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    pts
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_current_pts(alisl_handle handle,
    unsigned int *pts)    
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    struct ali_avsync_ioctl_command io_param;
    
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	
    io_param.ioctl_cmd = AVSYNC_IO_GET_CURRENT_PLAY_PTS;
    io_param.param = (unsigned long)pts;
    pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_GET_CURRENT_PLAY_PTS, priv->fd: %d\n",priv->fd);	
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_IO_COMMAND failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d, pts: %d\n",
	       m_avsync_dev, m_avsync_dev->fd, *(unsigned int *)io_param.param);
	return AVSYNC_ERR_NOERR;        
}

/**
 * @brief         get current stc
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle is avsync device pointer
 * @param[out]    stc
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_current_stc(alisl_handle handle,
    unsigned int *stc)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    struct ali_avsync_ioctl_command io_param;
    
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	
    io_param.ioctl_cmd = AVSYNC_IO_GET_CURRENT_STC;
    io_param.param = (unsigned long)stc;
    pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_GET_CURRENT_STC, priv->fd: %d\n",priv->fd);	
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_IO_COMMAND failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d, stc: %d\n",
	       m_avsync_dev, m_avsync_dev->fd, *(unsigned int *)io_param.param);

	return AVSYNC_ERR_NOERR;
}  

/**
 * @brief         get current status
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    status           current status
 * @return        alisl_retcode    0:success, other: fail 
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_status(alisl_handle handle,
    struct avsync_cur_status *status)
{    
   	struct avsync_private *priv = (struct avsync_private *)handle;
    avsync_status_t avsync_status;
    
	if (priv == NULL || status == NULL) {
		SL_ERR("AVSYNC NULL data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);    
	if (ioctl(priv->fd, ALI_AVSYNC_GET_STATUS, &avsync_status)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_GET_STATUS failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}
    SL_DBG("IOCTL_NAME:ALI_AVSYNC_GET_STATUS, priv->fd: %d,device_status:%d, vpts_offset: %d, apts_offset: %d\n"\
           "v_sync_flg: %d, a_sync_flg: %d, cur_vpts: %d, cur_apts: %d\n",
	    priv->fd,avsync_status.device_status,avsync_status.vpts_offset,avsync_status.apts_offset,avsync_status.v_sync_flg,
	    avsync_status.a_sync_flg,avsync_status.cur_vpts,avsync_status.cur_apts);	
	pthread_mutex_unlock(&m_mutex);
	status->device_status = avsync_status.device_status;
	status->vpts_offset = avsync_status.vpts_offset;
	status->apts_offset = avsync_status.apts_offset;
	status->v_sync_flg = avsync_status.v_sync_flg;
	status->a_sync_flg = avsync_status.a_sync_flg;
	status->cur_vpts =  avsync_status.cur_vpts;
	status->cur_apts = avsync_status.cur_apts;	
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d\n",
	       m_avsync_dev, m_avsync_dev->fd);
	return AVSYNC_ERR_NOERR;
}    

/**
 * @brief         get statistics info
 * @author        nick.li
 * @date          2017-03-30
 * @param[in]     handle           avsync device pointer
 * @param[out]    statistics       statistics info
 * @return        alisl_retcode    0:success, other: fail 
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_statistics(alisl_handle handle,
    struct avsync_statistics_info *statistics)
{    
    struct avsync_private *priv = (struct avsync_private *)handle;
    avsync_statistics_t avsync_statistics;
    
    if (priv == NULL || statistics == NULL) {
        SL_ERR("AVSYNC NULL data structure!\n");
        return AVSYNC_ERR_INVAL;
    }
    pthread_mutex_lock(&m_mutex);
    if (ioctl(priv->fd, ALI_AVSYNC_GET_STATISTICS, &avsync_statistics)) {
        pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_GET_STATISTICS failed\n");
        return AVSYNC_ERR_IOCTLFAILED;
    }
    SL_DBG("IOCTL_NAME:ALI_AVSYNC_GET_STATISTICS, priv->fd: %d,total_v_play_cnt:%d, total_v_drop_cnt: %d,"\
           "total_v_hold_cnt: %d, total_v_freerun_cnt: %d, total_a_play_cnt: %d\n"\
           "total_a_drop_cnt: %d, total_a_hold_cnt: %d, total_a_freerun_cnt: %d\n",
	    priv->fd,avsync_statistics.total_v_play_cnt,avsync_statistics.total_v_drop_cnt,avsync_statistics.total_v_hold_cnt,
	    avsync_statistics.total_v_freerun_cnt,avsync_statistics.total_a_play_cnt,avsync_statistics.total_a_drop_cnt,avsync_statistics.total_a_hold_cnt,
	    avsync_statistics.total_a_freerun_cnt);	
    pthread_mutex_unlock(&m_mutex);
    statistics->total_v_play_cnt = avsync_statistics.total_v_play_cnt;             //!<Synchronization video frame numbers.
    statistics->total_v_drop_cnt = avsync_statistics.total_v_drop_cnt;             //!< Dropped video frame numbers.
    statistics->total_v_hold_cnt = avsync_statistics.total_v_hold_cnt;             //!< Repeated video frame numbers.
    statistics->total_v_freerun_cnt = avsync_statistics.total_v_freerun_cnt;       //!<Free-run video frame numbers.
    statistics->total_a_play_cnt = avsync_statistics.total_a_play_cnt;             //!< Synchronization audio frame numbers.
    statistics->total_a_drop_cnt = avsync_statistics.total_a_drop_cnt;             //!< Dropped audio frame numbers.
    statistics->total_a_hold_cnt = avsync_statistics.total_a_hold_cnt;             //!<Repeated audio frame numbers.
    statistics->total_a_freerun_cnt = avsync_statistics.total_a_freerun_cnt;       //!< Free-run audio frame numbers.
    SL_DBG("avsync_dev:%p avsync_dev->fd:%d\n", m_avsync_dev, m_avsync_dev->fd);
    return AVSYNC_ERR_NOERR;
}

/**
 * @brief         set fast channel change mode enable or disable
 * @author        andy.yu
 * @date          2014-12-31
 * @param[in]     handle is avsync device pointer
 * @param[in]   1: enable ,0:disable
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_set_fcc_onoff(alisl_handle handle,
    int enable)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    struct ali_avsync_ioctl_command io_param;
    alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	
    io_param.ioctl_cmd = AVSYNC_IO_SET_FASTMODE;
    io_param.param = enable;
    SL_DBG("alislavsync_set_fcc_onoff,flag =%d\n",enable);
    pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME: AVSYNC_IO_SET_FASTMODE, priv->fd: %d, enable: %d\n",
	    priv->fd,io_param.param);	
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
        SL_ERR("ioctl ALI_AVSYNC_IO_COMMAND failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret;
}  

/**
 * @brief         flush av sync
 * @author        andy.yu
 * @date          2015-3-20
 * @param[in]     handle is avsync device pointer
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_flush(alisl_handle handle)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    struct ali_avsync_ioctl_command io_param;
    alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
    io_param.ioctl_cmd = AVSYNC_IO_FLUSH;
    SL_DBG("AVSYNC_IO_FLUSH\n");
    pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_FLUSH, priv->fd: %d\n",priv->fd);	
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
        SL_ERR("ioctl ALI_AVSYNC_IO_COMMAND failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
    SL_DBG("IOCTL_NAME:ALI_AVSYNC_STOP\n");
	if (ioctl(priv->fd, ALI_AVSYNC_STOP)) {
        SL_ERR("ioctl ALI_AVSYNC_STOP failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
    SL_DBG("IOCTL_NAME:ALI_AVSYNC_START\n");
	if (ioctl(priv->fd, ALI_AVSYNC_START)) {
        SL_ERR("ioctl ALI_AVSYNC_START failed\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret;	
}

/**
 * @brief         get params of av sync
 * @author        wendy.he
 * @date          2015-7-13
 * @param[in]     handle is avsync device pointer
 * @param[out]    params           current parameters
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_advance_params(alisl_handle handle, 
    struct avsync_advance_param * params)
{    
   	struct avsync_private *priv = (struct avsync_private *)handle;
    avsync_adv_param_t avsync_param;
    
	if (priv == NULL || params == NULL) {
		SL_ERR("AVSYNC NULL data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);    
	if (ioctl(priv->fd, ALI_AVSYNC_GET_ADVANCE_PARAMS, &avsync_param)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_GET_ADVANCE_PARAMS failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}
    SL_DBG("IOCTL_NAME:ALI_AVSYNC_GET_ADVANCE_PARAMS, priv->fd: %d,afreerun_thres:%d, vfreerun_thres: %d, disable_monitor: %d\n"\
        "disable_first_video_freerun: %d, dual_output_sd_delay: %d, pts_adjust_threshold: %d, rsvd2: %d, rsvd3: %d\n",
        priv->fd,avsync_param.afreerun_thres,avsync_param.vfreerun_thres,avsync_param.disable_monitor,avsync_param.disable_first_video_freerun
        ,avsync_param.dual_output_sd_delay,avsync_param.pts_adjust_threshold,avsync_param.rsvd2,avsync_param.rsvd3);
	pthread_mutex_unlock(&m_mutex);
	params->afreerun_thres = avsync_param.afreerun_thres;
	params->vfreerun_thres = avsync_param.vfreerun_thres;
	params->disable_monitor = avsync_param.disable_monitor;
	params->disable_first_video_freerun = avsync_param.disable_first_video_freerun;
	params->dual_output_sd_delay = avsync_param.dual_output_sd_delay;
	params->pts_adjust_threshold = avsync_param.pts_adjust_threshold;
	params->rsvd2 = avsync_param.rsvd2;
	params->rsvd3 = avsync_param.rsvd3;
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d\n",
	       m_avsync_dev, m_avsync_dev->fd);
	return AVSYNC_ERR_NOERR;
}

/**
 * @brief         configure av sync
 * @author        wendy.he
 * @date          2015-7-13
 * @param[in]     handle is avsync device pointer
 * @param[out]    params           parameters to set
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_config_advance_params(alisl_handle handle, 
    struct avsync_advance_param * params)
{    
   	struct avsync_private *priv = (struct avsync_private *)handle;
    avsync_adv_param_t avsync_param;
    
	if (priv == NULL || params == NULL) {
		SL_ERR("AVSYNC NULL data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	    
	avsync_param.afreerun_thres = params->afreerun_thres;
	avsync_param.vfreerun_thres = params->vfreerun_thres;
	avsync_param.disable_monitor = params->disable_monitor;
	avsync_param.disable_first_video_freerun = params->disable_first_video_freerun;
	avsync_param.dual_output_sd_delay = params->dual_output_sd_delay;
	avsync_param.pts_adjust_threshold = params->pts_adjust_threshold;
	avsync_param.rsvd2 = params->rsvd2;
	avsync_param.rsvd3 = params->rsvd3;
	SL_DBG("avsync_dev:%p avsync_dev->fd:%d\n",
	       m_avsync_dev, m_avsync_dev->fd);
	SL_DBG("IOCTL_NAME:ALI_AVSYNC_CONFIG_ADVANCE_PARAMS, afreerun_thres:%d, vfreerun_thres:%d, disable_monitor:%d\n" \
	       "disable_first_video_freerun:%d, dual_output_sd_delay: %d, pts_adjust_threshold: %d\n",
	       params->afreerun_thres, params->vfreerun_thres,
	       params->disable_monitor, params->disable_first_video_freerun,
	       params->dual_output_sd_delay, params->pts_adjust_threshold);
	pthread_mutex_lock(&m_mutex);    
	if (ioctl(priv->fd, ALI_AVSYNC_CONFIG_ADVANCE_PARAMS, &avsync_param)) {
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_CONFIG_ADVANCE_PARAMS failed\n");
		return AVSYNC_ERR_IOCTLFAILED;
	}  
	pthread_mutex_unlock(&m_mutex);
	return AVSYNC_ERR_NOERR;
}

/**
 * @brief         configure av sync
 * @author        wendy.he
 * @date          2015-7-13
 * @param[in]     handle is avsync device pointer
 * @param[in]     level           avsync video smoothly level
 * @param[in]     interval        parameters to set
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_set_video_smoothly_play(alisl_handle handle,  
        enum avsync_video_smooth_level level, unsigned char interval)
{
	struct ali_avsync_rpc_pars rpc_pars;		
	unsigned char onoff = (interval==0)?0:1;
   	struct avsync_private *priv = (struct avsync_private *)handle;
//    avsync_adv_param_t avsync_param;
    
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.arg_num = 3;
	rpc_pars.arg[0].arg = (void *)&onoff;
	rpc_pars.arg[0].arg_size = sizeof(onoff);			
	rpc_pars.arg[1].arg = (void *)&level;
	rpc_pars.arg[1].arg_size = sizeof(level);
	rpc_pars.arg[2].arg = (void *)&interval;
	rpc_pars.arg[2].arg_size = sizeof(interval);
    pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:ALI_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF, priv->fd: %d,onoff: %d, level: %d, interval: %d\n",
        priv->fd,onoff,level,interval);
	if(ioctl(priv->fd, ALI_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF, &rpc_pars))
	{
		pthread_mutex_unlock(&m_mutex);
        SL_ERR("ioctl ALI_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF failed\n");
        return AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return AVSYNC_ERR_NOERR;	
}

alisl_retcode alislavsync_set_data_type(alisl_handle handle,
	enum avsync_data_type type)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
    struct ali_avsync_ioctl_command io_param;
    alisl_retcode ret = AVSYNC_ERR_NOERR;
    unsigned long pts_unit = 45000;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	pthread_mutex_lock(&m_mutex);
	if (type == AVSYNC_DATA_TYPE_TS) {
		pts_unit = 45000;
		memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
		//register callback function to decv/deca/dmx for ts play
		io_param.ioctl_cmd = AVSYNC_IO_REG_CALLBACK;
		SL_DBG("IOCTL_NAME:AVSYNC_IO_REG_CALLBACK, priv->fd: %d\n",priv->fd);
		if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
			SL_ERR("AVSYNC reg callback fail!\n");
			ret = AVSYNC_ERR_IOCTLFAILED;
			goto quit;
		}
		
	} else if (type == AVSYNC_DATA_TYPE_ES) {
		pts_unit = 1000;
		memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
		//unregister callback function to decv/deca/dmx for media player
		io_param.ioctl_cmd = AVSYNC_IO_UNREG_CALLBACK;
		SL_DBG("IOCTL_NAME:AVSYNC_IO_UNREG_CALLBACK, priv->fd: %d\n",priv->fd);
		if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
			SL_ERR("AVSYNC unreg callback fail!\n");
			ret = AVSYNC_ERR_IOCTLFAILED;
			goto quit;
		}
		
	} else {
		SL_ERR("AVSYNC not support the data type: %d\n", type);
		ret = AVSYNC_ERR_INVAL;
		goto quit;
	}
	SL_DBG("AVSYNC_IO_SET_PTS_UNIT_HZ: %d\n", pts_unit);
	memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
	io_param.param = pts_unit;
	io_param.ioctl_cmd = AVSYNC_IO_SET_PTS_UNIT_HZ;
    SL_DBG("IOCTL_NAME:AVSYNC_IO_SET_PTS_UNIT_HZ, priv->fd: %d,pts_unit: %ld\n",priv->fd,io_param.param);
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		SL_ERR("AVSYNC set pts unit fail!\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
		goto quit;
	}
quit:
	pthread_mutex_unlock(&m_mutex);
	return ret;
}  

alisl_retcode alislavsync_set_video_id(alisl_handle handle, 
    enum avsync_video_id video_id)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
   	struct ali_avsync_ioctl_command io_param;
   	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	SL_DBG("AVSYNC set video id: %d\n", video_id);
	memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
	io_param.ioctl_cmd = AVSYNC_IO_SET_VDEC_ID;
	io_param.param = video_id;
	priv->video_id = video_id;
	pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_SET_VDEC_ID, priv->fd: %d,video_id: %d\n",priv->fd,io_param.param);
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		SL_ERR("AVSYNC set video id fail!\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret; 
}  

alisl_retcode alislavsync_get_video_id(alisl_handle handle, 
    enum avsync_video_id* video_id)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;

	if (priv == NULL || video_id == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	*video_id = priv->video_id;
	SL_DBG("AVSYNC get video id: %d\n", *video_id);
	return AVSYNC_ERR_NOERR; 
}    

alisl_retcode alislavsync_change_audio_track(alisl_handle handle)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
   	struct ali_avsync_ioctl_command io_param;
   	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	SL_DBG("avsync change audio track\n");
	memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
	io_param.ioctl_cmd = AVSYNC_IO_CHANGE_AUDIO_TRACK;
	pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_CHANGE_AUDIO_TRACK, priv->fd: %d\n",priv->fd);
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		SL_ERR("AVSYNC change audio track fail!\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}	
	pthread_mutex_unlock(&m_mutex);
	return ret; 
}

alisl_retcode alislavsync_pip_swap_audio(alisl_handle handle, 
                                         struct avsync_pip_swap_param* swap_param)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
   	struct ali_avsync_ioctl_command io_param;
   	struct avsync_pip_param_t pip_param;
   	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL || swap_param == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	SL_DBG("avsync swap pip audio\n");
	memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
	memset(&pip_param,0,sizeof(struct avsync_pip_param_t));
	pip_param.vdec_id = swap_param->vdec_id;
	pip_param.dmx_id = swap_param->see_dmx_id;
	if (swap_param->data_type == AVSYNC_DATA_TYPE_ES) {
		pip_param.unit = 1000;
		pip_param.live_play = 0;
	} else {
		pip_param.unit = 45000;
		pip_param.live_play = 1;
	}

	switch(swap_param->src_type){
	    case AVSYNC_FROM_TUNER:
	        pip_param.src_type = AVSYNC_SRC_TURNER;
	        break;
	    case AVSYNC_FROM_SWDMX:
	        pip_param.src_type = AVSYNC_SRC_SWDMX;
	        break;
	    case AVSYNC_FROM_HDD_MP:    
	        pip_param.src_type = AVSYNC_SRC_HDD_MP;
	        break;
	    case AVSYNC_FROM_NETWORK_MP:
	    	pip_param.src_type = AVSYNC_SRC_NETWORK_MP;
	    	break;
	    default:
	        pip_param.src_type = AVSYNC_SRC_TURNER;
	        break;
	}

	switch(swap_param->sync_mode){
	    case AVSYNC_PCR:
	        pip_param.sync_mode = AVSYNC_MODE_PCR;
	        break;
	    case AVSYNC_AUDIO:
	        pip_param.sync_mode = AVSYNC_MODE_AUDIO;
	        break;
	    case AVSYNC_AV_FREERUN:    
	        pip_param.sync_mode = AVSYNC_MODE_AV_FREERUN;
	        break;
	    default:
	        pip_param.sync_mode = AVSYNC_MODE_PCR;
	        break;
	}
	io_param.ioctl_cmd = AVSYNC_IO_PIP_SWITCH;
	io_param.param = (unsigned long)&pip_param;
	pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_PIP_SWITCH, priv->fd: %d, vdec_id: %d, dmx_id:%d\n,"\
        "unit: %d, live_play: %d,src_type: %d, sync_mode: %d\n",priv->fd,
        pip_param.vdec_id,pip_param.dmx_id,pip_param.unit,
        pip_param.live_play,pip_param.src_type,pip_param.sync_mode);
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		SL_ERR("AVSYNC change audio track fail!\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}	
	pthread_mutex_unlock(&m_mutex);
	return ret; 
}   

alisl_retcode alislavsync_set_see_dmx_id(alisl_handle handle, int see_dmx_id)
{
   	struct avsync_private *priv = (struct avsync_private *)handle;
   	struct ali_avsync_ioctl_command io_param;
   	alisl_retcode ret = AVSYNC_ERR_NOERR;
	if (priv == NULL) {
		SL_ERR("AVSYNC NULL private data structure!\n");
		return AVSYNC_ERR_INVAL;
	}
	SL_DBG("AVSYNC set see dmx id: %d\n", see_dmx_id);
	memset(&io_param,0,sizeof(struct ali_avsync_ioctl_command));
	io_param.ioctl_cmd = AVSYNC_IO_SET_DMX_ID;
	io_param.param = see_dmx_id;
	pthread_mutex_lock(&m_mutex);
    SL_DBG("IOCTL_NAME:AVSYNC_IO_SET_DMX_ID, priv->fd: %d, see_dmx_id: %d\n",priv->fd,io_param.param);
	if (ioctl(priv->fd, ALI_AVSYNC_IO_COMMAND, &io_param)) {
		SL_ERR("AVSYNC set see dmx id fail!\n");
		ret = AVSYNC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret; 
}
