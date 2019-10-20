/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislhdmi.h
 *  @brief          data structure and type definition for HDMI here
 *
 *  @version        1.0
 *  @date           Thu 26 Sep 2013 01:08:18 PM CST
 *  @revision       none
 *
 *  @author         Bill Kuo <bill.kuo@alitech.com>
 */
#ifndef _ALISLHDMI_H_
#define _ALISLHDMI_H_

/* ALi common headerfiles */

#include <inttypes.h>
#include <stdbool.h>

/* Shared library headerfiles */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C" {
#endif

enum alislhdmi_errcode {
    HDMI_ERROR_NONE     = ERROR_NONE,
    HDMI_ERROR_GETMSG   = ERROR_GETMSG,
    HDMI_ERROR_BADCB    = ERROR_BADCB,
    HDMI_ERROR_OPEN     = ERROR_OPEN,
    HDMI_ERROR_NOMEM    = ERROR_NOMEM,
    HDMI_ERROR_DEAMON   = ERROR_OPEN,
    HDMI_ERROR_GETRES   = ERROR_GETRES,
    HDMI_ERROR_INVAL    = ERROR_INVAL,
    HDMI_ERROR_IOCTL    = ERROR_IOCTL,
};

typedef enum alisl_audio_coding_type {
	ALISL_EDID_AUDIO_LPCM			= 0x0001,
	ALISL_EDID_AUDIO_AC3			= 0x0002,
	ALISL_EDID_AUDIO_MPEG1			= 0x0004,
	ALISL_EDID_AUDIO_MP3			= 0x0008,
	ALISL_EDID_AUDIO_MPEG2			= 0x0010,
	ALISL_EDID_AUDIO_AAC			= 0x0020,
	ALISL_EDID_AUDIO_DTS			= 0x0040,
	ALISL_EDID_AUDIO_ATRAC			= 0x0080,
	ALISL_EDID_AUDIO_ONEBITAUDIO	= 0x0100,
	ALISL_EDID_AUDIO_DD_PLUS		= 0x0200,
	ALISL_EDID_AUDIO_DTS_HD			= 0x0400,
	ALISL_EDID_AUDIO_MAT_MLP		= 0x0800,
	ALISL_EDID_AUDIO_DST			= 0x1000,
	ALISL_EDID_AUDIO_WMAPRO			= 0x2000,
} e_alisl_audio_coding_type;

typedef enum alisl_hdmi_res {
    ALISL_HDMI_RES_INVALID = 0,
    ALISL_HDMI_RES_480I,
    ALISL_HDMI_RES_480P,
    ALISL_HDMI_RES_576I,
    ALISL_HDMI_RES_576P,
    ALISL_HDMI_RES_720P_50,
    ALISL_HDMI_RES_720P_60,
    ALISL_HDMI_RES_1080I_25,
    ALISL_HDMI_RES_1080I_30,
    ALISL_HDMI_RES_1080P_24,
    ALISL_HDMI_RES_1080P_25,
    ALISL_HDMI_RES_1080P_30,
    ALISL_HDMI_RES_1080P_50,
    ALISL_HDMI_RES_1080P_60,
	ALISL_HDMI_RES_4096X2160_24,
	ALISL_HDMI_RES_3840X2160_24,
	ALISL_HDMI_RES_3840X2160_25,
	ALISL_HDMI_RES_3840X2160_30,

    /* --- extend for 861-F --- */
	ALISL_HDMI_RES_480I_60,
	ALISL_HDMI_RES_480P_120,
	ALISL_HDMI_RES_480I_120,
	ALISL_HDMI_RES_480P_240,
	ALISL_HDMI_RES_576I_50,
	ALISL_HDMI_RES_576P_100,
	ALISL_HDMI_RES_576I_100,
	ALISL_HDMI_RES_576P_200,
	ALISL_HDMI_RES_720P_24,
	ALISL_HDMI_RES_720P_25,
	ALISL_HDMI_RES_720P_30,
	ALISL_HDMI_RES_720P_100,
	ALISL_HDMI_RES_720P_120,
	ALISL_HDMI_RES_1080I_50,
	ALISL_HDMI_RES_1080I_60,	
	ALISL_HDMI_RES_1080P_100,
	ALISL_HDMI_RES_1080P_120,	
	ALISL_HDMI_RES_3840X2160_50,
	ALISL_HDMI_RES_3840X2160_60,
	ALISL_HDMI_RES_4096X2160_25,
	ALISL_HDMI_RES_4096X2160_30,
	ALISL_HDMI_RES_4096X2160_50,
	ALISL_HDMI_RES_4096X2160_60

} e_alisl_hdmi_res;

