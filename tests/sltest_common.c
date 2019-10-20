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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include <alislnim.h>
#include <alisltsi.h>
#include <alisldmx.h>
#include <alislsnd.h>
#include <alisldis.h>
#include <alislvdec.h>
#include <alislhdmi.h>
#include <alislvbi.h>
#include <sltest_common.h>

#define MAX_VALUE_LEN   64
char value_tsg[MAX_VALUE_LEN] = {0};

sltest_handle *p_sltest = NULL;

alisl_handle dis_hd_hdl;
alisl_handle dis_sd_hdl;
alisl_handle hdmi_hdl;

static int display_started = 0;

sltest_board_type board_type = eSL_BOARD_MAX;
sltest_frontend_type frontend_type = eSL_FRONTEND_MAX;

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
						  unsigned char fec)
{
	slnim_config nim_config;
	slnim_freq_lock_t lock_param;
	alisl_handle nim_hdl = NULL;
	int lock_flag = 0;
	int lock_cnt = 0;
	unsigned int agc;
	unsigned int snr;

	if (0 != alislnim_open(&nim_hdl, test_nim_config->nimID)) {
		SLTEST_DEBUG("alislnim open faile!");
		return -1;
	}
	*p_nim_hdl = nim_hdl;

	memset(&nim_config, 0, sizeof(slnim_config));
	nim_config.config_dvbs.demo_config.QPSK_Config		= test_nim_config->qpsk_config;
	nim_config.config_dvbs.demo_config.i2c_base_addr	= test_nim_config->demod_i2c_base;
	nim_config.config_dvbs.demo_config.i2c_type_id		= test_nim_config->demod_i2c_type;
	nim_config.config_dvbs.tunner_config.freq_high		= test_nim_config->freq_high;
	nim_config.config_dvbs.tunner_config.freq_low		= test_nim_config->freq_low;
	nim_config.config_dvbs.tunner_config.i2c_base_addr	= test_nim_config->tuner_i2c_base;
	nim_config.config_dvbs.tunner_config.i2c_type_id	= test_nim_config->tuner_i2c_type;
	nim_config.config_dvbs.tunner_config.tuner_id		= test_nim_config->tuner_id;
	if (0 != alislnim_set_cfg(nim_hdl, &nim_config)) {
		SLTEST_DEBUG("alinim set cfg fail!");
		return -1;
	}

	alislnim_set_polar(nim_hdl, (unsigned char)test_nim_config->polar);

	memset(&lock_param, 0, sizeof(slnim_freq_lock_t));
	lock_param.freq = freq;
	lock_param.sym = sym_rate;
	lock_param.fec = fec;
	if (0 != alislnim_freqlock(nim_hdl, &lock_param)) {
		SLTEST_DEBUG("alislnim freqlock faile!");
		return -1;
	}

	while(1) {
		if (0 != alislnim_get_lock(nim_hdl, &lock_flag)) {
			SLTEST_DEBUG("alislnim get log error");
			return -1;
		}
		if (lock_flag) {
			SLTEST_DEBUG("lock success!");
			break;
		}
		++ lock_cnt;
		sleep(1);
		SLTEST_DEBUG("unlock");

		if (lock_cnt > 10) {
			return -1;
		}
	}

	return 0;
}

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
						  unsigned char modulation)
{
	slnim_config nim_config;
	slnim_freq_lock_t lock_param;
	alisl_handle nim_hdl = NULL;
	int lock_flag = 0;
	int lock_cnt = 0;
	unsigned int agc;
	unsigned int snr;

	if (0 != alislnim_open(&nim_hdl, test_nim_config->nimID)) {
		SLTEST_DEBUG("alislnim open faile!");
		return -1;
	}
	*p_nim_hdl = nim_hdl;

	memset(&nim_config, 0, sizeof(slnim_config));
	nim_config.config_dvbc.demo_config.qam_mode 		= test_nim_config->mode_value | test_nim_config->nim_clk;
	nim_config.config_dvbc.tunner_config.i2c_base_addr	= test_nim_config->tuner_i2c_base;
	nim_config.config_dvbc.tunner_config.i2c_type_id	= test_nim_config->tuner_i2c_type;
	nim_config.config_dvbc.tunner_config.tuner_id		= test_nim_config->tuner_id;
	nim_config.config_dvbc.tunner_config.rf_agc_max		= test_nim_config->rf_agc_max;
	nim_config.config_dvbc.tunner_config.rf_agc_min		= test_nim_config->rf_agc_min;
	nim_config.config_dvbc.tunner_config.if_agc_max		= test_nim_config->if_agc_max;
	nim_config.config_dvbc.tunner_config.if_agc_min		= test_nim_config->if_agc_min;
	nim_config.config_dvbc.tunner_config.agc_ref		= test_nim_config->agc_ref;
	nim_config.config_dvbc.tunner_config.tuner_crystal	= test_nim_config->tuner_crystal;
	nim_config.config_dvbc.tunner_config.chip			= test_nim_config->chip;
	nim_config.config_dvbc.tunner_config.tuner_special_config	= test_nim_config->tuner_special_config;
	nim_config.config_dvbc.tunner_config.tuner_ref_divratio	= test_nim_config->tuner_ref_divratio;
	nim_config.config_dvbc.tunner_config.wtuner_if_freq	= test_nim_config->wtuner_if_freq;
	nim_config.config_dvbc.tunner_config.tuner_agc_top	= test_nim_config->tuner_agc_top;
	nim_config.config_dvbc.tunner_config.tuner_step_freq	= test_nim_config->tuner_step_freq;
	nim_config.config_dvbc.tunner_config.tuner_if_freq_J83A	= test_nim_config->tuner_if_freq_J83A;
	nim_config.config_dvbc.tunner_config.tuner_if_freq_J83B	= test_nim_config->tuner_if_freq_J83B;
	nim_config.config_dvbc.tunner_config.tuner_if_freq_J83C	= test_nim_config->tuner_if_freq_J83C;
	nim_config.config_dvbc.tunner_config.tuner_if_J83AC_type	= test_nim_config->tuner_if_J83AC_type;
	if (0 != alislnim_set_cfg(nim_hdl, &nim_config)) {
		SLTEST_DEBUG("alinim set cfg fail!");
		return -1;
	}

	memset(&lock_param, 0, sizeof(slnim_freq_lock_t));
	lock_param.freq = freq;
	lock_param.sym = sym_rate;
	lock_param.fec = fec;
	lock_param.modulation = modulation; 
	if (0 != alislnim_freqlock(nim_hdl, &lock_param)) {
		SLTEST_DEBUG("alislnim freqlock faile!");
		//return -1;
	}

	while(1) {
		if (0 != alislnim_get_lock(nim_hdl, &lock_flag)) {
			SLTEST_DEBUG("alislnim get log error");
			return -1;
		}
		if (lock_flag) {
			SLTEST_DEBUG("lock success!");
			break;
		}
		++ lock_cnt;
		sleep(1);
		SLTEST_DEBUG("unlock");

		if (lock_cnt > 10) {
			return -1;
		}
	}

	return 0;
}

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
					 test_tsi_config *tsi_config)
{
	alisl_handle tsi_hdl = NULL;

	alisltsi_open(&tsi_hdl, tsi_config->tsi_id, 0);
	*p_tsi_hdl = tsi_hdl;

	alisltsi_setinput(tsi_hdl, tsi_config->input_id, tsi_config->input_attr);
	alisltsi_setchannel(tsi_hdl, tsi_config->input_id, tsi_config->channel_id);
	alisltsi_setoutput(tsi_hdl, tsi_config->tsi_dmx, tsi_config->channel_id);

	return 0;
}

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
					 test_frequency_info *freq_info)
{
	alisl_handle dmx_hdl = NULL;
	struct dmx_channel_attr attr;
	unsigned int channel_id;
	unsigned int video_pid = 0x1ffff;
	unsigned int audio_pid = 0x1ffff;

	video_pid = (VDEC_DECODER_AVC == freq_info->v_type) ?   \
				(freq_info->v_pid + 0x2000) : freq_info->v_pid;

	switch(freq_info->a_type) {
		case SND_TYPE_AC3:
			audio_pid = freq_info->a_pid | 0x2000;
			break;
		case SND_TYPE_MPEG_AAC:
			audio_pid = freq_info->a_pid | 0x8000;
			break;
		default:
			audio_pid = freq_info->a_pid;
	}

	if (0 != alisldmx_open(&dmx_hdl, dmxid, 0)) {
		SLTEST_DEBUG("alisldmx open fail\n");
		return -1;
	}

	if (0 != alisldmx_start(dmx_hdl)) {
		SLTEST_DEBUG("alisldmx start fail\n");
		return -1;
	}

	memset(&attr, 0, sizeof(attr));
	alisldmx_set_front_end(dmx_hdl, 0);
	alisldmx_set_nim_chipid(dmx_hdl, 0);

	if (video_pid != 0x1FFF) {
		attr.stream = DMX_STREAM_VIDEO;
		alisldmx_allocate_channel(dmx_hdl, DMX_CHANNEL_STREAM, &channel_id);
		alisldmx_set_channel_attr(dmx_hdl, channel_id, &attr);
		alisldmx_set_channel_pid(dmx_hdl, channel_id, video_pid);
		alisldmx_control_channel(dmx_hdl, channel_id, DMX_CTRL_ENABLE);
	}

	if (freq_info->a_pid != 0x1FFF) {
		attr.stream = DMX_STREAM_AUDIO;
		alisldmx_allocate_channel(dmx_hdl, DMX_CHANNEL_STREAM, &channel_id);
		alisldmx_set_channel_attr(dmx_hdl, channel_id, &attr);
		alisldmx_set_channel_pid(dmx_hdl, channel_id, audio_pid);
		alisldmx_control_channel(dmx_hdl, channel_id, DMX_CTRL_ENABLE);
	}

	if (video_pid != 0x1FFF &&
		freq_info->a_pid != 0x1FFF && freq_info->p_pid & 0x1FFF) {
		attr.stream = DMX_STREAM_PCR;
		alisldmx_allocate_channel(dmx_hdl, DMX_CHANNEL_STREAM, &channel_id);
		alisldmx_set_channel_attr(dmx_hdl, channel_id, &attr);
		alisldmx_set_channel_pid(dmx_hdl, channel_id, freq_info->p_pid);
		alisldmx_control_channel(dmx_hdl, channel_id, DMX_CTRL_ENABLE);
	}

	alisldmx_set_avsync_mode(dmx_hdl, DMX_AVSYNC_LIVE);

	alisldmx_avstart(dmx_hdl);

	*p_dmx_hdl = dmx_hdl;

	return 0;
}

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
					 alisl_handle *p_dis_sd)
{
	if (ERROR_NONE != alisldis_open(DIS_HD_DEV, p_dis_hd)) {
		SLTEST_DEBUG("alisldis open faile\n");
		return -1;
	}

	if (ERROR_NONE != alisldis_win_onoff_by_layer(*p_dis_hd, 1, DIS_LAYER_MAIN)) {
		SLTEST_DEBUG("win onoff fail");
		return -1;
	}

	if (ERROR_NONE != alisldis_set_attr(*p_dis_hd, DIS_ATTR_DISAUTO_WIN_ONOFF, 1)) {
		SLTEST_DEBUG("dis set attr fail!");
		return -1;
	}

	if (ERROR_NONE != alisldis_open(DIS_SD_DEV, p_dis_sd)) {
		SLTEST_DEBUG("alisldis open faile\n");
		return -1;
	}

	
	if (ERROR_NONE != alisldis_win_onoff_by_layer(*p_dis_sd, 1, DIS_LAYER_MAIN)) {
		printf("win onoff fail");
		return -1;
	}

#if 0
	if (ERROR_NONE != alisldis_set_attr(*p_dis_sd, DIS_ATTR_DISAUTO_WIN_ONOFF, 1)) {
		SLTEST_DEBUG("dis set attr fail!");
		return -1;
	}
#endif
	if (ERROR_NONE != alisldis_set_tvsys(*p_dis_hd, DIS_TVSYS_LINE_720_25, 1)) {
		SLTEST_DEBUG("set tvsys fail!");
		return -1;
	}

	/** cvbs only support interlace scan!!!! */
	if (ERROR_NONE != alisldis_set_tvsys(*p_dis_sd, DIS_TVSYS_PAL, 0)) {
		SLTEST_DEBUG("set tvsys fail!");
		return -1;
	}

	return 0;
}

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
					  enum vdec_decoder_type v_type)
{
	alisl_handle vdec_hdl = NULL;

