/** @file     libsnd.h
 *  @brief    include struct and function defination used in snd share library
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef _SND_ALISL_
#define _SND_ALISL_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

typedef struct snd_audio_ioctl_tone_voice_s
{
    unsigned int buffer_add;//tone voice data address
    unsigned int buffer_len;//tone voice data length
}snd_audio_ioctl_tone_voice;
struct snd_audio_mix_info
{
    //unsigned int enable;
    unsigned int ch_num;
    unsigned int sample_rate;
    unsigned int bit_per_samp;
    unsigned int sbm_id;
};

struct Snd_stream_info
{
    unsigned char str_type;
    unsigned char bit_depth;
    unsigned long sample_rate;
    unsigned long samp_num;
    unsigned long chan_num;
    unsigned long frm_cnt;
    unsigned long reserved1;
    unsigned long reserved2;
    unsigned long input_ts_cnt;
    unsigned long sync_error_cnt;
    unsigned long sync_success_cnt;
    unsigned long sync_frm_len;
    unsigned long decode_error_cnt;
    unsigned long decode_success_cnt;
    unsigned long cur_frm_pts;
};

struct snd_deca_buf_info
{
    unsigned long buf_base_addr;   //!< Real BS buffer address
    unsigned long buf_len;         //!< BS buffer length
    unsigned long used_len;        //!< Length used already
    unsigned long remain_len;      //!< Remaining length which has not been used yet
    unsigned long cb_rd;           //!< Read index for control block
    unsigned long cb_wt;           //!< Write index for control block
    unsigned long es_rd;           //!< Read index of BS buffer
    unsigned long es_wt;           //!< Write index of BS buffer
};


struct snd_audio_config
{
    long decode_mode;
    long sync_mode;
    long sync_unit;
    long deca_input_sbm;
    long deca_output_sbm;
    long snd_input_sbm;
    long pcm_sbm;
    long codec_id;
    long bits_per_coded_sample;
    long sample_rate;
    long channels;
    long bit_rate;
    unsigned long pcm_buf;
    unsigned long pcm_buf_size;
    long block_align;
    unsigned char extra_data[512];
    unsigned long codec_frame_size;
    unsigned long extradata_size;
    unsigned char extradata_mode;
    unsigned char cloud_game_prj;
    unsigned char encrypt_mode; //0 clear; 1 full sample; 2 sub sample
};

struct snd_audio_decore_status
{
    unsigned long sample_rate;
    unsigned long channels;
    unsigned long bits_per_sample;
    long first_header_got;
    long first_header_parsed;
    unsigned long frames_decoded;
};

struct snd_audio_frame
{
    long long pts;
    unsigned long size;
    unsigned long pos;
    unsigned long stc_id;
    unsigned long delay;
};

typedef enum snd_mix_end_type
{
    /**
    The playing of the mix audio is to the end
    */
    SL_SND_MIX_END_NORMAL,
    /**
    The mix audio is forced to pause because of no main audio output in playing mode,
    the mix audio can be heard when the main audio resumes.
    If user don't want to play mix audio when main audio resumes, they need to close
    audio mix by themselves
    */
    SL_SND_MIX_PAUSED
}snd_mix_end_type;

/*
snd_deca_cb_type should be sync from enum audio_cb_type of adf_snd.h
*/
typedef enum snd_deca_cb_type_e
{
    SL_DECA_INDEX, //this represents the first deca cb, please add deca callback between this and SL_SND_INDEX
    SL_DECA_MONITOR_NEW_FRAME = SL_DECA_INDEX,
    SL_DECA_MONITOR_START,
    SL_DECA_MONITOR_STOP,
    SL_DECA_MONITOR_DECODE_ERR,
    SL_DECA_MONITOR_OTHER_ERR,
    SL_DECA_FIRST_FRAME,
    SL_DECA_STATE_CHANGED,
    /*
    AUDIO_CB_SND_MONITOR_REMAIN_DATA_BELOW_THRESHOLD, // Moniter Sound card dma data is below the threshold.
    AUDIO_CB_SND_MONITOR_OUTPUT_DATA_END, // Moniter Sound card dma data is occured the end.
    AUDIO_CB_SND_MONITOR_ERRORS_OCCURED // Moniter Sound card is occured some errors.
    */
    SL_SND_INDEX, //this represents the first snd cb, please add snd callback from this. 
    SL_SND_MONITOR_REMAIN_DATA_BELOW_THRESHOLD = SL_SND_INDEX,
    SL_SND_MONITOR_OUTPUT_DATA_END,
    SL_SND_MONITOR_ERRORS_OCCURED,
    SL_SND_MONITOR_MIX_DATA_END,
    SL_DECA_CB_TYPE_MAX
}snd_deca_cb_type;

/*
audio_regcb_param_s should be sync from struct audio_callback_register_param of adf_snd.h
*/
typedef struct _audio_callback_register_param_s
{
    snd_deca_cb_type e_cb_type;      //!<The callback function type to be registered.
    unsigned int monitor_rate;             //!<The monitor rate to be registered.
    unsigned int timeout_threshold;        //!<The timeout threshold to be registered.
    unsigned int reversed;                 //!<The reversed param to be registered.
}audio_regcb_param_s;

typedef void (*alislsnd_callback) (snd_deca_cb_type type, uint32_t param1, uint32_t param2);

