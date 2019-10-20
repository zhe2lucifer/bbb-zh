/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislvdec.h
 *  @brief          VDEC share library module exported fundtions and types.
 *
 *  @version        1.0
 *  @date           31/12/2013
 *  @revision       none
 *
 *  @author         glen.dai <glen.dai@alitech.com>
 */

#ifndef _ALISLVDEC_H_
#define _ALISLVDEC_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "alipltfretcode.h"
#include "alisl_types.h"

#define VDEC_MP_FLAG_HAS_LICENSED   (1 << 0)  /**< Has full license (Internal use) */
#define VDEC_MP_FLAG_AVC1_FORMAT    (1 << 1)  /**< AVC nalu has nalu_size on the head */
#define VDEC_MP_FLAG_MPEG4_DECODER  (1 << 2)  /**< I'm a general mpeg4 decoder (Internal use) */
#define VDEC_MP_FLAG_LAST_REQ_FRAME (1 << 16) /**< An flag to free the last requested frame (Internal use) */

enum vdec_decoder_type
{
    VDEC_DECODER_AVC,                   /**< AVC decoder */
    VDEC_DECODER_MPEG,                  /**< MPEG decoder */
	VDEC_DECODER_AVS,					/**< AVS+ decoder */
	VDEC_DECODER_HEVC,                  /**< HEVC decoder */
	VDEC_DECODER_VC1,
	VDEC_DECODER_MPEG4,
	VDEC_DECODER_VP8,
	VDEC_DECODER_RV,
	VDEC_DECODER_MJPG,
};

struct vdec_color
{
    char y;                             /**< y channel */
    char u;                             /**< u channel */
    char v;                             /**< v channel */
};

struct vdec_rect
{
    uint32_t x;                         /**< x of rectangle's topleft point */
    uint32_t y;                         /**< y of rectangle's topleft point */
    uint32_t w;                         /**< width of rectangle */
    uint32_t h;                         /**< height of rectangle */
};

struct vdec_pvr_config_param
{
    int32_t is_scrambled;
};


struct vdec_frame_cap
{
    uint8_t *buf_addr;                  /**< buf address to put the captured picture */
    uint32_t buf_sz;                    /**< size of the buffer */
    uint32_t cap_buf_sz;                /**< real buffer size used by VDEC device */
    uint32_t cap_pic_width;             /**< captured picture's width */
    uint32_t cap_pic_height;            /**< captured picture's height */
};

enum vdec_info_change_flag {
	eVDEC_CHANGE_DIMENSIONS = 0,
	eVDEC_CHANGE_FRAMERATE,
	eVDEC_CHANGE_AFD,
	eVDEC_CHANGE_SAR,
};

enum vdec_vbv_buf_mode {
    /** VBV Buffer overflow need reset, for tuner live play */
    VBV_BUF_OVERFLOW_RST_MODE = 0,
    /** VBV Buffer overflow no need reset, for pvr playback */ 
    VBV_BUF_BLOCK_FULL_MODE = 1,
};

struct vdec_info_from_cb
{
	enum vdec_info_change_flag flag;
	uint32_t pic_width;					/**< picture width */
	uint32_t pic_height;				/**< picture height */
	uint32_t frame_rate;				/**< frame rate */
	uint8_t active_format;				/**< frame rate */
	uint32_t sar_width;                   /**< Sample aspect ratio width */
    uint32_t sar_height;                  /**< Sample aspect ratio height */
};

enum vdec_playback_speed
{
	VDEC_PLAYBACK_SPEED_INVALID=-1,
	VDEC_PLAYBACK_SPEED_1_2,
    VDEC_PLAYBACK_SPEED_1_4,
    VDEC_PLAYBACK_SPEED_1_8,
    VDEC_PLAYBACK_SPEED_STEP,
    VDEC_PLAYBACK_SPEED_1,
    VDEC_PLAYBACK_SPEED_2,
    VDEC_PLAYBACK_SPEED_4,
    VDEC_PLAYBACK_SPEED_8,
    VDEC_PLAYBACK_SPEED_16,
    VDEC_PLAYBACK_SPEED_32, 
};

enum vdec_playback_direction
{
	VDEC_PLAYBACK_INVALID=-1,
	VDEC_PLAYBACK_FORWARD,
    VDEC_PLAYBACK_BACKWARD,
};

enum vdec_playback_method
{
	VDEC_PLAYBACK_METHOD_INVALID=-1,
    VDEC_PLAYBACK_METHOD_CONTINUOUS,        /* stream is injected in continous chunks */
	VDEC_PLAYBACK_METHOD_NONCONTINUOUS,     /* stream is injected in non-continous chunks */
};

