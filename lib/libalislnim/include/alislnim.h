/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislnim.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 17:01:56
 *  @revision           none
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 */


#ifndef __ALISLNIM__H_
#define __ALISLNIM__H_

#include <stdint.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define NIM_I2C_TYPE_SCB            0x00000000
#define NIM_I2C_TYPE_GPIO           0x00010000

#define NIM_I2C_TYPE_SCB0           (NIM_I2C_TYPE_SCB|0)
#define NIM_I2C_TYPE_SCB1           (NIM_I2C_TYPE_SCB|1)
#define NIM_I2C_TYPE_SCB2           (NIM_I2C_TYPE_SCB|2)

#define NIM_I2C_TYPE_GPIO0          (NIM_I2C_TYPE_GPIO|0)
#define NIM_I2C_TYPE_GPIO1          (NIM_I2C_TYPE_GPIO|1)
#define NIM_I2C_TYPE_GPIO2          (NIM_I2C_TYPE_GPIO|2)

#define NIM_PORLAR_HORIZONTAL   0x00
#define NIM_PORLAR_VERTICAL     0x01

typedef enum {
    NIM_TUNNER_DCT70701 = 1,    //DVBC
    NIM_TUNNER_DCT7044,         //DVBC
    NIM_TUNNER_ALPSTDQE,        //DVBC
    NIM_TUNNER_ALPSTDAE,        //DVBC
    NIM_TUNNER_TDCCG0X1F,       //DVBC
    NIM_TUNNER_DBCTE702F1,      //DVBC
    NIM_TUNNER_CD1616LF,        //DVBC
    NIM_TUNNER_ALPSTDAC,        //DVBC, DTMB
    NIM_TUNNER_RT810,           //DVBC
    NIM_TUNNER_VRT820C,         //DVBC

    NIM_TUNNER_MXL603,
    NIM_TUNNER_SHARP_VZ7306,
    NIM_TUNNER_AV_2012,
    NIM_TUNNER_TDA18250,
    NIM_TUNNER_SANYO,
    NIM_TUNNER_CD1616LF_GIH,
    NIM_TUNNER_NXP,
    NIM_TUNNER_MAXLINEAR,
    NIM_TUNNER_MICROTUNE,
    NIM_TUNNER_QUANTEK,
    NIM_TUNNER_RFMAGIC,
    NIM_TUNNER_ALPS,
    NIM_TUNNER_PHILIPS,
    NIM_TUNNER_INFINEON,
    NIM_TUNER_RDA5815M,
    NIM_TUNER_MXL214C,
    NIM_TUNER_M3031,
    NIM_TUNER_SI2141,
    NIM_TUNER_CXD2872,
    NIM_TUNER_R858,
	NIM_TUNNER_TDA18250AB,
	NIM_TUNER_R836,
    NIM_TUNER_NB
} tuner_id_e;

typedef enum nim_id {
    NIM_ID_M3501_0 = 0,  /**< Nim id 0, related device is m3501_0 */
    NIM_ID_M3501_1,      /**< Nim id 1, related device is m3501_1 */
    NIM_ID_M3503_0,      /**< Nim id 2, related device is m3503_0 */
    NIM_ID_M3503_1,      /**< Nim id 3, related device is m3503_1 */
    NIM_ID_M3200_0,      /**< Nim id 4, related device is m3200_0 */
    NIM_ID_M3200_1,      /**< Nim id 5, related device is m3200_1 */
    NIM_ID_M3281_0,      /**< Nim id 6, related device is m3281_0 */
    NIM_ID_M3281_1,      /**< Nim id 7, related device is m3281_1 */
    NIM_ID_10025_0,      /**< Nim id 8, related device is tunner tda10025 */
    NIM_ID_10025_1,      /**< Nim id 9, related device is tunner tda10025 */
    NIM_ID_S3821_0,
    NIM_ID_HERCULES_0,
    NIM_ID_HERCULES_1,
    NIM_ID_HERCULES_2,
    NIM_ID_HERCULES_3,
    NIM_ID_HD2818_MXL608_0,
    NIM_ID_M3501_2,
    NIM_ID_M3501_3,
    NIM_ID_MXL214C_0,
    NIM_ID_MXL214C_1,
    NIM_ID_MXL214C_2,
    NIM_ID_MXL214C_3,
    NIM_ID_C3505_0,
    NIM_ID_C3505_1,
    NIM_ID_CXD2837_0,
    NIM_ID_C3501H_0,
	NIM_ID_CXD2856_0,
    NIM_NB               /**< Number of nim */
} nim_id_t;

