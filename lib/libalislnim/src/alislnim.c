
/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislnim.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 17:06:37
 *  @revision           none
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 */

/* system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <bits_op.h>
#include <flag_op.h>

//#include <process.h>
#include <sys/socket.h>
#include <linux/netlink.h>


/* kernel headers */

#include <linux/ioctl.h>
#include <dvb_frontend_common.h>
//#include <ali_netlink_common.h>

#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislnim.h>
#include <alipltfintf.h>

/* local headers */
#include "internel.h"

// t2_signal [I/O]
// To special terrestrial type.
// To get the terrestrial type after locked in auto scan mode.
// 0: To lock ISDB-T, using USAGE_TYPE_CHANCHG.
// 1: To lock T only, using USAGE_TYPE_CHANCHG. Had locked T signal.
// 2: To lock T2 only, using USAGE_TYPE_CHANCHG. Had locked T2 signal.
// 3: To lock T/T2, auto scan mode, using USAGE_TYPE_AUTOSCAN.
#define T2_SIGNAL_ISDB_T (0)
#define T2_SIGNAL_T (1)
#define T2_SIGNAL_T2 (2)
#define T2_SIGNAL_COMBO (3)


typedef struct dev_nim {
    enum nim_id     id;
    int             type;
    const char      *path;
    const char      *pathfeed;      /**< when playback, we need to feed data\n
                                         to device specified by pathfeed */
    unsigned int open_cnt;
    struct nim_device *dev;
} dev_nim_t;

/********temp struct,it will be come true in driver***********/
struct ali_nim_hercules_cfg
{
   __u32 i2c_base_addr;
   __u32 i2c_type_id;
   __u8 qam_mode;
   __u32 demod_id;
   __u32 reset_pio;
};

struct ali_nim_hd2818_mxl608_cfg
{
	__u32 tuner_i2c_addr;   //!<The address of I2C used by tuner
   __u32 tuner_i2c_id;     //!<The identifier of I2C used by tuner
 	__u32 demod_i2c_addr;    //!<The address of I2C used by demodulator
	__u32 demod_i2c_id;      //!<The identifier of I2C used by demodulator
	__u32 tuner_id;         //!<The identifier of tuner
	__u16 recv_freq_low;      //!<Low value of Tuner Receiving Frequency range
	__u16 recv_freq_high;     //!<High value of Tuner Receiving Frequency range
};

/**************************************************/

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static struct dev_nim dev_nim[] = {
    {NIM_ID_M3501_0, ALISL_NIM_QPSK, "/dev/ali_m3501_nim0",    NULL, 0, NULL},
    {NIM_ID_M3501_1, ALISL_NIM_QPSK, "/dev/ali_m3501_nim1",    NULL, 0, NULL},
    {NIM_ID_M3503_0, ALISL_NIM_QPSK, "/dev/ali_m3503_nim0",    NULL, 0, NULL},
    {NIM_ID_M3503_1, ALISL_NIM_QPSK, "/dev/ali_m3503_nim1",    NULL, 0, NULL},

    {NIM_ID_M3200_0, ALISL_NIM_QAM, "/dev/ali_m3200_nim0",    NULL, 0, NULL},
    {NIM_ID_M3200_1, ALISL_NIM_QAM, "/dev/ali_m3200_nim1",    NULL, 0, NULL},
    {NIM_ID_M3281_0, ALISL_NIM_QAM, "/dev/ali_m3281_nim0",    NULL, 0, NULL},
    {NIM_ID_M3281_1, ALISL_NIM_QAM, "/dev/ali_m3281_nim1",    NULL, 0, NULL},
    {NIM_ID_10025_0, ALISL_NIM_QAM, "/dev/ali_tda10025_nim0", NULL, 0, NULL},
    {NIM_ID_10025_1, ALISL_NIM_QAM, "/dev/ali_tda10025_nim1", NULL, 0, NULL},

    {NIM_ID_S3821_0, ALISL_NIM_OFDM, "/dev/ali_s3821_nim0",    NULL, 0, NULL}, // ISDBT

    {NIM_ID_HERCULES_0, ALISL_NIM_QAM, "/dev/ali_hercules_nim0", NULL, 0, NULL},
    {NIM_ID_HERCULES_1, ALISL_NIM_QAM, "/dev/ali_hercules_nim1", NULL, 0, NULL},
    {NIM_ID_HERCULES_2, ALISL_NIM_QAM, "/dev/ali_hercules_nim2", NULL, 0, NULL},
    {NIM_ID_HERCULES_3, ALISL_NIM_QAM, "/dev/ali_hercules_nim3", NULL, 0, NULL},

    {NIM_ID_HD2818_MXL608_0, ALISL_NIM_OFDM, "/dev/ali_hd2818_mxl608_nim0", NULL, 0, NULL},

    {NIM_ID_M3501_2, ALISL_NIM_QPSK, "/dev/ali_m3501_nim2",    NULL, 0, NULL},
    {NIM_ID_M3501_3, ALISL_NIM_QPSK, "/dev/ali_m3501_nim3",    NULL, 0, NULL},
	{NIM_ID_MXL214C_0, ALISL_NIM_QAM, "/dev/ali_mxl214c_nim0",	  NULL, 0, NULL},
	{NIM_ID_MXL214C_1, ALISL_NIM_QAM, "/dev/ali_mxl214c_nim1",	  NULL, 0, NULL},
	{NIM_ID_MXL214C_2, ALISL_NIM_QAM, "/dev/ali_mxl214c_nim2",	  NULL, 0, NULL},
	{NIM_ID_MXL214C_3, ALISL_NIM_QAM, "/dev/ali_mxl214c_nim3",	  NULL, 0, NULL},
	{NIM_ID_C3505_0, ALISL_NIM_QPSK, "/dev/ali_c3505_nim0",    NULL, 0, NULL},
	{NIM_ID_C3505_1, ALISL_NIM_QPSK, "/dev/ali_c3505_nim1",    NULL, 0, NULL},
	{NIM_ID_CXD2837_0, ALISL_NIM_OFDM, "/dev/ali_cxd2837_nim0",    NULL, 0, NULL},//SONY DVBT2
	{NIM_ID_C3501H_0, ALISL_NIM_QPSK, "/dev/ali_nim_c3501h_0",    NULL, 0, NULL},
	{NIM_ID_CXD2856_0, ALISL_NIM_OFDM, "/dev/ali_cxd2856_nim0",    NULL, 0, NULL} //SONY DVBT2
};

//static char netlink_msg_buffer_array[ARRAY_SIZE(dev_nim)][NLMSG_SPACE(MAX_NETLINK_MSG_SIZE)];
static pthread_mutex_t m_mutex      = PTHREAD_MUTEX_INITIALIZER;

static struct ali_nim_m3501_cfg   nim_m3501_cfg;
static struct ali_nim_m3200_cfg   nim_m3200_cfg;
static struct ali_nim_mn88436_cfg nim_m8843_cfg;
static struct ali_nim_hercules_cfg nim_hercules_cfg;
static struct ali_nim_hd2818_mxl608_cfg nim_hd2818_mxl608_cfg;

static void * ali_nim_cfg_ptr = NULL;

#define MAX_KUMSG_SIZE 1024

