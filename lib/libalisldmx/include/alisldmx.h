/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alisldmx.h
 *  @brief              ALi demux function interfaces
 *
 *  @version            1.0
 *  @date               06/15/2013 09:10:49 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef __ALISLDMX__H_
#define __ALISLDMX__H_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>
#include <ali_dmx_common.h>	


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum dmx_id {
    DMX_ID_DEMUX0 = 0,     /* Demux id 0, related device is ali_m36_dmx_0 */
    DMX_ID_DEMUX1,         /* Demux id 1, related device is ali_m36_dmx_1 */
    DMX_ID_SW_DEMUX0,      /* Demux id 2, used for playback, related device is ali_dmx_pb_0_out & ali_dmx_pb_0_in */
    DMX_ID_DEMUX2,         /* Demux id 3, related device is ali_m36_dmx_2 */
    DMX_ID_DEMUX3,         /* Demux id 4, related device is ali_m36_dmx_3 */
    DMX_ID_SW_DEMUX1 = 64, /* Demux id 64, used for playback, related device is ali_dmx_pb_1_out & ali_dmx_pb_1_in */
    DMX_NB_DEMUX           /* Number of demux in this enum */
} dmx_id_t;

typedef enum dmx_see_id {
	DMX_SEE_ID_DEMUX0 = 0, /**<see demux id 0, related device is ali_m36_dmx_see_0 */
	DMX_SEE_ID_DEMUX1      /**<see demux id 0, related device is ali_m36_dmx_see_1 */
}dmx_see_id;

enum dmx_io_command {
	DMX_IOCMD_GET_AUDIO_BITRATE = 0, /**< get audio bitrate */
	DMX_IOCMD_GET_FREE_BUF_LEN,      /**< get free hw buffer count */
	DMX_IOCMD_SET_PLAYBACK_SPEED,    /**< set playback speed */
	DMX_IOCMD_SET_DEC_HANDLE,        /**< set dec handle */
	DMX_IOCMD_CRYPTO_START,          /**< start demux crypto */
	DMX_IOCMD_CRYPTO_STOP,           /**< stop demux crypto */
	DMX_IOCMD_GET_DISCONTINUE_COUNT, /**< get discontinue count */
	DMX_IOCMD_CLEAR_DISCONTINUE_COUNT, /**< clear discontinue count */
	DMX_IOCMD_GET_MAIN2SEEBUF_REAMIN_DATA_LEN, /** get main-cpu to see-cpu
	                                              remain data length */
	DMX_IOCMD_DO_DDP_CERTIFICATION,
	DMX_IOCMD_SET_HW_INFO,           /**< set hardware information */
	DMX_IOCMD_SET_SAT2IP,            /**< set sat2ip */
	DMX_IO_SET_FTA_REC_MODE,		/**< set PVR record mode,0:FTA to FTA;1:FTA to Encrypt*/
	DMX_IO_GET_FTA_REC_MODE,		/**< get PVR record mode,0:FTA to FTA;1:FTA to Encrypt*/
	DMX_IOCMD_GET_TOTAL_BUF_LEN,      /**< get dmx total buffer size */ 
	DMX_IOCMD_SET_RECORD_MODE,		/**< set PVR record mode, DMX_REC_OUT_TS:record ts; DMX_REC_OUT_BLOCK:record block*/
	DMX_IOCMD_SET_RECORD_BLOCKSIZE,	/**< set PVR record block size*/
	
	DMX_IOCMD_GET_RECORD_MODE,		/**< set PVR record mode, DMX_REC_OUT_TS:record ts; DMX_REC_OUT_BLOCK:record block*/
	DMX_IOCMD_GET_RECORD_BLOCKSIZE,	/**< set PVR record block size*/
	DMX_IOCMD_MAIN2SEE_BUF_VALIDSIZE_SET,
	DMX_IOCMD_MAIN2SEE_BUF_VALIDSIZE_GET,
	DMX_IOCMD_MAIN2SEE_SRC_SET,
	DMX_IOCMD_MAIN2SEE_SRC_GET,
	DMX_IOCMD_MAIN2SEE_BUF_REQ_SIZE,
	DMX_IOCMD_MAIN2SEE_BUF_RET_SIZE,
	DMX_IOCMD_RCV_PKG_CNT_GET,  /** get the total number of ts packets, but now only contains video and audio */
	DMX_IOCMD_RCV_TS_PKG_CNT_GET_BY_PID,  /** get the number of ts packets by pid, but now only support video pid and audio pid */
	DMX_IOCMD_BIND_CRYPTO_SESSION,  /**< bind crypt session */
	DMX_IOCMD_RESET_MAIN2SEEBUF_REAMIN_BUF,
};