typedef enum lnb_type_e {
    SLNIM_LNB_NONE = 0,
    SLNIM_LNB_A8304,
	SLNIM_LNB_TPS65233,
}lnb_type_e;

enum nim_io_command {
    NIM_IOCMD_READ_RSUB,               /** <Read Reed Solomon Uncorrected block */
    NIM_IOCMD_STOP_AUTOSCAN,           /** <Stop autoscan */
    NIM_IOCMD_SET_RESET_CALLBACK,      /** <When nim device need to be reset, call an callback to notice app */
    NIM_IOCMD_STOP_CHANSCAN,           /** <Stop channel change because some low symbol rate TP too long to be locked */
    NIM_IOCMD_GET_REC_PERFORMANCE_INFO,/** <get receiver performance info. */
    NIM_IOCMD_CHANGE_TS_GAP,           /** <Change ts gap */
    NIM_IOCMD_SET_SSI_CLK,             /** <Set SSI Clock */
    NIM_IOCMD_SET_POLAR ,             /** <DVB-S NIM Device set LNB polarization */
    NIM_IOCMD_TUNER_POWER_CONTROL,    /** <Tunner set standby */
    NIM_IOCMD_SET_BLSCAN_MODE,        /** <Para = 0, NIM_SCAN_FAST; Para = 1, NIM_SCAN_ACCURATE(slower) */
    NIM_IOCMD_AUTO_SCAN,              /** <Do AutoScan Procedure */
    NIM_IOCMD_CHANNEL_CHANGE,         /** <Do Channel Change */
    NIM_IOCMD_CHANNEL_SEARCH,         /** <Do Channel Search */
    NIM_IOCMD_QUICK_CHANNEL_CHANGE,   /** <Do Quick Channel Change without waiting lock */
    NIM_IOCMD_GET_ID,                 /** <Get Nim ID; 3501 type: M3501A/M3501B */
    NIM_IOCMD_DISEQC_OPERATION,       /** <NIM DiSEqC Device Opearation */
    NIM_IOCMD_DISEQC2X_OPERATION,     /** <NIM DiSEqC2X Device Opearation */
    NIM_IOCMD_AS_SYNC_WITH_LIB,       /** <Nim sync with share lib for AUTOSCAN*/
    NIM_IOCMD_GET_LOCK_STATUS,        /** <Get lock status */
	NIM_IOCMD_REG_RW,		  /** read write nim register  */
    NIM_IOCMD_NUM,
    NIM_IOCMD_SYM_LIMIT_RANGE,
    NIM_IOCMD_SET_LNB_POWER_ONOFF,

    /* below IOCTL are not directly mapped on kernel IOCTL */
    NIM_IOCMD_GET_INFO,                /** <Get channel structure info */
    NIM_IOCMD_T2_SIGNAL_ONLY          /** <Set flag locking T2 signal only */
};

typedef enum {
	ALISL_NIM_QPSK,  /**< DVB-S modulation type */
	ALISL_NIM_QAM,   /**< DVB-C modulation type */
	ALISL_NIM_OFDM   /**< DVB-T modulation type */
} sldemod_type;

enum slnim_sample_clk {
    NIM_DEMO_SAMPLE_CLK_27M = 0x00,
    NIM_DEMO_SAMPLE_CLK_54M = 0x10,
};

enum slnim_dvbc_mode {
    NIM_DVBC_MODE_J83AC = 0x00,
    NIM_DVBC_MODE_J83B = 0x01,
};