enum Snd_decoder_type
{
    SND_TYPE_MPEG1,            // MPEG I
    SND_TYPE_MPEG2,            // MPEG II
    SND_TYPE_MPEG_AAC,
    SND_TYPE_AC3,                      // AC-3
    SND_TYPE_DTS,              //DTS audio for DVD-Video
    SND_TYPE_PPCM,             //Packet PCM for DVD-Audio
    SND_TYPE_LPCM_V,           //Linear PCM audio for DVD-Video
    SND_TYPE_LPCM_A,           //Linear PCM audio for DVD-Audio
    SND_TYPE_PCM,                      //PCM audio
    SND_TYPE_WMA,              //WMA audio
    SND_TYPE_RA8,                      //Real audio 8
    SND_TYPE_MP3,                      //MP3 audio 
    SND_TYPE_MPEG_ADTS_AAC,
    SND_TYPE_OGG,
    SND_TYPE_EC3,
    SND_TYPE_INVALID
};

enum snd_status
{
	DECA_STATUS_DETACH = 0,
	DECA_STATUS_ATTACH = 1,
	DECA_STATUS_IDLE = 2,
	DECA_STATUS_PLAY = 4,
	DECA_STATUS_PAUSE = 8
};

enum SndSyncMode
{
	SND_AVSYNC_FREERUN = 1,  /**<Audio decoder just decode and send decoded
 	                            frame to OUTPUT, not caring APTS and STC*/
	SND_AVSYNC_PTS           /**<Audio decoder free run, but it will modify
	                            STC frequency according to the difference
	                            between STC value and APTS at output. And
	                            decoder need to compare APTS of 1st audio
	                            frame and STC to decide when to decode and
	                            send it to output.*/
};

enum SndTrackMode
{
	SND_TRACK_NONE,
	SND_TRACK_L,
	SND_TRACK_R,
	SND_TRACK_MONO
};


enum SndOutFormat
{
	SND_OUT_FORMAT_INVALID = -1,
	SND_OUT_FORMAT_PCM = 0,
	SND_OUT_FORMAT_BS = 1,
	SND_OUT_FORMAT_FORCE_DD = 2
};

enum SndIoPort
{
	SND_IO_ALL = 0,
	SND_IO_RCA,
	SND_IO_HDMI,
	SND_IO_SPDIF,
	SND_IO_I2S,
};