typedef enum alisl_video_picfmt {
    // YCbCr Format
    ALISL_Video_PicFmt_YCBCR_411,
    ALISL_Video_PicFmt_YCBCR_420,
    ALISL_Video_PicFmt_YCBCR_422,
    ALISL_Video_PicFmt_YCBCR_444,

    //RGB format
    ALISL_Video_PicFmt_RGB_MODE1,     //rgb (16-235)
    ALISL_Video_PicFmt_RGB_MODE2,     //rgb (0-255)
} e_alisl_video_picfmt;


typedef enum _EDID_VEDIO_RESOLUTION {
    NATIVE_VIDEO_RESOLUTION,
    ALL_VIDEO_RESOLUTION
} e_edid_vres_type;

typedef enum _HDMI_KEY_TYPE {
    CEC_LOAD_KEY,
    SW_LOAD_KEY
} e_hdmi_key_type;

typedef enum _HDMI_CALLBACK_TYPE {
    HDMI_CALLBACK_EDID_READY,
    HDMI_CALLBACK_HOT_PLUGIN,
    HDMI_CALLBACK_HOT_PLUGOUT,
    HDMI_CALLBACK_CEC_MSGRCV,
    HDMI_CALLBACK_HDCP_FAIL
} e_hdmi_callback_type;

typedef enum _HDMI_EVENT_TYPE {
    HDMI_RECEIVER_PLUGIN,
    HDMI_RECEIVER_PLUGOUT
} e_hdmi_event_type;

enum HDMI_LinkStatus
{
	ALISL_HDMI_STATUS_UNLINK = 0x01,				/**< HDMI not link */
	ALISL_HDMI_STATUS_LINK_HDCP_SUCCESSED = 0x02,
	ALISL_HDMI_STATUS_LINK_HDCP_FAILED = 0x04,
	ALISL_HDMI_STATUS_LINK_HDCP_IGNORED = 0x08,
	ALISL_HDMI_STATUS_MAX = 0x10,
	ALISL_HDMI_STATUS_LINK = 0x20,
};


/****************** HDMI deep color definition ***********************/
enum alisl_hdmi_deepcolor
{	
	ALISL_HDMI_DEEPCOLOR_24 = 0,	
	ALISL_HDMI_DEEPCOLOR_30,	
	ALISL_HDMI_DEEPCOLOR_36,	
	ALISL_HDMI_DEEPCOLOR_48,
};

enum alisl_hdmi_cea_db_type
{
	ALISL_HDMI_CEA_AUD_TYPE = 0,
	ALISL_HDMI_CEA_VIDEO_TYPE,
	ALISL_HDMI_CEA_VSD_TYPE,
	ALISL_HDMI_CEA_SPK_TYPE
}CEA_DB_TYPE;

typedef struct alisl_short_cea_desc
{
	unsigned char cea_data;
	struct alisl_short_cea_desc *next;
}alisl_short_cea_desc;

typedef struct alisl_hdmi_3d_descriptor
{
	unsigned char hdmi_3d_multi_present;
	unsigned char hdmi_vic_len;
	unsigned char hdmi_3d_len;
	alisl_short_cea_desc	*short_hdmi_vic_desc;
	alisl_short_cea_desc	*short_3d_desc;
}alisl_hdmi_3d_descriptor;