	if (dis_hdl) {
		alisldis_win_onoff_by_layer(dis_hd_hdl, 0, DIS_LAYER_MAIN);
		alisldis_win_onoff_by_layer(dis_sd_hdl, 0, DIS_LAYER_MAIN);
	}

	if (ERROR_NONE != alislvdec_open(&vdec_hdl, 0)) {
		SLTEST_DEBUG("vdec open fail");
		return -1;
	}

	if (ERROR_NONE != alislvdec_set_decoder(vdec_hdl, v_type, 0)) {
		SLTEST_DEBUG("vdec set decoder fail");
		return -1;
	}

	if (ERROR_NONE != alislvdec_start(vdec_hdl)) {
		SLTEST_DEBUG("vdec tart fail");
		return -1;
	}

	if (ERROR_NONE != alislvdec_set_sync_mode(vdec_hdl, VDEC_AV_SYNC_PTS)) {
		SLTEST_DEBUG("vdec set sync mode fail!");
		return -1;
	}

	*p_vdec_hdl = vdec_hdl;

	return 0;
}

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
					 enum Snd_decoder_type audio_type)
{
	alisl_handle snd_hdl = NULL;

	if (0 != alislsnd_open(&snd_hdl)) {
		SLTEST_DEBUG("snd open fail");
		return -1;
	}