enum vdec_playback_mode
{
    VDEC_PLAYBACK_MODE_NORMAL,    /**< Normal */
    VDEC_PLAYBACK_MODE_FF,        /**< Fast forward */
    VDEC_PLAYBACK_MODE_FB         /**< Fast backward */
};

enum vdec_decoder_status
{
	VDEC_DECODER_STATUS_NODATA = 0,
	VDEC_DECODER_STATUS_DECODING,
};

enum vdec_error_code
{
	VDEC_ERROR_CODE_NODATA = 0,
	VDEC_ERROR_CODE_HARDWARE,
	VDEC_ERROR_CODE_SYNC,
	VDEC_ERROR_CODE_FRAMEDROP,
	VDEC_ERROR_CODE_FRAMEHOLD,
	VDEC_ERROR_CODE_GIVEUPSEQ,
	VDEC_ERROR_CODE_INVDATA,
};

enum vdec_callback_type
{
    SL_VDEC_CB_FIRST_SHOWED = 0,			/**< callback type for Frame showed event */
    SL_VDEC_CB_MODE_SWITH_OK,				/**< callback type for MODE switck OK */
    SL_VDEC_CB_BACKWARD_RESTART_GOP,		/**< callback type for backward restart GOP */
    SL_VDEC_CB_FIRST_HEAD_PARSED,			/**< callback type for head parsed */
    SL_VDEC_CB_FRIST_I_FRAME_DECODED,		/**< callback type for I frame decodered */
    SL_VDEC_CB_USER_DATA_PARSED,			/**< callback type for user data parsed, used for get colse caption data. */
    SL_VDEC_CB_INFO_CHANGED,				/**< callback type for video information changed. */
    SL_VDEC_CB_STATUS_CHANGED,				/**< callback type for decoder status changed. */
    SL_VDEC_CB_ERROR,						/**< callback type for decoder error */
    SL_VDEC_CB_FRAME_DISPLAYED,             /**< callback type for new frame displayed */
    SL_VDEC_CB_MONITOR_GOP,                 /**< callback type for GOP decoded */
};

typedef void (*vdec_callback)(void*, enum vdec_callback_type, uint32_t, uint32_t); /**< callback types */

enum vdec_out_mode
{
    VDEC_OUT_FULLVIEW,                  /**< VDEC output in fullview mode */
    VDEC_OUT_PREVIEW,                   /**< VDEC output in preview mode */
    VDEC_OUT_SWPASS,                    /**< VDEC output in SWPASS mode */
    VDEC_OUT_LAST
};

struct vdec_out_setting {
    enum vdec_out_mode out_mode;        /**< out mode */
    bool               progressive; /**< is currently output progressively? */
    enum dis_tvsys     out_tvsys;  /**< output(DIS)'s current tvsys */
    bool               smooth_switch_en; /**< enable smooth switch or not */
};


enum vdec_attr_type
{
    VDEC_ATTR_SET_MODULE_INFO,          /**< attribute type for setting module info */
    VDEC_ATTR_CONTINUE_ON_ERROR,        /**< ignore error on eror option on or not */
    VDEC_ATTR_FIRST_I_FREERUN,          /**< first I frame free run or not  */
    VDEC_ATTR_SAR_ENABLE,               /**< sar enabled or not, decided by attr_val set */
    VDEC_ATTR_PARSING_ENABLE,           /**< parsing enabled or not, decided by attr_val set */
    VDEC_ATTR_SET_DMA_CHANNEL,          /**< dma channel to be set to VDEC device */
    VDEC_ATTR_SHOW_COLORBAR,            /**< show colorbar or not  */
    VDEC_ATTR_GET_RAW_STATUS,           /**< get all status returned by HLD struct vdec_status_info */
};

enum vdec_state
{
    VDEC_STATE_STARTED = 1,
    VDEC_STATE_STOPPED,
    VDEC_STATE_PAUSE,
};

enum vdec_aspect
{
    VDEC_ASPECT_FORBIDDEN,
    VDEC_ASPECT_SAR,
    VDEC_ASPECT_4_3,
    VDEC_ASPECT_16_9,
    VDEC_ASPECT_DAR_221_1,
};

enum vdec_av_sync_mode
{
    VDEC_AV_SYNC_PTS = 0,
    VDEC_AV_SYNC_FREERUN,
    VDEC_AV_SYNC_AUDIO,
};

enum vdec_frame_type
{  
    VDEC_FRAME_TYPE_ALL,  /**< decode all frames (I, P and B) */  
    VDEC_FRAME_TYPE_I,    /**< decode only I frames */ 
};

