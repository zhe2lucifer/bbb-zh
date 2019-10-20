/*
typedef struct channel {
	uint16_t        sat_id;
	DB_TP_ID        tp_id;

	uint32_t        prog_id;

	uint32_t        level;
	uint32_t        preset_flag;
	uint32_t        av_flag;
	uint32_t        ca_mode;
	uint32_t        video_pid;
	uint32_t        pcr_pid;

	uint32_t        prog_number;
	uint32_t        pmt_pid;
	uint32_t        tuner1_valid;
	uint32_t        tuner2_valid;
	uint32_t        h264_flag;

	uint32_t        fav_group[0];
	uint8_t         fav_grp0;
	uint8_t         fav_grp1;
	uint8_t         fav_grp2;
	uint8_t         fav_grp3;
	uint8_t         fav_grp4;
	uint8_t         fav_grp5;
	uint8_t         fav_grp6;
	uint8_t         fav_grp7;
	uint8_t         fav_group_byte2;
	uint8_t         fav_group_byte3;
	uint8_t         fav_group_byte4;

	uint16_t        pmt_version;
	uint16_t        service_type;
	uint16_t        audio_channel;
	uint16_t        audio_select;

	uint16_t        user_modified_flag;
	uint16_t        lock_flag;
	uint16_t        skip_flag;
	uint16_t        audio_volume;
	uint16_t        mpeg4_flag;
	uint16_t        reserve_2;

	uint32_t        mheg5_exist;
	uint32_t        orig_LCN;
	uint32_t        reserve_3;
	uint32_t        LCN_true;
	uint32_t        LCN;
	uint32_t        default_index;

	uint32_t        num_sel_flag;
	uint32_t        visible_flag;
	uint32_t        ait_pid;
	uint32_t        reserved_4;

	uint32_t        user_order;

	uint32_t        provider_lock;
	uint32_t        subtitle_pid;
	uint32_t        teletext_pid;
	
	uint32_t        cur_audio;

	uint32_t        hd_lcn_ture;
	uint32_t        hd_lcn;
	uint32_t        reserve6;
	uint16_t        nvod_sid;
	uint16_t        nvod_tpid;
	uint16_t        bouquet_id;
	uint16_t        logical_channel_num;

	uint32_t        cas_count;
	uint16_t        cas_sysid[MAX_CAS_CNT];

	uint32_t        audio_count;
	uint16_t        audio_pid[MAX_AUDIO_CNT];
	uint16_t        audio_lang[MAX_AUDIO_CNT];
	uint8_t         audio_type[MAX_AUDIO_CNT];
	uint32_t        name_len;
	uint8_t         service_name[2*(ADB_DVB_PRG_MAX_SERVICE_NAME_LENGTH + 1)];

	uint32_t        provider_name_len;
	uint8_t         service_provider_name[2*(ADB_DVB_PRG_MAX_SERVICE_NAME_LENGTH + 1)];
	uint16_t        Internal_number;
	uint16_t        unique_number;
	uint16_t        crypt_type;
	uint16_t        pmc_pid;
	uint32_t        genre;
	uint32_t        flag1;
	uint32_t        flag2;
	uint32_t        reserved5;
	uint8_t         audio_com_tag[MAX_AUDIO_CNT];
} channel_t;
*/

-- channel table
DROP TABLE IF EXISTS `channel`;
CREATE TABLE channel (
	id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	prog_number INTEGER UNSIGNED NULL,
	sat_id INTEGER UNSIGNED NULL,
	tp_id INTEGER UNSIGNED NULL,
	pcr_pid INTEGER UNSIGNED NULL,
	pmt_pid INTEGER UNSIGNED NULL,
	h264_flag INTEGER UNSIGNED NULL,
	audio_count INTEGER UNSIGNED NULL,
	audio_pid TEXT NULL,
	audio_lang TEXT NULL,
	audio_type TEXT NULL,
	name_len INTEGER UNSIGNED NULL,
	service_name TEXT NULL
	);

-- Initialize channel
INSERT INTO channel
       VALUES (0, 0, 0, 0, 0,
	       0, 0, 0,
	       "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19",
	       "0 0 0 0 0 0 0 0 0 0 0  0  0  0  0  0  0  0  0  0",
	       "0 0 0 0 0 0 0 0 0 0 0  0  0  0  0  0  0  0  0  0",
	       12, "aabbcc");

INSERT INTO channel
       VALUES (1, 2, 0, 0, 0,
	       0, 0, 0,
	       "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19",
	       "0 0 0 0 0 0 0 0 0 0 0  0  0  0  0  0  0  0  0  0",
	       "0 0 0 0 0 0 0 0 0 0 0  0  0  0  0  0  0  0  0  0",
	       12, "ccddaa");

/*
typedef struct tp {
	uint32_t        sat_id;

	uint32_t        frq;
	uint32_t        sym;

	uint16_t        pol;
	uint16_t        FEC_inner;
	uint16_t        universal_22k_option;
	uint16_t        Big5_indicator;
	uint16_t        ft_type;
	uint16_t        inverse;
	uint16_t        band_type;

	uint16_t        preset_flag;
	uint16_t        usage_status;
	uint16_t        nit_pid;

	uint32_t        t_s_id;
	uint32_t        network_id;

	uint32_t        net_id;

	uint32_t        sdt_version;

	uint32_t        guard_interval;
	uint32_t        FFT;
	uint32_t        modulation;
	uint32_t        bandwidth;

	uint32_t        intensity;
	uint32_t        quality;
	uint32_t        crc_h8;

	uint32_t        remote_control_key_id;
	uint32_t        crc_t24;
} tp_t;
*/

-- tp table
DROP TABLE IF EXISTS `tp`;
CREATE TABLE tp (
	id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	sat_id INTEGER UNSIGNED NULL,
	frq INTEGER UNSIGNED NULL,
	sym INTEGER UNSIGNED NULL,
	pol INTEGER UNSIGNED NULL
	);

-- Initialize tp
INSERT INTO tp
       VALUES (0, 0, 0, 0, 0);
INSERT INTO tp
       VALUES (1, 0, 0, 0, 0);
INSERT INTO tp
       VALUES (2, 1, 0, 0, 0);
INSERT INTO tp
       VALUES (3, 1, 0, 0, 0);