enum SndIoCmd
{
	SND_DECA_SET_STR_TYPE = 0x1,//DECA_SET_STR_TYPE
	SND_DECA_GET_STR_TYPE,//2 DECA_GET_STR_TYPE
	SND_DECA_SET_DOLBY_ONOFF,//3 DECA_SET_DOLBY_ONOFF
	SND_DECA_AUDIO_KEY,//4 DECA_AUDIO_KEY
	SND_DECA_BEEP_START,//5 DECA_BEEP_START
	SND_DECA_BEEP_STOP,//6 DECA_BEEP_STOP
	SND_DECA_BEEP_INTERVAL,//7 DECA_BEEP_INTERVAL
	SND_DECA_SET_PLAY_SPEED,//8 DECA_SET_PLAY_SPEED
	SND_DECA_HDD_PLAYBACK,//9 DECA_HDD_PLAYBACK
	SND_DECA_STR_PLAY,  //10 DECA_STR_PLAY
	SND_DECA_SET_MULTI_CH,//11 DECA_SET_MULTI_CH
	SND_DECA_STR_PAUSE,//12 DECA_STR_PAUSE
	SND_DECA_STR_RESUME,//13 DECA_STR_RESUME
	SND_DECA_STR_STOP,//14 DECA_STR_STOP
	SND_DECA_GET_AUDIO_INFO,//15 DECA_GET_AUDIO_INFO
	SND_DECA_GET_HIGHEST_PTS,//16 DECA_GET_HIGHEST_PTS
	SND_DECA_MP3DEC_INIT,//17 DECA_MP3DEC_INIT
	SND_DECA_MP3DEC_CLOSE,//18 DECA_MP3DEC_CLOSE
	SND_DECA_MP3_CAN_DECODE,//19 DECA_MP3_CAN_DECODE
	SND_DECA_MP3_GET_ELAPSE_TIME, //20 DECA_MP3_GET_ELAPSE_TIME
	SND_DECA_MP3_JUMP_TIME,//21 DECA_MP3_JUMP_TIME
	SND_DECA_MP3_SET_TIME,//22 DECA_MP3_SET_TIME
	SND_DECA_MP3_IS_PLAY_END,//23 DECA_MP3_IS_PLAY_END
	SND_DECA_PCM_FRM_LATE,//24 DECA_PCM_FRM_LATE
	SND_DECA_SET_AV_SYNC_LEVEL,//25 DECA_SET_AV_SYNC_LEVEL
	SND_DECA_SOFTDEC_REGISTER_CB,//26 DECA_SOFTDEC_REGISTER_CB
	SND_DECA_SOFTDEC_INIT,//27 DECA_SOFTDEC_INIT
	SND_DECA_SOFTDEC_CLOSE,//28 DECA_SOFTDEC_CLOSE
	SND_DECA_SOFTDEC_JUMP_TIME,//29 DECA_SOFTDEC_JUMP_TIME
	SND_DECA_SOFTDEC_SET_TIME,  //30 DECA_SOFTDEC_SET_TIME
	SND_DECA_SOFTDEC_IS_PLAY_END,//31 DECA_SOFTDEC_IS_PLAY_END
	SND_DECA_SOFTDEC_INIT2,//32 DECA_SOFTDEC_INIT2
	SND_DECA_SOFTDEC_CLOSE2,//33 DECA_SOFTDEC_CLOSE2
	SND_DECA_SOFTDEC_CAN_DECODE2,//34 DECA_SOFTDEC_CAN_DECODE2
	SND_DECA_SOFTDEC_GET_ELAPSE_TIME2,//35 DECA_SOFTDEC_GET_ELAPSE_TIME2
	SND_DECA_SOFTDEC_GET_MUSIC_INFO2,//36 DECA_SOFTDEC_GET_MUSIC_INFO2
	SND_DECA_SOFTDEC_JUMP_TIME2,//37 DECA_SOFTDEC_JUMP_TIME2
	SND_DECA_SOFTDEC_IS_PLAY_END2,//38 DECA_SOFTDEC_IS_PLAY_END2
	SND_DECA_SOFTDEC_REGISTER_CB2,//39 DECA_SOFTDEC_REGISTER_CB2
	SND_DECA_PLAY_MEDIA_STR, //40 DECA_PLAY_MEDIA_STR
	SND_DECA_EMPTY_BS_SET,//41 DECA_EMPTY_BS_SET
	SND_DECA_ADD_BS_SET,//42 DECA_ADD_BS_SET
	SND_DECA_DEL_BS_SET,//43 DECA_DEL_BS_SET
	SND_DECA_IS_BS_MEMBER,//44 DECA_IS_BS_MEMBER
	SND_DECA_AUDIO_PTS_SYNC_STC,//45 DECA_AUDIO_PTS_SYNC_STC
	SND_DECA_REG_PCM_PROCESS_FUNC,//46 DECA_REG_PCM_PROCESS_FUNC
	SND_DECA_SYNC_BY_SOFT,//47 DECA_SYNC_BY_SOFT
	SND_DECA_DOLBYPLUS_CONVERT_ONOFF,//48 DECA_DOLBYPLUS_CONVERT_ONOFF
	SND_DECA_DOLBYPLUS_CONVERT_STATUS,//49 DECA_DOLBYPLUS_CONVERT_STATUS
	SND_DECA_RESET_BS_BUFF,  //50 DECA_RESET_BS_BUFF
	SND_DECA_REG_PCM_BS_PROCESS_FUNC,//51 DECA_REG_PCM_BS_PROCESS_FUNC
	SND_DECA_GET_AUDIO_DECORE,//52 DECA_GET_AUDIO_DECORE
	SND_DECA_DOLBYPLUS_DEMO_ONOFF,//53 DECA_DOLBYPLUS_DEMO_ONOFF
	SND_DECA_SET_BUF_MODE,//54 DECA_SET_BUF_MODE
	SND_DECA_GET_BS_FRAME_LEN,//55 DECA_GET_BS_FRAME_LEN
	SND_DECA_INDEPENDENT_DESC_ENABLE,//56 DECA_INDEPENDENT_DESC_ENABLE
	SND_DECA_GET_DESC_STATUS,//57 DECA_GET_DESC_STATUS
	SND_DECA_GET_DECODER_HANDLE,//58 DECA_GET_DECODER_HANDLE
	SND_DECA_SYNC_NEXT_HEADER,//59 DECA_SYNC_NEXT_HEADER
	SND_DECA_DO_DDP_CERTIFICATION, //60 DECA_DO_DDP_CERTIFICATION
	SND_DECA_DYNAMIC_SND_DELAY,//61 DECA_DYNAMIC_SND_DELAY
	SND_DECA_GET_DDP_INMOD,//62 DECA_GET_DDP_INMOD
	SND_DECA_GET_DECA_STATE,//63 DECA_GET_DECA_STATE
	SND_DECA_GET_DDP_PARAM,//64 DECA_GET_DDP_PARAM
	SND_DECA_SET_DDP_PARAM,//65 DECA_SET_DDP_PARAM
	SND_DECA_CONFIG_BS_BUFFER,//66 DECA_CONFIG_BS_BUFFER
	SND_DECA_CONFIG_BS_LENGTH,//67 DECA_CONFIG_BS_LENGTH
	SND_DECA_BS_BUFFER_RESUME,//68 DECA_BS_BUFFER_RESUME
	SND_DECA_PTS_DELAY,//69 DECA_PTS_DELAY
	SND_DECA_DOLBY_SET_VOLUME_DB,//70 DECA_DOLBY_SET_VOLUME_DB
	SND_DECA_GET_PLAY_PARAM,//71 DECA_GET_PLAY_PARAM
	SND_DECA_MPEG_M8DB_ENABLE,//72 DECA_MPEG_M8DB_ENABLE	
	SND_DECA_EABLE_INIT_TONE_VOICE,//73 DECA_EABLE_INIT_TONE_VOICE	
	SND_DECA_EABLE_DVR_ENABLE,//74 DECA_EABLE_DVR_ENABLE
	SND_DECA_PCM_SIGNED_SET,//75 DECA_PCM_SIGNED_SET
	SND_DECA_GET_HDD_PLAYBACK,//76 DECA_GET_HDD_PLAYBACK
    SND_DECA_PCM_HEAD_INFO_INIT, //77 DECA_PCM_HEAD_INFO_INIT
    SND_SND_GET_DECA_CTRL_BLOCK,//78 SND_GET_DECA_CTRL_BLOCK
    SND_DECA_GET_TS_MP3_INIT,//79 DECA_GET_TS_MP3_INIT
    SND_DECA_IO_REG_CALLBACK,//82 DECA_IO_REG_CALLBACK
    SND_DECA_CREATE_GET_CPU_DATA_TASK,//83 DECA_CREATE_GET_CPU_DATA_TASK
    SND_DECA_START_GET_CPU_DATA_TASK,//84 DECA_START_GET_CPU_DATA_TASK
    SND_DECA_STOP_GET_CPU_DATA_TASK,//85 DECA_STOP_GET_CPU_DATA_TASK
    SND_DECA_SET_BEY1_STREAM_NUMBER, //86 DECA_SET_BEY1_STREAM_NUMBER
    SND_DECA_SOFTDEC_GET_CURRENT_INDEX, //87 DECA_SOFTDEC_GET_CURRENT_INDEX
    SND_DECA_SET_PCM_DECODER_PARAMS, //88 DECA_SET_PCM_DECODER_PARAMS
    SND_DECA_DATA_IO_REG_CALLBACK, //89 DECA_DATA_IO_REG_CALLBACK
    