static unsigned int sl_disecmode_to_driver_mode(enum slnim_disec_mode mode)
{
	unsigned int ret_mode = 0;
	switch(mode) {
		case SLNIM_DISEQC_MODE_22KOFF:
			ret_mode = NIM_DISEQC_MODE_22KOFF;
			break;
		case SLNIM_DISEQC_MODE_22KON:
			ret_mode = NIM_DISEQC_MODE_22KON;
			break;
		case SLNIM_DISEQC_MODE_BURST0:
			ret_mode = NIM_DISEQC_MODE_BURST0;
			break;
		case SLNIM_DISEQC_MODE_BURST1:
			ret_mode = NIM_DISEQC_MODE_BURST1;
			break;
		case SLNIM_DISEQC_MODE_BYTES:
			ret_mode = NIM_DISEQC_MODE_BYTES;
			break;
		case SLNIM_DISEQC_MODE_ENVELOP_ON:
			ret_mode = NIM_DISEQC_MODE_ENVELOP_ON;
			break;
		case SLNIM_DISEQC_MODE_ENVELOP_OFF:
			ret_mode = NIM_DISEQC_MODE_ENVELOP_OFF;
			break;
		case SLNIM_DISEQC_MODE_OTHERS:
			ret_mode = NIM_DISEQC_MODE_OTHERS;
			break;
		case SLNIM_DISEQC_MODE_BYTES_EXT_STEP1:
			ret_mode = NIM_DISEQC_MODE_BYTES_EXT_STEP1;
			break;
		case SLNIM_DISEQC_MODE_BYTES_EXT_STEP2:
			ret_mode = NIM_DISEQC_MODE_BYTES_EXT_STEP2;
			break;
		default:
			break;
	}

	return ret_mode;
}

/**
 *  Function Name:     alislnim_construct
 *  @brief
 *
 *  @param handle      pointer to struct nim_device
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

static alisl_retcode alislnim_construct(alisl_handle *handle)
{
    struct nim_device *dev = malloc(sizeof(*dev));
    if (dev == NULL) {
        SL_DBG("malloc memory failed!\n");
        return ERROR_NOMEM;
    }

    memset(dev, 0, sizeof(*dev));

    flag_init(&dev->status, NIM_STATUS_CONSTRUCT);
//    dev->portid = -1;
//    dev->nl.sockfd = -1;
    dev->kumsgfd = -1;
    *handle = dev;

    return 0;

}

/**
 *  Function Name:     alislnim_destruct
 *  @brief
 *
 *  @param  handle     pointer to struct nim_device
 *  @param
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

static alisl_retcode alislnim_destruct(alisl_handle *handle)
{
    struct nim_device *dev = (struct nim_device *)(*handle);

    if (dev != NULL) {
        free(dev);
        *handle = NULL;
    } else {
        SL_DBG("nim private is NULL before destruct");
        return 0;
    }

    return 0;
}


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

alisl_retcode alislnim_open(alisl_handle *handle, enum nim_id id)
{
    struct nim_device *dev = NULL;
    uint32_t i;

    for (i = 0; i < ARRAY_SIZE(dev_nim); i++) {
        if (id == dev_nim[i].id) {
            break;
        }
    }

    if (i == ARRAY_SIZE(dev_nim)) {
        SL_DBG("Invalidate nim id!\n");
        return ERROR_INVAL;
    }
    pthread_mutex_lock(&m_mutex);
    if (dev_nim[i].open_cnt == 0) {

        alislnim_construct((alisl_handle *)&dev_nim[i].dev);

        dev = dev_nim[i].dev;
        dev->id = id;
        dev->path = dev_nim[i].path;
        dev->pathfeed = dev_nim[i].pathfeed;

        dev->fd = open(dev->path, O_RDWR);
        if (dev->fd < 0) {
            SL_DBG("%s open failed!\n", dev->path);
            goto ERROR;
        }

        if (dev->pathfeed != NULL) {
            dev->feedfd = open(dev->pathfeed, O_RDWR);
            if (dev->feedfd < 0) {
                close(dev->fd);
                SL_DBG("%s open failed\n", dev->pathfeed);
                goto ERROR;
            }
        }

        int flags = O_CLOEXEC;
        if ((dev->kumsgfd = ioctl(dev->fd, ALI_NIM_GET_KUMSGQ, &flags)) < 0) {
            SL_DBG("ALI_NIM_GET_KUMSGQ fail\n");
            // Some demodulation do not support KUMSG.
            // Do not return fail here. It's no need to return fail.
            //return ERROR_STATUS;
        }

        flag_bit_set(&dev->status, NIM_STATUS_OPEN);

        /*   todo socket communication*/
        //alislnim_set_portid(dev, netlink_init(&(dev->nl), dev->id));

        flag_bit_clear(&dev->status, NIM_STATUS_CLOSE);
        flag_bit_set(&dev->status, NIM_STATUS_START);
    }

    dev_nim[i].open_cnt++;
    *handle = dev_nim[i].dev;
    pthread_mutex_unlock(&m_mutex);
    return 0;

ERROR:
    pthread_mutex_unlock(&m_mutex);
    return  ERROR_INVAL;
}

static uint32_t get_tuner_id(tuner_id_e id) {
	uint32_t tunerId = -1;
	switch (id) {
	
#ifdef TUN_R836
	case NIM_TUNER_R836:		
		tunerId = TUN_R836;
		//printf("in platform get_tuner_id id = %d, tunerId = 0x%x\n",id,tunerId);//orange
	break;
#endif

#ifdef SHARP_VZ7306
	case NIM_TUNNER_SHARP_VZ7306:
		tunerId = SHARP_VZ7306;
		break;
#endif

#ifdef AV_2012
	case NIM_TUNNER_AV_2012:
		tunerId = AV_2012;
		break;
#endif

#ifdef DCT70701
	case NIM_TUNNER_DCT70701:
		tunerId = DCT70701;
		break;
#endif

#ifdef DCT7044
	case NIM_TUNNER_DCT7044:
		tunerId = DCT7044;
		break;
#endif

#ifdef ALPSTDQE
	case NIM_TUNNER_ALPSTDQE:
		tunerId = ALPSTDQE;
		break;
#endif

#ifdef ALPSTDAE
	case NIM_TUNNER_ALPSTDAE:
		tunerId = ALPSTDAE;
		break;
#endif

#ifdef TDCCG0X1F
	case NIM_TUNNER_TDCCG0X1F:
		tunerId = TDCCG0X1F;
		break;
#endif

#ifdef DBCTE702F1
	case NIM_TUNNER_DBCTE702F1:
		tunerId = DBCTE702F1;
		break;
#endif

#ifdef CD1616LF
	case NIM_TUNNER_CD1616LF:
		tunerId = CD1616LF;
		break;
#endif

#ifdef ALPSTDAC
	case NIM_TUNNER_ALPSTDAC:
		tunerId = ALPSTDAC;
		break;
#endif

#ifdef RT810
	case NIM_TUNNER_RT810:
		tunerId = RT810;
		break;
#endif

#ifdef RT820
	case NIM_TUNNER_VRT820C:
		tunerId = RT820;
		break;
#endif

#ifdef MXL603
	case NIM_TUNNER_MXL603: /* DVB-T */
		tunerId = MXL603;
		break;
#endif

#ifdef TDA18250
	case NIM_TUNNER_TDA18250:
		tunerId = TDA18250;
		break;
#endif

#ifdef RDA5815M
	case NIM_TUNER_RDA5815M:
		tunerId = RDA5815M;
		break;
#endif

#ifdef MXL214C
	case NIM_TUNER_MXL214C:
		tunerId = MXL214C;
		break;
#endif

#ifdef M3031
	case NIM_TUNER_M3031:
		tunerId = M3031;
		break;
#endif

#ifdef SI2141
	case NIM_TUNER_SI2141:
		tunerId = SI2141;
		break;
#endif

#ifdef CXD2872
	case NIM_TUNER_CXD2872:
		tunerId = CXD2872;
		break;
#endif

#ifdef R858
	case NIM_TUNER_R858:
		tunerId = R858;
		break;
#endif

#ifdef TDA18250AB
	case NIM_TUNNER_TDA18250AB:
		tunerId = TDA18250AB;
		break;
#endif

	default:
	    SL_ERR("invalid tuner ID %d\n", id);
		break;
	}
	return tunerId;
}