typedef enum dmx_channel_type {
	/**
	 * If a channel was not allocated, the channel type
	 * could be DMX_CHANNEL_NONE.
	 */
	DMX_CHANNEL_NONE = 0,

	/**
	 * Usually used in channel scanning.
	 */
	DMX_CHANNEL_SECTION,

	/**
	 * Try to get data from demux and record data into
	 * hardware storages. Usually a record channel corresponds
	 * to several pids.
	 */
	DMX_CHANNEL_RECORD,

	/**
	 * User application can get data from demux
	 * and decode them or do something else. Usually
	 * a service corresponds to a pid.
	 */
	DMX_CHANNEL_SERVICE,

	/**
	 * User application can create a stream and start to
	 * play the steam. The stream type is specified detailed
	 * in enum dmx_stream_type. Usually the data of this type
	 * of channel was sent to kernel space and decoded by
	 * corresponding decoder. No callback functions need to
	 * be registered.
	 */
	DMX_CHANNEL_STREAM
} dmx_channel_type_t;

typedef enum dmx_stream_type {
	DMX_STREAM_NONE = 0,
	DMX_STREAM_VIDEO,
	DMX_STREAM_AUDIO,
	DMX_STREAM_AUDIO_DESCRIPTION,
	DMX_STREAM_PCR,
	DMX_STREAM_TELETEXT,
	DMX_STREAM_SUBTITLE,
	DMX_STREAM_CC,
	DMX_STREAM_TP
} dmx_stream_type_t;

typedef enum dmx_output_format {
	DMX_OUTPUT_FORMAT_NONE = 0,
	DMX_OUTPUT_FORMAT_TS,
	DMX_OUTPUT_FORMAT_PES,
	DMX_OUTPUT_FORMAT_SEC,
	DMX_OUTPUT_FORMAT_PCR
} dmx_output_format_t;

typedef enum dmx_avsync_mode
{
	DMX_AVSYNC_NONE = 0,
	DMX_AVSYNC_LIVE,
	DMX_AVSYNC_PLAYBACK,
	DMX_AVSYNC_TSG_TIMESHIT,
	DMX_AVSYNC_TSG_LIVE
} dmx_avsync_mode_t;

typedef enum Dmx_ClearStreamEncryptMode{
	DMX_FTA_TO_FTA = 0,         //!< FTA to unencrypted  
	DMX_FTA_TO_ENCRYPT = 1      //!< FTA to encryption  
} dmx_clearstream_encrypt_mode;

#define DMX_ILLEGAL_CHANNELID   (0xFFFFFFFF)
#define DMX_ILLEGAL_FILTERID    (0xFFFFFFFF)
#define DMX_ILLEGAL_PID         (0x1FFF)
#define DMX_MASK_PID            (0x1FFF)

#define DMX_MAX_SECTIONS        (32)
#define DMX_MAX_RECORDS         (96)
#define DMX_MAX_SERVICES        (16)
#define DMX_MAX_STREAMS         (10)              /**< @todo may be changed */
#define DMX_MAX_FILTERS         (176)             /**< @todo may be changed */

#define DMX_CHID2TYPE(channelid)  (((channelid) >> 16) & 0xFFFF)
#define DMX_CHID2INDEX(channelid) ((channelid) & 0xFFFF)
#define DMX_TYPEINDEX2CHID(type, index) (((type) << 16) | ((index) & 0xFFFF))

#define DMX_CACHE_PID_LIST_MAX_LEN 32   /**< Max length of demux cache pid list */

typedef enum dmx_control {
	DMX_CTRL_RESET = 0,     /**< reset a channel or filter */
	DMX_CTRL_ENABLE,        /**< enable a channel or filter */
	DMX_CTRL_DISABLE        /**< disable a channel or filter */
} dmx_control_t;