typedef void (*hdmi_edid_callback)(void);
typedef void (*hdmi_hotplug_callback)(void);
typedef void (*hdmi_cec_callback)(uint8_t*, uint8_t);
typedef void (*hdmi_callback)(e_hdmi_event_type event, int index, \
                              void *event_data, void *user_data);
typedef void (*hdmi_hdcp_fail_callback)(uint8_t*, uint8_t);

alisl_retcode alislhdmi_callback_register(alisl_handle handle,
                                          e_hdmi_callback_type type,
                                          alisl_handle cb_func);

alisl_retcode alislhdmi_open(alisl_handle *handle,
                             uint8_t *hdcp_key);

alisl_retcode alislhdmi_close(alisl_handle handle);

alisl_retcode alislhdmi_get_edid_video_format(alisl_handle handle,
                                              enum alisl_video_picfmt *format);

alisl_retcode alislhdmi_get_edid_nativeres(alisl_handle handle,
                                           enum alisl_hdmi_res *res);

alisl_retcode alislhdmi_get_edid_allres(alisl_handle handle,
                                        uint32_t *native_res_index,
                                        enum alisl_hdmi_res *video_res);

alisl_retcode alislhdmi_get_vic_num(alisl_handle handle,
                                         unsigned int *vic_num);

alisl_retcode alislhdmi_get_aud_num(alisl_handle handle,
                                         unsigned int *aud_num);

alisl_retcode alislhdmi_get_edid_CEA_item(alisl_handle handle,
                                        uint8_t index,
                                        enum alisl_hdmi_cea_db_type cea_db_type,
                                        void* item);

alisl_retcode alislhdmi_get_edid_audio_prefercodetype(alisl_handle handle,
                                                      enum alisl_audio_coding_type *aud_fmt);

alisl_retcode alislhdmi_get_edid_audio_allcodetype(alisl_handle handle,
                                                   unsigned short *aud_fmt);

alisl_retcode alislhdmi_set_switch_status(alisl_handle handle,
                                          unsigned int on_off);

alisl_retcode alislhdmi_get_switch_status(alisl_handle handle,
                                          unsigned int *status);

alisl_retcode alislhdmi_set_hdcp_status(alisl_handle handle,
                                        unsigned int on_off);

alisl_retcode alislhdmi_get_hdcp_status(alisl_handle handle,
                                        unsigned int *status);

alisl_retcode alislhdmi_set_cec_status(alisl_handle handle,
                                       unsigned int on_off);

alisl_retcode alislhdmi_get_cec_status(alisl_handle handle,
                                       unsigned int *status);

alisl_retcode alislhdmi_set_audio_status(alisl_handle handle,
                                         unsigned int on_off);

alisl_retcode alislhdmi_get_audio_status(alisl_handle handle,
                                         unsigned int *status);
alisl_retcode alislhdmi_mem_sel(alisl_handle handle,
                                unsigned int mem_sel);

alisl_retcode alislhdmi_set_hdcpkey(alisl_handle handle,
                                    unsigned char *key,
                                    unsigned int length);

alisl_retcode alislhdmi_transmit_cec(alisl_handle handle,
                                     uint8_t *message,
                                     uint8_t message_length);

alisl_retcode alislhdmi_cec_get_pa(alisl_handle handle,
                                   unsigned short *physical_address);

alisl_retcode alislhdmi_cec_set_la(alisl_handle handle,
                                   uint8_t logical_address);

alisl_retcode alislhdmi_cec_get_la(alisl_handle handle,
                                   uint8_t *logical_address);

alisl_retcode alislhdmi_cec_set_res(alisl_handle handle,
                                    enum alisl_hdmi_res *res);

alisl_retcode alislhdmi_cec_get_res(alisl_handle handle,
                                    enum alisl_hdmi_res *res);