static uint32_t get_lnb_id(lnb_type_e id) {
	uint32_t LNB_Id = -1;
	switch (id) {

    case SLNIM_LNB_NONE:
        LNB_Id = 0;
	    break;
    
	case SLNIM_LNB_A8304:		
		LNB_Id = LNB_A8304;
	    break;

    case SLNIM_LNB_TPS65233:		
		LNB_Id = LNB_TPS65233;
	    break;

	default:
	    SL_ERR("invalid LNB ID %d\n", id);
		break;
	}
	return LNB_Id;
}

/**
 *  Function Name:     alislnim_set_cfg
 *  @brief
 *
 *  @param handle      pointer to struct nim_tunner_config
 *  @param id          enum nim_id
 *
 *  @return            alisl_retcode
 *
 *  @author             Franky.Liang  <franky.liang@alitech.com>
 *  @date               7/19/2013, Created
 *
 *  @note
 */

alisl_retcode alislnim_set_cfg(alisl_handle handle, slnim_config *nim_config)
{
	struct nim_device *dev = (struct nim_device *)handle;
	unsigned long cmd;
	uint32_t tuner_id;
	uint32_t lnb_id;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, NIM_STATUS_START)) {
		SL_DBG("Nim have not start\n");
		return ERROR_STATUS;
	}

	dev->type = dev_nim[dev->id].type;

	switch (dev->type) {
	case ALISL_NIM_QPSK: {
		sl_nim_config_dvbs *p_conf = &nim_config->config_dvbs;

		SL_DBG("%s tuner_id %d\n", __func__, p_conf->tunner_config.tuner_id);

		tuner_id = get_tuner_id(p_conf->tunner_config.tuner_id);
		lnb_id = get_lnb_id(p_conf->lnb_config.lnb_name);
		memset(&nim_m3501_cfg, 0, sizeof(struct ali_nim_m3501_cfg));
		nim_m3501_cfg.recv_freq_high = p_conf->tunner_config.freq_high;
		nim_m3501_cfg.recv_freq_low  = p_conf->tunner_config.freq_low;
		nim_m3501_cfg.tuner_i2c_id   = p_conf->tunner_config.i2c_type_id;
		nim_m3501_cfg.tuner_i2c_addr = p_conf->tunner_config.i2c_base_addr;
		nim_m3501_cfg.tuner_id       = tuner_id;
		if ((0 == p_conf->tunner_config.diseqc_polar_gpio.gpio_val) 
			&& (0 == p_conf->tunner_config.diseqc_polar_gpio.io)) {
			nim_m3501_cfg.disqec_polar_position = -1;
		} else {
			nim_m3501_cfg.disqec_polar_position = p_conf->tunner_config.diseqc_polar_gpio.position;
		}

		nim_m3501_cfg.qpsk_config    = p_conf->demo_config.QPSK_Config;
		nim_m3501_cfg.demod_i2c_id   = p_conf->demo_config.i2c_type_id;
		nim_m3501_cfg.demod_i2c_addr = p_conf->demo_config.i2c_base_addr;

		nim_m3501_cfg.lnb_i2c_addr = p_conf->lnb_config.lnb_i2c_base_addr;
		nim_m3501_cfg.lnb_i2c_id = p_conf->lnb_config.lnb_i2c_type_id;
		nim_m3501_cfg.lnb_name = lnb_id;
        
		ali_nim_cfg_ptr = &nim_m3501_cfg;
		cmd = ALI_NIM_HARDWARE_INIT_S;

//		SL_DBG("\n---   ali_nim_m3501_cfg cmd: %lu   ---\n "
//				"recv_freq_low: %u\n recv_freq_high: %u\n qpsk_config: 0x%x\n "
//				"reserved: %u\n tuner_i2c_id: 0x%x\n tuner_i2c_addr: 0x%x\n "
//				"demod_i2c_id: 0x%x\n demod_i2c_addr: 0x%x\n tuner_id: 0x%x\n"
//				"---   ali_nim_m3501_cfg   ---\n\n",
//				cmd,
//				nim_m3501_cfg.recv_freq_low, nim_m3501_cfg.recv_freq_high,
//				nim_m3501_cfg.qpsk_config, nim_m3501_cfg.reserved,
//				nim_m3501_cfg.tuner_i2c_id, nim_m3501_cfg.tuner_i2c_addr,
//				nim_m3501_cfg.demod_i2c_id, nim_m3501_cfg.demod_i2c_addr,
//				nim_m3501_cfg.tuner_id);

		SL_DBG("%s demod i2c 0x%x\n", __func__, nim_m3501_cfg.demod_i2c_addr);
		break;
	}
	case ALISL_NIM_QAM: {

		if (dev->id >= NIM_ID_HERCULES_0 && dev->id <= NIM_ID_HERCULES_3) {
			//tuner_id = dev->id - NIM_ID_HERCULES_0;

			memset(&nim_hercules_cfg, 0, sizeof(struct ali_nim_hercules_cfg));
			nim_hercules_cfg.qam_mode = nim_config->config_dvbc.demo_config.qam_mode;
			nim_hercules_cfg.i2c_base_addr = nim_config->config_dvbc.demo_config.i2c_base_addr;
			nim_hercules_cfg.i2c_type_id = nim_config->config_dvbc.demo_config.i2c_type_id;
			SL_DBG("nim_hercules_cfg.qam_mode = 0x%x, nim_hercules_cfg.i2c_base_addr = 0x%x, " \
			       "nim_hercules_cfg.i2c_type_id = 0x%x\n",
			       nim_hercules_cfg.qam_mode, nim_hercules_cfg.i2c_base_addr, nim_hercules_cfg.i2c_type_id);
			ali_nim_cfg_ptr = &nim_hercules_cfg;
			cmd = ALI_NIM_HARDWARE_INIT_C;
			break;
		}

		sl_nim_config_dvbc *p_conf = &nim_config->config_dvbc;

		tuner_id = get_tuner_id(p_conf->tunner_config.tuner_id);

		memset(&nim_m3200_cfg, 0, sizeof(struct ali_nim_m3200_cfg));
		nim_m3200_cfg.tuner_id = tuner_id;
		nim_m3200_cfg.tuner_config_data.AGC_REF = p_conf->tunner_config.agc_ref;
		nim_m3200_cfg.tuner_config_data.RF_AGC_MIN = p_conf->tunner_config.rf_agc_min;
		nim_m3200_cfg.tuner_config_data.RF_AGC_MAX = p_conf->tunner_config.rf_agc_max;
		nim_m3200_cfg.tuner_config_data.IF_AGC_MAX = p_conf->tunner_config.if_agc_max;
		nim_m3200_cfg.tuner_config_data.IF_AGC_MIN = p_conf->tunner_config.if_agc_min;

		nim_m3200_cfg.tuner_config_ext.c_tuner_crystal = p_conf->tunner_config.tuner_crystal;
		nim_m3200_cfg.tuner_config_ext.c_tuner_base_addr = p_conf->tunner_config.i2c_base_addr;
		nim_m3200_cfg.tuner_config_ext.c_chip = p_conf->tunner_config.chip;
		nim_m3200_cfg.tuner_config_ext.c_tuner_special_config = p_conf->tunner_config.tuner_special_config;
		nim_m3200_cfg.tuner_config_ext.c_tuner_ref_divratio = p_conf->tunner_config.tuner_ref_divratio;
		nim_m3200_cfg.tuner_config_ext.w_tuner_if_freq = p_conf->tunner_config.wtuner_if_freq;
		nim_m3200_cfg.tuner_config_ext.c_tuner_agc_top = p_conf->tunner_config.tuner_agc_top;
		nim_m3200_cfg.tuner_config_ext.c_tuner_step_freq = p_conf->tunner_config.tuner_step_freq;
		nim_m3200_cfg.tuner_config_ext.i2c_type_id = p_conf->tunner_config.i2c_type_id;
		nim_m3200_cfg.tuner_config_ext.c_tuner_freq_param = p_conf->tunner_config.tunner_freq_param;
		nim_m3200_cfg.tuner_config_ext.c_tuner_reopen = p_conf->tunner_config.tuner_reopen;
		nim_m3200_cfg.tuner_config_ext.w_tuner_if_freq_j83a = p_conf->tunner_config.tuner_if_freq_J83A;
		nim_m3200_cfg.tuner_config_ext.w_tuner_if_freq_j83b = p_conf->tunner_config.tuner_if_freq_J83B;
		nim_m3200_cfg.tuner_config_ext.w_tuner_if_freq_j83c = p_conf->tunner_config.tuner_if_freq_J83C;
		//nim_m3200_cfg.tuner_config_ext.w_tuner_if_j83ac_type = p_conf->tunner_config.tuner_if_J83AC_type;

		nim_m3200_cfg.tuner_config_ext.w_tuner_if_j83ac_type = p_conf->demo_config.qam_mode;
		nim_m3200_cfg.qam_mode = p_conf->demo_config.qam_mode;

		ali_nim_cfg_ptr = &nim_m3200_cfg;
		cmd = ALI_NIM_HARDWARE_INIT_C;
		break;
	}
	case ALISL_NIM_OFDM: {

		if (dev->id == NIM_ID_HD2818_MXL608_0) {
			memset(&nim_hd2818_mxl608_cfg, 0, sizeof(struct ali_nim_hd2818_mxl608_cfg));
			nim_hd2818_mxl608_cfg.tuner_i2c_addr= nim_config->config_dvbt.tunner_config.i2c_base_addr;
			nim_hd2818_mxl608_cfg.tuner_i2c_id= nim_config->config_dvbt.tunner_config.i2c_type_id;
			nim_hd2818_mxl608_cfg.tuner_id= nim_config->config_dvbt.tunner_config.tuner_id;
			nim_hd2818_mxl608_cfg.demod_i2c_addr= nim_config->config_dvbt.demo_config.i2c_base_addr;
			nim_hd2818_mxl608_cfg.demod_i2c_id= nim_config->config_dvbt.demo_config.i2c_type_id;
			SL_DBG("[%s]tuner_i2c_addr = 0x%x, tuner_i2c_id = 0x%x,demod_i2c_addr = 0x%x, demod_i2c_id = 0x%x\n",
			       __FUNCTION__, nim_hd2818_mxl608_cfg.tuner_i2c_addr, nim_hd2818_mxl608_cfg.tuner_i2c_id,
			       nim_hd2818_mxl608_cfg.demod_i2c_addr, nim_hd2818_mxl608_cfg.demod_i2c_id);
			ali_nim_cfg_ptr = &nim_hd2818_mxl608_cfg;
			cmd = ALI_NIM_HARDWARE_INIT_T;
			break;
		}
		sl_nim_config_dvbt *p_conf = &nim_config->config_dvbt;

		tuner_id = get_tuner_id(p_conf->tunner_config.tuner_id);

		memset(&nim_m8843_cfg, 0, sizeof(struct ali_nim_mn88436_cfg));
		nim_m8843_cfg.tuner_id = tuner_id;

		//0:ISDBT_TYPE, 1:DVBT_TYPE, 2:DVBT2_TYPE, 3:DVBT2-COMBO, 4...
		nim_m8843_cfg.cofdm_data.flag = p_conf->tunner_config.standard;

		nim_m8843_cfg.cofdm_data.connection_config = 0x00;  //no I/Q swap.
		nim_m8843_cfg.cofdm_data.cofdm_config = 0x03;        //low-if, dirct , if-enable.

		nim_m8843_cfg.cofdm_data.AGC_REF = p_conf->tunner_config.agc_ref;
		nim_m8843_cfg.cofdm_data.IF_AGC_MAX = p_conf->tunner_config.if_agc_max;
		nim_m8843_cfg.cofdm_data.IF_AGC_MIN = p_conf->tunner_config.if_agc_min;


		nim_m8843_cfg.tuner_config.c_tuner_crystal = p_conf->tunner_config.tuner_crystal;
		nim_m8843_cfg.tuner_config.w_tuner_if_freq = p_conf->tunner_config.wtuner_if_freq;
		nim_m8843_cfg.tuner_config.c_tuner_agc_top = p_conf->tunner_config.tuner_agc_top;
		nim_m8843_cfg.tuner_config.c_chip = p_conf->tunner_config.chip;
		//nim_m8843_cfg.tuner_config.tuner_ref_divratio = p_conf->tunner_config.tuner_ref_divratio;
		//nim_m8843_cfg.tuner_config.tuner_step_freq = p_conf->tunner_config.tuner_step_freq;

		nim_m8843_cfg.tuner_config.i2c_type_id = p_conf->tunner_config.i2c_type_id;
		nim_m8843_cfg.tuner_config.c_tuner_base_addr = p_conf->tunner_config.i2c_base_addr;

		nim_m8843_cfg.ext_dm_config.i2c_base_addr = p_conf->demo_config.i2c_base_addr;
		nim_m8843_cfg.ext_dm_config.i2c_type_id   = p_conf->demo_config.i2c_type_id;


		// nim_m8843_cfg.ext_dm_config is not in used!
		ali_nim_cfg_ptr = &nim_m8843_cfg;

//		SL_DBG("\n---   ali_nim_mn88436_cfg cmd: %lu   ---\n "
//				"IF_AGC_MAX: %u\n tuner_id: 0x%x\n c_chip: %d\n"
//				"---   ali_nim_mn88436_cfg   ---\n\n",
//				cmd,
//				nim_m8843_cfg.cofdm_data.IF_AGC_MAX,
//				nim_m8843_cfg.tuner_id,
//				nim_m8843_cfg.tuner_config.c_chip);

		cmd = ALI_NIM_HARDWARE_INIT_T;
		break;
	}
	default:
		SL_DBG("invalid nim_id\n");
		return ERROR_INVAL;
	}

	if (ioctl(dev->fd, cmd, ali_nim_cfg_ptr)) {
		SL_DBG("NIM Hardware Init Failed\n");
		return ERROR_INVAL;
	}
	SL_DBG("NIM Hardware Init Success\n");
	return 0;
}

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