	SND_DECA_SET_DESC_STREAM_TYPE = SND_DECA_SET_STR_TYPE+98, //99 DECA_SET_DESC_STREAM_TYPE
    SND_DECA_GET_ES_BUFF_STATE,//100 DECA_GET_ES_BUFF_STATE
	SND_DECA_SET_CACHE_INVALID_FLAG,//101 DECA_SET_CACHE_INVALID_FLAG
	SND_DECA_SET_QUICK_PLAY_MODE,//102 DECA_SET_QUICK_PLAY_MODE
	SND_DECA_SET_BYPASS_INFO,//103 DECA_SET_BYPASS_INFO
    SND_DECA_PCM_DUMP_ON,//104 DECA_PCM_DUMP_ON
    SND_DECA_PCM_DUMP_OFF,//105 DECA_PCM_DUMP_OFF
    SND_DECA_CHK_AUD_ENGINE_STATUS, //106 DECA_CHK_AUD_ENGINE_STATUS
    SND_DECA_INIT_AUD_ENGINE,//107 DECA_INIT_AUD_ENGINE
    SND_DECA_START_AUD_ENGINE,//108 DECA_START_AUD_ENGINE
    SND_DECA_DTS_FALL_BACK,//109 DECA_DTS_FALL_BACK
    SND_DECA_CHANGE_AUD_TRACK,//110 DECA_CHANGE_AUD_TRACK
	
	SND_DECA_SET_REVERB = 0x201,//DECA_SET_REVERB
	SND_DECA_SET_PL_II,//2  DECA_SET_PL_II
	SND_DECA_SET_AC3_MODE,//3 DECA_SET_AC3_MODE
	SND_DECA_SET_AC3_STR_MODE,//4 DECA_SET_AC3_STR_MODE
	SND_DECA_GET_AC3_BSMOD,//5 DECA_GET_AC3_BSMOD
	SND_SET_PASS_CI,//6 SET_PASS_CI
	SND_DECA_CHECK_DECODER_COUNT,//7 DECA_CHECK_DECODER_COUNT
	SND_DECA_SET_DECODER_COUNT,//8 DECA_SET_DECODER_COUNT
	SND_DECA_SET_AC3_COMP_MODE,//9 DECA_SET_AC3_COMP_MODE
	SND_DECA_SET_AC3_STEREO_MODE,//10 DECA_SET_AC3_STEREO_MODE
	SND_DECA_SET_AVSYNC_MODE,//11 DECA_SET_AVSYNC_MODE
	SND_DECA_SET_AVSYNC_TEST,//12 DECA_SET_AVSYNC_TEST
	SND_DECA_SET_PTS_SHIFT,//13 DECA_SET_PTS_SHIFT
	SND_DECA_GET_FREE_BSBUF_SIZE,//14 DECA_GET_FREE_BSBUF_SIZE
	SND_DECA_GET_BSBUF_SIZE,//15 DECA_GET_BSBUF_SIZE
	SND_DECA_IO_GET_INPUT_CALLBACK_ROUTINE,//16 DECA_IO_GET_INPUT_CALLBACK_ROUTINE

	SND_DECORE_INIT = 0x301, // DECA_DECORE_INIT
	SND_DECORE_RLS,//2 DECA_DECORE_RLS
	SND_DECORE_SET_BASE_TIME,//3 DECA_DECORE_SET_BASE_TIME
	SND_DECORE_GET_PCM_TRD,//4 DECA_DECORE_GET_PCM_TRD
	SND_DECORE_PAUSE_DECODE,//5 DECA_DECORE_PAUSE_DECODE
	SND_DECORE_FLUSH,//6 DECA_DECORE_FLUSH
	SND_DECORE_SET_QUICK_MODE,//7 DECA_DECORE_SET_QUICK_MODE
	SND_DECORE_SET_SYNC_MODE,//8 DECA_DECORE_SET_SYNC_MODE
	SND_DECORE_GET_CUR_TIME,//9 DECA_DECORE_GET_CUR_TIME
	SND_DECORE_GET_STATUS,//10 DECA_DECORE_GET_STATUS