typedef struct dmx_channel_attr {
	/**
	 * RECORD CHANNEL could set this parameter. It could be:\n
	 * DMX_OUTPUT_FORMAT_PES or DMX_OUTPUT_FORMAT_TS.\n
	 * And the default value of record channel is
	 * DMX_OUTPUT_FORMAT_TS.
	 * Other type of channel will have its own default value
	 * of this parameter. User needn't care, and please
	 * make it be DMX_OUTPUT_FORMAT_NONE.
	 */
	enum dmx_output_format  output_format;

	/**
	 * If it's a STREAM CHANNEL, and the stream type is one of
	 * these: \n
	 * DMX_STREAM_VIDEO\n
	 * DMX_STREAM_AUDIO\n
	 * DMX_STREAM_AUDIO_DESCRIPTION\n
	 * DMX_STREAM_PCR\n
	 * the stream type should be set.
	 */
	enum dmx_stream_type    stream;

	/**
	 * If it's a RECORD CHANNEL, and you want to record
	 * the whole tp, this parameter should be set.
	 */
	uint32_t                rec_whole_tp;

	/**
	 Driver of pdk 1.9 change uint32_t type to void *.
	 * It's used when the channel is a RECORD CHANNEL.
	 */
	void *                enc_para;

	/**
	 * Parameter continuous is used and valid when the
	 * channel is a SECTION CHANNEL.
	 * It specified whether need to get sections continuously.
	 */
	bool                    continuous;

	/**
	 * Parameter video_id is used and valid when the
	 * channel is a STREAM CHANNEL and type is DMX_STREAM_VIDEO.
	 * It specified the dmx send data to which video decoder.
	 */
	unsigned char           video_id;

    /** The type to specify the source is scrambled or not.  
     *  0 - clean, 1 - scrambled.   
    */
    unsigned int is_encrypted:1;

    /** The type to specify the driver would do cache or not.  
     *  0 - cached, 1 - no cache.   
    */
    unsigned int uncache_para:1;

	/**
	  * Video PID, used for record channel, to pass the video pid to PVR SEE driver. For PVR3.2.
	  */
	unsigned short video_pid;
	/**
	  * Video type, used for record channel, to pass the video type to PVR SEE driver. For PVR3.2.
	  */
	unsigned char video_type;
} dmx_channel_attr_t;

typedef enum dmx_cache_type
{
	DMX_SL_NO_CACHE = 0,                                 /**< No Cache */
	DMX_SL_CHACE_PID,                                    /**< Cache special pid */
	DMX_SL_CACHE_TP                                      /**< Cache Whole tp */
}dmx_cache_type_t;

typedef struct dmx_cache_param
{
	enum dmx_cache_type type;                            /**< When type is DMX_CACHE_TP or DMX_NO_CHACE, pid_list member is invalid */
	unsigned int pid_list_len;                           /**< Pid list length of demux cache setting */
	short int *pid_list;                                 /**< Pid list array of demux cache setting */
	void *pid_list_attr;                                 /**< reserved for future use  */
}dmx_cache_param_t;

typedef struct dmx_cache_retrace_param
{
	uint32_t pid_list_len;                               /**< Pid list length of demux cache setting */
	uint16_t pid_list[DMX_CACHE_PID_LIST_MAX_LEN];       /**< Pid list array of demux cache setting */
}dmx_cache_retrace_param_t;

typedef void (*alisldmx_channel_requestbuf_callback) (void *private,
						uint32_t channelid,
						uint32_t filterid,
						uint32_t length,
						uint8_t  **buffer,
						uint32_t *actlen);

typedef void (*alisldmx_channel_updatebuf_callback) (void *private,
						uint32_t channelid,
						uint32_t filterid,
						uint32_t valid_len,
						uint16_t ifm_offset);

typedef struct dmx_channel_callback {
	alisldmx_channel_requestbuf_callback request_buffer;
	alisldmx_channel_updatebuf_callback  update_buffer;
	void                    *priv;
} dmx_channel_callback_t;

typedef void (*alisldmx_playfeed_requestdata_callback) (void *private,
						uint32_t length,
						uint8_t  **buffer,
						uint32_t *actlen,
						bool *fastcopy);

typedef struct dmx_playfeed_callback {
	alisldmx_playfeed_requestdata_callback request_data;
} dmx_playfeed_callback_t;

typedef struct dmx_playback_param {
	bool                    is_radio;

	/**
	 * User should register playfeed callback functions when
	 * playback local stream. The pointer of callback functions
	 * will be saved into this structure.
	 */
	struct dmx_playfeed_callback cb_feed;

	/**
	 * It's used to pass private data from client
	 * for feed data callback functions.
	 */
	void                    *priv;
	/**
	 * Use to recored the size write to playback dmx
	 * cooprate with alisldmx_direct_write_playback()
	 */
	unsigned long write_size;
} dmx_playback_param_t;