	if (alislsnd_set_mute(snd_hdl, 0, 0)) {
		SLTEST_DEBUG("snd set mute");
	}
	if (alislsnd_set_decoder_type(snd_hdl, audio_type)) {
		SLTEST_DEBUG("snd set audio type");
	}

	if (alislsnd_set_volume(snd_hdl, volume, 0)) {
		SLTEST_DEBUG("snd set volume");
	}
	
	if (alislsnd_start(snd_hdl)) {
		SLTEST_DEBUG("snd start");
	}
	
	if (alislsnd_set_av_sync_mode(snd_hdl, SND_AVSYNC_PTS)) {
		SLTEST_DEBUG("snd set sync mode fail!");
	}

	return 0;
}

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
					  unsigned char *hdcp_key)
{
	if (0 != alislhdmi_open(hdmi_hdl, hdcp_key)) {
		SLTEST_DEBUG("HDMI open fail!");
		return -1;
	}

	if (0 != alislhdmi_mem_sel(*hdmi_hdl, 0)) {
		SLTEST_DEBUG("HDMI fail!\n");
		return -1;
	}

	return 0;
}

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
int sltest_hdmi_stop(alisl_handle hdmi_hdl)
{
	alislhdmi_close(hdmi_hdl);

	return 0;
}

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
int sltest_nim_stop(alisl_handle nim_hdl)
{
	alislnim_close(nim_hdl);

	return 0;
}

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
int sltest_tsi_stop(alisl_handle tsi_hdl)
{
	alisltsi_close(tsi_hdl);

	return 0;
}

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
int sltest_dmx_stop(alisl_handle dmx_hdl)
{
	alisldmx_stop(dmx_hdl);
	alisldmx_close(dmx_hdl);

	return 0;
}

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
int sltest_dis_stop(alisl_handle dis_hdl)
{
	alisldis_close(dis_hdl);

	return 0;
}

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
int sltest_vdec_stop(alisl_handle vdec_hdl)
{
	alislvdec_stop(vdec_hdl, 1, 1);
	alislvdec_close(vdec_hdl);

	return 0;
}

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
int sltest_snd_stop(alisl_handle snd_hdl)
{
	alislsnd_stop(snd_hdl);
	alislsnd_close(snd_hdl);

	return 0;
}