enum vdec_id
{
    VDEC_ID_H264,    /**< h264 */
    VDEC_ID_XVID,    /**< xvid */
    VDEC_ID_MPG2,    /**< mpg2 */
    VDEC_ID_FLV1,    /**< flv1 */
    VDEC_ID_VP8,     /**< vp8 */     
    VDEC_ID_WVC1,    /**< wvc1 */
    VDEC_ID_WMV3,    /**< wmv3 */
    VDEC_ID_WX3,     /**< wx3 */
    VDEC_ID_RMVB,    /**< rmvb */
    VDEC_ID_MJPG,    /**< mjpg */
    VDEC_ID_VC1,     /**< vc1 */
    VDEC_ID_XD,      /**< XD */
    VDEC_ID_HEVC,    /**< HEVC */
    VDEC_ID_LAST
};

enum vdec_work_mode
{
    VDEC_WORK_MODE_NORMAL = 0,           /**< Normal decode */
    VDEC_WORK_MODE_VOL,                  /**< Parse all headers above (including) VOP level without VOP reconstuction (Internal use) */
    VDEC_WORK_MODE_HEADER,               /**< Parse header to get current frame's prediction type (Internal use) */
    VDEC_WORK_MODE_SKIP_B_FRAME,         /**< Skip b frame (Internal use) */
    VDEC_WORK_MODE_SKIP_B_P_FRAME,       /**< Only decode i frame (Internal use) */
    VDEC_WORK_MODE_SBM,                  /**< Decode from sbm */
    VDEC_WORK_MODE_SBM_STREAM,           /**< Decode from sbm */
    VDEC_WORK_MODE_LAST
};

enum vdec_video_id {
	VDEC_ID_VIDEO_0 = 0, 
	VDEC_ID_VIDEO_1, 
	VDEC_NB_VIDEO        /**< Number of video decoder */
};

enum vdec_dis_layer
{
	VDEC_DIS_MAIN_LAYER = 0,
	VDEC_DIS_AUX_LAYER,
	VDEC_DIS_LAYER_MAX
};

typedef enum vdec_show_last_frame_mode {   
    //driver will show the last frame(s) when there is no data about 100ms
    VDEC_SHOW_LAST_FRAM_AFTER_NO_DATA = 0,
    //driver will show the last frame(s) immediately
    VDEC_SHOW_LAST_FRAM_IMMEDIATE
}vdec_show_last_frame_mode;

typedef enum vdec_user_data_type {   
    //user data started with 0x4741 3934
    VDEC_USER_DATA_DEFAULT = 0,
    //all of the user data with top_field_first and repeat_first_field 
    //lenth = data lenth+4, data = userdata + 32bits(little  endian , 0: top_field_first, 1: repeat_first_field)
    VDEC_USER_DATA_ALL
}vdec_user_data_type;

struct vdec_info
{
    enum vdec_state              state;
    uint8_t                      first_header_parsed;
    uint8_t                      first_pic_decoded;
    uint8_t                      first_pic_showed;
    uint16_t                     pic_width;
    uint16_t                     pic_height;
    enum vdec_aspect             aspect;
    uint32_t                     frames_displayed;
    uint32_t                     frames_decoded;
    uint64_t                     frame_last_pts;
    int32_t                      show_frame;
    uint8_t                      queue_count;
    uint32_t                     buffer_size;
    uint32_t                     buffer_used;
    uint32_t                     frame_rate;
    int32_t                      interlaced_frame;
    int32_t                      top_field_first;
    int32_t                      hw_dec_error;
    int32_t                      is_support;
    uint32_t                     sar_width;             //Sample aspect horizontal ratio
    uint32_t                     sar_height;            //Sample apsect vertical ratio
    uint8_t                      active_format;          //Active format
    enum vdec_out_mode           out_mode;
    enum vdec_playback_direction playback_dir;
    enum vdec_playback_speed     playback_speed;
    enum vdec_playback_direction api_playback_dir;
    enum vdec_playback_speed     api_playback_speed;
    enum vdec_dis_layer          layer;
    uint32_t                     decode_error_cnt;
	uint8_t  ff_mode;
	uint8_t rect_switch_done;       //!<Whether set display rect done
	uint16_t                     max_width;
    uint16_t                     max_height;
    uint16_t                     max_frame_rate;
};

struct vdec_av_packet
{
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    int64_t pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    int64_t dts;
    uint8_t *data;
    int32_t size;
    int32_t stream_index;
    int32_t flags;
    uint16_t width, height;
    int32_t param_change_flags;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    int32_t  duration;
    void  (*destruct)(struct vdec_av_packet *);
    void  *priv;
    int64_t pos;///< byte position in stream, -1 if unknown
    int64_t convergen;
	uint8_t nalu_num;              //!<Numbers of NALU in the packet, -1 if unknown
};