typedef struct dmx_write_playback_param {
	unsigned long length;
	unsigned char *start_buf;
	unsigned char fastcopy;
} dmx_write_playback_param_t;

/** Struct to get the number of TS packets related to the specified PID */
typedef struct sl_dmx_get_ts_pkg_cnt_by_pid {

	/** Member to specify the PID of which getting the related number of TS packets */
	unsigned long ul_pid;

	/** Member to specify the number of TS packets related to the specified PID */
	unsigned long ul_ts_pkg_cnt;

} sl_dmx_get_ts_pkg_cnt_by_pid, *p_sl_dmx_get_ts_pkg_cnt_by_pid;

/**
 *  Function Name:      alisldmx_allocate_filter
 *  @brief              Allocate a filter for a specific channel.
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param[out] filterid Save the filter id when allocate success.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note               Some type of channel will support filter, for example:
 *                      section channel. \n
 *                      But some type of channel only support pid or pid_list.
 */
alisl_retcode alisldmx_allocate_filter(alisl_handle handle,
				uint32_t channelid, uint32_t *filterid);

/**
 *  Function Name:      alisldmx_free_filter
 *  @brief              Free a filter.
 *
 *  @param handle       pointer to module handle
 *  @param filterid     free the specific filter
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_free_filter(alisl_handle handle, uint32_t filterid);

/**
 *  Function Name:      alisldmx_free_filter_all
 *  @brief              Free all filters of a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    filters of the specific channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_free_filter_all(alisl_handle handle, uint32_t channelid);

/**
 *  Function Name:      alisldmx_set_filter
 *  @brief              
 *
 *  @param handle       pointer to module handle
 *  @param filterid     Specific filter
 *  @param size         size of filter/mask/mode
 *  @param filter       The value to check for section filtering
 *                      (length bytes are skipped).
 *  @param mask         The bit to check in filter value.
 *  @param mode         The filtering mode to apply on the mask
 *                      (positive or negative).
 *	@param continuous	get the section data continuously.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 *                      Example1: Positive filtering mode on table id
 *                                and Extension Id. \n
 *                      Filter: 0x42 0x00 0x02 0x00 0x00 0x00 0x00 0x00 \n
 *                      Mask:   0XFF 0XFF 0XFF 0X00 0X00 0X00 0x00 0x00 \n
 *                      Mode:   0XFF 0XFF 0XFF 0X00 0X00 0X00 0x00 0x00 \n
 *                      Result: \n
 *                      All sections with TableId 0x42 and extensionId
 *                      0x0002 will be filtered. \n
 *                      \n
 *                      Example2: Negative filtering mode on version number. \n
 *                      Filter: 0x42 0x00 0x02 0x02 0x00 0x00 0x00 \n
 *                      Mask:   0XFF 0XFF 0XFF 0X3E 0X00 0X00 0X00 \n
 *                      Mode:   0XFF 0XFF 0XFF 0XC1 0X00 0X00 0X00 \n
 *                      Result: \n
 *                      All sections with TableId 0x42, extensionId  0x0002, \n
 *                      and version number different than 0x01 will be filtered.
 */
alisl_retcode alisldmx_set_filter(alisl_handle handle,
				uint32_t filterid, uint32_t size,
				uint8_t *filter, uint8_t *mask, uint8_t *mode, uint8_t continuous);

/**
 *  Function Name:      alisldmx_set_filter_crc_check
 *  @brief              
 *
 *  @param handle       pointer to module handle
 *  @param filterid     Specific filter
 *  @param crc_check    crc_check flag value
 *  @return             alisl_retcode
 *  @author             Amu.Tu
 *  @date               10/29/2015, Created
*/
alisl_retcode alisldmx_set_filter_crc_check(alisl_handle handle, uint32_t filterid, uint8_t crc_check);

/**
 *  Function Name:      alisldmx_control_filter
 *  @brief              Reset/Enable/Disable a filter
 *
 *  @param handle       pointer to module handle
 *  @param filterid     Specific filter
 *  @param ctrl         Refer to enum dmx_control
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_control_filter(alisl_handle handle,
				uint32_t filterid, enum dmx_control ctrl);

/**
 *  @brief			Register callback functions for the specified section filter
 *
 *  @param[in]		handle		demux dev handle
 *  @param[in]		filterid	the id of the filter will be configured.
 *  @param[in]		callback	the new callback functions for this filter.
 *
 *  @return			0 		- success
 *  @return			others	- failed
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			2/5/2015  11:16:52
 *
 *  @note
 */