/* temporary for PDK which does not define ALI_NIM_TURNER_SET_STANDBY yet */
#ifndef ALI_NIM_TURNER_SET_STANDBY
#define ALI_NIM_TURNER_SET_STANDBY NIM_TURNER_SET_STANDBY
#endif

/* table must be aligned with enum nim_io_command */
static struct ali_cmd {
	uint32_t cmd;
	int get_value;
} sl2ali_cmd[] = {
	{ ALI_NIM_READ_RSUB, 1 }, /* NIM_IOCMD_READ_RSUB, Read Reed Solomon Uncorrected block */
	{ ALI_NIM_STOP_AUTOSCAN, 0 }, /* NIM_IOCMD_STOP_ATUOSCAN, Stop autoscan */
	{ 0, 0 }, /* NIM_IOCMD_SET_RESET_CALLBACK */
	{ 0, 0 }, /* NIM_IOCMD_STOP_CHANSCAN */
	{ 0, 1 }, /* NIM_IOCMD_GET_REC_PERFORMANCE_INFO, get receiver performance info. */
	{ 0, 1 }, /* NIM_IOCMD_CHANGE_TS_GAP, Change ts gap */
	{ 0, 1 }, /* NIM_IOCMD_SET_SSI_CLK, Set SSI Clock */
	{ ALI_NIM_SET_POLAR, 0 }, /* NIM_IOCMD_SET_POLAR, DVB-S NIM Device set LNB polarization */
	{ ALI_NIM_TURNER_SET_STANDBY, 0 }, /* NIM_IOCMD_TUNER_POWER_CONTROL, Tunner set standby */
	{ 0, 0 }, /* NIM_IOCMD_SET_BLSCAN_MODE, Para = 0, NIM_SCAN_FAST; Para = 1, NIM_SCAN_ACCURATE(slower) */
	{ 0, 0 }, /* NIM_IOCMD_AUTO_SCAN, Do AutoScan Procedure */
	{ 0, 0 }, /* NIM_IOCMD_CHANNEL_CHANGE,         <Do Channel Change */
	{ 0, 0 }, /* NIM_IOCMD_CHANNEL_SEARCH,         <Do Channel Search */
	{ 0, 0 }, /* NIM_IOCMD_QUICK_CHANNEL_CHANGE, Do Quick Channel Change without waiting lock */
	{ ALI_NIM_DRIVER_GET_ID, 1 }, /* NIM_IOCMD_GET_ID, Get Nim ID; 3501 type: M3501A/M3501B */
	{ 0, 0 }, /* NIM_IOCMD_DISEQC_OPERATION, NIM DiSEqC Device Opearation */
	{ 0, 0 }, /* NIM_IOCMD_DISEQC2X_OPERATION, NIM DiSEqC2X Device Opearation */
	{ ALI_NIM_AS_SYNC_WITH_LIB, 0 }, /* NIM_IOCMD_AS_SYNC_WITH_LIB, Nim sync with share lib for AUTOSCAN*/
	{ ALI_NIM_GET_LOCK_STATUS, 1 }, /* NIM_IOCMD_GET_LOCK_STATUS */
};