struct vdec_mem_info
{
    void *mem_start;               /**< Start address of memory allocated to video in main cpu */
    unsigned long mem_size;        /**< Size of memory allocated to video in main cpu */
    void *priv_mem_start;          /**< Start address of memory allocated to video in see cpu */
    unsigned long priv_mem_size;   /**< Size of memory allocated to video in see cpu */
    void *mp_mem_start;            /**< Start address of memory allocated to media player */
    unsigned long mp_mem_size;     /**< Size of memory allocated to media player */
	void *vbv_mem_start;		   /**< Start address of memory allocated to vbv */
	unsigned long vbv_mem_size;    /**< Size of memory allocated to vbv */
};

struct vdec_mp_init_config
{
    enum vdec_id codec_id;      /**< Specified decoder's type */
    enum vdec_work_mode decode_mode;    /**< Decode mode */
    uint32_t decoder_flag;   /**< Decode flag */
    uint32_t preview;        /**< Whether in preview */
    uint32_t frame_rate;     /**< Frame rate */
    int32_t  pic_width;      /**< Picture width */
    int32_t  pic_height;     /**< Picture height */
    int32_t  pixel_aspect_x; /**< Pixel aspect raio width */
    int32_t  pixel_aspect_y; /**< Pixel aspect raio height */
    uint32_t dec_buf_addr;   /**< Frame buffer start address */
    uint32_t dec_buf_size;   /**< Frame buffer total size */ 
    uint8_t encrypt_mode;    /**<0: clear data 1: full sample 2: subsample */
};

struct vdec_mp_sbm_config
{
    uint8_t packet_header;   /**< SBM to hold packet header */
    uint8_t packet_data;     /**< SBM to hold packet data */
    uint8_t decode_output;   /**< SBM to hold decoder's output frames */
    uint8_t display_input;   /**< SBM to hold frames to display */
};

struct vdec_mp_extra_data_config
{
    uint8_t *extra_data;      /**< Extra data buffer */
    uint32_t extra_data_size; /**< Extra data size */
};

struct vdec_mp_sync_config
{
    enum vdec_av_sync_mode sync_mode;   /**< Synchronization mode */
    uint8_t sync_unit;               /**< Synchronization uint */
};

struct vdec_mp_info
{
    uint32_t decode_status;     //!<Decode state
    uint32_t pic_width;         //!<Picture width
    uint32_t pic_height;        //!<Picture height   
    uint32_t sar_width;         //!<Sample aspect ratio width
    uint32_t sar_heigth;        //!<Sample aspect ratio height
    uint32_t frame_rate;        //!<Frame rate
    int32_t interlaced_frame;   //!<Whether the stream is interlaced
    int32_t top_field_first;    //!<Whether the stream is top field first
    int32_t first_header_got;   //!<Whether the first header is parsed
    int32_t first_pic_showed;   //!<Whether the first picture is displayed
    uint32_t frames_decoded;    //!<Number of frames decoded
    uint32_t frames_displayed;  //!<Number of frames displayed
    int64_t frame_last_pts;     //!<PTS of last displayed frame
    uint32_t buffer_size;       //!<Total es buffer size
    uint32_t buffer_used;       //!<Used size of es buffer
    uint32_t decode_error;      //!<Decoder error type
    uint32_t decoder_feature;   //!<Decoder feature
    uint32_t under_run_cnt;     //!<Times of under run
    int32_t first_pic_decoded;  //!<Whether the first picture is decoded
    int32_t output_mode;        //!<Decoder's output mode
    uint32_t frame_angle;       //!<Decoder's output frame angle
    uint8_t layer;              //!<Which display layer deocder associated with
    uint32_t decode_error_cnt;  // decode error count
};

struct vdec_display_setting{
    enum vdec_out_mode out_mode;
    struct vdec_rect srect;
    struct vdec_rect drect;
};

/**
 * function Name: alislvdec_open
 *
 * @brief         Init VDEC device, and return a handle for further control of the device.
 *
 * @param[out] hdl Return handle for further control.
 * @param[in]  video id.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note          Call alislvdec_close() to release the device if you do \n
 *                not need it anymore. Mutiple open/close is supported, \n
 *                make sure to make open/close matched or the device is \n
 *                not relase correctly, causing unexpect result.
 */