alisl_retcode alisldmx_register_filter_callback(alisl_handle handle,
												uint32_t filterid,
												struct dmx_channel_callback *callback);

/**
 *  Function Name:      alisldmx_allocate_channel
 *  @brief              Allocate a channel with specific channel type
 *
 *  @param handle       pointer to module handle
 *  @param type         Channel type, refer to enum dmx_channel_type
 *  @param[out] channelid Save the channel id when allocate success. \n
 *                        channelid = (type << 16) | realid.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_allocate_channel(alisl_handle handle,
				enum dmx_channel_type type,
				uint32_t *channelid);

/**
 *  Function Name:      alisldmx_free_channel
 *  @brief              Free a channel.
 *
 *  @param handle       pointer to module handle
 *  @param channelid    free the specific channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_free_channel(alisl_handle handle, uint32_t channelid);

/**
 *  Function Name:      alisldmx_control_channel
 *  @brief              Reset/Enable/Disable a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param ctrl         Refer to enum dmx_control
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_control_channel(alisl_handle handle,
				uint32_t channelid, enum dmx_control ctrl);

/**
 *  Function Name:      alisldmx_channel_read
 *  @brief              Read ts or pes from a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param buf          buffer to store ts or pes 
 *  @param len          buffer length
 *  @param[out] p_read    storing size has been read
 *  @param timeout_ms      timeout for read in ms
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               15/06/2015, Created
 *
 */
alisl_retcode alisldmx_channel_read(alisl_handle handle, uint32_t channelid,
                unsigned char *buf, unsigned long len, unsigned long *p_read, int timeout_ms);

/**
 *  Function Name:      alisldmx_register_channel_callback
 *  @brief              Register callback functions to a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param callback     Pointer to callback functions
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note               Usually we should register callback functions \n
 *                      when record data from demux or implement a specific \n
 *                      service or get the filtered section packet data. \n
 *                      But when we play a tream, usually the data of the \n
 *                      stream is sent to decoder. Client don't need to register \n
 *                      callback functions to get the data.
 */
alisl_retcode alisldmx_register_channel_callback(alisl_handle handle,
				uint32_t channelid,
				struct dmx_channel_callback *callback);

/**
 *  Function Name:      alisldmx_set_channel_pidlist
 *  @brief              set pid list to a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid_list     pids to be set to the channel
 *  @param nb_pid       number of pids that to be set to the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note               In some cases, only the first valid pid in
 *                      pid_list is used.\n
 *                      But for record channel, a pid list will be
 *                      used.
 */
alisl_retcode alisldmx_set_channel_pidlist(alisl_handle handle,
				uint32_t channelid,
				uint16_t *pid_list,
				uint32_t nb_pid);

/**
 *  Function Name:      alisldmx_set_channel_pid
 *  @brief              set pid to a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid          pid to be set to the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/08/2013, Created
 *
 *  @note
 */

alisl_retcode alisldmx_set_channel_pid(alisl_handle handle,
				uint32_t channelid,
				uint16_t pid);

/**
 *  Function Name:      alisldmx_add_channel_pidlist
 *  @brief              add pidlist to a record channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid_list     pids to be set to the channel
 *  @param nb_pid       number of pids that to be set to the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               09/02/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_add_channel_pidlist(alisl_handle handle,
				uint32_t channelid,
				uint16_t *pid_list,
				uint32_t nb_pid);

/**
 *  Function Name:      alisldmx_del_channel_pidlist
 *  @brief              delete pidlist from a record channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid_list     pids to be deleted from the channel
 *  @param nb_pid       number of pids that to be deleted from the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               09/02/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_del_channel_pidlist(alisl_handle handle,
				uint32_t channelid,
				uint16_t *pid_list,
				uint32_t nb_pid);

/**
 *  Function Name:      alisldmx_set_channel_attr
 *  @brief              set channel attribute
 *
 *  @param handle       pointer to module handle
 *  @param channelid    id of the channel
 *  @param attr         attribute of the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/08/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_channel_attr(alisl_handle handle,
				uint32_t channelid,
				struct dmx_channel_attr *attr);

/**
 *  Function Name:      alisldmx_set_avsync_mode
 *  @brief              Set av sync mode to demux
 *
 *  @param handle       pointer to module handle
 *  @param avsync       av sync mode, refer to enum dmx_avsync_mode
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_avsync_mode(alisl_handle handle, enum dmx_avsync_mode avsync);

/**
 *  Function Name:      alisldmx_avstart
 *  @brief              Start av play.
 *                      After all stream channels are enabled(video/audio/
 *                      audio description/pcr), this function should
 *                      be called to start av play.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note               After all stream channels are enabled(video/audio/
 *                      audio description/pcr), this function should
 *                      be called to start av play.
 */