enum slnim_disec_mode {
    SLNIM_DISEQC_MODE_22KOFF = 0,       /* 22kHz off */
    SLNIM_DISEQC_MODE_22KON,            /* 22kHz on */
    SLNIM_DISEQC_MODE_BURST0,           /* Burst mode, on for 12.5mS = 0 */
    SLNIM_DISEQC_MODE_BURST1,           /* Burst mode, modulated 1:2 for 12.5mS = 1 */
    SLNIM_DISEQC_MODE_BYTES,            /* Modulated with bytes from DISEQC INSTR */
    SLNIM_DISEQC_MODE_ENVELOP_ON,       /* Envelop enable*/
    SLNIM_DISEQC_MODE_ENVELOP_OFF,      /* Envelop disable, out put 22K wave form*/
    SLNIM_DISEQC_MODE_OTHERS,           /* Undefined mode */
    SLNIM_DISEQC_MODE_BYTES_EXT_STEP1,  /*Split NIM_DISEQC_MODE_BYTES to 2 steps to improve the speed,*/
    SLNIM_DISEQC_MODE_BYTES_EXT_STEP2,  /*(30ms--->17ms) to fit some SPEC */
};


/* DVB-C demodulator qam type parameter definition */
enum slnim_qam_mode {
	SLNIM_QAM_AUTO = 0,
	SLNIM_QAM16 = 4,
	SLNIM_QAM32,
	SLNIM_QAM64,
	SLNIM_QAM128,
	SLNIM_QAM256
};

enum slnim_guard_inter {
	SLNIM_GUARD_INTER_AUTO,
	SLNIM_GUARD_INTER_1_4,
	SLNIM_GUARD_INTER_1_8,
	SLNIM_GUARD_INTER_1_16,
	SLNIM_GUARD_INTER_1_32,
};

enum slnim_fft_mode {
	SLNIM_FFT_MODE_AUTO,
	SLNIM_FFT_MODE_2K,
	SLNIM_FFT_MODE_4K,
	SLNIM_FFT_MODE_8K
};

enum slnim_fec{
	SLNIM_FEC_AUTO,
	SLNIM_FEC_1_2,
	SLNIM_FEC_2_3,
	SLNIM_FEC_3_4,
	SLNIM_FEC_5_6,
	SLNIM_FEC_7_8
};

enum slnim_modulation {
	SLNIM_MODUL_AUTO,
	SLNIM_MODUL_DQPSK,
	SLNIM_MODUL_QPSK,
	SLNIM_MODUL_16QAM,
	SLNIM_MODUL_64QAM,
};

enum slnim_spectrum {
	SLNIM_SPEC_AUTO,
	SLNIM_SPEC_NORM,
	SLNIM_SPEC_INVERT
};

/* Enum matching ISDBT_TYPE, DVBT_TYPE, DVBT2_TYPE and DVBT2_COMBO */
enum slnim_standard {
	SLNIM_STD_ISDBT,        /**< ISDB-T standard */
	SLNIM_STD_DVBT,         /**< DVB-T standard */
	SLNIM_STD_DVBT2,        /**< DVB-T2 standard */
	SLNIM_STD_DVBT2_COMBO,  /**< automatic detection between DVBT & DVBT2 */
	SLNIM_STD_DVBS,
	SLNIM_STD_DVBS2,
	SLNIM_STD_DVBC_ANNEX_AC,
	SLNIM_STD_DVBC_ANNEX_B,
	SLNIM_STD_OTHER
};

// Must match aui_nim_bandwidth
enum slnim_bandwidth {
	SLNIM_BANDWIDTH_0_MHZ,
	SLNIM_BANDWIDTH_6_MHZ,
	SLNIM_BANDWIDTH_7_MHZ,
	SLNIM_BANDWIDTH_8_MHZ,
};

#define SLNIM_PLP_AUTO (-1)
#define SLNIM_PLP_NEXT (-2)

typedef struct slnim_gpio_info {
    int gpio_val;
    int io;
    int position;
} slnim_gpio_info;