alisl_retcode
alislvdec_open(alisl_handle *hdl, enum vdec_video_id id);
/**
 * Function Name: alislvdec_close
 * @brief         Close and release the device.
 *
 * @param[in] hdl Device handle to be released
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note          Mutiple open/close is supported, make sure to \n
 *                make open/close matched or the device is \n
 *                not relase correctly, causing unexpect result.
 */
alisl_retcode
alislvdec_close(alisl_handle hdl);

/**
 * Function Name: alislvdec_start
 * @brief         Start the VDEC device
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_start(alisl_handle hdl);

/**
 * Function Name: alislvdec_stop
 * @brief         Stop the VDEC device
 *
 * @param[in] hdl Device handle.
 * @param[in] close_display Boolean value to tell VDEC to close display after stop
 * @param[in] fill_black  Boolean value to tell VDEC to fill screen with black
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_stop(alisl_handle hdl,
               bool         close_display,
               bool         fill_black);
/**
 * Function Name: alislvdec_reset
 * @brief         Reset the VDEC device when VBV buffer full
 *
 * @param[in] hdl Device handle.
 * @param[in] vbv_buf_mode
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_vbv_buf_mode(alisl_handle hdl, enum vdec_vbv_buf_mode vbv_buf_mode);

/**
 * @brief pause the video decoder.
 * @param[in] hdl shared library handle
 * @return alisl_retcode
 */
alisl_retcode
alislvdec_pause(alisl_handle hdl);

/**
 * Function Name: alislvdec_buffer_reset
 * @brief         reset the vbv buffer
 *
 * @param[in] hdl Device handle.
 * @param[in] whether to reset the vbv buffer really
 * if 0, not reset buffer, reset one_frame_got flag to false
 * if not 0, reset vbv buffer and reset one_frame_got flag to false.
 * @return        alisl_retcode
 */
alisl_retcode
alislvdec_buffer_reset(alisl_handle hdl, unsigned int reset_buffer);

/**
 * @brief set color bar in one frame and show screen.
 * @param[in] hdl shared library handle
 * @param[in] color_bar_addr : address which allocate one frame buffer from vp
 * @return alisl_retcode
 */
alisl_retcode
alislvdec_set_colorbar(alisl_handle hdl, unsigned long color_bar_addr);


/**
 * @brief resume the video decoder.
 * @param[in] hdl shared library handle
 * @return alisl_retcode
 */
alisl_retcode
alislvdec_resume(alisl_handle hdl);

/**
 * Function Name: alislvdec_set_output
 * @brief         Set VDEC's output mode
 *
 * @param[in] hdl Device handl
 * @param[in] out_mode VDEC output settings
 * @param[out] mpcb Opaque MP callback data set to DIS if not null
 * @param[out] pipcb Opaque PIP callback data set to DIS if not null
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_output(alisl_handle             hdl,
                     struct vdec_out_setting *setting,
                     uint32_t                *mpcb,
                     uint32_t                *pipcb);

/**
 * Function Name: alislvdec_set_sync_mode
 * @brief         Set VDEC's sync mode
 *
 * @param[in] hdl Device handl
 * @param[in] sync_mode Sync mode to set to VDEC device
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_sync_mode(alisl_handle          hdl,
                        enum vdec_av_sync_mode sync_mode);

/**
 * Function Name: alislvdec_set_sync_delay
 * @brief         Delay for sync in ms, limited between -500ms to 500ms.
 *
 * @param[in] hdl Device handle
 * @param[in] delay Sync delay in ms
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_sync_delay(alisl_handle hdl,
                         uint32_t     delay);

/**
 * Function Name: alislvdec_set_iframe_freerun
 * @brief         Set iframe free run enabled or disabled
 *
 * @param[in] hdl Device handle
 * @param[in] enable Enable for disable
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_iframe_freerun(alisl_handle hdl,
                             bool         enable);

/**
 * Function Name: alislvdec_set_output_rect
 * @brief         Specify output rect for VDEC device
 *
 * @param[in] hdl Device handle
 * @param[in] srect Source rectangle
 * @param[in[ drect Destination rectangle to be shown
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_output_rect(alisl_handle     hdl,
                          struct vdec_rect *srect,
                          struct vdec_rect *drect);

/**
 * Function Name: alislvdec_captrue_frame
 * @brief         Capture picture of current frame.
 *
 * @param[in] hdl Device handle, return by alislvdec_open()
 * @param[in] cap Cap paramter.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_captrue_frame(alisl_handle           hdl,
                        struct vdec_frame_cap *cap);

/**
 * Function Name: alislvdec_set_decoder
 * @brief         Set current decoder type
 *
 * @param[in] hdl Device handl
 * @param[in] type Decoder type
 * @param[in] preview Boolean value to tell VDEC to change to preview mode
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_decoder(alisl_handle           hdl,
                      enum vdec_decoder_type type,
                      bool                   preview);

/**
 * Function Name: alislvdec_get_decoder
 * @brief Get current decoder for TS playing.
 *
 * @param[in] hdl Device handle
 * @param[in] type Decoder type
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_get_decoder(alisl_handle            hdl,
                      enum vdec_decoder_type *type);

/**
 * Function Name: alislvdec_reg_callback
 * @brief         Register callback to VDEC device.
 *
 * @param[in] hdl Device handle
 * @param[in] type Callback type
 * @param[in] new_cb New callback to be set
 * @param[out] old_cb Old callback to return to caller.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_reg_callback(alisl_handle             hdl,
                       void                     *user_data,
                       enum vdec_callback_type  type,
                       vdec_callback            new_cb,
                       vdec_callback           *old_cb);

/**
 * Function Name: alislvdec_set_playmode
 * @brief         Set play mode (direction, speed) for VDEC device
 *
 * @param[in] hdl Device handle
 * @param[in] dir Direction, forward or backwared
 * @param[in] speed Play speed
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_playmode(alisl_handle                 hdl,
                       enum vdec_playback_direction dir,
                       enum vdec_playback_speed     speed);

/**
 * Function Name: alislvdec_set_trickmode
 * @brief         Set trick mode (direction, speed) for VDEC device
 *
 * @param[in] hdl Device handle
 * @param[in] dir Direction, forward or backwared
 * @param[in] speed Play speed
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_trickmode(alisl_handle                 hdl,
                       enum vdec_playback_direction dir,
                       enum vdec_playback_speed     speed,
                       enum vdec_playback_method    mode);

/**
 * Function Name: alislvdec_fill_frame
 * @brief         Fill Frame with a specific color
 *
 * @param[in] hdl Device handle
 * @param[in] color Color to be filled
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note          Must be called with VDEC stoped or paused
 */