alisl_retcode alislhdmi_set_vendor_name(alisl_handle handle,
                                        unsigned char *vendor_name,
                                        unsigned char length);

alisl_retcode alislhdmi_set_product_desc(alisl_handle handle,
                                         unsigned char *product_desc,
                                         unsigned char length);

alisl_retcode alislhdmi_get_vendor_name(alisl_handle handle,
                                        unsigned char *vendor_name,
                                        unsigned char *length);

alisl_retcode alislhdmi_get_monitor_name(alisl_handle handle,
                                        unsigned char *monitor_name,
                                        unsigned char *length);

alisl_retcode alislhdmi_get_manufacturer_name(alisl_handle handle,
                                        unsigned char *manufacturer_name,
                                        unsigned char *length);

alisl_retcode alislhdmi_get_product_desc(alisl_handle handle,
                                         unsigned char *product_desc,
                                         unsigned char *length);

alisl_retcode alislhdmi_get_link_status(alisl_handle handle,
                                        unsigned char *link_status);

alisl_retcode alislhdmi_get_3d_present(alisl_handle handle,
                                       int *present);

alisl_retcode alislhdmi_set_color_space(alisl_handle handle,
                                      enum alisl_video_picfmt color_space);

alisl_retcode alislhdmi_get_color_space(alisl_handle handle,
                                      enum alisl_video_picfmt *color_space);


alisl_retcode alislhdmi_set_deep_color(alisl_handle handle,
                                      enum alisl_hdmi_deepcolor deep_mode);

alisl_retcode alislhdmi_get_deep_color(alisl_handle handle,
                                      enum alisl_hdmi_deepcolor *deep_mode);
alisl_retcode alislhdmi_get_edid_deep_color(alisl_handle handle,
                                      int*dc_fmt);

alisl_retcode alislhdmi_get_edid_total_length(alisl_handle handle,unsigned int *datalen);

alisl_retcode alislhdmi_get_edid_block(alisl_handle handle,unsigned char *p_ediddata, 
											unsigned int *datalen, unsigned int block_idx);

alisl_retcode alislhdmi_set_av_blank(alisl_handle handle,
                                      unsigned int on_off);

alisl_retcode alislhdmi_get_av_blank(alisl_handle handle,
                                         unsigned int *on_off);

alisl_retcode alislhdmi_set_av_mute(alisl_handle handle,
                                      unsigned int on_off);

alisl_retcode alislhdmi_get_hdmi_type(alisl_handle handle,
                                         unsigned int *hdmi_type);

///////////////////////////niker.li add code start////////////////////////////////////////////
alisl_retcode alislhdmi_get_prefer_video_formt(alisl_handle handle,
                                         unsigned int *video_formt, unsigned char idx);

alisl_retcode alislhdmi_get_3d_descriptor(alisl_handle handle,
										 alisl_hdmi_3d_descriptor *desc_data);

alisl_retcode alislhdmi_get_edid_product_id(alisl_handle handle,
										 unsigned short *product_id);

alisl_retcode alislhdmi_get_edid_serial_number(alisl_handle handle,
										 unsigned long *serial_number);

alisl_retcode alislhdmi_get_edid_week_manufacturer(alisl_handle handle,
										 unsigned char *w_manufacture);

alisl_retcode alislhdmi_get_edid_year_manufacturer(alisl_handle handle,
										 unsigned short *y_manufacture);

alisl_retcode alislhdmi_get_edid_version(alisl_handle handle,
										 unsigned short *version);

alisl_retcode alislhdmi_get_edid_revision(alisl_handle handle,
										 unsigned short *revision);

alisl_retcode alislhdmi_get_raw_edid(alisl_handle handle,
										 unsigned int length,unsigned char *p_ediddata);
///////////////////////////niker.li add code end////////////////////////////////////////////

alisl_retcode alislhdmi_get_hdcp_support(alisl_handle handle,
                                                   unsigned char *hdcp_support);
#ifdef __cplusplus
}
#endif
#endif