alisl_retcode alisldmx_avstart(alisl_handle handle);

/**
 *  Function Name:      alisldmx_avstop
 *  @brief              Stop av play.
 *                      After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop av play.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note               After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop av play.
 */
alisl_retcode alisldmx_avstop(alisl_handle handle);

/**
 *  Function Name:      alisldmx_avstop_with_param
 *  @brief              Stop/Pause av play.
 *                      After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop/pause av play.
 *
 *  @param handle       pointer to module handle
 *  @param param        pause -- DMA_INVALID_CHA, stop -- 0
 *
 *  @return             alisl_retcode
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               08/06/2015, Created
 *
 *  @note               After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop/pause av play.
 */
alisl_retcode alisldmx_avstop_with_param(alisl_handle handle, unsigned int param);

/**
 *  Function Name:      alisldmx_avpause
 *  @brief              Pause av play.
 *                      After all stream channels are enabled(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to pause av play.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Nick Li <nick.li@alitech.com>
 *  @date               02/09/2017, Created
 *
 *  @note               After use alisldmx_avpause to pause av play, 
 *                      should be called alisldmx_avstart to enable playback.
 */
alisl_retcode alisldmx_avpause(alisl_handle handle);

/**
 *  Function Name:      alisldmx_change_audio_pid
 *  @brief              change audio pid to see.
 *
 *  @param handle       pointer to module handle
 *  @param audio_pid    new audio pid
 *  @param audio_desc_pid new audio description pid
 *  @param audio_chid   new channel id of audio
 *  @param audio_desc_chid new channelid of audio description
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/10/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_change_audio_pid(alisl_handle handle,
				uint16_t audio_pid, uint16_t audio_desc_pid,
				uint32_t *audio_chid, uint32_t *audio_desc_chid);

/**
 *  Function Name:      alisldmx_set_playback_param
 *  @brief              Set playback parameters.
 *
 *  @param handle       pointer to module handle
 *  @param
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_playback_param(alisl_handle handle,
				struct dmx_playback_param *param);

/**
 *  Function Name:      alisldmx_open
 *  @brief              Open ali_m36_dmx_see_0 here, and open linux demux device
 *                      for common usage. But don't open linux demux
 *                      device for each channel. Because we will open linux demux
 *                      device When really allocate a channel. We just remember
 *                      which demux will be used later.
 *                      When open a demux device, we will create some daemon
 *                      pthread. The daemon pthreads are blocked and wait for
 *                      alisldmx_start. When call alisldmx_start, they will begin
 *                      to process data for all opened channels.
 *
 *  @param handle       pointer to module handle
 *  @param id           demux id
 *  @param see_id       see demux id
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/17/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_open(alisl_handle *handle, enum dmx_id id, enum dmx_see_id see_id);

/**
 *  Function Name:      alisldmx_set_nim_chipid
 *  @brief              Set nim chip id. So the module will remembor it
 *                      and set to channel parameter when start the channel.
 *
 *  @param handle       pointer to module handle
 *  @param nim_chipid   nim chip id
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_nim_chipid(alisl_handle handle, uint32_t nim_chipid);

/**
 *  Function Name:      alisldmx_set_front_end
 *  @brief              Set front_end. So the module will remember it
 *                      and set to channel parameter when start the channel.
 *
 *  @param handle       pointer to module handle
 *  @param frong_end    front_end
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_front_end(alisl_handle handle, uint32_t front_end);

/**
 *  Function Name:      alisldmx_fastcopy_enable
 *  @brief              Enable or disable fast copy feature.
 *
 *  @param handle       pointer to module handle
 *  @param enable       set 0 to disable fast copy feature,\n
 *                      Other to enable fast copy feature.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               09/25/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_fastcopy_enable(alisl_handle handle, bool enable);

/**
 *  Function Name:      alisldmx_start
 *  @brief              Start a demux device, then we can allocate a channel
 *                      to get section, record data...
 *                      The daemon pthreads will running when call this function.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/17/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_start(alisl_handle handle);

/**
 *  Function Name:      alisldmx_pause
 *  @brief              Pause a demux.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/29/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_pause(alisl_handle handle);

/**
 *  Function Name:      alisldmx_stop
 *  @brief              Stop a demux. It will clear device status bit
 *                      DMX_STATUS_START. And all the daemon pthread will block
 *                      on flag_wait_bit_any(&dev->status, DMX_STATUS_START)
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/29/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_stop(alisl_handle handle);

/**
 *  Function Name:      alisldmx_close
 *  @brief              close demux device
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_close(alisl_handle handle);

/**
 *  Function Name:      alisldmx_get_scram_status
 *  @brief              Get a/v stream scramble status
 *
 *  @param handle       pointer to module handle
 *  @param[out] status  a/v stream scramble status
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_get_scram_status(alisl_handle *handle, uint32_t *status);

/**
 *  Function Name:      alisldmx_get_scram_status_ext
 *  @brief              Get a/v stream scramble status.
 *                      It is different from alisldmx_get_scram_status.
 *
 *  @param handle       pointer to module handle
 *  @param[out] status  a/v stream scramble status
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_get_scram_status_ext(alisl_handle *handle,
				uint16_t *pid_list, uint32_t nb_pid,
				uint32_t *status);

/**
 *  Function Name:      alisldmx_section_poll
 *  @brief              poll which section channel is hit.
 *
 *  @param handle       pointer to module handle
 *  @param id_mask      Specify which channel to poll
 *  @param timeout      timeout
 *  @param[out] flag    result flag
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_poll(alisl_handle handle, uint32_t id_mask,
				uint32_t timeout, uint32_t *flag);

/**
 *  Function Name:      alisldmx_section_poll_release
 *  @brief              poll release, means stop poll
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_poll_release(alisl_handle handle);

/**
 *  Function Name:      alisldmx_section_poll_reset
 *  @brief              poll reset, means ready for next poll
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_poll_reset(alisl_handle handle);

/**
 *  Function Name:      alisldmx_section_request_poll
 *  @brief              poll which section channel is hit. \n
 *                      It's very similar to alisldmx_section_poll
 *
 *  @param handle       pointer to module handle
 *  @param id_mask      Specify which channel to poll
 *  @param timeout      timeout
 *  @param[out] flag    result flag
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_request_poll(alisl_handle handle, uint32_t id_mask,
				uint32_t timeout, uint32_t *flag);

/**
 *  Function Name:      alisldmx_section_request_poll_release
 *  @brief              poll release, means stop request poll. \n
 *                      It's very similar to alisldmx_section_poll_release
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_request_poll_release(alisl_handle handle);

/**
 *  Function Name:      alisldmx_ioctl
 *  @brief              ioctl of demux module, implement some misc
 *                      function interfaces.
 *
 *  @param handle       pointer to module handle
 *  @param cmd          ioctl command
 *  @param param        parameter of the ioctl command
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/24/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_ioctl(alisl_handle handle,
                             unsigned int cmd, unsigned long param);

/**
 *  Function Name:      alisldmx_direct_write_playback
 *  @brief              Write playback dmx.
 *
 *  @param handle       pointer to module handle
 *  @param
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_direct_write_playback(alisl_handle handle,
				struct dmx_write_playback_param *param);

/**
 *  Function Name:      alisldmx_get_write_pb_length
 *  @brief              Get length of write playback dmx.
 *
 *  @param handle       pointer to module handle
 *  @param
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
 alisl_retcode alisldmx_get_write_pb_length(alisl_handle handle,
				unsigned long *param);

/**
 *  Function Name:      alisldmx_cache_set_by_index
 *  @brief              set dmx cache by dmx index
 *
 *  @param              dmx idx
 *  @param              cache param
 *
 *  @return
 *
 *  @author             Stephen Xiao <stephen.xiao@alitech.com>
 *  @date               12/01/2014, Created
 *
 *  @note
 */
 alisl_retcode alisldmx_cache_set_by_index(uint32_t, dmx_cache_param_t* );


 /**
  *  Function Name:      alisldmx_cache_set
  *  @brief              set dmx cache by share library's handle, use this function
  *                      you can avoid to open the dmx device again.
  *
  *  @param              share libaray's handle
  *  @param              cache param
  *
  *  @return
  *
  *  @author             Stephen Xiao <stephen.xiao@alitech.com>
  *  @date               12/01/2014, Created
  *
  *  @note
  */
 alisl_retcode alisldmx_cache_set(alisl_handle handle,
                             dmx_cache_param_t* param);