static int sltest_get_freq_config(char *config_file,
								  const char *group,
								  test_frequency_info *freq_info)
{
	char value[MAX_VALUE_LEN] = {0};

	/** get stream name */
	read_conf_value_ex("stream_name", value, group, MAX_VALUE_LEN, config_file);
	strncpy(freq_info->stream_name, value, sizeof(value));
	memset(value, 0, MAX_VALUE_LEN);

	/** get freq */
	read_conf_value_ex("frequency", value, group, MAX_VALUE_LEN, config_file);
	freq_info->freq = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** get sym */
	read_conf_value_ex("sym_rate", value, group, MAX_VALUE_LEN, config_file);
	freq_info->sym_rate = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** get video pid */
	read_conf_value_ex("video_pid", value, group, MAX_VALUE_LEN, config_file);
	freq_info->v_pid = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** get audio pid */
	read_conf_value_ex("audio_pid", value, group, MAX_VALUE_LEN, config_file);
	freq_info->a_pid = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** get video pid */
	read_conf_value_ex("pcr_pid", value, group, MAX_VALUE_LEN, config_file);
	freq_info->p_pid = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** get vedio type */
	read_conf_value_ex("v_type", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "VDEC_DECODER_MPEG")) {
		freq_info->v_type = VDEC_DECODER_MPEG;
	} else {
		freq_info->v_type = VDEC_DECODER_AVC;
	}
	memset(value, 0, MAX_VALUE_LEN);
	
	/** get audio type */
	read_conf_value_ex("a_type", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "SND_DECODER_MPEG1")) {
		freq_info->a_type = SND_TYPE_MPEG1;
	} else if(!strcmp(value, "SND_DECODER_MPEG2")) {
		freq_info->a_type = SND_TYPE_MPEG2;
	} else if(!strcmp(value, "SND_DECODER_AC3")) {
		freq_info->a_type = SND_TYPE_AC3;
	} else if(!strcmp(value, "SND_DECODER_AAC")) {
		freq_info->a_type = SND_TYPE_MPEG_AAC;
	} else if(!strcmp(value, "SND_DECODER_ADTS_AAC")) {
		freq_info->a_type = SND_TYPE_MPEG_ADTS_AAC;
	} else {
		freq_info->a_type = SND_TYPE_MPEG2;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** get modulation type */
	read_conf_value_ex("modulation", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "QAM16")) {
		freq_info->modulation = QAM16;
	} else if (!strcmp(value, "QAM32")) {
		freq_info->modulation = QAM32;
	} else if (!strcmp(value, "QAM64")) {
		freq_info->modulation = QAM64;
	} else if (!strcmp(value, "QAM128")) {
		freq_info->modulation = QAM128;
	} else if (!strcmp(value, "QAM256")) {
		freq_info->modulation = QAM256;
	} else {
		freq_info->modulation = 0;
	}

	return 0;
}
static int sltest_get_tsi_config(char *config_file,
								 const char *group,
								 test_tsi_config *tsi_config,
								 dmx_id_t *i_dmx)
{
	char value[MAX_VALUE_LEN] = {0};

	/** tsi id */
	read_conf_value_ex("tsi_id", value, group, MAX_VALUE_LEN, config_file);
	tsi_config->tsi_id = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** input id */
	read_conf_value_ex("input_id", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "ALISL_TSI_SPI_0")) {
		tsi_config->input_id = ALISL_TSI_SPI_0;
	} else if (!strcmp(value, "ALISL_TSI_SPI_1")) {
		tsi_config->input_id = ALISL_TSI_SPI_1;
	} else if (!strcmp(value, "ALISL_TSI_SPI_TSG")) {
		tsi_config->input_id = ALISL_TSI_SPI_TSG;
	} else if (!strcmp(value, "ALISL_TSI_SPI_3")) {
		tsi_config->input_id = ALISL_TSI_SPI_3;
	} else if (!strcmp(value, "ALISL_TSI_SSI_0")) {
		tsi_config->input_id = ALISL_TSI_SSI_0;
	} else if (!strcmp(value, "ALISL_TSI_SSI_2")) {
		tsi_config->input_id = ALISL_TSI_SSI_2;
	} else if (!strcmp(value, "ALISL_TSI_SSI_3")) {
		tsi_config->input_id = ALISL_TSI_SSI_3;
	} else if (!strcmp(value, "ALISL_PARA_MODE_SRC")) {
		tsi_config->input_id = ALISL_PARA_MODE_SRC;
	} else if (!strcmp(value, "ALISL_TSI_SSI2B_0")) {
		tsi_config->input_id = ALISL_TSI_SSI2B_0;
	} else if (!strcmp(value, "ALISL_TSI_SSI2B_1")) {
		tsi_config->input_id = ALISL_TSI_SSI2B_1;
	} else if (!strcmp(value, "ALISL_TSI_SSI4B_0")) {
		tsi_config->input_id = ALISL_TSI_SSI4B_0;
	} else if (!strcmp(value, "ALISL_TSI_SSI4B_1")) {
		tsi_config->input_id = ALISL_TSI_SSI4B_1;
	} else {
		SLTEST_DEBUG("please set the right tsi input id!");
		return -1;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** input attr */
	read_conf_value_ex("input_attr", value, group, MAX_VALUE_LEN, config_file);
	tsi_config->input_attr = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	/** channel id */
	read_conf_value_ex("channel_id", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "ALISL_TSI_TS_A")) {
		tsi_config->channel_id = ALISL_TSI_TS_A;
	} else {
		tsi_config->channel_id = ALISL_TSI_TS_B;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** tsi dmx id */
	read_conf_value_ex("tsi_dmx", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "ALISL_TSI_DMX_0")) {
		tsi_config->tsi_dmx = ALISL_TSI_DMX_0;
	} else if (!strcmp(value, "ALISL_TSI_DMX_1")) {
		tsi_config->tsi_dmx = ALISL_TSI_DMX_1;
	} else {
		tsi_config->tsi_dmx = ALISL_TSI_DMX_3;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** dmx id */
	read_conf_value_ex("dmx_id", value, group, MAX_VALUE_LEN, config_file);
	*i_dmx = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	return 0;
}