/** Structure for frequency lock parameters */
typedef struct slnim_freq_lock {
    uint32_t freq;            /** <Channel Center Frequency: in MHz unit */
    uint32_t sym;             /** <Channel Symbol Rate: in KHz unit */
    uint32_t bandwidth;       /** <Channel Symbol Rate: same as Channel Symbol Rate for DVB-T */
    uint8_t fec;              /** <Channel FEC rate */
    uint8_t guard_interval;   /** <Guard Interval -- for DVB-T */
    uint8_t standard;         /** <-- for DVB-T2 and DVBT2 combo */
    uint8_t fft_mode;         /** <-- for DVB-T */
    uint8_t modulation;       /** <-- for DVB-T */
    uint8_t usage_type;       /** <-- for DVB-T */
    uint8_t inverse;          /** <-- for DVB-T */
    int32_t plp_index;        /** <-- for DVB-T2 */
    int32_t plp_id;           /** <-- for DVB-T2 */
} slnim_freq_lock_t;

struct slnim_signal_dvbt_status {
	enum slnim_fec fec;
	enum slnim_fft_mode fft_mode;
	enum slnim_modulation modulation;
	enum slnim_guard_inter guard_inter;
	enum slnim_spectrum spectrum;
};

struct slnim_signal_dvbt2_status {
	enum slnim_fec fec;                 /**< FEC */
	enum slnim_fft_mode fft_mode;       /**< FFT mode */
	enum slnim_modulation modulation;   /**< Modulation, constellation */
	enum slnim_guard_inter guard_inter; /**< guard interval */
	enum slnim_spectrum spectrum;       /**< spectrum */

	int plp_num;                 /**< number of data PLP in the channel */
	int plp_id;                  /**< Current data PLP ID */
	int plp_index;               /**< Current data PLP index */
	int system_id;               /**< System ID */
	int network_id;              /**< network ID */
	int trans_stream_id;         /**< transport stream ID */
};

struct slnim_signal_dvbs_status {
	unsigned long ul_polar_status;		/* 1: polar_H	0:polar_V */
};

struct slnim_signal_info {
	enum slnim_standard std; /* DVBT_ISDBT, DVBT2 or OTHERS */
	union {
		/**< DVB-T and ISDB-T info */
		struct slnim_signal_dvbt_status dvbt_isdbt;
		/**< DVB-T2 info */
		struct slnim_signal_dvbt2_status dvbt2;
		/* DVB-S info  */
		struct slnim_signal_dvbs_status dvbs;
	} u;
};

// Cause: The AUI struct aui_signal_status must keep the same as
// struct slnim_signal_status, include all the member.
typedef struct slnim_signal_status {
	unsigned long ul_freq;              /**< frequency in MHz */
	unsigned long ul_signal_strength;   /**< signal strength */
	unsigned long ul_signal_quality;    /**< signal quality */
	unsigned long ul_bit_error_rate;    /**< BER */
	unsigned long ul_rf_level;          /**< RF signal level */
	unsigned long ul_signal_cn;         /**< Carrier to noise ratio */
	unsigned long ul_mer;         /**< Modulation error ratio */
    unsigned long ul_per;         /**< signal per*/
    unsigned long ul_pre_ber;         /**< BER befor error correction */

	struct slnim_signal_info info;
} slnim_signal_status;

typedef struct {
    uint32_t i2c_base_addr;
    uint32_t i2c_type_id;
    /**bit0:QPSK_FREQ_OFFSET,\n
      bit1:EXT_ADC,\n
      bit2:IQ_AD_SWAP,\n
      bit3:I2C_THROUGH,\n
      bit4:polar revert,\n
      bit5:NEW_AGC1,\n
      bit6bit7:QPSK bitmode: \n
      00:1bit,01:2bit,10:4bit,11:8bit*/
    uint16_t  QPSK_Config;
} sl_demo_config_dvbs;

typedef struct {
    uint32_t i2c_base_addr;
    uint32_t i2c_type_id;
    /**bit0:QPSK_FREQ_OFFSET,\n
      bit1:EXT_ADC,\n
      bit2:IQ_AD_SWAP,\n
      bit3:I2C_THROUGH,\n
      bit4:polar revert,\n
      bit5:NEW_AGC1,\n
      bit6bit7:QPSK bitmode: \n
      00:1bit,01:2bit,10:4bit,11:8bit*/
    uint16_t ofdm_config;

} sl_demo_config_dvbt;