alisl_retcode
alislvdec_fill_frame(alisl_handle        hdl,
                     struct vdec_color *color);

/**
 * Function Name: alislvdec_config_pvr
 * @brief         Set parameter for PVR
 *
 * @param[in] hdl Device handle
 * @param[in] prv_param Config parameter
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_config_pvr(alisl_handle                  hdl,
                     struct vdec_pvr_config_param *param);

/**
 * Function Name: alislvdec_pause_pvr
 * @brief         Pause VDEC device when doing PVR.
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_pause_pvr(alisl_handle hdl);

/**
 * Function Name: alislvdec_resume_pvr
 * @brief         Resume VDEC when doing PVR
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_resume_pvr(alisl_handle hdl);

/**
 * Function Name: alislvdec_step_pvr
 * @brief         Step VDEC when doing PVR
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_step_pvr(alisl_handle hdl);

/**
 * Function Name: alislvdec_get_info
 * @brief         Get VDEC's infomation(status)
 *
 * @param[in] hdl Device handle
 * @param[out] status Status to be return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_get_info(alisl_handle      hdl,
                   struct vdec_info *info);

/**
 * Function Name: alislvdec_get_tvsys
 * @brief         Get stream's TV system currently being decoded.
 *
 * @param[in] hdl Device handle
 * @param[out] tvsys TV system to be returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_get_tvsys(alisl_handle    hdl,
                    enum dis_tvsys *ret_tvsys);

/**
 * Function Name: alislvdec_is_stream_progressive
 * @brief         Check current stream is progressive or not.
 *
 * @param[in] hdl Device handle
 * @param[out] progressive Boolean value indicates whether stream is progressive or not
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_is_stream_progressive(alisl_handle  hdl,
                                bool         *progressive);

/**
 * Function Name: alislvdec_inject
 * @brief         Inject raw data into vdec
 *
 * @param[in] hdl Device handle
 * @param[in] buf Buffer addr
 * @param[in] buf_sz Buffer size
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_inject(alisl_handle  hdl,
                 uint32_t     *buf,
                 uint32_t      buf_sz);

/**
 * Function Name: alislvdec_set_attr
 * @brief         Set VDEC's attribute
 *
 * @param[in] hdl Device handle
 * @param[in] attr_type Attribute type to be set
 * @param[in] attr_val Attribute value to be set
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_attr(alisl_handle        hdl,
                   enum vdec_attr_type attr_type,
                   uint32_t            attr_val);

/**
 * Function Name: alislvdec_set_frame_type
 * @brief         Set VDEC's frame type
 *
 * @param[in] hdl Device handl
 * @param[in] frame_type Frame type to set to VDEC device
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_frame_type(alisl_handle          hdl,
                         enum vdec_frame_type frame_type);

/**
 * Function Name: alislvdec_get_mem_info
 * @brief         Get VDEC's memory infomation
 *
 * @param[in] hdl Device handle
 * @param[in] mem_info Mem info to be return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_get_mem_info(alisl_handle          hdl,
                       struct vdec_mem_info *mem_info);

#if 0
/**
 * Function Name: alislvdec_get_av_frame_struct_size
 * @brief         Get size of av frame struct
 *
 * @param[in]  hdl Device handle
 * @param[out] size Size to be return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_get_av_frame_struct_size(unsigned int *size);
#endif
/**
 * Function Name: alislvdec_mp_init
 * @brief         Init VDEC in mp mode
 *
 * @param[in] hdl Device handle
 * @param[in] init_config Config parameter
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_init(alisl_handle                hdl,
                  struct vdec_mp_init_config *init_config);

/**
 * Function Name: alislvdec_mp_release
 * @brief         Release VDEC in mp mode
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_release(alisl_handle hdl);

/**
 * Function Name: alislvdec_mp_pause
 * @brief         Pause VDEC in mp mode
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_pause(alisl_handle hdl);

/**
 * Function Name: alislvdec_mp_resume
 * @brief         Resume VDEC in mp mode
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_resume(alisl_handle hdl);

/**
 * Function Name: alislvdec_mp_flush
 * @brief         Flush VDEC in mp mode
 *
 * @param[in] hdl Device handle
 * @param[in] playback_mode playback mode
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_flush(alisl_handle            hdl, 
                   enum vdec_playback_mode playback_mode);

/**
 * Function Name: alislvdec_mp_set_sbm
 * @brief         Set VDEC's sbm in mp mode
 *
 * @param[in] hdl Device handl
 * @param[in] sbm_config sbm info to set to mp VDEC device
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_set_sbm(alisl_handle               hdl,
                     struct vdec_mp_sbm_config *sbm_config);

/**
 * Function Name: alislvdec_mp_set_extra_data
 * @brief         Set VDEC's extra data in mp mode
 *
 * @param[in] hdl Device handl
 * @param[in] data_config Extra data to set to mp VDEC device
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_set_extra_data(alisl_handle                      hdl,
                            struct vdec_mp_extra_data_config *data_config);

/**
 * Function Name: alislvdec_mp_set_sync_mode
 * @brief         Set VDEC's sync mode in mp mode
 *
 * @param[in] hdl Device handl
 * @param[in] sync_config Sync mode to set to mp VDEC device
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_set_sync_mode(alisl_handle                hdl,
                           struct vdec_mp_sync_config *sync_config);

/**
 * Function Name: alislvdec_mp_set_frame_type
 * @brief         Set VDEC's frame type in mp mode
 *
 * @param[in] hdl Device handl
 * @param[in] frame_type Frame type to set to VDEC device
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_set_frame_type(alisl_handle          hdl,
                            enum vdec_frame_type frame_type);

/**
 * Function Name: alislvdec_mp_set_output_rect
 * @brief         Set VDEC's output rect in mp mode
 *
 * @param[in] hdl Device handl
 * @param[in] srect Source rectangle
 * @param[in[ drect Destination rectangle to be shown
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_set_output_rect(alisl_handle      hdl,
                             struct vdec_rect *srect,
                             struct vdec_rect *drect);
                            
/**
 * Function Name: alislvdec_mp_get_info
 * @brief         Get VDEC's infomation(status) in mp mode
 *
 * @param[in] hdl Device handle
 * @param[out] info Status to be return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_mp_get_info(alisl_handle         hdl,
                      struct vdec_mp_info *info);   

/**
 * Function Name: alislvdec_set_variable_resolution
 * @brief         Set VDEC to surport variable resolution
 * @param[in]     hdl Device handle
 * @param[in]     enable Enable for disable
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          04/06/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_variable_resolution(alisl_handle  hdl,
                                  bool          enable);

/**
 * Function Name: alislvdec_set_display_mode
 * @brief         a new interface integrates setting output and setting output rect 
 * @param[in]     hdl Device handle
 * @param[in]     out put mode and rect
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          08/06/2015, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_display_mode(alisl_handle                 hdl,
                           struct vdec_display_setting  *setting);    
                           
/**
 * Function Name: alislvdec_set_continue_on_error
 * @brief         when the hardware decoder error is occured, 
 *                continue without any handling or drop the sequence
 * @param[in]     hdl Device handle
 * @param[in]     enable Enable or Disable
 *                0: drop the sequence
 *                1: without any handling
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          14/12/2015, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_continue_on_error(alisl_handle  hdl,
                                bool          enable);  

/**
 * Function Name: alislvdec_parse_afd
 * @brief         parse afd or not
 * @param[in]     hdl Device handle
 * @param[in]     enable Enable or Disable
 *                0: parse afd, cowork with VPO to display in match mode in afd mode
 *                1: not parse afd, cowork with VPO to display in match mode in normal mode
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          20/04/2016, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_parse_afd(alisl_handle  hdl, bool enable);

/**
 * Function Name: alislvdec_get_id
 * @brief         get current video id
 * @param[in]     hdl Device handle
 * @param[out]    video id
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          04/03/2016, Created
 *
 * @note
 */                                