/**
 *  Function Name:      alisldmx_hw_buffer_clean
 *  @brief              clean the dmx's cache set, by dmx idx
 *
 *  @param              dmx idx
 *
 *  @return
 *
 *  @author             Stephen Xiao <stephen.xiao@alitech.com>
 *  @date               12/01/2014, Created
 *
 *  @note
 */
 alisl_retcode alisldmx_hw_buffer_clean(uint32_t);


 /**
  *  Function Name:      alisldmx_cache_retrace_set
  *  @brief              set cache retrace, we need invoke this function before
  *                      av_start
  *
  *  @param              share library's handle
  *  @param              retrace param
  *
  *  @return
  *
  *  @author             Stephen Xiao <stephen.xiao@alitech.com>
  *  @date               12/01/2014, Created
  *
  *  @note
  */
 alisl_retcode alisldmx_cache_retrace_set(alisl_handle handle, dmx_cache_retrace_param_t* param);


 /**
  *  Function Name:      alisldmx_cache_retrace_start
  *  @brief              set cache retrace start, we need invoke this function 
  *                      after av_start
  *
  *  @param              share library's handle
  *
  *  @return
  *
  *  @author             Stephen Xiao <stephen.xiao@alitech.com>
  *  @date               12/01/2014, Created
  *
  *  @note
  */
 alisl_retcode alisldmx_cache_retrace_start(alisl_handle handle);
 
 /**
  *  Function Name:      alisldmx_get_see_dmx_id
  *  @brief              get see dmx id for config avsync
  *
  *  @param[in]          share library's handle
  *  @param[out]         current see dmx id
  *
  *  @return
  *
  *  @author             Wendy He <wendy.he@alitech.com>
  *  @date               17/03/2016, Created
  *
  *  @note
  */

 alisl_retcode alisldmx_get_see_dmx_id(alisl_handle handle, int * id);	
 
 /*
CA process of Multi-process get internal dsc id by dsc fd
*/
alisl_retcode alisldmx_en_rec_info_get(alisl_handle handle, struct dmx_record_deencrypt_info* info);

