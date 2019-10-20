/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           sltest_common.h
 *  @brief          Common interface for SL layer unit test.
 *
 *  @Version:       1.0
 *  @date:          04/08/2014 10:42:59 AM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */
#ifndef __SLTEST_COMMON_H__
#define __SLTEST_COMMON_H__

#include <alisltsi.h>
#include <alisldmx.h>
#include <alislvdec.h>
#include <alislsnd.h>
#include <alislnim.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SLTEST_DEBUG(log)   do {    \
    printf("%s - %d: %s\n", __func__, __LINE__, log);   \
}while(0);

#define QAM16	4
#define QAM32	5
#define QAM64	6
#define QAM128	7
#define QAM256	8

typedef enum {
	eSL_BOARD_3515_DEMO = 1,
	eSL_BOARD_3733_DEMO,
	eSL_BOARD_3715_DEMO,
	eSL_BOARD_MAX,
} sltest_board_type;

typedef enum {
	eSL_FRONTEND_DVBS = 1,
	eSL_FRONTEND_DVBC,
	eSL_FRONTEND_MAX,
} sltest_frontend_type;

typedef struct {
	alisl_handle nim_hdl;
	alisl_handle tsi_hdl;
	alisl_handle dmx_hdl;
	alisl_handle vdec_hdl;
	alisl_handle snd_hdl;
} sltest_handle;

typedef struct {
	enum tsi_id tsi_id;
	unsigned int input_id;
	unsigned int input_attr;
	unsigned int channel_id;
	unsigned int tsi_dmx;
} test_tsi_config;

typedef struct {
	nim_id_t    nimID;
	unsigned int polar;
	unsigned short qpsk_config;
	unsigned short freq_high;
	unsigned short freq_low;
	unsigned int tuner_i2c_base;
	unsigned int tuner_i2c_type;
	unsigned int tuner_id;
	unsigned int demod_i2c_base;
	unsigned int demod_i2c_type;
} test_dvbs_nim_config;

typedef struct {
	nim_id_t nimID;
	unsigned int mode_value;
	unsigned int nim_clk;
	unsigned int tuner_i2c_base;
	unsigned int tuner_i2c_type;
	unsigned int tuner_id;
	unsigned char rf_agc_max;
	unsigned char rf_agc_min;
	unsigned char if_agc_max;
	unsigned char if_agc_min;
	unsigned char agc_ref;
	unsigned char tuner_crystal;
	unsigned char chip;
	unsigned char tuner_special_config;
	unsigned char tuner_ref_divratio;
	unsigned short wtuner_if_freq;
	unsigned char tuner_agc_top;
	unsigned char tuner_step_freq;
	unsigned short tuner_if_freq_J83A;
	unsigned short tuner_if_freq_J83B;
	unsigned short tuner_if_freq_J83C;
	unsigned char  tuner_if_J83AC_type;
} test_dvbc_nim_config;

typedef struct {
	char stream_name[64];
	unsigned int freq;
	unsigned int sym_rate;
	unsigned int v_pid;
	unsigned int a_pid;
	unsigned int p_pid;
	enum vdec_decoder_type v_type;
	enum Snd_decoder_type a_type;
	unsigned char modulation;
} test_frequency_info;

typedef struct {
	dmx_id_t		dmxID;
	int				smc_dev_id;
	test_dvbs_nim_config	dvbs_config;
	test_dvbc_nim_config	dvbc_config;
	test_tsi_config			tsi_config;
	test_frequency_info		freq_config;
} sltest_config;

typedef struct {
	dmx_id_t		dmxID;
	char *tsg_stream_path;
	test_tsi_config			tsi_config;
	test_frequency_info		freq_config;
} sltest_tsg_config;

/*
 *  @brief          stop the current playback
 *
 *  @param[in]      void
 *
 *  @return         0
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           04/08/2014 03:38:40 PM
 *
 *  @note
 */
int sltest_stop_stream(void);

/*
 *  @brief          Get test configuration from ini file.
 *
 *  @param[out]     config      output configure data
 *  @param[in]      nim_cnt     which nim
 *  @param[in]      freq_group  frequency group
 *
 *  @return         0 - success
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           04/23/2014 10:08:37 AM
 *
 *  @note
 */
int sltest_get_config(sltest_config *config,
					  int nim_number,
					  const char *freq_group);


int sltest_get_tsg_config(sltest_tsg_config *config);


/*
 *  @brief          start play a live stream.
 *
 *  @param[in]      nim_cnt     which nim is selected.
 *  @param[in]      group       which stream selected to play, set in sltest.ini
 *
 *  @return         0 - success
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           04/08/2014 03:39:26 PM
 *
 *  @note
 */
int sltest_play_stream(int nim_cnt, const char *group);

/**
 *  @brief          init av output
 *
 *  @param[in]      null        null
 *
 *  @return         0
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           4/22/2014  17:48:31
 *
 *  @note
 */
int sltest_display_start(void);

/**
 *  @brief          close av output
 *
 *  @param[in]      null        null
 *
 *  @return         0
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           4/22/2014  17:49:7
 *
 *  @note
 */
int sltest_display_stop(void);