typedef struct {
    uint32_t i2c_base_addr;
    uint32_t i2c_type_id;
    unsigned int qam_mode;
} sl_demo_config_dvbc;

typedef struct {
    uint16_t       freq_low;
    uint16_t       freq_high;
    uint32_t i2c_base_addr;
    uint32_t i2c_type_id;
    tuner_id_e tuner_id;                       /**< tunner type for defferent tuner >*/
    slnim_gpio_info diseqc_polar_gpio;
} sl_tunner_config_dvbs;

typedef struct {
    uint16_t       freq_low;
    uint16_t       freq_high;
    uint32_t i2c_base_addr;
    uint32_t i2c_type_id;
    tuner_id_e tuner_id;

    //uint8_t rf_agc_max;
    //uint8_t rf_age_min;
    uint8_t if_agc_max;
    uint8_t if_agc_min;
    uint8_t agc_ref; /**<the average amplitude to full scale of A/D. % percentage rate.*/
    uint8_t tuner_tsi_setting;
    uint8_t  tuner_crystal;
    uint8_t  chip;
    uint8_t  tuner_special_config;
    //uint8_t  tuner_ref_divratio;
    uint16_t wtuner_if_freq;
    uint8_t  tuner_agc_top;
    //uint8_t  tuner_step_freq;
    //uint8_t  tunner_freq_param;/**<RT810_Standard_Type >*/
    //uint16_t tuner_reopen;

    int standard; /* enum slnim_standard  */
} sl_tunner_config_dvbt;

typedef struct {
    uint32_t i2c_base_addr;
    uint32_t i2c_type_id;
    tuner_id_e tuner_id;
    /**x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.*/
    uint8_t rf_agc_max;
    uint8_t rf_agc_min;
    uint8_t if_agc_max;
    uint8_t if_agc_min;
    uint8_t agc_ref;/**<the average amplitude to full scale of A/D. % percentage rate.*/
    uint8_t  tuner_crystal;
    uint8_t  chip;
    uint8_t  tuner_special_config;/**<0x01, RF AGC is disabled*/
    uint8_t  tuner_ref_divratio;
    uint16_t wtuner_if_freq;
    uint8_t  tuner_agc_top;
    uint8_t  tuner_step_freq;
    uint8_t  tunner_freq_param;/**<RT810_Standard_Type >*/
    uint16_t tuner_reopen;
    uint16_t tuner_if_freq_J83A;
    uint16_t tuner_if_freq_J83B;
    uint16_t tuner_if_freq_J83C;
    uint8_t  tuner_if_J83AC_type; /**<0x00 j83a , 0x01 j83c>*/
} sl_tunner_config_dvbc;

typedef struct {
    uint32_t lnb_i2c_base_addr;
    uint32_t lnb_i2c_type_id;
    lnb_type_e lnb_name;
} sl_lnb_config_dvbs;

typedef struct {
    sl_demo_config_dvbs demo_config;
    sl_tunner_config_dvbs tunner_config;
    sl_lnb_config_dvbs lnb_config;
} sl_nim_config_dvbs;

typedef struct {
    sl_demo_config_dvbt demo_config;
    sl_tunner_config_dvbt tunner_config;
} sl_nim_config_dvbt;


typedef struct {
    sl_demo_config_dvbc demo_config;
    sl_tunner_config_dvbc tunner_config;
} sl_nim_config_dvbc;

typedef union {
    sl_nim_config_dvbs config_dvbs;
    sl_nim_config_dvbt config_dvbt;
    sl_nim_config_dvbc config_dvbc;
} slnim_config;

/** Structure for Auto Scan parameters*/
typedef struct slnim_auto_scan {
    uint8_t unicable;
    uint16_t Fub;     /** <Unicable: UB slots centre freq (MHz)*/
    uint32_t sfreq;   /** <Start Frequency of the Scan procedure: in MHz unit.*/
    uint32_t efreq;   /** <End Frequency of the Scan procedure: in MHz unit.*/
    /* draw Callback Function pointer */
    int32_t (*callback)(uint8_t status, uint8_t polar, uint32_t freq, uint32_t sym, uint8_t fec, void *usr_data, uint32_t stream_id);
    void *usr_data;
} slnim_auto_scan_t;