uint32_t alislnim_ioctl(alisl_handle handle, unsigned int cmd, uint32_t param)
{
    struct ali_cmd *ali_cmd;
    int ret = 0;

    struct nim_device *dev = (struct nim_device *)handle;
    if (dev == NULL) {
        SL_DBG("Try to open before construct!\n");
        return ERROR_NULLDEV;
    }
    if (!flag_bit_test_any(&dev->status, NIM_STATUS_START)) {
        SL_DBG("Nim have not start\n");
        return ERROR_STATUS;
    }
    /*  CMD GET_INFO is not simply forwarded to kernel */
    if (cmd == NIM_IOCMD_GET_INFO) {
       struct slnim_signal_status *info = (struct slnim_signal_status *)param;
       unsigned char reg_polar_status[16] = {0x01,0x7c,0x01,0xff};
       if (!param)
          return ERROR_INVAL;

       memset(info, 0, sizeof(struct slnim_signal_status));
       info->ul_freq = ioctl(dev->fd, ALI_NIM_READ_FREQ, 0);
       info->ul_signal_quality = ioctl(dev->fd, ALI_NIM_READ_SNR, 0);
       info->ul_signal_strength = ioctl(dev->fd, ALI_NIM_READ_AGC, 0);
       info->ul_bit_error_rate = ioctl(dev->fd, ALI_NIM_READ_QPSK_BER, 0);
       info->ul_per = ioctl(dev->fd, ALI_NIM_READ_RSUB, 0);
       info->ul_pre_ber = ioctl(dev->fd, ALI_NIM_READ_PRE_BER, 0);
       // ALi NIM support RF level C/N and MER now
       //if (dev->type != ALISL_NIM_QPSK) {
           /* The 2 parameter is just supported by DVB-T/T2, ISDB-T, DVB-C */
           info->ul_rf_level = ioctl(dev->fd, ALI_NIM_GET_RF_LEVEL, 0);
           info->ul_signal_cn = ioctl(dev->fd, ALI_NIM_GET_CN_VALUE, 0);
           //info->ul_mer = ioctl(dev->fd, ALI_NIM_GET_MER, 0);
       //}
       if (dev->type == ALISL_NIM_QPSK) {
           // polar info is just supported by DVB-S/S2
           if(ioctl(dev->fd, ALI_NIM_REG_RW, reg_polar_status)){
               SL_ERR("Get NIM polar status error!\n");
               return ERROR_INVAL;
           }
           info->info.u.dvbs.ul_polar_status = (reg_polar_status[3]>>6) & 0x01; // bit6: polar status

		   unsigned char work_mode = 0;       
		   if (ioctl(dev->fd, ALI_NIM_GET_WORK_MODE, &work_mode)) {
		       SL_ERR("Get NIM work mode error!\n");
			   return ERROR_INVAL;
		   }
		   if (work_mode) {
		       info->info.std = SLNIM_STD_DVBS2;
		   } else {
			   info->info.std = SLNIM_STD_DVBS;
		   }
       } else if (dev->type == ALISL_NIM_OFDM) /* DVB-T, DVB-T2, ISDB-T */
          memcpy((void *)&info->info, &dev->info, sizeof(struct slnim_signal_info));
       else {
          info->info.std = SLNIM_STD_OTHER;
       }

       return ret;
    } else if (cmd == NIM_IOCMD_T2_SIGNAL_ONLY) {
        if(ioctl(dev->fd, ALI_NIM_DRIVER_T2_SIGNAL_ONLY, (unsigned char*)param)){
            SL_ERR("Invalid NIM ioctl cmd %d\n", cmd);
            return ERROR_INVAL;
        } else {
            return 0;
        }
    }
	else if (cmd == NIM_IOCMD_SYM_LIMIT_RANGE) {
		if(ioctl(dev->fd, ALI_NIM_DRIVER_SYM_LIMIT_RANGE, (unsigned char*)param)){
			SL_ERR("Invalid NIM ioctl cmd %d\n", cmd);
			return ERROR_INVAL;
		} else {
			return 0;
		}
	}
	else if (cmd == NIM_IOCMD_SET_LNB_POWER_ONOFF) {
		if(ioctl(dev->fd, ALI_NIM_SET_LNB_POWER_ONOFF, (unsigned char*)param)){
			SL_ERR("Invalid NIM ioctl cmd %d\n", cmd);
			return ERROR_INVAL;
		} else {
			return 0;
		}
	}

    if (cmd >= NIM_IOCMD_NUM) {
        SL_ERR("Invalid NIM ioctl cmd %d\n", cmd);
        return ERROR_INVAL;
    }

	if(NIM_IOCMD_REG_RW == cmd){
		if(ioctl(dev->fd, ALI_NIM_REG_RW, (unsigned char*)param)){
		    SL_ERR("Invalid NIM ioctl cmd %d\n", cmd);
			return ERROR_INVAL;
		}
		else
			return 0;
	}

    ali_cmd = sl2ali_cmd + cmd;

    if (ali_cmd->get_value) {
        int ret;
        if (!param)
          return ERROR_INVAL;

        ret = ioctl(dev->fd, ali_cmd->cmd, 0);
        if (ret < 0)
           return ERROR_INVAL;
        *((uint32_t *)param) = ret;
        //SL_DBG("[sl nim]: get cmd %d(0x%x) = %d\n", cmd, ali_cmd->cmd, *((uint32_t *)param));
    } else {
        if (cmd == NIM_IOCMD_SET_POLAR) {
            unsigned char tmp = param;
            ret = ioctl(dev->fd,  ali_cmd->cmd, &tmp);
        } else
            ret = ioctl(dev->fd,  ali_cmd->cmd, param);
        //SL_DBG("[sl nim]: set cmd %d(0x%x) with %d\n", cmd, ali_cmd->cmd, param);
    }
    return ret ? ERROR_INVAL : 0;
}

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