/**
 *  @brief			start nim to lock a frequency
 *
 *  @param[out]		p_nim_hdl		point to the SL nim device
 *  @param[in]		test_nim_config		nim configuration, get from the .ini file.
 *  @param[in]		freq			frequency
 *  @param[in]		sym_rate		symbol rate
 *  @param[in]		fec				fec
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  18:49:39
 *
 *  @note
 */
int sltest_nim_start_dvbs(alisl_handle *p_nim_hdl,
						  test_dvbs_nim_config *test_nim_config,
						  unsigned int freq,
						  unsigned int sym_rate,
						  unsigned char fec);

/**
 *  @brief			init dvbc frontend and lock the specified frequency
 *
 *  @param[out]		p_nim_hdl		the sl nim handle
 *  @param[in]		test_nim_config	nim configuration get by sltest_get_dvbc_config.
 *  @param[in]		freq			frequency, unit: 10K. example, input 5700 for 570MHz.
 *  @param[in]		sym_rate		symbol rate
 *  @param[in]		fec				forward error correction.
 *  @param[in]		modulation		modulation method, for dvb-c/t
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			8/6/2014  19:0:23
 *
 *  @note
 */
int sltest_nim_start_dvbc(alisl_handle *p_nim_hdl,
						  test_dvbc_nim_config *test_nim_config,
						  unsigned int freq,
						  unsigned int sym_rate,
						  unsigned char fec,
						  unsigned char modulation);

/**
 *  @brief			start the TSI device
 *
 *  @param[out]		p_tsi_hdl		point to the SL TSI device
 *  @param[in]		tsi_config		the configuration of TSI, get from the .ini file.
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  18:52:51
 *
 *  @note
 */
int sltest_tsi_start(alisl_handle *p_tsi_hdl,
					 test_tsi_config *tsi_config);

/**
 *  @brief			Start demux
 *
 *  @param[out]		p_dmx_hdl		Point to the SL Demux device
 *  @param[in]		dmxid			Demux ID, get from the .ini file.
 *  @param[in]		freq_info		The information of the frequency, pids, video/audio type...
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  18:55:11
 *
 *  @note
 */
int sltest_dmx_start(alisl_handle *p_dmx_hdl,
					 enum dmx_id dmxid,
					 test_frequency_info *freq_info);

/**
 *  @brief			Start display
 *
 *  @param[out]		p_dis_hd		Point to the SL HD display device
 *  @param[out]		p_dis_sd		Point to the SL SD display device
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  18:59:12
 *
 *  @note
 */
int sltest_dis_start(alisl_handle *p_dis_hd,
					 alisl_handle *p_dis_sd);

/**
 *  @brief			Start the video decoder
 *
 *  @param[out]		p_vdec_hdl		Point to the SL VDEC deivce
 *  @param[in]		dis_hdl			SL display handle
 *  @param[in]		v_type			Video type
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:1:51
 *
 *  @note
 */
int sltest_vdec_start(alisl_handle *p_vdec_hdl,
					  alisl_handle dis_hdl,
					  enum vdec_decoder_type v_type);

/**
 *  @brief			Start the sound
 *
 *  @param[out]		s_snd_hdl		Point to the SL sound device
 *  @param[in]		volume			The volume you want to set
 *  @param[in]		audio_type		The type of the audio
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:4:29
 *
 *  @note
 */
int sltest_snd_start(alisl_handle *p_snd_hdl,
					 unsigned char volume,
					 enum Snd_decoder_type audio_type);

/**
 *  @brief			Start HDMI display
 *
 *  @param[out]		hdmi_hdl		Point to the SL HDMI device
 *  @param[out]		hdcp_key		HDCP key
 *
 *  @return			0 - success
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:6:20
 *
 *  @note
 */
int sltest_hdmi_start(alisl_handle *hdmi_hdl,
					  unsigned char *hdcp_key);

/**
 *  @brief			Stop the HDMI device
 *
 *  @param[in]		hdmi_hdl		SL HDMI handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:11:10
 *
 *  @note
 */
int sltest_hdmi_stop(alisl_handle hdmi_hdl);

/**
 *  @brief			Stop the nim device
 *
 *  @param[in]		nim_hdl		SL NIM handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:10:41
 *
 *  @note
 */
int sltest_nim_stop(alisl_handle nim_hdl);

/**
 *  @brief			Stop the TSI device
 *
 *  @param[in]		tsi_hdl		SL TSI handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:10:5
 *
 *  @note
 */
int sltest_tsi_stop(alisl_handle tsi_hdl);

/**
 *  @brief			Stop the demux
 *
 *  @param[in]		dmx_hdl		SL demux handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:9:32
 *
 *  @note
 */
int sltest_dmx_stop(alisl_handle dmx_hdl);

/**
 *  @brief			Stop the display module
 *
 *  @param[in]		dis_hdl		SL display handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:9:2
 *
 *  @note
 */
int sltest_dis_stop(alisl_handle dis_hdl);

/**
 *  @brief			Stop the video decoder
 *
 *  @param[in]		vdec_hdl		SL video decoder handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:8:31
 *
 *  @note
 */
int sltest_vdec_stop(alisl_handle vdec_hdl);

/**
 *  @brief			Stop the sound
 *
 *  @param[in]		snd_hdl		SL sound handle
 *
 *  @return			0
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			7/8/2014  19:7:54
 *
 *  @note
 */
int sltest_snd_stop(alisl_handle snd_hdl);

#ifdef __cplusplus
}
#endif

#endif