	SND_IS_SND_RUNNING= 0x401, //1 IS_SND_RUNNING
	SND_IS_SND_MUTE,//2 IS_SND_MUTE
	SND_SND_CC_MUTE,//3 SND_CC_MUTE
	SND_SND_CC_MUTE_RESUME,//4 SND_CC_MUTE_RESUME
	SND_SND_SET_FADE_SPEED,//5 SND_SET_FADE_SPEED
	SND_IS_PCM_EMPTY,//6 IS_PCM_EMPTY
	SND_SND_PAUSE_MUTE,//7 SND_PAUSE_MUTE
	SND_SND_SPO_ONOFF,//8 SND_SPO_ONOFF
	SND_SND_REQ_REM_DATA,//9 SND_REQ_REM_DATA
	SND_SND_SPECTRUM_START,//10 SND_SPECTRUM_START
	SND_SND_SPECTRUM_STOP,//11 SND_SPECTRUM_STOP
	SND_SND_SPECTRUM_CLEAR,//12 SND_SPECTRUM_CLEAR
	SND_SND_BYPASS_VCR,//13 SND_BYPASS_VCR
	SND_FORCE_SPDIF_TYPE,//14 FORCE_SPDIF_TYPE
	SND_SND_DAC_MUTE,//15 SND_DAC_MUTE
	SND_SND_CHK_SPDIF_TYPE,//16 SND_CHK_SPDIF_TYPE
	SND_SND_CHK_DAC_PREC,//17 SND_CHK_DAC_PREC
	SND_SND_CHK_PCM_BUF_DEPTH,//18 SND_CHK_PCM_BUF_DEPTH
	SND_SND_POST_PROCESS_0,//19 SND_POST_PROCESS_0
	SND_SND_SPECIAL_MUTE_REG,//20 SND_SPECIAL_MUTE_REG
	SND_STEREO_FUN_ON,//21 STEREO_FUN_ON
	SND_SND_REQ_REM_PCM_DATA,//22 SND_REQ_REM_PCM_DATA
	SND_SND_SPECTRUM_STEP_TABLE,//23 SND_SPECTRUM_STEP_TABLE
	SND_SND_SPECTRUM_VOL_INDEPEND,//24 SND_SPECTRUM_VOL_INDEPEND
	SND_SND_SPECTRUM_CAL_COUNTER,//25 SND_SPECTRUM_CAL_COUNTER
	SND_SND_SET_SYNC_DELAY,//26 SND_SET_SYNC_DELAY
	SND_SND_REQ_REM_PCM_DURA,//27 SND_REQ_REM_PCM_DURA 
	SND_SND_SET_SYNC_LEVEL,//28 SND_SET_SYNC_LEVEL 
	SND_SND_GET_SPDIF_TYPE,//29 SND_GET_SPDIF_TYPE
	SND_SND_SET_BS_OUTPUT_SRC,//30 SND_SET_BS_OUTPUT_SRC
	SND_SND_SET_MUTE_TH,//31 SND_SET_MUTE_TH
	SND_SND_GET_MUTE_TH,//32 SND_GET_MUTE_TH
	SND_SND_SET_SPDIF_SCMS,//33 SND_SET_SPDIF_SCMS
	SND_SND_GET_SAMPLES_REMAIN,//34 SND_GET_SAMPLES_REMAIN
	SND_SND_SECOND_DECA_ENABLE,//35 SND_SECOND_DECA_ENABLE
	SND_SND_SET_DESC_VOLUME_OFFSET,//36 SND_SET_DESC_VOLUME_OFFSET
	SND_SND_GET_TONE_STATUS,//37 SND_GET_TONE_STATUS
	SND_SND_DO_DDP_CERTIFICATION,//38 SND_DO_DDP_CERTIFICATION
	SND_SND_AUTO_RESUME,//39 SND_AUTO_RESUME 
	SND_SND_SET_SYNC_PARAM,//40 SND_SET_SYNC_PARAM
	SND_SND_RESET_DMA_BUF,//41 SND_RESET_DMA_BUF
	SND_SND_I2S_OUT,//42 SND_I2S_OUT
	SND_SND_HDMI_OUT,//43 SND_HDMI_OUT
	SND_SND_SPDIF_OUT,//44 SND_SPDIF_OUT
	SND_SND_SET_FRAME_SHOW_PTS_CALLBACK,//45 SND_SET_FRAME_SHOW_PTS_CALLBACK
	SND_SND_MPEG_M8DB_ENABLE,//46 SND_MPEG_M8DB_ENABLE
	SND_SND_HDMI_ENABLE, //47 SND_HDMI_ENABLE
	SND_SND_GET_SYNC_PARAM,//48 SND_GET_SYNC_PARAM
	SND_SND_RESTART,//49 SND_RESTART
	SND_SND_STOP_IMMD,//50 SND_STOP_IMMD
	SND_SND_DMX_SET_VIDEO_TYPE,//51 SND_DMX_SET_VIDEO_TYPE
	SND_SND_DO_DDP_CERTIFICATION_EX,//52 SND_DO_DDP_CERTIFICATION_EX
	SND_SND_BUF_DATA_REMAIN_LEN,//53 SND_BUF_DATA_REMAIN_LEN
	SND_SND_STC_DELAY_GET,//54 SND_STC_DELAY_GET
	SND_SND_EABLE_INIT_TONE_VOICE,//55 SND_EABLE_INIT_TONE_VOICE
	SND_SND_IO_REG_CALLBACK,//56 SND_IO_REG_CALLBACK
	SND_SND_IO_SET_FADE_ENBALE,//57 SND_IO_SET_FADE_ENBALE
	SND_SND_ONLY_SET_SPDIF_DELAY_TIME,//58 SND_ONLY_SET_SPDIF_DELAY_TIME
	SND_SND_HDMI_CONFIG_SPO_CLK,//59 SND_HDMI_CONFIG_SPO_CLK
	SND_SND_ENABLE_DROP_FRAME,//60 SND_ENABLE_DROP_FRAME
	SND_SND_REG_GET_SYNC_FLAG_CB,//61 SND_REG_GET_SYNC_FLAG_CB
	SND_SND_SET_HW_HOLD_THRESHOLD,//62 SND_REG_GET_SYNC_FLAG_CB
	SND_SND_GET_RESET_PARAM,//63 SND_GET_RESET_PARAM
	SND_SND_SET_RESET_PARAM,//64 SND_SET_RESET_PARAM
	SND_SND_GET_STATUS,//65 SND_GET_STATUS
	SND_SND_GET_RAW_PTS,//66 SND_GET_RAW_PTS
	SND_SND_IO_PAUSE_SND,//67 SND_IO_PAUSE_SND
	SND_SND_IO_RESUME_SND,//68 SND_IO_RESUME_SND
	SND_SND_SET_AUD_AVSYNC_PARAM,//69 SND_SET_AUD_AVSYNC_PARAM
	SND_SND_GET_AUD_AVSYNC_PARAM,//70 SND_GET_AUD_AVSYNC_PARAM
	SND_SND_SET_UPDATE_PTS_TO_DMX_CB,//71 SND_SET_UPDATE_PTS_TO_DMX_CB
	SND_SND_IO_SET_CC_MUTE_RESUME_FRAME_COUNT_THRESHOLD,//72 SND_IO_SET_CC_MUTE_RESUME_FRAME_COUNT_THRESHOLD	
	SND_SND_IO_SPO_INTF_CFG,//73 SND_IO_SPO_INTF_CFG
    SND_SND_IO_DDP_SPO_INTF_CFG,//74 SND_IO_DDP_SPO_INTF_CFG
    SND_SND_IO_SPO_INTF_CFG_GET,    //75 SND_IO_SPO_INTF_CFG_GET
    SND_SND_IO_DDP_SPO_INTF_CFG_GET, //76 SND_IO_DDP_SPO_INTF_CFG_GET
	SND_SND_IO_GET_PLAY_PTS,//77 SND_IO_GET_PLAY_PTS
	SND_SND_ONLY_GET_SPDIF_DELAY_TIME,//78 SND_ONLY_GET_SPDIF_DELAY_TIME