alisl_retcode alislnim_autoscan(alisl_handle handle, slnim_auto_scan_t* param)
{
    unsigned char *nim_data = NULL;
    unsigned char polar = 0;
    unsigned char lck = 0;
    unsigned char fec = 0;
    unsigned short freq = 0;
    unsigned int sym = 0;
    unsigned char as_stat = 0;
	unsigned int stream_id = 0;
	struct nim_dvbs_isid isid_info;
    alisl_retcode ret = 0;
    struct NIM_AUTO_SCAN autoscan_param;
    struct nim_device *dev = (struct nim_device *)handle;
    unsigned char data[MAX_KUMSG_SIZE] = {0};
    nim_data = data;

    memset(&autoscan_param, 0, sizeof(struct NIM_AUTO_SCAN));
    if (dev == NULL || dev->kumsgfd < 0) {
        SL_DBG("Try to open before construct!\n");
        return ERROR_NULLDEV;
    }
    if (dev->type != ALISL_NIM_QPSK) {
        SL_DBG("NIM type does not support autoscan\n");
        return ERROR_STATUS;
    }
    if (!flag_bit_test_any(&dev->status, NIM_STATUS_START)) {
        SL_DBG("Nim have not start\n");
        return ERROR_STATUS;
    }

    autoscan_param.unicable = param->unicable;
    autoscan_param.fub      = param->Fub;
    autoscan_param.sfreq    = param->sfreq;
    autoscan_param.efreq    = param->efreq;
	autoscan_param.scan_type = 1;
    //autoscan_param.callback = param->callback;

    ioctl(dev->fd, ALI_NIM_AUTO_SCAN, &autoscan_param);

    flag_bit_set(&dev->status, NIM_STATUS_AS);

    while(1) {
        /*todo socket communication*/
        //nim_data = netlink_receive_msg(&dev->nl);
        //if (!nim_data) {
        if (read(dev->kumsgfd, nim_data, MAX_KUMSG_SIZE) < 0) {
            //SL_DBG("%s in: receive msg fail!\n", __FUNCTION__);
            //ret = ERROR_INVAL;
            usleep(100 * 1000);
            continue;
            //break;
        }

        memcpy((void *)&lck, ((unsigned char *)nim_data) + 3, 1);
        memcpy((void *)&polar, ((unsigned char *)nim_data) + 4, 1);
        memcpy((void *)&fec, ((unsigned char *)nim_data) + 5, 1);
        memcpy((void *)&freq, ((unsigned char *)nim_data) + 6, 2);
        memcpy((void *)&sym, ((unsigned char *)nim_data) + 8, 4);
        memcpy((void *)&as_stat, ((unsigned char *)nim_data) + 12, 1);

		memset(&isid_info, 0, sizeof(isid_info));
		if (ioctl(dev->fd, ALI_NIM_ACM_MODE_GET_ISID, (unsigned int)(&isid_info))) {
			SL_DBG("dvbs get multistream info failed\n");
        //   ret = ERROR_INVAL; Bug #83197
        //    break;
		}

		if(isid_info.isid_num > 0) {
			stream_id = isid_info.isid_write;
		} else {
			stream_id = 0xff;  // if it not a multistream, set the stream_id to invalid value -- 0xff
		}
		
        if (as_stat)
            break;

        if ((param) && (param->callback)) {
            ret = param->callback(lck, polar, freq, sym, fec, param->usr_data, stream_id);
            if (ret) {
                //may space full, so exit auto scan
                ioctl(dev->fd, ALI_NIM_STOP_AUTOSCAN, 1);
                SL_DBG("call back error in nim share lib\n");
                ret = ERROR_INVAL;
                break;
            }
        }
        ioctl(dev->fd, ALI_NIM_AS_SYNC_WITH_LIB, 0);
    }
    flag_bit_clear(&dev->status, NIM_STATUS_AS);
    return ret;
}