alisl_retcode alislvdec_get_id(alisl_handle  hdl, enum vdec_video_id *id);   

/**
 * Function Name: alislvdec_get_decoder_by_id
 * @brief         get current video handle
 * @param[in]     video id
 * @param[out]    video handle
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          04/03/2016, Created
 *
 * @note
 */ 
alisl_retcode alislvdec_get_decoder_by_id(enum vdec_video_id id, alisl_handle *hdl);

/**
 * Function Name: alislvdec_set_display_layer
 * @brief         set display layer for current video
 * @param[in]     hdl Device handle
 * @param[in]     display layer
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          04/03/2016, Created
 *
 * @note
 */ 
alisl_retcode alislvdec_set_display_layer(alisl_handle hdl, enum vdec_dis_layer layer);

/**
 * Function Name: alislvdec_get_display_layer
 * @brief         get display layer for current video
 * @param[in]     hdl Device handle
 * @param[out]     display layer
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          04/03/2016, Created
 *
 * @note
 */ 
alisl_retcode alislvdec_get_display_layer(alisl_handle hdl, enum vdec_dis_layer* layer);

/**
 * Function Name: alislvdec_store_display_rect
 * @brief         store the display info for the the specific layer, 
 *                set the info to the video decoder before starting 
 *                or after media initialization
 * @param[in]     display layer
 * @param[in]     display rectangle
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          04/03/2016, Created
 *
 * @note
 */ 