	SND_SND_START = 0x501, //AUDIO_SND_START
    SND_SND_STOP, //AUDIO_SND_STOP
    SND_DECA_INIT_TONE_VOICE, //AUDIO_INIT_TONE_VOICE
	SND_DECA_GEN_TONE_VOICE,  //AUDIO_GEN_TONE_VOICE
	SND_DECA_STOP_TONE_VOICE, //AUDIO_STOP_TONE_VOICE
};

enum Snd_decoder_codec_id //refer to enum codec_id in avplay_sbm.h
{
	/* various PCM "codecs" */
    SND_CODEC_ID_PCM_S16LE= 0x10000,
    SND_CODEC_ID_PCM_S16BE,
    SND_CODEC_ID_PCM_U16LE,
    SND_CODEC_ID_PCM_U16BE,
    SND_CODEC_ID_PCM_S8,
    SND_CODEC_ID_PCM_U8,
    SND_CODEC_ID_PCM_MULAW,
    SND_CODEC_ID_PCM_ALAW,
    SND_CODEC_ID_PCM_S32LE,
    SND_CODEC_ID_PCM_S32BE,
    SND_CODEC_ID_PCM_U32LE,
    SND_CODEC_ID_PCM_U32BE,
    SND_CODEC_ID_PCM_S24LE,
    SND_CODEC_ID_PCM_S24BE,
    SND_CODEC_ID_PCM_U24LE,
    SND_CODEC_ID_PCM_U24BE,
    SND_CODEC_ID_PCM_S24DAUD,
    SND_CODEC_ID_PCM_ZORK,
    SND_CODEC_ID_PCM_S16LE_PLANAR,
    SND_CODEC_ID_PCM_DVD,
    SND_CODEC_ID_PCM_F32BE,
    SND_CODEC_ID_PCM_F32LE,
    SND_CODEC_ID_PCM_F64BE,
    SND_CODEC_ID_PCM_F64LE,
    SND_CODEC_ID_PCM_BLURAY,

    /* various ADPCM codecs */
    SND_CODEC_ID_ADPCM_IMA_QT= 0x11000,
    SND_CODEC_ID_ADPCM_IMA_WAV,
    SND_CODEC_ID_ADPCM_IMA_DK3,
    SND_CODEC_ID_ADPCM_IMA_DK4,
    SND_CODEC_ID_ADPCM_IMA_WS,
    SND_CODEC_ID_ADPCM_IMA_SMJPEG,
    SND_CODEC_ID_ADPCM_MS,
    SND_CODEC_ID_ADPCM_4XM,
    SND_CODEC_ID_ADPCM_XA,
    SND_CODEC_ID_ADPCM_ADX,
    SND_CODEC_ID_ADPCM_EA,
    SND_CODEC_ID_ADPCM_G726,
    SND_CODEC_ID_ADPCM_CT,
    SND_CODEC_ID_ADPCM_SWF,
    SND_CODEC_ID_ADPCM_YAMAHA,
    SND_CODEC_ID_ADPCM_SBPRO_4,
    SND_CODEC_ID_ADPCM_SBPRO_3,
    SND_CODEC_ID_ADPCM_SBPRO_2,
    SND_CODEC_ID_ADPCM_THP,
    SND_CODEC_ID_ADPCM_IMA_AMV,
    SND_CODEC_ID_ADPCM_EA_R1,
    SND_CODEC_ID_ADPCM_EA_R3,
    SND_CODEC_ID_ADPCM_EA_R2,
    SND_CODEC_ID_ADPCM_IMA_EA_SEAD,
    SND_CODEC_ID_ADPCM_IMA_EA_EACS,
    SND_CODEC_ID_ADPCM_EA_XAS,
    SND_CODEC_ID_ADPCM_EA_MAXIS_XA,
    SND_CODEC_ID_ADPCM_IMA_ISS,

    /* AMR */
    SND_CODEC_ID_AMR_NB= 0x12000,
    SND_CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    SND_CODEC_ID_RA_144= 0x13000,
    SND_CODEC_ID_RA_288,

    /* various DPCM codecs */
    SND_CODEC_ID_ROQ_DPCM= 0x14000,
    SND_CODEC_ID_INTERPLAY_DPCM,
    SND_CODEC_ID_XAN_DPCM,
    SND_CODEC_ID_SOL_DPCM,