static unsigned int sltest_get_i2c_type(char *i2c_type)
{
	if (!strcmp("NIM_I2C_TYPE_SCB0", i2c_type)) {
		return NIM_I2C_TYPE_SCB0;
	} else if (!strcmp("NIM_I2C_TYPE_SCB1", i2c_type)) {
		return NIM_I2C_TYPE_SCB1;
	} else if (!strcmp("NIM_I2C_TYPE_SCB2", i2c_type)) {
		return NIM_I2C_TYPE_SCB2;
	} else if (!strcmp("NIM_I2C_TYPE_GPIO0", i2c_type)) {
		return NIM_I2C_TYPE_GPIO0;
	} else if (!strcmp("NIM_I2C_TYPE_GPIO1", i2c_type)) {
		return NIM_I2C_TYPE_GPIO1;
	} else if (!strcmp("NIM_I2C_TYPE_GPIO2", i2c_type)) {
		return NIM_I2C_TYPE_GPIO2;
	} else {
		SLTEST_DEBUG("unknow demod i2c type!");
		return 0xffffffff;
	}

	return 0xffffffff;
}

static nim_id_t sltest_get_nim_id(char *nim_id)
{
	if (!strcmp("NIM_ID_M3503_0", nim_id)) {
		return NIM_ID_M3503_0;
	} else if (!strcmp("NIM_ID_M3501_0", nim_id)) {
		return NIM_ID_M3501_0;
	} else if (!strcmp("NIM_ID_M3501_1", nim_id)) {
		return NIM_ID_M3501_1;
	} else if (!strcmp("NIM_ID_M3281_0", nim_id)) {
		return NIM_ID_M3281_0;
	}else {
		SLTEST_DEBUG("please add your nim type here!");
		return NIM_NB;
	}

	return NIM_NB;
}

