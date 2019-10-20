/** @file     alislavsync.h
 *  @brief    include struct and function defination used in avsync share library
 *  @author   wendy.he
 *  @date     2014-8-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2014-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef _AVSYNC_ALISL_
#define _AVSYNC_ALISL_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum avsync_sync_mode
{
    AVSYNC_PCR,        //default is AVSYNC_PCR. Audio, video synchronize on a PCR basis
    AVSYNC_AUDIO,      //Video synchronize audio on an audio basis
    AVSYNC_AV_FREERUN, //Audio and video free run
    AVSYNC_INVALID
}avsync_sync_mode_t;

typedef enum avsync_srctype
{
    AVSYNC_FROM_TUNER,      //from tuner 
    AVSYNC_FROM_SWDMX,       //from software demux
    AVSYNC_FROM_HDD_MP,      //from local media player
    AVSYNC_FROM_NETWORK_MP,  //from network media player
}avsync_srctype_t;

typedef enum aysync_dec_status
{
    AVSYNC_DEV_ATTACHED,   //dev is attached
    AVSYNC_DEV_OPENED,     //dev is opend
    AVSYNC_DEV_CLOSED,     //dev is close
    AVSYNC_DEV_STARTED,    //dev is started
    AVSYNC_DEV_STOPPED,	   //dev is stopped
}aysync_dec_status_t;

/* The level of video playback smoothly. */
typedef enum avsync_video_smooth_level
{
	AVSYNC_VIDEO_SMOOTH_LEVEL_1, 
	AVSYNC_VIDEO_SMOOTH_LEVEL_2, 
}avsync_video_smooth_level_t;

typedef enum avsync_data_type
{
    AVSYNC_DATA_TYPE_TS,        //for ts stream
    AVSYNC_DATA_TYPE_ES     //for media(es)
}avsync_data_type_t;

typedef enum avsync_video_id
{
	AVSYNC_VIDEO_0 = 0,
	AVSYNC_VIDEO_1,
	AVSYNC_VIDEO_NB
}avsync_video_id_t;

typedef struct avsync_cur_status 
{
	enum aysync_dec_status device_status; //device status
	unsigned int vpts_offset; //video pts offset
	unsigned int apts_offset; //audio pts offset
	unsigned char v_sync_flg; //video sync flag: 0:async, 1:sync
	unsigned char a_sync_flg; //audio sync flag: 0:async, 1:sync
	unsigned int cur_vpts; //current video pts
	unsigned int cur_apts; //current audio pts
}avsync_cur_status_t;

typedef struct avsync_statistics_info
{
	unsigned int total_v_play_cnt;            //!<Synchronization video frame numbers.
	unsigned int total_v_drop_cnt;            //!< Dropped video frame numbers.
	unsigned int total_v_hold_cnt;            //!< Repeated video frame numbers.
	unsigned int total_v_freerun_cnt;         //!<Free-run video frame numbers.
	unsigned int total_a_play_cnt;            //!< Synchronization audio frame numbers.
	unsigned int total_a_drop_cnt;            //!< Dropped audio frame numbers.
	unsigned int total_a_hold_cnt;            //!<Repeated audio frame numbers.
	unsigned int total_a_freerun_cnt;         //!< Free-run audio frame numbers.
}avsync_statistics_info_t;

typedef struct avsync_advance_param 
{
	unsigned int afreerun_thres;                   //!< Audio free run threadhold, with the default value of 10s.
	unsigned int vfreerun_thres;                  //!< Video free run threadhold, with the default value of 15s.
	unsigned char  disable_monitor;                //!< Disable monitor function, with the default value of 1.
	unsigned char disable_first_video_freerun;    //!< Disable free run function of the video first frame, with the default value of 1.
	unsigned short  dual_output_sd_delay;        //!< Standard definition output delay of video dual output.
	unsigned int  pts_adjust_threshold;
	unsigned int rsvd2; 
	unsigned int rsvd3;
}avsync_advance_param_t;