alisl_retcode alislnim_autoscan_active(alisl_handle handle)
{
    struct nim_device *dev = (struct nim_device *)handle;
    if (dev->type != ALISL_NIM_QPSK) {
        SL_DBG("NIM type does not support autoscan\n");
        return ERROR_STATUS;
    }
    return flag_bit_test_any(&dev->status, NIM_STATUS_AS);
}

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
static int g_plp_count = -1;
alisl_retcode alislnim_freqlock(alisl_handle handle, slnim_freq_lock_t* param)
{
	int v;
	struct NIM_CHANNEL_CHANGE nim_param;
	struct nim_device *dev = (struct nim_device *)handle;
	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	struct slnim_signal_info *info = &dev->info;

	if (!flag_bit_test_any(&dev->status, NIM_STATUS_START)) {
		SL_DBG("Nim have not start\n");
		return ERROR_STATUS;
	}

	memset(&nim_param, 0, sizeof(struct NIM_CHANNEL_CHANGE));
	nim_param.freq           =  param->freq;
	nim_param.sym            =  param->sym;

	switch (param->fec) {
	case SLNIM_FEC_AUTO: v = 0 /*FEC_AUTO not implemented */; break;
	case SLNIM_FEC_1_2: v = FEC_1_2; break;
	case SLNIM_FEC_2_3: v = FEC_2_3; break;
	case SLNIM_FEC_3_4: v = FEC_3_4; break;
	case SLNIM_FEC_5_6: v = FEC_5_6; break;
	case SLNIM_FEC_7_8: v = FEC_7_8; break;
	default: return ERROR_INVAL;
	}
	nim_param.fec = v;

	switch (dev->type) {
	case ALISL_NIM_OFDM: /* DVB-T */

		switch (param->bandwidth) {
		case SLNIM_BANDWIDTH_6_MHZ: v = BW_6M; break;
		case SLNIM_BANDWIDTH_7_MHZ: v = BW_7M; break;
		case SLNIM_BANDWIDTH_8_MHZ: v = BW_8M; break;
		default: return ERROR_INVAL;
		}
		nim_param.bandwidth = v;

		switch (param->guard_interval) {
		case SLNIM_GUARD_INTER_AUTO: v = GUARD_AUTO; break;
		case SLNIM_GUARD_INTER_1_4:  v = GUARD_1_4; break;
		case SLNIM_GUARD_INTER_1_8:  v = GUARD_1_8; break;
		case SLNIM_GUARD_INTER_1_16: v = GUARD_1_16; break;
		case SLNIM_GUARD_INTER_1_32: v = GUARD_1_32; break;
		default: return ERROR_INVAL;
		}
		nim_param.guard_interval =  v;

		switch (param->fft_mode) {
		case SLNIM_FFT_MODE_AUTO: v = MODE_AUTO; break;
		case SLNIM_FFT_MODE_2K: v = MODE_2K; break;
		case SLNIM_FFT_MODE_4K: v = MODE_4K; break;
		case SLNIM_FFT_MODE_8K: v = MODE_8K; break;
		default: return ERROR_INVAL;
		}
		nim_param.fft_mode =  v;

		switch (param->modulation) {
		case SLNIM_MODUL_AUTO:  v = TPS_CONST_AUTO; break;
		case SLNIM_MODUL_DQPSK: v = TPS_CONST_DQPSK; break;
		case SLNIM_MODUL_QPSK:  v = TPS_CONST_QPSK; break;
		case SLNIM_MODUL_16QAM: v = TPS_CONST_16QAM; break;
		case SLNIM_MODUL_64QAM: v = TPS_CONST_64QAM; break;
		default: return ERROR_INVAL;
		}
		nim_param.modulation = v;

		switch (param->inverse) {
		case SLNIM_SPEC_AUTO: v = 0 /* SPEC_AUTO not implemented*/; break;
		case SLNIM_SPEC_NORM: v = SPEC_NORM; break;
		case SLNIM_SPEC_INVERT: v = SPEC_INVERTED; break;
		default: return ERROR_INVAL;
		}
		nim_param.inverse =  v;

		switch (param->standard) {
		case SLNIM_STD_ISDBT:
		case SLNIM_STD_DVBT:
			nim_param.usage_type = USAGE_TYPE_CHANCHG;
			if (param->standard == SLNIM_STD_ISDBT) {
			    nim_param.t2_signal = T2_SIGNAL_ISDB_T;
			} else {
	            nim_param.t2_signal = T2_SIGNAL_T;
			}
			break;
		case SLNIM_STD_DVBT2:
		case SLNIM_STD_DVBT2_COMBO:
			if (param->standard == SLNIM_STD_DVBT2) {
				nim_param.t2_signal = T2_SIGNAL_T2; /* t2_signal should not be taken into account in autoscan mode */
			} else {
				nim_param.t2_signal = T2_SIGNAL_COMBO;
			}
			if (param->plp_index == SLNIM_PLP_AUTO) {
                g_plp_count = 1;
				nim_param.usage_type = USAGE_TYPE_AUTOSCAN;
				nim_param.t2_signal = T2_SIGNAL_T2;
			} else if(param->plp_index == SLNIM_PLP_NEXT) {
				//Try to auto detect the next signal pipe within this channel,
				//such as the next PLP of (MPLP of DVB-T2), or the next priority signal(Hierarchy of DVB-T).
				//Before USAGE_TYPE_NEXT_PIPE_SCAN can be used, you must call USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANSCAN first.
				nim_param.usage_type = USAGE_TYPE_NEXT_PIPE_SCAN;
				nim_param.t2_signal = T2_SIGNAL_T2; // USAGE_TYPE_NEXT_PIPE_SCAN and T2_SIGNAL_COMBO will connect fail
				nim_param.plp_index = g_plp_count++;
			} else {
				nim_param.usage_type = USAGE_TYPE_CHANCHG;
				nim_param.plp_index = param->plp_index;
				nim_param.plp_id = param->plp_id;
				//if (param->standard == SLNIM_STD_DVBT2_COMBO) {
				//    nim_param.usage_type = USAGE_TYPE_AUTOSCAN;
				//}
			}
			break;
		default: return ERROR_INVAL;
		}
		break;
	case ALISL_NIM_QAM: /* DVB-C */
		nim_param.modulation = param->modulation;
		nim_param.bandwidth = param->bandwidth;
		break;
	case  ALISL_NIM_QPSK: /* DVB-T */
		break;
	}

	//SL_DBG("sl usage %d plp_index %d\n", nim_param.usage_type, nim_param.plp_index);

	if (ioctl(dev->fd, ALI_NIM_CHANNEL_CHANGE, &nim_param) < 0) {
		SL_DBG("NIM channel change error %s\n", strerror(errno));
		return ERROR_INVAL;
	}
	//SL_DBG("sl usage %d plp num %d id %d\n", nim_param.usage_type, nim_param.plp_num, nim_param.plp_id);

	if (dev->type == ALISL_NIM_OFDM) { /* DVB-T */

		int fec, guard, mode, modul, spect;
		switch (nim_param.fec) {
		case FEC_1_2: fec = SLNIM_FEC_1_2; break;
		case FEC_2_3: fec = SLNIM_FEC_2_3; break;
		case FEC_3_4: fec = SLNIM_FEC_3_4; break;
		case FEC_5_6: fec = SLNIM_FEC_5_6; break;
		case FEC_7_8: fec = SLNIM_FEC_7_8; break;
		default: fec = SLNIM_FEC_AUTO;
		}
		switch (nim_param.guard_interval) {
		case GUARD_1_4: guard = SLNIM_GUARD_INTER_1_4; break;
		case GUARD_1_8: guard = SLNIM_GUARD_INTER_1_8; break;
		case GUARD_1_16: guard = SLNIM_GUARD_INTER_1_16; break;
		case GUARD_1_32: guard = SLNIM_GUARD_INTER_1_32; break;
		default: guard = SLNIM_GUARD_INTER_AUTO;
		}
		switch (nim_param.fft_mode) {
		case MODE_2K: mode = SLNIM_FFT_MODE_2K; break;
		case MODE_4K: mode = SLNIM_FFT_MODE_4K; break;
		case MODE_8K: mode = SLNIM_FFT_MODE_8K; break;
		default: mode = SLNIM_FFT_MODE_AUTO;
		}
		switch (nim_param.modulation) {
		case TPS_CONST_DQPSK: modul = SLNIM_MODUL_DQPSK; break;
		case TPS_CONST_QPSK: modul = SLNIM_MODUL_QPSK; break;
		case TPS_CONST_16QAM: modul = SLNIM_MODUL_16QAM; break;
		case TPS_CONST_64QAM: modul = SLNIM_MODUL_64QAM; break;
		default: modul = SLNIM_MODUL_AUTO;
		}
		switch (nim_param.inverse) {
		case SPEC_NORM: spect = SLNIM_SPEC_NORM; break;
		case SPEC_INVERTED: spect = SLNIM_SPEC_INVERT; break;
		default: spect = SLNIM_SPEC_AUTO;
		}
		//SL_DBG("nim_param.t2_signal: %d\n", nim_param.t2_signal);
		if (nim_param.t2_signal == T2_SIGNAL_T2) {
			info->u.dvbt2.fec = fec;
			info->u.dvbt2.guard_inter = guard;
			info->u.dvbt2.fft_mode = mode;
			info->u.dvbt2.modulation = modul;
			info->u.dvbt2.spectrum = spect;
			info->u.dvbt2.plp_num = nim_param.plp_num;
			info->u.dvbt2.plp_index = nim_param.plp_index;
			info->u.dvbt2.plp_id = nim_param.plp_id;
			info->u.dvbt2.system_id = nim_param.t2_system_id;
			info->u.dvbt2.trans_stream_id = nim_param.t_s_id;
			info->u.dvbt2.network_id = nim_param.network_id;
			info->std = SLNIM_STD_DVBT2;
		} else {
			info->u.dvbt_isdbt.fec = fec;
			info->u.dvbt_isdbt.guard_inter = guard;
			info->u.dvbt_isdbt.fft_mode = mode;
			info->u.dvbt_isdbt.modulation = modul;
			info->u.dvbt_isdbt.spectrum = spect;
			info->std = SLNIM_STD_DVBT;
		}
		if (param->standard == SLNIM_STD_ISDBT) {
			info->std = SLNIM_STD_ISDBT;
		}
	}
	return 0;
}

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