static tuner_id_e sltest_get_tuner_id(char *tuner_id)
{
	if (!strcmp("NIM_TUNNER_SHARP_VZ7306", tuner_id)) {
		return NIM_TUNNER_SHARP_VZ7306;
	} else if (!strcmp("NIM_TUNNER_AV_2012", tuner_id)) {
		return NIM_TUNNER_AV_2012;
	} else if (!strcmp("NIM_TUNNER_MXL603", tuner_id)) {
		return NIM_TUNNER_MXL603;
	} else {
		SLTEST_DEBUG("please add your tuner type here!");
		return NIM_TUNER_NB;
	}

	return NIM_TUNER_NB;
}

static int sltest_get_dvbs_config(char *config_file,
								  const char *group,
								  test_dvbs_nim_config *dvbs_config)
{
	char value[MAX_VALUE_LEN] = {0};

	/** nim id */
	read_conf_value_ex("nim_id", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->nimID = sltest_get_nim_id(value);
	if (NIM_NB == dvbs_config->nimID) {
		return -1;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** get polar H/V */
	read_conf_value_ex("polar", value, group, MAX_VALUE_LEN, config_file);
	if (!strcmp(value, "NIM_PORLAR_HORIZONTAL")) {
		dvbs_config->polar = NIM_PORLAR_HORIZONTAL;
	} else if(!strcmp(value, "NIM_PORLAR_VERTICAL")) {
		dvbs_config->polar = NIM_PORLAR_VERTICAL;
	} else {
		dvbs_config->polar = NIM_PORLAR_HORIZONTAL;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** tuner id */
	read_conf_value_ex("tuner_id", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->tuner_id = sltest_get_tuner_id(value);
	if (NIM_TUNER_NB == dvbs_config->tuner_id) {
		return -1;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** qpsk config */
	read_conf_value_ex("qpsk_config", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->qpsk_config = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	/** freq high */
	read_conf_value_ex("freq_high", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->freq_high = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** freq low */
	read_conf_value_ex("freq_low", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->freq_low = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	/** tuner i2c base */
	read_conf_value_ex("tuner_i2c_base", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->tuner_i2c_base = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	/** tuner i2c type id */
	read_conf_value_ex("tuner_i2c_type", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->tuner_i2c_type = sltest_get_i2c_type(value);
	if (0xffffffff == dvbs_config->tuner_i2c_type) {
		return -1;
	}
	memset(value, 0, MAX_VALUE_LEN);

	/** tuner i2c base */
	read_conf_value_ex("demod_i2c_base", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->demod_i2c_base = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	/** tuner i2c type id */
	read_conf_value_ex("demod_i2c_type", value, group, MAX_VALUE_LEN, config_file);
	dvbs_config->demod_i2c_type = sltest_get_i2c_type(value);
	if (0xffffffff == dvbs_config->demod_i2c_type) {
		return -1;
	}

	memset(value, 0, MAX_VALUE_LEN);

	return 0;
}

static unsigned int sltest_get_qam_mode_value(char *value)
{
	if (!strcmp("NIM_DVBC_J83AC_MODE", value)) {
		return 0x0;
	} else if (!strcmp("NIM_DVBC_J83B_MODE", value)) {
		return 0x01;
	} else {
		return 0xffffffff;
	}
}

static unsigned int sltest_get_qam_nim_clk(char *value)
{
	if (!strcmp("NIM_SAMPLE_CLK_27M", value)) {
		return 0x0;
	} else if (!strcmp("NIM_SAMPLE_CLK_54M", value)) {
		return 0x01;
	} else {
		return 0xffffffff;
	}
}

static int sltest_get_dvbc_config(char *config_file,
								  const char *group,
								  test_dvbc_nim_config *dvbc_config)
{
	char value[MAX_VALUE_LEN] = {0};

	read_conf_value_ex("nim_id", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->nimID = sltest_get_nim_id(value);
	if (NIM_NB == dvbc_config->nimID) {
		return -1;
	}
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("mode_value", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->mode_value = sltest_get_qam_mode_value(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("nim_clk", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->nim_clk = sltest_get_qam_nim_clk(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_i2c_base", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_i2c_base = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_i2c_type", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_i2c_type = sltest_get_i2c_type(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_id", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_id = sltest_get_tuner_id(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("rf_agc_max", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->rf_agc_max = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("rf_agc_min", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->rf_agc_min = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("if_agc_max", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->if_agc_max = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("if_agc_min", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->if_agc_min = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("agc_ref", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->agc_ref = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_crystal", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_crystal = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("chip", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->chip = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_special_config", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_special_config = strtoul(value, 0, 16);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_ref_divratio", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_ref_divratio = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("wtuner_if_freq", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->wtuner_if_freq = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_agc_top", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_agc_top = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_if_freq_J83A", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_if_freq_J83A = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_if_freq_J83B", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_if_freq_J83B = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_if_freq_J83C", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_if_freq_J83C = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	read_conf_value_ex("tuner_if_J83AC_type", value, group, MAX_VALUE_LEN, config_file);
	dvbc_config->tuner_if_J83AC_type = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);

	return 0;
}

static int sltest_get_smc_config(char *config_file,
								 const char *group,
								 int *smc_dev_id)
{
	char value[MAX_VALUE_LEN] = {0};

	/** get stream name */
	read_conf_value_ex("smc_dev_id", value, group, MAX_VALUE_LEN, config_file);
	*smc_dev_id = atoi(value);
	memset(value, 0, MAX_VALUE_LEN);
}

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
					  const char *freq_group)
{
	char *config_file = NULL;
	char nim_group[32] = {0};
	char tsi_group[32] = {0};
	char ext_freq_group[32] = {0};
	char config_file_name[PATH_MAX] = {0};
	char ext_config_file_name[PATH_MAX] = {0};

	if (eSL_BOARD_3515_DEMO == board_type) {
		strcat(config_file_name, "/usr/share/aliplatform_test_config_3515.ini");
		strcat(ext_config_file_name, "../share/aliplatform_test_config_3515.ini");
	} else if (eSL_BOARD_3733_DEMO == board_type) {
		strcat(config_file_name, "/usr/share/aliplatform_test_config_3733.ini");
		strcat(ext_config_file_name, "../share/aliplatform_test_config_3733.ini");
	} else if (eSL_BOARD_3715_DEMO == board_type) {
		strcat(config_file_name, "/usr/share/aliplatform_test_config_3715.ini");
		strcat(ext_config_file_name, "../share/aliplatform_test_config_3715.ini");
	} else {
		printf("Wrong board type!\n");
		return -1;
	}

	/** find the configuration file */
	if (0 != access(config_file_name, F_OK)) {
		if (0 != access(ext_config_file_name, F_OK)) {
			SLTEST_DEBUG("There is no configuration file!");
			return -1;
		}
		config_file = ext_config_file_name;
	} else {
		config_file = config_file_name;
	}

	if (eSL_FRONTEND_DVBS == frontend_type) {
		sprintf(nim_group, "[dvbs_nim%d]", nim_number);
		sprintf(tsi_group, "[tsi_dvbs_nim%d]", nim_number);
	} else if (eSL_FRONTEND_DVBC == frontend_type) {
		sprintf(nim_group, "[dvbc_nim%d]", nim_number);
		sprintf(tsi_group, "[tsi_dvbc_nim%d]", nim_number);
	}
	sprintf(ext_freq_group, "[%s]", freq_group);

	/** get the frequency settings from configuration file. */
	sltest_get_freq_config(config_file, ext_freq_group, &(config->freq_config));

	/** get tsi settings */
	sltest_get_tsi_config(config_file, tsi_group, &(config->tsi_config),
						  &(config->dmxID));

	/** get frontend settings */
	if (eSL_FRONTEND_DVBS == frontend_type) {
		sltest_get_dvbs_config(config_file, nim_group, &(config->dvbs_config));
	} else {
		sltest_get_dvbc_config(config_file, nim_group, &(config->dvbc_config));
	}

	/** get smc settings */
	sltest_get_smc_config(config_file, "[smc]", &(config->smc_dev_id));

	return 0;
}

int sltest_get_tsg_config(sltest_tsg_config *config)
{
	char *config_file = NULL;
	char tsi_group[32] = {0};
	char ext_freq_group[32] = {0};
	char config_file_name[PATH_MAX] = {0};
	char ext_config_file_name[PATH_MAX] = {0};

	if (eSL_BOARD_3515_DEMO == board_type) {
		//strcat(config_file_name, "/usr/share/aliplatform_test_config_3515.ini");
		//strcat(ext_config_file_name, "../share/aliplatform_test_config_3515.ini");
		printf("have not yat allocate!!!\n");
	} else if (eSL_BOARD_3733_DEMO == board_type) {
		strcat(config_file_name, "/usr/share/aliplatform_test_config_3733.ini");
		strcat(ext_config_file_name, "../share/aliplatform_test_config_3733.ini");
	} else if (eSL_BOARD_3715_DEMO == board_type) {
		//strcat(config_file_name, "/usr/share/aliplatform_test_config_3715.ini");
		//strcat(ext_config_file_name, "../share/aliplatform_test_config_3715.ini");
		printf("have not yat allocate!!!\n");
	} else {
		printf("Wrong board type!\n");
		return -1;
	}

	/** find the configuration file */
	if (0 != access(config_file_name, F_OK)) {
		if (0 != access(ext_config_file_name, F_OK)) {
			SLTEST_DEBUG("There is no configuration file!");
			return -1;
		}
		config_file = ext_config_file_name;
	} else {
		config_file = config_file_name;
	}

	strcat(tsi_group, "[tsg_tsi]");
	strcat(ext_freq_group, "[tsg_freq]");

	/** get the frequency settings from configuration file. */
	sltest_get_freq_config(config_file, ext_freq_group, &(config->freq_config));

	/** get tsi settings */
	sltest_get_tsi_config(config_file, tsi_group, &(config->tsi_config),
						  &(config->dmxID));

	

	read_conf_value_ex("tsg_stream_path", value_tsg, "[tsg_path]", MAX_VALUE_LEN, config_file);
	config->tsg_stream_path=value_tsg;
	
	return 0;
}



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
int sltest_play_stream(int nim_cnt, const char *group)
{
	sltest_config test_config;
	test_frequency_info freq_info;
	test_dvbs_nim_config dvbs_config;
	test_tsi_config tsi_config;
	dmx_id_t i_dmx;

	if (p_sltest) {
		sltest_stop_stream();
	}

	p_sltest = (sltest_handle *)malloc(sizeof(sltest_handle));
	memset(p_sltest, 0, sizeof(sltest_handle));

	sltest_display_start();

	if (0 != sltest_get_config(&test_config, nim_cnt, group)) {
		printf("No config file!\n");
		return -1;
	}
	i_dmx = test_config.dmxID;
	memcpy(&freq_info, &(test_config.freq_config), sizeof(freq_info));
	memcpy(&dvbs_config, &(test_config.dvbs_config), sizeof(dvbs_config));
	memcpy(&tsi_config, &(test_config.tsi_config), sizeof(tsi_config));
	printf("freq: %d, sym: %d, vpid: %d, apid: %d, ppid: %d\n",
		   freq_info.freq, freq_info.sym_rate, freq_info.v_pid,
		   freq_info.a_pid, freq_info.p_pid);

	//sltest_hdmi_start(&(p_sltest->hdmi_hdl), hdcp_key);

	if (eSL_FRONTEND_DVBS == frontend_type) {
		if (0 != sltest_nim_start_dvbs(&(p_sltest->nim_hdl), &dvbs_config,
									   freq_info.freq, freq_info.sym_rate, 0)) {
			return -1;
		}
	} else if (eSL_FRONTEND_DVBC == frontend_type) {
		if (0 != sltest_nim_start_dvbc(&(p_sltest->nim_hdl), &(test_config.dvbc_config),
									   freq_info.freq, freq_info.sym_rate, 0, freq_info.modulation)) {
			return -1;
		}
	} else {
		return -1;
	}

	if (0 != sltest_tsi_start(&(p_sltest->tsi_hdl), &tsi_config)) {
		return -1;
	}

	if (0 != sltest_dmx_start(&(p_sltest->dmx_hdl), i_dmx, &freq_info)) {
		return -1;
	}
#if 0
	if (0 != sltest_dis_start(&(p_sltest->dis_hd_hdl), &(p_sltest->dis_sd_hdl))) {
		return -1;
	}
#endif
	if (0 != sltest_vdec_start(&(p_sltest->vdec_hdl), dis_hd_hdl, freq_info.v_type)) {
		return -1;
	}

	if (0 != sltest_snd_start(&(p_sltest->snd_hdl), 30, freq_info.a_type)) {
		return -1;
	}

	//sltest_avsync_start();

	return 0;
}

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
int sltest_stop_stream(void)
{
	if (!p_sltest) {
		return 0;
	}

	sltest_nim_stop(p_sltest->nim_hdl);

	sltest_tsi_stop(p_sltest->tsi_hdl);

	sltest_dmx_stop(p_sltest->dmx_hdl);

	sltest_snd_stop(p_sltest->snd_hdl);

	sltest_vdec_stop(p_sltest->vdec_hdl);

	//sltest_display_stop();

	free(p_sltest);
	p_sltest = NULL;

	return 0;
}

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
int sltest_display_start(void)
{
	char hdcp_key[286] = {0};

	if (display_started) {
		return 0;
	}

	sltest_hdmi_start(&hdmi_hdl, hdcp_key);
	sltest_dis_start(&dis_hd_hdl, &dis_sd_hdl);

	display_started = 1;

	return 0;
}

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
int sltest_display_stop(void)
{
	if (!display_started) {
		return 0;
	}

	sltest_dis_stop(dis_hd_hdl);

	sltest_dis_stop(dis_sd_hdl);

	sltest_hdmi_stop(hdmi_hdl);

	display_started = 0;

	return 0;
}