    /* audio codecs */
    SND_CODEC_ID_MP2= 0x15000,
    SND_CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    SND_CODEC_ID_AAC,
    SND_CODEC_ID_AC3,
    SND_CODEC_ID_DTS,
    SND_CODEC_ID_VORBIS,
    SND_CODEC_ID_DVAUDIO,
    SND_CODEC_ID_BYE1V1,
    SND_CODEC_ID_BYE1V2,
    SND_CODEC_ID_MACE3,
    SND_CODEC_ID_MACE6,
    SND_CODEC_ID_VMDAUDIO,
    SND_CODEC_ID_SONIC,
    SND_CODEC_ID_SONIC_LS,
    SND_CODEC_ID_FLAC,
    SND_CODEC_ID_MP3ADU,
    SND_CODEC_ID_MP3ON4,
    SND_CODEC_ID_SHORTEN,
    SND_CODEC_ID_ALAC,
    SND_CODEC_ID_WESTWOOD_SND1,
    SND_CODEC_ID_GSM, ///< as in Berlin toast format
    SND_CODEC_ID_QDM2,
    SND_CODEC_ID_COOK,
    SND_CODEC_ID_TRUESPEECH,
    SND_CODEC_ID_TTA,
    SND_CODEC_ID_SMACKAUDIO,
    SND_CODEC_ID_QCELP,
    SND_CODEC_ID_WAVPACK,
    SND_CODEC_ID_DSICINAUDIO,
    SND_CODEC_ID_IMC,
    SND_CODEC_ID_MUSEPACK7,
    SND_CODEC_ID_MLP,
    SND_CODEC_ID_GSM_MS, /* as found in WAV */
    SND_CODEC_ID_ATRAC3,
    SND_CODEC_ID_VOXWARE,
    SND_CODEC_ID_APE,
    SND_CODEC_ID_NELLYMOSER,
    SND_CODEC_ID_MUSEPACK8,
    SND_CODEC_ID_SPEEX,
    SND_CODEC_ID_BYE1VOICE,
    SND_CODEC_ID_BYE1PRO,
    SND_CODEC_ID_BYE1LOSSLESS,
    SND_CODEC_ID_ATRAC3P,
    SND_CODEC_ID_EAC3,
    SND_CODEC_ID_SIPR,
    SND_CODEC_ID_MP1,
    SND_CODEC_ID_TWINVQ,
    SND_CODEC_ID_TRUEHD,
    SND_CODEC_ID_MP4ALS,
    SND_CODEC_ID_ATRAC1,
    SND_CODEC_ID_BINKAUDIO_RDFT,
    SND_CODEC_ID_BINKAUDIO_DCT,
    SND_CODEC_ID_AAC_LATM,
    SND_CODEC_ID_OGG,

    SND_CODEC_ID_LAST
};

/**
*    @brief        snd open, just get ali_m36_audio0 handle priv->fd, then
*                  can use this handle, do not need to do other hardware ini,
*                  because it is done by deca module.
*    @author       ze.hong
*    @date         2013-6-19
*    @param[in]    handle is sound device pointer
*    @param[out]
*    @return       alisl_retcode
*    @note
*
*/
alisl_retcode alislsnd_open(void **handle);

/**
 * @brief         sound device close, close snd handle priv->fd.
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_close(void *handle);

/**
 * @brief         start sound device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_start(void *handle);

/**
 * @brief         pause sound device
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_stop(void *handle);

/**
 * @brief         set sound mute
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_pause(void *handle);

/**
 * @brief        resume after pause
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_resume(void *handle);

/**
 * @brief        init ASE
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_init_ase(void *handle);


/**
 * @brief         set audio decoder type
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_decoder_type(void *handle,enum Snd_decoder_type decoder_type);

/**
 * @brief         get audio decoder type
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]    decoder_type
 * @return        decoder_type
 * @note
 *
 */
alisl_retcode alislsnd_get_decoder_type(void *handle,enum Snd_decoder_type *decoder_type);