alisl_retcode alislnim_diseqc_operate(alisl_handle handle, slnim_diseqc_operate_t* param)
{
    int32_t ret, i;
    struct ali_nim_diseqc_cmd diseqc_cmd;
    struct nim_device *dev = (struct nim_device *)handle;
    if (dev == NULL) {
        SL_DBG("Try to open before construct!\n");
        return ERROR_NULLDEV;
    }
    if (dev->type != ALISL_NIM_QPSK) {
        SL_DBG("NIM type does not support DISEQC\n");
        return ERROR_STATUS;
    }
    if (!flag_bit_test_any(&dev->status, NIM_STATUS_START)) {
        SL_DBG("Nim have not start\n");
        return ERROR_STATUS;
    }

    diseqc_cmd.mode = sl_disecmode_to_driver_mode(param->mode);
    for(i = 0; i < param->cnt; i++) {
        diseqc_cmd.cmd[i] = param->cmd[i];
    }
    diseqc_cmd.cmd_size = param->cnt;
    diseqc_cmd.diseqc_type = 1; //nim_DiSEqC_operate;

    ret = ioctl(dev->fd, ALI_NIM_DISEQC_OPERATE, &diseqc_cmd);

    if(ret) {
        SL_DBG("ALI_NIM_DISEQC_OPERATE in alislnim_diseqc_operate error!\n");
        return ERROR_INVAL;
    }

    return 0;
}

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

alisl_retcode alislnim_diseqc2x_operate(alisl_handle handle, slnim_diseqc_operate_t* param)
{
    int32_t ret, i;
    struct ali_nim_diseqc_cmd diseqc_cmd;
    struct nim_device *dev = (struct nim_device *)handle;
    if (dev == NULL) {
        SL_DBG("Try to open before construct!\n");
        return ERROR_NULLDEV;
    }
    if (dev->type != ALISL_NIM_QPSK) {
        SL_DBG("NIM type does not support DISEQC\n");
        return ERROR_STATUS;
    }
    if (!flag_bit_test_any(&dev->status, NIM_STATUS_START)) {
        SL_DBG("Nim have not start\n");
        return ERROR_STATUS;
    }

    diseqc_cmd.mode = sl_disecmode_to_driver_mode(param->mode);;
    for(i = 0; i < param->cnt; i++) {
        diseqc_cmd.cmd[i] = param->cmd[i];
    }
    diseqc_cmd.cmd_size = param->cnt;
    diseqc_cmd.diseqc_type = 2; //nim_DiSEqC2X_operate;

    ret = ioctl(dev->fd, ALI_NIM_DISEQC_OPERATE, &diseqc_cmd);

    if(0 == ret) {
        for(i = 0; i < diseqc_cmd.ret_len; i++) {
            param->rt_value[i] = diseqc_cmd.ret_bytes[i];
        }
        param->rt_cnt = diseqc_cmd.ret_len;

        return 0;
    } else {
        SL_DBG("ALI_NIM_DISEQC_OPERATE error in alislnim_diseq2c_operate !\n");

        return ERROR_INVAL;
    }
}

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

alisl_retcode alislnim_close(alisl_handle handle)
{
    struct nim_device *dev = (struct nim_device *)handle;
    uint32_t i;

    if (dev == NULL) {
        SL_DBG("NULL dev data structure!\n");
        return ERROR_NULLDEV;
    }

    for (i = 0; i < ARRAY_SIZE(dev_nim); i++) {
        if (dev->id == dev_nim[i].id) {
            break;
        }
    }

    pthread_mutex_lock(&m_mutex);

	if (i >= ARRAY_SIZE(dev_nim)) {
        SL_DBG("Invalidate nim id!\n");
		pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    if (--dev_nim[i].open_cnt) {
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }


    if (!flag_bit_test_any(&dev->status, NIM_STATUS_OPEN)) {
        SL_DBG("Nim device not opened!\n");
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    if (flag_bit_test_any(&dev->status, NIM_STATUS_CLOSE)) {
        SL_DBG("NIM device already closed!\n");
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }
    //netlink_deinit(&dev->nl);
    if (dev->feedfd > 0) {
        close(dev->feedfd);
        dev->feedfd = -1;
    }

    close(dev->fd);
    dev->fd = -1;

	if (dev->kumsgfd > 0) {
		close(dev->kumsgfd);
		dev->kumsgfd = -1;
	}

    flag_bit_clear(&dev->status, NIM_STATUS_OPEN | NIM_STATUS_START);
    flag_bit_set(&dev->status, NIM_STATUS_CLOSE);
    alislnim_destruct(&handle);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}