typedef struct avsync_pip_swap_param
{
    avsync_video_id_t vdec_id;
    avsync_data_type_t data_type;//ts or es
    avsync_sync_mode_t sync_mode;
    avsync_srctype_t src_type;
    int see_dmx_id;
}avsync_pip_swap_param_t;

/**  
 * @brief         Open ali_avsync0 for common usage. 
 *                It should be opend before opening demux, audio and video devices.
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]   
 * @return        alisl_retcode    0:success, other: fail
 * @note
 *
 */
alisl_retcode alislavsync_open(alisl_handle *handle);

/**
 * @brief         close avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislavsync_close(alisl_handle handle);

/**
 * @brief         start avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer, 
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislavsync_start(alisl_handle handle);

/**
 * @brief         stop avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislavsync_stop(alisl_handle handle);

/**
 * @brief         reset avsync device
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    
 * @return        alisl_retcode    0:success, other: fail
 * @note          
 *
 */
alisl_retcode alislavsync_reset(alisl_handle handle);

/**
 * @brief         set av sync mode
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[in]     sync_mode        the sync mode to be setted, default: AVSYNC_PCR  
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_set_av_sync_mode(alisl_handle handle, 
    enum avsync_sync_mode sync_mode);

/**
 * @brief         get av sync mode
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    sync_mode        current sync mode  
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_get_av_sync_mode(alisl_handle handle, 
    enum avsync_sync_mode *sync_mode);

/**
 * @brief         set source type
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[in]     srctype          the source type to be setted 
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_set_sourcetype(alisl_handle handle,
    enum avsync_srctype srctype);

/**
 * @brief         get source type
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    srctype          current source type  
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */    
alisl_retcode alislavsync_get_sourcetype(alisl_handle handle,
    enum avsync_srctype *srctype);

/**
 * @brief         get current pts               
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    pts              audio pts: both audio and video,
 *                                 audio pts: only audio
 *                                 video pts: only video  
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_current_pts(alisl_handle handle,
    unsigned int *pts);    

/**
 * @brief         get current stc
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[out]    pts              current stc
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */ 
alisl_retcode alislavsync_get_current_stc(alisl_handle handle,
    unsigned int *stc);

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
    struct avsync_cur_status *status);    

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
    struct avsync_statistics_info *statistics);

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
    int enable);
    
/**
 * @brief         flush av sync
 * @author        andy.yu
 * @date          2015-3-20
 * @param[in]     handle is avsync device pointer
 * @return        alisl_retcode
 * @note           
 *
 */ 
alisl_retcode alislavsync_flush(alisl_handle handle);

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
    struct avsync_advance_param * params);

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
    struct avsync_advance_param * params);

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
    enum avsync_video_smooth_level level, unsigned char interval);
    

/**
 * @brief         set av data type
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[in]     type             data type
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_set_data_type(alisl_handle handle, 
    enum avsync_data_type type);

/**
 * @brief         set av video id
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[in]     video_id         video id
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_set_video_id(alisl_handle handle, 
    enum avsync_video_id video_id);


/**
 * @brief         get av video id
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @param[in]     video_id         video id
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_get_video_id(alisl_handle handle, 
    enum avsync_video_id* video_id);
    
/**
 * @brief         change audio track
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_change_audio_track(alisl_handle handle);

/**
 * @brief         swap audio
 * @author        wendy.he
 * @date          2014-8-8
 * @param[in]     handle           avsync device pointer
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_pip_swap_audio(alisl_handle handle, 
                                         struct avsync_pip_swap_param* swap_param);

/**
 * @brief         set see dmx id to avsync
 * @author        wendy.he
 * @date          2016-3-17
 * @param[in]     handle           avsync device pointer
 * @param[in]     see_dmx_id       current 0, 1
 * @return        alisl_retcode    0:success, other: fail
 * @note           
 *
 */
alisl_retcode alislavsync_set_see_dmx_id(alisl_handle handle, int see_dmx_id);

#ifdef __cplusplus
}
#endif

#endif /*_AVSYNC_ALISL_*/