/*
Non-CA process(pvrlib->hld-sl) of Multi-process set internal dsc to record
*/
alisl_retcode alisldmx_dsc_id_set(alisl_handle handle, unsigned int dmx_dsc_id);

/*
Non-CA process of Multi-process set ali pvr decrypt handle got by PVR_IO_CAPTURE_DECRYPT_RES 
when calling aui_dmx_data_path_set.
*/
alisl_retcode alisldmx_ali_pvr_de_hdl_set(alisl_handle handle, unsigned int ali_pvr_de_hdl);

/*
Non-CA process(pvrlib->hld-sl) of Multi-process get ali pvr decrypt handle  for
PVR_IO_SET_DECRYPT_RES & PVR_IO_DECRYPT_EVO & PVR_IO_RELEASE_DECRYPT_RES
*/
alisl_retcode alisldmx_ali_pvr_de_hdl_get(alisl_handle handle, unsigned int *p_ali_pvr_de_hdl);

/*
Get the number of discontinuous packets of video and audio stream when live play.
Function Name:      alisldmx_get_discontinue_pkt_cnt
@param[in]  handle        share library's handle
@param[out]  cnt        the number of discontinuous packets
*/
alisl_retcode alisldmx_get_discontinue_pkt_cnt(alisl_handle handle, unsigned int *cnt);

/*
Clear the number of discontinuous packets of video and audio stream when live play.
Function Name:      alisldmx_get_discontinue_pkt_cnt
@param[in]  handle        share library's handle
*/
alisl_retcode alisldmx_clear_discontinue_av_pkt_cnt(alisl_handle handle);
 
/*
control the behavior of alisldmx_free_filte
@param[in]  block, if block is not zero, alisldmx_free_filte will  block 
until the user callback of fitler returns, otherwise, it won`t block.
*/
void alisldmx_control_free_flt_behavior(int block);
#ifdef __cplusplus
}
#endif

#endif /* __ALISLDMX__H_ */