alisl_retcode alislvdec_store_display_rect(enum vdec_dis_layer layer, struct vdec_display_setting  *setting);

alisl_retcode alislvdec_get_display_rect(enum vdec_dis_layer layer, struct vdec_display_setting  *setting);
/*
**
 * Function Name:      alislvdec_get_trick_info
 * @brief         when trick play with dis_continue mode, get information of hardware decode error and frame displayed 
 * @param[in]     hdl Device handle
 * @param[out]	  pointer storing whether happening decode error
 * @param[out]	  pointer storing whether new frame was displayed
 * @return        alisl_retcode
 *
 * @author        amu.tu
 * @date          29/05/2016, Created
 */
alisl_retcode alislvdec_get_trick_info(alisl_handle hdl, int *if_cb_err, int *if_frm_dis);

/*
 *
 * Function Name: alislvdec_show_last_frame
 * @brief         flush the data in frame buffer to display
 * @param[in]     hdl Device handle
 * @param[in]	  flush mode
 * @return        alisl_retcode
 *
 * @author        wendy.he
 * @date          12/09/2016, Created
 */
alisl_retcode alislvdec_show_last_frame(alisl_handle hdl, vdec_show_last_frame_mode mode);

/*
 *
 * Function Name: alislvdec_set_user_data_type
 * @brief         set user data type, then we will get the specific user data
 * @param[in]     hdl Device handle
 * @param[in]	  user data type
 * @return        alisl_retcode
 *
 * @author        wendy.he
 * @date          22/09/2016, Created
 */
alisl_retcode alislvdec_set_user_data_type(alisl_handle hdl, vdec_user_data_type type);

/*
  *
  * Function Name: alislvdec_set_sync_repeat_interval
  * @brief		set a interval to let vdec show a frame twice.
  * @param[in]	hdl device handle
  * @param[in]	interval, in frames
  * @return		alisl_retcode
 */
alisl_retcode alislvdec_set_sync_repeat_interval(alisl_handle hdl, unsigned int interval);

/**
 * Function Name: alislvdec_get_decoder_ext
 * @brief Get current decoder including TS and ES playing.
 *
 * @param[in] hdl Device handle
 * @param[in] type Decoder type
 *
 * @return        alisl_retcode
 *
 * @author        wendy.he
 * @date          19/01/2018, Created
 *
 * @note
 */
alisl_retcode
alislvdec_get_decoder_ext(alisl_handle            hdl,
                          enum vdec_decoder_type *type);
#ifdef __cplusplus
}
#endif

#endif    /* _ALISLVDEC_H_ */