typedef struct slnim_diseqc_operate {
    enum slnim_disec_mode mode;
    uint8_t *cmd;
    uint8_t *rt_value;
    uint8_t rt_cnt;
    uint8_t cnt;
} slnim_diseqc_operate_t;

/**
 *  Function Name:        alislnim_open
 *  @brief
 *
 *  @param  handle        pointer to struct nim_device
 *  @param  id            enum nim_id
 *  tunner_config_handle  pointer to struct nim_tunner_config
 *  @return               alisl_retcode
 *
 *  @author               Franky.Liang  <franky.liang@alitech.com>
 *  @date                 7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_open(alisl_handle *handle, enum nim_id id);

/**
 *  Function Name:     alislnim_set_cfg
 *  @brief
 *
 *  @param handle      pointer to struct nim_tunner_config
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_set_cfg(alisl_handle handle, slnim_config *nim_config);

/**
 *  Function Name:      alislnim_ioctl
 *  @brief
 *
 *  @param handle       pointer to struct nim_device
 *  @param cmd          cmd type
 *  @param param        pointer to Parameter List
 *  @return             result for ioctl commd
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

uint32_t alislnim_ioctl(alisl_handle handle, unsigned int cmd, uint32_t param);

/**
 *  Function Name:     alislnim_autoscan
 *  @brief
 *
 *  @param handle      point to struct nim_device
 *  @param param       point to struct slnim_auto_scan_t
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_autoscan(alisl_handle handle, slnim_auto_scan_t* param);

/**
 *  Function Name:     alislnim_autoscan_active
 *  @brief             return 1 if autoscan is running
 *
 *  @param handle      point to struct nim_device
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */
alisl_retcode alislnim_autoscan_active(alisl_handle handle);

/**
 *  Function Name:     alislnim_autoscan_stop
 *  @brief
 *
 *  @param handle      point to struct nim_device
 *  @param param       point to struct slnim_auto_scan_t
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */
#define alislnim_autoscan_stop(hdl) alislnim_ioctl(hdl, NIM_IOCMD_STOP_AUTOSCAN, 1)

/**
 *  Function Name:     alislnim_freqlock
 *  @brief
 *
 *  @param handle      point to struct nim_device
 *  @param param       slnim_channel_change_t
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_freqlock(alisl_handle handle, slnim_freq_lock_t* param);

/**
 *  Function Name:      alislnim_diseqc_operate
 *  @brief
 *
 *  @param handle       point to struct nim_device
 *  @param param        point to struct slnim_diseqc_operate_t
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_diseqc_operate(alisl_handle handle, slnim_diseqc_operate_t* param);

/**
 *  Function Name:      alislnim_diseqc2x_operate
 *  @brief
 *
 *  @param handle       point to struct nim_device
 *  @param param        point to struct slnim_diseqc_operate_t
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_diseqc2x_operate(alisl_handle handle, slnim_diseqc_operate_t* param);


/**
 *  Function Name:      alislnim_close
 *  @brief
 *
 *  @param handle       point to struct nim_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_close(alisl_handle handle);

/**
 *  Function Name:     alislnim_get_lock
 *  @brief
 *
 *  @param handle       point to struct nim_device
 *  @param value        point to result lock
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */
#define alislnim_get_lock(hdl, value) alislnim_ioctl(hdl, NIM_IOCMD_GET_LOCK_STATUS, (unsigned int)value)


/**
 *  Function Name:      alislnim_set_polar
 *  @brief
 *
 *  @param handle       point to struct nim_device
 *  @param value        polar: NIM_PORLAR_HORIZONTAL or NIM_PORLAR_VERTICAL
 *
 *  @return             alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */
#define alislnim_set_polar(hdl, value) alislnim_ioctl(hdl, NIM_IOCMD_SET_POLAR, (unsigned int)value)

#ifdef __cplusplus
}
#endif

#endif /* __ALISLNIM__H_ */