/**
 * @brief         set sound mute
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[in]     mute is mute or unmute flag,
 * @param[in]     io_port is to control witch io port,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_mute(void *handle, bool mute,enum SndIoPort io_port);

/**
 * @brief         get mute state
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[in]     mute is mute or unmute flag,
 * @param[in]     io_port is to control witch io port,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_mute_state(void *handle,enum SndIoPort io_port,bool *mute_state);

/**
 * @brief         set sound device volume
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer, volume is value to be set
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_volume(void *handle, uint8_t volume,enum SndIoPort io_port);

/**
 * @brief         get  volume
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[in]     io_port
 * @param[out]    volume
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_volume(void *handle,enum SndIoPort io_port,
											uint8_t *volume);
/**
 * @brief         get  underrun_times
 * @date          2017-1-9
 * @param[in]     handle is sound device pointer
 * @param[in]     io_port
 * @param[out]    underrun_times
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_underrun_times(void *handle,enum SndIoPort io_port,
											uint8_t *underrun_times);

/**
 * @brief         set sound channel to duplicate
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     channel indicates which channel be selected
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_trackmode(void *handle, enum SndTrackMode trackmode,
												enum SndIoPort io_port);

/**
 * @brief         set sound channel to duplicate
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     channel indicates which channel be selected
 * @param[in]     io_port
 * @param[out]    trackmode
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_trackmode(void *handle,enum SndIoPort io_port,
 									enum SndTrackMode *trackmode);


/**
 * @brief         set data output format, select pcm or undecoder audio output
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     type is spdif type, pcm or undecoder audio
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_output_format(void *handle, enum SndOutFormat output_format,
													enum SndIoPort io_port);

/**
 * @brief         get data output format, select pcm or undecoder audio output
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     type is spdif type, pcm or undecoder audio
 * @param[in]     io_port
 * @param[out]    output_format
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_output_format(void *handle,enum SndIoPort io_port,
 										enum SndOutFormat *output_format);



/**
 * @brief         set audio sync mode with video
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[in]    sync_mode
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_av_sync_mode(void *handle, enum SndSyncMode sync_mode);

/**
 * @brief         set audio sync mode with video
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]    sync_mode
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_av_sync_mode(void *handle, enum SndSyncMode *sync_mode);

/**
 * @brief         get audio running status
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_status(void *handle,enum snd_status *status);

/**
 * @brief         snd  io control
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cmd is io control command from uplayer
 * @param[in]    param2 param2_description
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_ioctl(void *handle, enum SndIoCmd cmd, unsigned int param);


/**
 * @brief         snd decore io control
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cmd is io control command from uplayer
 * @param[in]    param2 param2_description
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_decore_ioctl(void *handle, enum SndIoCmd cmd, void *param1, void *param2);

/**
 * @brief         init decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[in]     config audio config info
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_init(void *handle, struct snd_audio_config *config);

/**
 * @brief         release decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_release(void *handle);

/**
 * @brief         pause decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_pause(void *handle);

/**
 * @brief         resume decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_resume(void *handle);

/**
 * @brief         flush decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_flush(void *handle);

/**
 * @brief         flush decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_get_status(void *handle,
    struct snd_audio_decore_status *status);

/**
 * @brief         register callback to alislsnd     
 * @author        amu.tu    
 * @date          2015-08-03
 * @param[in]     handle is sound device pointer
 * @param[in]     p_regcb_param -> callback param
 * @param[in]     cb -> cb to be registered
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_reg_callback(alisl_handle hdl, audio_regcb_param_s *p_regcb_param, alislsnd_callback cb);

/**
 * @brief         enable/disable audio description
 * @author        wendy.he
 * @date          2016-10-18
 * @param[in]     handle is sound device pointer
 * @param[in]     enable, default value 1
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_audio_description_enable(void *handle, unsigned char enable);

/**
 * @brief         set mix balance for main audio and audio description
 * @author        wendy.he
 * @date          2016-10-18
 * @param[in]     handle is sound device pointer
 * @param[in]     balance, value range[AUDIO_DESC_LEVEL_MIN(-16), AUDIO_DESC_LEVEL_MAX(16)]
 * @return        alisl_retcode
 * @note          there are two way to affect the audio description volume, the other is 
 *                SND_SND_SET_DESC_VOLUME_OFFSET. Only one affect the result at the same time. 
 *                and the one which is the last setting will affect the result.
 *                (1)SND_SND_SET_DESC_VOLUME_OFFSET: only change the volume of audio description
 *                (2)This function change the proportion of main audio and audio description,
 *                <0: decrease the volume of audio description
 *                >0: decrease the volume of main audio
 */
alisl_retcode alislsnd_set_mix_balance(void *handle, int balance);

/**
 * @brief         set audio mix info
 * @author        tom.xie
 * @date          2016-4-28
 * @param[in]     
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_audio_mix_info(void *handle, struct snd_audio_mix_info *p_mix_info);

/**
 * @brief         set audio mix end
 * @author        tom.xie
 * @date          2016-4-28
 * @param[in]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_audio_mix_end(void *handle);


/**
 * @brief         stop audio mix
 * @author        wendy.he
 * @date          2016-7-21
 * @param[in]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_audio_mix_stop(void *handle);

/**
 * @brief         pause audio mix
 * @author        wendy.he
 * @date          2016-11-02
 * @param[in]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_audio_mix_pause(void *handle);

/**
 * @brief         resume audio mix
 * @author        wendy.he
 * @date          2016-11-02
 * @param[in]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_audio_mix_resume(void *handle);

typedef struct sl_snd_capture_buffer {
    void* pv_buffer_data; //the address of buffer storing pcm data without header
    unsigned long ul_buffer_length; // the length of buffer storing pcm data without header
    unsigned long ul_sample_rate; //sample rate of pcm frame
} sl_snd_capture_buffer;

/**
 * @brief         init pcm capture environment, mmap for example
 * @author        Amu.tu
 * @date          2017-02-28
 * @param[in]     the opened alislsnd handle 
 * @return        alisl_retcode
 */
alisl_retcode alislsnd_pcm_capture_start(void *handle);
/**
 * @brief         deinit pcm capture environment, munmap for example
 * @author        Amu.tu
 * @date          2017-02-28
 * @param[in]     the opened alislsnd handle 
 * @return        alisl_retcode
 */
alisl_retcode alislsnd_pcm_capture_stop(void *handle);

/**
 * @brief         get information of pcm data stored in driver 
 * @author        Amu.tu
 * @date          2017-02-21
 * @param[in]     the opened alislsnd handle 
 * @param[out]    sl_info, pointer to a buffer storing sl_snd_capture_buffer array
 * @param[out]    p_cnt, pointer to a buffer storing sl_snd_capture_buffer array length
 * @return        alisl_retcode
 * @note
 */
alisl_retcode alislsnd_pcm_capture_buf_info_get(void *handle, sl_snd_capture_buffer **sl_info, int *p_cnt);

/**
 * @brief         get information of pcm data stored in driver 
 * @author        Amu.tu
 * @date          2017-02-21
 * @param[in]     the opened alislsnd handle 
 * @param[in]     rd, the sl_snd_capture_buffer array length got by alislsnd_pcm_capture_buf_info_get
 * @return        alisl_retcode
 * @note
 */
alisl_retcode alislsnd_pcm_capture_buf_rd_update(void *handle, unsigned long rd);

#endif /*_SND_ALISL_*/

