#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <ali_video_common.h>
#include <ali_media_common.h>

#include <alipltflog.h>
#include <alipltfretcode.h>

#include "internal.h"

static struct alisldis_dev *m_hd_dev = NULL;
static struct alisldis_dev *m_sd_dev = NULL;

#define VPO_NOT_OPENED -1
static int s_vpo_dev_fd = VPO_NOT_OPENED;

#define GMA1_NOT_OPENED  -1
static int s_gma1_dev_fd = GMA1_NOT_OPENED;

#define GMA2_NOT_OPENED  -1
static int s_gma2_dev_fd = GMA2_NOT_OPENED;

static int dev_open_cnt = 0;

#define DIS_DEV_FB0 "/dev/fb0"
#define DIS_DEV_FB1 "/dev/fb1"
#define DIS_DEV_FB2 "/dev/fb2"

#define CHECK_HANDLE(hdl)                                               \
    do {                                                                \
        if (!hdl ||                                                     \
            (((alisl_handle)hdl != (alisl_handle) m_hd_dev) &&          \
             ((alisl_handle)hdl != (alisl_handle) m_sd_dev)) ||			\
             (((struct alisldis_dev *)hdl)->fd_vpo != s_vpo_dev_fd)) {         \
            SL_ERR("invalidate handle = 0x%08x\n", (uint32_t)hdl); \
            return DIS_ERR_INVALHANDLE;                                 \
        }                                                               \
    } while(0)

#define VPO_IOCTL(cmd, param)                                           \
    do {                                                                \
        if (ioctl(dev->fd_vpo, cmd, param) < 0) {                       \
            SL_ERR("ioctl with cmd = 0x%08x, failed\n", cmd); \
            ret = DIS_ERR_IOCTLFAILED;                                  \
            goto quit;                                                  \
        }                                                               \
    } while(0)

#define GMA1_IOCTL(cmd, param)                                          \
    do {                                                                \
        if (ioctl(dev->fd_gma1, cmd, param) < 0) {                      \
            SL_ERR("ioctl with cmd = 0x%08x, failed\n", cmd); \
            ret = DIS_ERR_IOCTLFAILED;                                  \
            goto quit;                                                  \
        }                                                               \
    } while(0)

#define GMA2_IOCTL(cmd, param)                                          \
    do {                                                                \
        if (ioctl(dev->fd_gma2, cmd, param) < 0) {                      \
            SL_ERR("ioctl with cmd = 0x%08x, failed\n", cmd); \
            ret = DIS_ERR_IOCTLFAILED;                                  \
            goto quit;                                                  \
        }                                                               \
    } while(0)
    
static pthread_mutex_t m_hd_mutex   = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_sd_mutex   = PTHREAD_MUTEX_INITIALIZER;

static enum vp_display_layer

vpo_layer(enum dis_layer layer)
{
    switch (layer) {
        case DIS_LAYER_MAIN:
                return VPO_LAYER_MAIN;
        case DIS_LAYER_AUXP:
            return VPO_LAYER_AUXP;
        default:
            SL_ERR("invalidate layer\n");
            return -1;
    }
}

static enum tvmode
vpo_tvmode(enum dis_aspect_ratio ratio)
{
    switch (ratio) {
        case DIS_AR_16_9:
                return TV_16_9;
        case DIS_AR_4_3:
            return TV_4_3;
        case DIS_AR_AUTO:
            return TV_AUTO;
        default:
            SL_ERR("Invlidate ration = %d\n", ratio);
            return -1;
    }
}

static enum tvsystem
sldis_tvsys(enum dis_tvsys tvsys)
{
    switch (tvsys) {
        case DIS_TVSYS_PAL:
                return PAL;
        case DIS_TVSYS_NTSC:
            return NTSC;
        case DIS_TVSYS_PAL_M:
            return PAL_M;
        case DIS_TVSYS_PAL_N:
            return PAL_N;
        case DIS_TVSYS_PAL_60:
            return PAL_60;
        case DIS_TVSYS_NTSC_443:
            return NTSC_443;
        case DIS_TVSYS_SECAM:
            return SECAM;
        case DIS_TVSYS_MAC:
            return MAC;
        case DIS_TVSYS_LINE_720_25:
            return LINE_720_25;
        case DIS_TVSYS_LINE_720_30:
            return LINE_720_30;
        case DIS_TVSYS_LINE_1080_25:
            return LINE_1080_25;
        case DIS_TVSYS_LINE_1080_30:
            return LINE_1080_30;
        case DIS_TVSYS_LINE_1080_50:
            return LINE_1080_50;
        case DIS_TVSYS_LINE_1080_60:
            return LINE_1080_60;
        case DIS_TVSYS_LINE_1080_24:
            return LINE_1080_24;
        case DIS_TVSYS_LINE_1152_ASS:
            return LINE_1152_ASS;
        case DIS_TVSYS_LINE_1080_ASS:
            return LINE_1080_ASS;
        case DIS_TVSYS_PAL_NC:
            return PAL_NC;
        case DIS_TVSYS_LINE_576P_50_VESA:
            return LINE_576P_50_VESA;
        case DIS_TVSYS_LINE_720P_60_VESA:
            return LINE_720P_60_VESA;
        case DIS_TVSYS_LINE_1080P_60_VESA:
            return LINE_1080P_60_VESA;
		case DIS_TVSYS_LINE_4096X2160_24:
			return LINE_4096X2160_24;
		case DIS_TVSYS_LINE_3840X2160_24:
			return LINE_3840X2160_24;
		case DIS_TVSYS_LINE_3840X2160_25:
			return LINE_3840X2160_25;
		case DIS_TVSYS_LINE_3840X2160_30:
			return LINE_3840X2160_30;
		case DIS_TVSYS_LINE_3840X2160_50:
			return LINE_3840X2160_50;
		case DIS_TVSYS_LINE_3840X2160_60:
			return LINE_3840X2160_60;
		case DIS_TVSYS_LINE_4096X2160_25:
			return LINE_4096X2160_25;
		case DIS_TVSYS_LINE_4096X2160_30:
			return LINE_4096X2160_30;
		case DIS_TVSYS_LINE_4096X2160_50:
			return LINE_4096X2160_50;
		case DIS_TVSYS_LINE_4096X2160_60:
			return LINE_4096X2160_60;

        default:
            SL_ERR("invalidate tvsys = 0x%08x\n", tvsys);
            return -1;
    }
}

static enum Video_DacType
vpo_dac_type(enum dis_dac_group_type gtype)
{
    switch (gtype) {
        case DIS_DAC_GROUP_YUV1:
                return Video_YUV_1;
        case DIS_DAC_GROUP_YUV2:
            return Video_YUV_2;
        case DIS_DAC_GROUP_RGB1:
            return Video_RGB_1;
        case DIS_DAC_GROUP_RGB2:
            return Video_RGB_2;
        case DIS_DAC_GROUP_SVIDEO1:
            return Video_SVIDEO_Y_1;
        case DIS_DAC_GROUP_CVBS1:
            return Video_CVBS_1;
        case DIS_DAC_GROUP_CVBS2:
            return Video_CVBS_2;
        case DIS_DAC_GROUP_CVBS3:
            return Video_CVBS_3;
        case DIS_DAC_GROUP_CVBS4:
            return Video_CVBS_4;
        case DIS_DAC_GROUP_CVBS5:
            return Video_CVBS_5;
        case DIS_DAC_GROUP_CVBS6:
            return Video_CVBS_6;
        default:
            SL_ERR("invalidate gtype = %d\n", gtype);
            return -1;
    }
}

static uint32_t
vpo_dac_id(enum dis_dac_id dac_id)
{
    switch(dac_id) {
        case DIS_DAC1:
            return VIDEO_DAC0;
        case DIS_DAC2:
            return VIDEO_DAC1;
        case DIS_DAC3:
            return VIDEO_DAC2;
        case DIS_DAC4:
            return VIDEO_DAC3;
        case DIS_DAC5:
            return VIDEO_DAC4;
        case DIS_DAC6:
            return VIDEO_DAC5;
        default:
            SL_ERR("invalidate dac_id = %d\n", dac_id);
            return -1;
    }
}


static enum display_mode
vpo_display_mode(enum dis_display_mode mode)
{
    switch(mode) {
        case DIS_DM_PANSCAN:
                return PANSCAN;
        case DIS_DM_PANSCAN_NOLINEAR:
            return PANSCAN_NOLINEAR;
        case DIS_DM_LETTERBOX:
            return LETTERBOX;
        case DIS_DM_TWOSPEED:
            return TWOSPEED;
        case DIS_DM_PILLBOX:
            return PILLBOX;
        case DIS_DM_VERTICALCUT:
            return VERTICALCUT;
        case DIS_DM_NORMAL_SCALE:
            return NORMAL_SCALE;
        case DIS_DM_LETTERBOX149:
            return LETTERBOX149;
        case DIS_DM_AFDZOOM:
            return AFDZOOM;
        case DIS_DM_PANSCAN43ON169:
            return PANSCAN43ON169;
        case DIS_DM_COMBINED_SCALE:
            return COMBINED_SCALE;
        case DIS_DM_IGNORE:
            return DONT_CARE;
        case DIS_DM_VERTICALCUT_149:
            return VERTICALCUT_149;
        default:
            SL_ERR("invalidate display mode\n");
            return -1;
    }
}

static unsigned int vpo_param_to_ioparam(dis_attr attr, unsigned int param)
{
	unsigned int ioparam = 0;
	
	switch(attr) {
		case DIS_ATTR_HDMI_OUT_PIC_FMT:
			switch (param) {
				case DIS_HDMI_YCBCR_422:
					ioparam = YCBCR_422;
					break;
				case DIS_HDMI_YCBCR_444:
					ioparam = YCBCR_444;
					break;
				case DIS_HDMI_RGB_MODE1:
					ioparam = RGB_MODE1;
					break;
				case DIS_HDMI_RGB_MODE2:
					ioparam = RGB_MODE2;
					break;
				default:
					ioparam = 0xffffffff;
					break;
			}
            return ioparam;
        case DIS_ATTR_TVESDHD_SOURCE_SEL:
            if(0 != param && 1 != param)
                return -1;
            return param;
		default:
            return param;
	}
}

static uint32_t
vpo_attr_to_iocmd(dis_attr attr)
{
    switch(attr) {
        case DIS_ATTR_FULL_SCREEN_CVBS:
            return VPO_FULL_SCREEN_CVBS;
        case DIS_ATTR_SET_PREVIEW_SAR_MODE:
            return VPO_SET_PREVIEW_SAR_MODE;
        case DIS_ATTR_SD_CC_ENABLE:
            return VPO_SD_CC_ENABLE;
        case DIS_ATTR_TVESDHD_SOURCE_SEL:
            return VPO_TVESDHD_SOURCE_SEL;
        case DIS_ATTR_SET_LAYER_ORDER:
            return VPO_SET_LAYER_ORDER;
        case DIS_ATTR_ALWAYS_OPEN_CGMS_INFO:
            return VPO_ALWAYS_OPEN_CGMS_INFO;
        case DIS_ATTR_HDMI_OUT_PIC_FMT:
            return VPO_HDMI_OUT_PIC_FMT;
        case DIS_ATTR_SET_PREVIEW_MODE:
            return VPO_SET_PREVIEW_MODE;
        case DIS_ATTR_704_OUTPUT:
            return VPO_704_OUTPUT;
        case DIS_ATTR_SWAFD_ENABLE:
            return VPO_SWAFD_ENABLE;
        case DIS_ATTR_DIT_CHANGE:
            return VPO_DIT_CHANGE;
        case DIS_ATTR_PLAYMODE_CHANGE:
            return VPO_PLAYMODE_CHANGE;
        case DIS_ATTR_ENABLE_VBI:
            return VPO_ENABLE_VBI;
        case DIS_ATTR_DISAUTO_WIN_ONOFF:
            return VPO_DISAUTO_WIN_ONOFF;
        case DIS_ATTR_MHEG_IFRAME_NOTIFY:
            return VPO_MHEG_IFRAME_NOTIFY;
        case DIS_ATTR_MHEG_SCENE_AR:
            return VPO_MHEG_SCENE_AR;
        case DIS_ATTR_WRITE_WSS:
            return VPO_WRITE_WSS;
        case DIS_ATTR_SET_VBI_OUT:
            return VPO_SET_VBI_OUT;
        case DIS_ATTR_GET_DISPLAY_MODE:
            return VPO_GET_DISPLAY_MODE;
        case DIS_ATTR_GET_SRC_ASPECT:
            return VPO_GET_SRC_ASPECT;
        case DIS_ATTR_GET_TV_ASPECT:
            return VPO_GET_TV_ASPECT;
        case DIS_ATTR_GET_REAL_DISPLAY_MODE:
            return VPO_GET_REAL_DISPLAY_MODE;
        case DIS_ATTR_GET_MP_SCREEN_RECT:
            return VPO_GET_MP_SCREEN_RECT;
        case DIS_ATTR_SET_AUTO_WIN_ONOFF:
            return VPO_DISAUTO_WIN_ONOFF;
        case DIS_ATTR_SET_CGMS_INFO:
            return VPO_SET_CGMS_INFO;
        case DIS_ATTR_SET_PARAMETER:
            return VPO_SET_PARAMETER;
        case DIS_ATTR_SET_AFD_CONFIG:
            return VPO_AFD_CONFIG;
        case DIS_ATTR_SET_OSD_SHOW_TIME:
            return VPO_SET_OSD_SHOW_TIME;
		case DIS_ATTR_GET_OSD0_SHOW_TIME:
			return VPO_GET_OSD0_SHOW_TIME;
		case DIS_ATTR_GET_OSD1_SHOW_TIME:
			return VPO_GET_OSD1_SHOW_TIME;
		case DIS_ATTR_UNREG_DAC:
			return VPO_UNREG_DAC;
        default:
            SL_ERR("invalidate dis_attr = 0x%08x\n", attr);
            return -1;
    }

}


static enum dis_display_mode
dis_display_mode(uint32_t vpo_display_mode)
{
    switch(vpo_display_mode) {
        case PANSCAN:
                return DIS_DM_PANSCAN;
        case PANSCAN_NOLINEAR:
            return DIS_DM_PANSCAN_NOLINEAR;
        case LETTERBOX:
            return DIS_DM_LETTERBOX;
        case TWOSPEED:
            return DIS_DM_TWOSPEED;
        case PILLBOX:
            return DIS_DM_PILLBOX;
        case VERTICALCUT:
            return DIS_DM_VERTICALCUT;
        case NORMAL_SCALE:
            return DIS_DM_NORMAL_SCALE;
        case LETTERBOX149:
            return DIS_DM_LETTERBOX149;
        case AFDZOOM:
            return DIS_DM_AFDZOOM;
        case PANSCAN43ON169:
            return DIS_DM_PANSCAN43ON169;
        case COMBINED_SCALE:
            return DIS_DM_COMBINED_SCALE;
        case DONT_CARE:
            return DIS_DM_IGNORE;
        case VERTICALCUT_149:
            return DIS_DM_VERTICALCUT_149;
        default:
            SL_ERR("Unexpected display mode = 0x%08x\n", vpo_display_mode);
            return -1;
    }
}

static enum dis_tvsys
dis_tvsys_convert(enum tvsystem tvsys)
{
    switch (tvsys) {
        case PAL:
                return DIS_TVSYS_PAL;
        case NTSC:
            return DIS_TVSYS_NTSC;
        case PAL_M:
            return DIS_TVSYS_PAL_M;
        case PAL_N:
            return DIS_TVSYS_PAL_N;
        case PAL_60:
            return DIS_TVSYS_PAL_60;
        case NTSC_443:
            return DIS_TVSYS_NTSC_443;
        case SECAM:
            return DIS_TVSYS_SECAM;
        case MAC:
            return DIS_TVSYS_MAC;
        case LINE_720_25:
            return DIS_TVSYS_LINE_720_25;
        case LINE_720_30:
            return DIS_TVSYS_LINE_720_30;
        case LINE_1080_25:
            return DIS_TVSYS_LINE_1080_25;
        case LINE_1080_30:
            return DIS_TVSYS_LINE_1080_30;
        case LINE_1080_50:
            return DIS_TVSYS_LINE_1080_50;
        case LINE_1080_60:
            return DIS_TVSYS_LINE_1080_60;
        case LINE_1080_24:
            return DIS_TVSYS_LINE_1080_24;
        case LINE_1152_ASS:
            return DIS_TVSYS_LINE_1152_ASS;
        case LINE_1080_ASS:
            return DIS_TVSYS_LINE_1080_ASS;
        case PAL_NC:
            return DIS_TVSYS_PAL_NC;
        case LINE_576P_50_VESA:
            return DIS_TVSYS_LINE_576P_50_VESA;
        case LINE_720P_60_VESA:
            return DIS_TVSYS_LINE_720P_60_VESA;
        case LINE_1080P_60_VESA:
            return DIS_TVSYS_LINE_1080P_60_VESA;
		case LINE_4096X2160_24:
			return DIS_TVSYS_LINE_4096X2160_24;
		case LINE_3840X2160_24:
			return DIS_TVSYS_LINE_3840X2160_24;
		case LINE_3840X2160_25:
			return DIS_TVSYS_LINE_3840X2160_25;
		case LINE_3840X2160_30:
			return DIS_TVSYS_LINE_3840X2160_30;

		case LINE_3840X2160_50:
			return DIS_TVSYS_LINE_3840X2160_50;
		case LINE_3840X2160_60:
			return DIS_TVSYS_LINE_3840X2160_60;
		case LINE_4096X2160_25:
			return DIS_TVSYS_LINE_4096X2160_25;
		case LINE_4096X2160_30:
			return DIS_TVSYS_LINE_4096X2160_30;
		case LINE_4096X2160_50:
			return DIS_TVSYS_LINE_4096X2160_50;
		case LINE_4096X2160_60:
			return DIS_TVSYS_LINE_4096X2160_60;
        default:
            //SL_ERR("invalidate tvsys = 0x%08x\n", tvsys);
            return -1;
    }
}

static enum dis_output_type
vpo_dac_type_convert(enum TVEDacOutput_e dac_type)
{
    switch (dac_type) {
        case DAC_CAV_Y:
            return DIS_TYPE_Y;
        case DAC_CAV_PB:
            return DIS_TYPE_U;
        case DAC_CAV_PR:
            return DIS_TYPE_V;
        case DAC_CAV_RGB_R:
            return DIS_TYPE_R;
        case DAC_CAV_RGB_G:
            return DIS_TYPE_G;
        case DAC_CAV_RGB_B:
            return DIS_TYPE_B;
        case DAC_CVBS:
            return DIS_TYPE_CVBS;
        case DAC_YC_Y:
            return DIS_TYPE_SVIDEO_Y;
        case DAC_YC_C:
            return DIS_TYPE_SVIDEO_C;
        default:
            SL_ERR("This dac registered error dac_type = %d\n", dac_type);
            return DIS_TYPE_NONE;
    }
}


/**
 * Function Name: alisldis_open
 * @brief         Open DIS module, and return a handle to control the device.
 *
 * @param[in] dev_id Device ID, should be DIS_HD_DEV or DIS_SD_DEV
 * @param[out] hdl Device handle return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_open(enum dis_dev_id  dev_id,
              alisl_handle    *hdl)
{
	SL_DBG("dev_id:%d\n", dev_id);
    if ((DIS_SD_DEV != dev_id) &&
        (DIS_HD_DEV != dev_id)) {
        SL_ERR("invalidate dev_id\n");
        return DIS_ERR_INVAL;
    }
	if(NULL == hdl)
	{
        SL_ERR("invalid handle!!\n");
        return DIS_ERR_INVAL;
	}

	if (0 == dev_open_cnt) {
		pthread_mutex_init(&m_hd_mutex, NULL);
		pthread_mutex_init(&m_sd_mutex, NULL);

		pthread_mutex_lock(&m_hd_mutex);
		
		s_vpo_dev_fd = open(DIS_DEV_FB1, O_RDWR);
		if (s_vpo_dev_fd < 0) {
			SL_ERR("FATAL: %s open failed\n", DIS_DEV_FB1);
			s_vpo_dev_fd = VPO_NOT_OPENED;
			pthread_mutex_unlock(&m_hd_mutex);
			goto dev_open_failed;
		}
		ioctl(s_vpo_dev_fd, FBIO_SET_DE_LAYER, DE_LAYER0);

		s_gma1_dev_fd = open(DIS_DEV_FB0, O_RDWR);
		if (s_gma1_dev_fd < 0) {
			SL_ERR("FATAL: %s open failed\n", DIS_DEV_FB0);
			pthread_mutex_unlock(&m_hd_mutex);
			goto dev_open_failed;
		}
		ioctl(s_gma1_dev_fd, FBIO_SET_DE_LAYER, DE_LAYER2);
		
		s_gma2_dev_fd = open(DIS_DEV_FB2, O_RDWR);
		if (s_gma2_dev_fd < 0) {
			SL_ERR("FATAL: %s open failed\n", DIS_DEV_FB2);
			pthread_mutex_unlock(&m_hd_mutex);
			goto dev_open_failed;
		}
		ioctl(s_gma2_dev_fd, FBIO_SET_DE_LAYER, DE_LAYER3);
		
		m_sd_dev = (struct alisldis_dev *)malloc(sizeof(struct alisldis_dev));
		if (NULL == m_sd_dev) {
			SL_ERR("Not enough memory\n");
			pthread_mutex_unlock(&m_hd_mutex);
			goto no_mem;
		}

		m_hd_dev = (struct alisldis_dev *)malloc(sizeof(struct alisldis_dev));
		if (NULL == m_hd_dev) {
			SL_ERR("Not enough memory\n");
			pthread_mutex_unlock(&m_hd_mutex);
			goto no_mem;
		}

		memset(m_sd_dev, 0, sizeof(struct alisldis_dev));
		m_sd_dev->dev_id = DIS_SD_DEV;
		m_sd_dev->fd_gma1 = s_gma1_dev_fd;
		m_sd_dev->fd_gma2 = s_gma2_dev_fd;
		m_sd_dev->fd_vpo = s_vpo_dev_fd;
		m_sd_dev->mutex = &m_sd_mutex;
		m_sd_dev->video_enhance_max = 100;//defaule range value 100

		memset(m_hd_dev, 0, sizeof(struct alisldis_dev));
		m_hd_dev->dev_id = DIS_HD_DEV;
		m_hd_dev->fd_gma1 = s_gma1_dev_fd;
		m_hd_dev->fd_gma2 = s_gma2_dev_fd;
		m_hd_dev->fd_vpo = s_vpo_dev_fd;
		m_hd_dev->mutex = &m_hd_mutex;
		m_hd_dev->video_enhance_max = 100;//defaule range value 100

		pthread_mutex_unlock(&m_hd_mutex);
	}

    switch(dev_id) {
        case DIS_SD_DEV:
            *hdl   = (alisl_handle)m_sd_dev;
            break;

        case DIS_HD_DEV:
            *hdl   = (alisl_handle)m_hd_dev;
            break;
        default:
            break;
    }

	++ dev_open_cnt;

    return DIS_ERR_NONE;

dev_open_failed:
    if (s_vpo_dev_fd > 0) {
		close(s_vpo_dev_fd);
		s_vpo_dev_fd = VPO_NOT_OPENED;
    }
	if (s_gma1_dev_fd > 0) {
		close(s_gma1_dev_fd);
		s_gma1_dev_fd = GMA1_NOT_OPENED;
	}
	if (s_gma2_dev_fd > 0) {
		close(s_gma2_dev_fd);
		s_gma2_dev_fd = GMA2_NOT_OPENED;
	}

no_mem:
	if (m_sd_dev)
		free(m_sd_dev);
	if (m_hd_dev)
		free(m_hd_dev);
	
    return DIS_ERR_NOMEM;
}

/**
 * Function Name: alisldis_close
 * @brief         Close the VDEC device by handle
 *
 * @param[in] hdl Device handle to be closed
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_close(alisl_handle hdl)
{
	SL_DBG("hdl:%d\n", hdl);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;

    CHECK_HANDLE(hdl);

	if (dev_open_cnt == 0) {
		return DIS_ERR_INVAL;
	}
	
	-- dev_open_cnt;

	if (0 != dev_open_cnt) {
		return DIS_ERR_NONE;
	}

	pthread_mutex_lock(dev->mutex);
	
	if (dev->fd_vpo > 0) {
		close(dev->fd_vpo);
		s_vpo_dev_fd = VPO_NOT_OPENED;
	}
	if (dev->fd_gma1 > 0) {
		close(dev->fd_gma1);
		s_gma1_dev_fd = GMA1_NOT_OPENED;
	}
	if (dev->fd_gma2 > 0) {
		close(dev->fd_gma2);
		s_gma2_dev_fd = GMA2_NOT_OPENED;
	}

	pthread_mutex_unlock(dev->mutex);

	if (m_hd_dev)
		free(m_hd_dev);
	if (m_sd_dev)
		free(m_sd_dev);

	m_hd_dev = NULL;
	m_sd_dev = NULL;

    return DIS_ERR_NONE;
}

/**
 * Function Name: alisldis_win_onoff_by_layer
 * @brief         Close the display by layer
 *
 * @param[in] hdl Device handle
 * @param[in] on Onoff value
 * @param[in] layer Layer to be controlled, currently only DIS_LAYER_MAIN is supported
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_win_onoff_by_layer(alisl_handle   hdl,
                            bool           on,
                            enum dis_layer layer)
{
	SL_DBG("hdl:%d on:%d layer:%d\n", hdl, on, layer);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_vpo_win_status_pars status_param;

    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if ((DIS_LAYER_MAIN == layer) || (DIS_LAYER_AUXP == layer)) {
        status_param.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
        status_param.on     = on ? 1 : 0;
        status_param.layer  = vpo_layer(layer);
    }

    pthread_mutex_lock(dev->mutex);

    if ((DIS_LAYER_MAIN == layer) || (DIS_LAYER_AUXP == layer)) {
    	if (dev->dev_id == DIS_HD_DEV) {
        	VPO_IOCTL(VPO_SET_WIN_ONOFF_EX, &status_param);//only HD support VPO_SET_WIN_ONOFF_EX
        } else {
        	VPO_IOCTL(VPO_SET_WIN_ONOFF, &status_param);
        }
    } else if (DIS_LAYER_GMA1 == layer) {
        GMA1_IOCTL(FBIO_WIN_ONOFF, on);
    } else if (DIS_LAYER_GMA2 == layer) {
    	GMA2_IOCTL(FBIO_WIN_ONOFF, on);
    }

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}


#define COPY_RECT(drv_rect, rect)               \
    do {                                        \
        (drv_rect)->u_start_x = (rect)->x;        \
        (drv_rect)->u_start_y = (rect)->y;        \
        (drv_rect)->u_width  = (rect)->w;        \
        (drv_rect)->u_height = (rect)->h;        \
    } while(0)

/**
 * Function Name: alisldis_zoom_by_layer
 * @brief         Zoom display by layer
 *
 * @param[in] hdl Device handle
 * @param[in] srect Source rectangle
 * @param[in] drect Destination rectangle
 * @param[in] layer Layer to operated, currently only DIS_LAYER_MAIN is supported
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */

alisl_retcode
alisldis_zoom_by_layer(alisl_handle           hdl,
                       struct dis_rect       *srect,
                       struct dis_rect       *drect,
                       enum   dis_layer       layer)
{
	SL_DBG("hdl:%d layer:%d\n", hdl, layer);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_vpo_zoom_pars zoom_pars;
    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (!srect || !drect) {
        SL_ERR("invalidate srect/drect \n");
        return DIS_ERR_INVAL;
    }

    zoom_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    COPY_RECT(&(zoom_pars.src_rect), srect);
    COPY_RECT(&(zoom_pars.dst_rect), drect);
    zoom_pars.layer = vpo_layer(layer);

    pthread_mutex_lock(dev->mutex);
	if (dev->dev_id == DIS_HD_DEV) {
   		VPO_IOCTL(VPO_WIN_ZOOM_EX, &zoom_pars);//only HD support VPO_WIN_ZOOM_EX
   	} else {
   		VPO_IOCTL(VPO_WIN_ZOOM, &zoom_pars);
   	}
quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 * Function Name: alisldis_set_tvsys
 * @brief         Set display TV system.
 *
 * @param[in] hdl Device handle
 * @param[in] tvsys TV system to be set
 * @param[in] progressive Boolean value indicates progressive or not.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_tvsys(alisl_handle   hdl,
                   enum dis_tvsys tvsys,
                   bool           progressive)
{
	SL_DBG("hdl:%d tvsys:%d progressive:%d\n", hdl, tvsys, progressive);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_vpo_tvsys_pars vpotvsys;
    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    vpotvsys.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    vpotvsys.progressive = progressive ? 1 : 0;
    vpotvsys.tvsys = sldis_tvsys(tvsys);

    pthread_mutex_lock(dev->mutex);

    VPO_IOCTL(VPO_SET_TVSYS_EX, &vpotvsys);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 * Function Name: alisldis_get_tvsys
 * @brief         Get TV system DIS module is currently using
 *
 * @param[in] hdl Device handle
 * @param[in] tvsys TV system currently using
 * @param[in] progressive Currently using progressive or not
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_tvsys(alisl_handle    hdl,
                   enum dis_tvsys *tvsys,
                   bool           *progressive)
{
	SL_DBG("hdl:%d tvsys:%p progressive:%p\n", hdl,tvsys,progressive);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_vpo_mp_info_pars info;
    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (!tvsys || !progressive) {
        SL_ERR("invalidate tvsys/progresive prarameter\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);

    info.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_MP_INFO, &info);
    *tvsys       = dis_tvsys_convert(info.mp_info.tvsys);
    *progressive = info.mp_info.bprogressive ? true : false;

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}


/**
 * Function Name: alisldis_set_bgcolor
 * @brief         Set the background color of the DIS moduel
 *
 * @param[in] hdl Device handle
 * @param[in] color Color to be set
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_bgcolor(alisl_handle      hdl,
                     struct dis_color *color)
{
	SL_DBG("hdl:%d color:%d \n",hdl,color);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_vpo_bgcolor_pars bg_color_pars;
    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (!color) {
        SL_ERR("invidate color\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    bg_color_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    bg_color_pars.yuv_color.u_cb = color->cb;
    bg_color_pars.yuv_color.u_cr = color->cr;
    bg_color_pars.yuv_color.u_y  = color->y;

    VPO_IOCTL(VPO_SET_BG_COLOR, &bg_color_pars);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 * Function Name: alisldis_reg_dac
 * @brief         Register DAC to DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] dac_grp DAC parameter to be set
 * @param[in] progressive Boolean value indicating whether DAC is output progressively
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_reg_dac(alisl_handle          hdl,
                 struct dis_dac_group *dac_grp,
                 bool                  progressive)
{
	SL_DBG("hdl:%d \n",hdl);
//	unsigned char flag_cvbs=0;
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode ret = DIS_ERR_NONE;
    struct ali_vpo_dac_pars dac_pars;
    struct ali_vpo_mp_info_pars info;

    CHECK_HANDLE(hdl);

    if (!dac_grp) {
        SL_ERR("dac group is invalidate\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
	
    info.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_MP_INFO, &info);

    dac_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    dac_pars.dac_param.e_dac_type = vpo_dac_type(dac_grp->dac_grp_type);
    dac_pars.dac_param.dac_info.b_enable = 1;
    dac_pars.dac_param.dac_info.e_vgamode = Video_VGA_NOT_USE;
    dac_pars.dac_param.dac_info.b_progressive = progressive ? 1 : 0;

    switch (dac_grp->dac_grp_type) {
        case DIS_DAC_GROUP_YUV1:
        case DIS_DAC_GROUP_YUV2: {
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_first = vpo_dac_id(dac_grp->yuv.y);
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_second = vpo_dac_id(dac_grp->yuv.u);
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_third = vpo_dac_id(dac_grp->yuv.v);
            break;
        }

        case DIS_DAC_GROUP_RGB1:
        case DIS_DAC_GROUP_RGB2: {
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_first = vpo_dac_id(dac_grp->rgb.r);
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_second = vpo_dac_id(dac_grp->rgb.g);
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_third = vpo_dac_id(dac_grp->rgb.b);
            break;
        }

        case DIS_DAC_GROUP_CVBS1:
        case DIS_DAC_GROUP_CVBS2:
        case DIS_DAC_GROUP_CVBS3:
        case DIS_DAC_GROUP_CVBS4:
        case DIS_DAC_GROUP_CVBS5:
        case DIS_DAC_GROUP_CVBS6: {
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_first = vpo_dac_id(dac_grp->cvbs);
			//flag_cvbs = 1;
            break;
        }

        case DIS_DAC_GROUP_SVIDEO1: {
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_first = vpo_dac_id(dac_grp->svideo.y);
            dac_pars.dac_param.dac_info.t_dac_index.u_dac_second = vpo_dac_id(dac_grp->svideo.c);
            break;
        }
    }

    VPO_IOCTL(VPO_REG_DAC, &dac_pars);

quit:
    pthread_mutex_unlock(dev->mutex);
    usleep(50*1000);
    return ret;
}

/**
 * Function Name: alisldis_unreg_dac
 * @brief         Unregister DAC to DIS
 *
 * @param[in] hdl Device handle
 * @param[in] dac_grp DAC group to be unregistered
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_unreg_dac(alisl_handle            hdl,
                   enum dis_dac_group_type dac_grp)
{
    struct alisldis_dev        *dev = (struct alisldis_dev *)hdl;
    alisl_retcode               ret = DIS_ERR_NONE;
    struct ali_vpo_ioctrl_pars  dis_ioctl;
	int i = 0;

    CHECK_HANDLE(hdl);

    if (dac_grp == DIS_DAC_GROUP_CVBS1){
        for(i=0;i<4;i++){
            pthread_mutex_lock(dev->mutex);
            dis_ioctl.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
            dis_ioctl.param  = Video_CVBS_1+i;
            VPO_IOCTL(VPO_UNREG_DAC, &dis_ioctl);
            pthread_mutex_unlock(dev->mutex);
            /* Need to sleep 50ms after every VPO_IOCTL, 
               or the unregister maybe not compelted. */
            usleep(50*1000);
        }
    }else{
        pthread_mutex_lock(dev->mutex);
        dis_ioctl.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
        dis_ioctl.param  = vpo_dac_type(dac_grp);
        VPO_IOCTL(VPO_UNREG_DAC, &dis_ioctl);
        pthread_mutex_unlock(dev->mutex);
        usleep(50*1000);
    }
    
quit:
    if (ret == DIS_ERR_IOCTLFAILED)
        pthread_mutex_unlock(dev->mutex);
    return ret;
}


/**
 * Function Name: alisldis_set_enhance
 * @brief         Set enhancement of display
 *
 * @param[in] hdl Device handle
 * @param[in] enhance Enhance paramter
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_enhance(alisl_handle        hdl,
                     struct dis_enhance *enhance)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_vpo_video_enhance_pars vpo_enhance;
    alisl_retcode    ret  = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (!enhance) {
        SL_ERR("Invlidate enhance\n");
        return DIS_ERR_INVAL;
    }
	dis_enhance_type type = DIS_ENHANCE_ALL & enhance->type;

	if (DIS_SD_DEV == dev->dev_id) {
		return DIS_ERR_NONE;
	}

    pthread_mutex_lock(dev->mutex);
#if 0
    SL_DBG("type: %d, dev_id = %d, brightness = %d, saturation = %d, constrast = %d, sharpeness = %d, hue = %d\n",
        type, dev->dev_id, enhance->brightness,
		enhance->saturation, enhance->contrast, enhance->sharpeness,
		enhance->hue);
#endif
#define enhance_ioctl(enhance_type, vpo_type, val, min, max)            \
    do {                                                                \
        if (type & enhance_type) {                                      \
            memset(&vpo_enhance, 0, sizeof(vpo_enhance));               \
            vpo_enhance.hd_dev  = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;  \
            vpo_enhance.video_enhance_param.changed_flag  = vpo_type;   \
            vpo_enhance.video_enhance_param.grade = val % (max + 1);    \
            vpo_enhance.video_enhance_param.grade_range_min = min;      \
            vpo_enhance.video_enhance_param.grade_range_max = max;      \
                                                                       \
            if (ioctl(dev->fd_vpo, VPO_VIDEO_ENHANCE, &vpo_enhance) < 0) { \
                SL_ERR("ioctl failed\n");          \
                ret = DIS_ERR_IOCTLFAILED;                              \
                goto quit;                                              \
            }                                                           \
        }                                                               \
    } while(0)

    enhance_ioctl(DIS_ENHANCE_BRIGHTNESS, VPO_IO_SET_ENHANCE_BRIGHTNESS, enhance->brightness, 
        0, dev->video_enhance_max);
    enhance_ioctl(DIS_ENHANCE_SATURATION, VPO_IO_SET_ENHANCE_SATURATION, enhance->saturation, 
        0, dev->video_enhance_max);
    enhance_ioctl(DIS_ENHANCE_CONTRAST, VPO_IO_SET_ENHANCE_CONTRAST, enhance->contrast, 
        0, dev->video_enhance_max);
    //sharpness can not be changed, only range 0-10
    enhance_ioctl(DIS_ENHANCE_SHARPNESS, VPO_IO_SET_ENHANCE_SHARPNESS, enhance->sharpeness, 
        0, 10);
    enhance_ioctl(DIS_ENHANCE_HUE, VPO_IO_SET_ENHANCE_HUE, enhance->hue, 
        0, dev->video_enhance_max);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}



/**
 * Function Name: alisldis_set_aspect_mode
 * @brief         Set aspect mode of display
 *
 * @param[in] hdl Device handle
 * @param[in] display_mode Display mode paramter
 * @param[in] aspect_mode Aspect mode
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_aspect_mode(alisl_handle          hdl,
                         enum dis_display_mode display_mode,
                         enum dis_aspect_ratio aspect_mode)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;

    struct ali_vpo_aspect_pars aspect_par;
//    enum tvmode tv_mode;

    CHECK_HANDLE(hdl);

    pthread_mutex_lock(dev->mutex);

    aspect_par.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    aspect_par.aspect = vpo_tvmode(aspect_mode);
    aspect_par.display_mode = vpo_display_mode(display_mode);

    VPO_IOCTL(VPO_SET_ASPECT_MODE, &aspect_par);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 * Function Name: alisldis_get_mp_screen_rect
 * @brief         Get show effective rectangle of display
 *
 * @param[in] hdl Device handle
 * @param[in] ret_rect Rectangle returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_mp_screen_rect(alisl_handle     hdl,
                            struct dis_rect *ret_rect)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_screem_rect_pars screem_rect_pars;

    CHECK_HANDLE(hdl);
    if (!ret_rect) {
        SL_ERR("invalidate ret_rect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    screem_rect_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_MP_SCREEN_RECT, &screem_rect_pars);

    ret_rect->x = screem_rect_pars.mp_screem_rect.u_start_x;
    ret_rect->y = screem_rect_pars.mp_screem_rect.u_start_y;
    ret_rect->w = screem_rect_pars.mp_screem_rect.u_width;
    ret_rect->h = screem_rect_pars.mp_screem_rect.u_height;

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}


/**
 * Function Name: alisldis_get_mp_info
 * @brief         Get MP info
 *
 * @param[in] hdl Device handle
 * @param[in] info Information return to caller.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_mp_info(alisl_handle     hdl,
                     struct dis_info *info)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_mp_info_pars mp_info_pars;

    CHECK_HANDLE(hdl);
    if (!info) {
        SL_ERR("invalidate ret_rect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    mp_info_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_MP_INFO, &mp_info_pars);

    //SL_ERR("width = %d\n", mp_info_pars.mp_info.des_height);
    info->progressive   = mp_info_pars.mp_info.bprogressive;
    info->dst_width     = mp_info_pars.mp_info.des_width;
    info->dst_height    = mp_info_pars.mp_info.des_height;
    info->source_width  = mp_info_pars.mp_info.source_width;
    info->source_height = mp_info_pars.mp_info.source_height;
    info->gma1_onoff    = mp_info_pars.mp_info.gma1_onoff;
    //SL_ERR("tvsys = %p\n", mp_info_pars.mp_info.tvsys);
    info->tvsys         = dis_tvsys_convert(mp_info_pars.mp_info.tvsys);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}

/**
 * Function Name: alisldis_get_real_display_mode
 * @brief         Get real display mode
 *
 * @param[in] hdl Device handle
 * @param[out] disp_mode Display mode returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_real_display_mode(alisl_handle           hdl,
                               enum dis_display_mode *disp_mode)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);
    if (!disp_mode) {
        SL_ERR("invalidate ret_rect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_REAL_DISPLAY_MODE, &ioctrl_pars);

    SL_ERR("display_mode = %x\n", ioctrl_pars.param);
    *disp_mode = dis_display_mode(ioctrl_pars.param);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}



/**
 * Function Name: alisldis_real_display_mode
 * @brief         Get real display mode
 *
 * @param[in] hdl Device handle
 * @param[out] disp_mode Display mode returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_display_mode(alisl_handle           hdl,
                          enum dis_display_mode *disp_mode)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);
    if (!disp_mode) {
        SL_ERR("invalidate ret_rect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_DISPLAY_MODE, &ioctrl_pars);
    *disp_mode = dis_display_mode(ioctrl_pars.param);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}

/**
 * Function Name: alisldis_get_aspect
 * @brief         Get aspect ration
 *
 * @param[in] hdl Device handle
 * @param[out] aspect Aspect return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_aspect(alisl_handle           hdl,
                    enum dis_aspect_ratio *aspect)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);
    if (!aspect) {
        SL_ERR("invalidate aspect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_TV_ASPECT, &ioctrl_pars);
    switch (ioctrl_pars.param) {
        case TV_16_9:
            *aspect = DIS_AR_16_9;
            break;
        case TV_4_3:
            *aspect = DIS_AR_4_3;
            break;
        case TV_AUTO:
            *aspect = DIS_AR_AUTO;
            break;
        default:
            SL_ERR("Unexpected TV ASPECT = 0x%08x\n", ioctrl_pars.param);
            ret = DIS_ERR_INVAL;
    }

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}

/**
 * Function Name: alisldis_get_src_aspect
 * @brief         Get source apsect
 *
 * @param[in] hdl Device handle
 * @param[in] aspect Aspect returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_src_aspect(alisl_handle           hdl,
                        enum dis_aspect_ratio *aspect)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);
    if (!aspect) {
        SL_ERR("invalidate aspect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_SRC_ASPECT, &ioctrl_pars);
    switch (ioctrl_pars.param) {
        case TV_16_9:
            *aspect = DIS_AR_16_9;
            break;
        case TV_4_3:
            *aspect = DIS_AR_4_3;
            break;
        case TV_AUTO:
            *aspect = DIS_AR_AUTO;
            break;
        default:
            SL_ERR("Unexpected TV ASPECT = 0x%08x\n", ioctrl_pars.param);
            ret = DIS_ERR_INVAL;
    }

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}

/**
 * Function Name: alisldis_backup_picture
 * @brief         Backup picture before channel change
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note          Must call alisldis_free_backup_picture() to release the backuped pitcture.
 */
alisl_retcode
alisldis_backup_picture(alisl_handle hdl, dis_change_chane_mode mode)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;

    struct ali_vpo_display_info_pars display_info_pars;

    CHECK_HANDLE(hdl);

    if (DIS_SD_DEV == dev->dev_id)
        return DIS_ERR_NONE;

    pthread_mutex_lock(dev->mutex);

	memset(&display_info_pars, 0, sizeof(display_info_pars));
    display_info_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
	display_info_pars.display_info.sw_hw=0;
	display_info_pars.display_info.de_index=0;  //must to be set 0, other value will be return fail in driver
	display_info_pars.display_info.reserved[0] = (mode ==DIS_CHG_STILL ) ? 0 : 1;
    VPO_IOCTL(VPO_BACKUP_CURRENT_PICTURE, &display_info_pars);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 * Function Name: alisldis_free_backup_picture
 * @brief         Free the backuped picture, used with alisldis_backup_picture()
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
alisldis_free_backup_picture(alisl_handle  hdl)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (DIS_SD_DEV == dev->dev_id)
        return DIS_ERR_NONE;

    pthread_mutex_lock(dev->mutex);

    VPO_IOCTL(VPO_FREE_BACKUP_PICTURE, NULL);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}


/**
 * Function Name: alisldis_win_mode
 * @brief         Set display win mode
 *
 * @param[in] hdl Device handle
 * @param[in] win_mode Win mode type to set to DIS
 * @param[in] mpcb Opaque MP callback from VCEC
 * @param[in] pipcb Opaque PIP callback from VDEC
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_win_mode(alisl_handle       hdl,
                  enum dis_win_mode  win_mode,
                  uint32_t          *mpcb,
                  uint32_t          *pipcb)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_winmode_pars win_mode_param;
    uint32_t vpo_win_mode;

    CHECK_HANDLE(hdl);

    if (!mpcb || !pipcb) {
        SL_ERR("Invalidate argument\n");
        return DIS_ERR_INVAL;
    }

#define VPO_MAINWIN 0x01
#define VPO_PIPWIN  0x02
#define VPO_PREVIEW 0x04

    switch (win_mode) {
        case DIS_WIN_MODE_MAINMAIN:
            vpo_win_mode = VPO_MAINWIN;
            break;
        case DIS_WIN_MODE_PIPWIN:
            vpo_win_mode = VPO_PIPWIN;
            break;
        case DIS_WIN_MODE_PREVIEW:
            vpo_win_mode = VPO_PREVIEW;
            break;
        default:
            SL_ERR("Unexpected WIN_MODE = 0x%08x\n", win_mode);
            return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);

    win_mode_param.hd_dev       = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    win_mode_param.win_mode     = vpo_win_mode;
    win_mode_param.mp_callback  = *((struct mpsource_call_back *)mpcb);
    win_mode_param.pip_callback = *((struct pipsource_call_back *)pipcb);

    VPO_IOCTL(VPO_SET_WIN_MODE, &win_mode_param);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}


/**
 * Function Name: alisldis_set_attr
 * @brief         Set attribute of DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] attr_type Attribute type
 * @param[in] attr_val Attribute value
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_attr(alisl_handle hdl,
                  dis_attr attr_type,
                  uint32_t attr_val)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    uint32_t iocmd;
//	uint32_t attr;

    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);

    pthread_mutex_lock(dev->mutex);
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    iocmd = vpo_attr_to_iocmd(attr_type);
    ioctrl_pars.param  = vpo_param_to_ioparam(attr_type, attr_val);
    if((-1 == iocmd) || (-1 == ioctrl_pars.param)){
        SL_ERR("Invalid parameters!\n");
        pthread_mutex_unlock(dev->mutex);
        return -1;
    }
    VPO_IOCTL(iocmd, &ioctrl_pars);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}



/**
 * Function Name: alisldis_get_attr
 * @brief         Set attribute of DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] attr_type Attribute type
 * @param[in] attr_val Attribute value
 *
 * @return        alisl_retcode
 *
 * @author        raynard Wang <raynard.wang@alitech.com>
 * @date          14/04/2014, Creatd
 *
 * @note
 */
alisl_retcode
alisldis_get_attr(alisl_handle hdl,
                  dis_attr attr_type,
                  uint32_t *attr_addr)
{
	SL_DBG("hdl:%d attr_type:%d \n",hdl,attr_type);
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    uint32_t iocmd;

    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);

    pthread_mutex_lock(dev->mutex);
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    iocmd = vpo_attr_to_iocmd(attr_type);
    //ioctrl_pars.param  = *attr_addr;
    VPO_IOCTL(iocmd, &ioctrl_pars);
	*attr_addr = ioctrl_pars.param;

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 *  @brief          scale the gma layer
 *
 *  @param[in]      hdl             alisldis module handle
 *  @param[in]      scale_param     scale parameters
 *  @param[in]      layer           layer type
 *
 *  @return         alisl_retcode
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/14/2014  14:42:57
 *
 *  @note
 */
alisl_retcode
alisldis_gma_scale(alisl_handle              hdl,
                   const struct scale_param *scale_param,
                   enum   dis_layer          layer)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode ret = DIS_ERR_NONE;
    struct alifbio_gma_scale_info_pars scale_info;
    gma_scale_param_t scale_pars;
    enum dis_tvsys tvsys;
    bool progressive;

    CHECK_HANDLE(hdl);

    if (!scale_param) {
        SL_ERR("invalidate srect/drect \n");
        return DIS_ERR_INVAL;
    }

    if (DIS_ERR_NONE != (ret = alisldis_get_tvsys(hdl, &tvsys, &progressive))) {
        return ret;
    }

    pthread_mutex_lock(dev->mutex);

    ///step1. set mode.
    scale_pars.tv_sys = sldis_tvsys(tvsys);
    scale_pars.h_mul = scale_param->h_dst;
    scale_pars.h_div = scale_param->h_src;
    scale_pars.v_mul = scale_param->v_dst;
    scale_pars.v_div = scale_param->v_src;
    /* Driver will do filtering processing, make the image more smooth.*/
    scale_info.scale_mode = GMA_RSZ_ALPHA_COLOR; 
    //scale_info.scale_mode = GMA_RSZ_DIRECT_RESIZE;
    scale_info.tv_sys = sldis_tvsys(tvsys);
    scale_info.h_dst = scale_param->h_dst;
    scale_info.h_src = scale_param->h_src;
    scale_info.v_dst = scale_param->v_dst;
    scale_info.v_src = scale_param->v_src;
    scale_info.uScaleCmd = GE_SET_SCALE_MODE;
    scale_info.uScaleParam = (unsigned long)&scale_pars; 
    // Support_Flow #52886, handle GMA1 and GMA2 separately
    if (DIS_LAYER_GMA1 == layer) {
        GMA1_IOCTL(FBIO_SET_GMA_SCALE_INFO, &scale_info);
    } else if (DIS_LAYER_GMA2 == layer) {
    	GMA2_IOCTL(FBIO_SET_GMA_SCALE_INFO, &scale_info);
    }
	
    ///step2. set scale param.
    /* Driver will do filtering processing, make the image more smooth.*/
    scale_info.scale_mode = GMA_RSZ_ALPHA_COLOR;
    //scale_info.scale_mode = GMA_RSZ_DIRECT_RESIZE;
    scale_info.uScaleCmd = GE_SET_SCALE_PARAM;
    scale_info.uScaleParam = (unsigned long)&scale_pars;
    // Support_Flow #52886, handle GMA1 and GMA2 separately
    if (DIS_LAYER_GMA1 == layer) {
        GMA1_IOCTL(FBIO_SET_GMA_SCALE_INFO, &scale_info);
    } else if (DIS_LAYER_GMA2 == layer) {
    	GMA2_IOCTL(FBIO_SET_GMA_SCALE_INFO, &scale_info);
    }

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 *  @brief          set a global alpha for the specified layer
 *
 *  @param[in]      hdl                 display module handle
 *  @param[in]      layer               layer id
 *  @param[in]      global_alhpa        global alpha value
 *
 *  @return         alisl_retcode
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/14/2014  14:49:15
 *
 *  @note			Only GMA1 layer is supported right now.
 */
alisl_retcode
alisldis_set_global_alpha_by_layer(alisl_handle   hdl,
                                   enum dis_layer layer,
                                   unsigned char  global_alpha)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (DIS_LAYER_GMA1 != layer) {
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);

    GMA1_IOCTL(FBIO_SET_GLOBAL_ALPHA, global_alpha);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

/**
 *  @brief          anti-flicker switcher
 *
 *  @param[in]      hdl     display module handle
 *  @param[in]      b_on    set anti-flicker on/off
 *
 *  @return         alisl_retcode
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           3/20/2014  18:7:23
 *
 *  @note
 */

alisl_retcode
alisldis_anti_flicker_onoff(alisl_handle hdl,
                            unsigned char b_on)
{
	struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
	alisl_retcode ret = DIS_ERR_NONE;
	struct ali_vpo_ioctrl_pars ioctrl_pars;
	uint32 value = 0;
	
	CHECK_HANDLE(hdl);
	    	
	pthread_mutex_lock(dev->mutex);

	//GMA1_IOCTL(FBIO_SET_GMA_ANTIFLICK, b_on); //not used
	if (1 == b_on) {
		value = 0x00caf080;
		ioctrl_pars.param = value;
		GMA1_IOCTL(VPO_SET_ANTIFLICK_THR, &ioctrl_pars);
		value = ((0<<3) | (1<<2) | (0<<1) | 0);
	}
	ioctrl_pars.param = value;
	GMA1_IOCTL(VPO_ENABLE_ANTIFLICK, &ioctrl_pars);

quit:
	pthread_mutex_unlock(dev->mutex);
	return ret;
}
#if 0
/* for HLD */
static int
alisldis_dev_fd(alisl_handle hdl)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;

    if (!hdl ||
        ((alisl_handle)hdl != (alisl_handle) m_hd_dev) ||
        ((alisl_handle)hdl != (alisl_handle) m_sd_dev)) {
        SL_ERR("invalidate handle = 0x%08x\n", (uint32_t)hdl);
        return -1;
    }

//quit:
    return dev->fd_vpo;
}
#endif
/**
 * @brief set the blending order of fb0 fb1 and fb2
 * @param[in] hdl shared lib handle
 * @param[in] order the order define in enum VP_LAYER_BLEND_ORDER
 * @return alisl_retcode
 */ 
alisl_retcode alisldis_set_fb_order(alisl_handle hdl, int order) 
{
	CHECK_HANDLE(hdl);
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
	struct ali_vpo_ioctrl_pars ioctrl_pars;
	
	memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
	
	ioctrl_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
	ioctrl_pars.param = order;
    if (ioctl(handle->fd_vpo, VPO_SET_LAYER_ORDER, &ioctrl_pars)) {
        return DIS_ERR_IOCTLFAILED;
    }
    return DIS_ERR_NONE;
}

/*
 * @set 3D parameters
 * @param[in] hdl shared lib handle
 * @param[in] dis 3d parameters
 * @return alisl_retcode
 * @ Note:
 */ 
alisl_retcode alisldis_set_3d(alisl_handle hdl, struct dis_3d_attr *dis_3d_paras) 
{
	CHECK_HANDLE(hdl);
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
	struct ali_vpo_ioctrl_pars ioctrl_pars;
	struct alifbio_3d_pars fb_3d_paras;
	
	memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
	memset((void *)&fb_3d_paras, 0, sizeof(fb_3d_paras));
	
	fb_3d_paras.depth_2d_to_3d = dis_3d_paras->depth_2d_to_3d;
	fb_3d_paras.display_2d_to_3d = dis_3d_paras->display_2d_to_3d;
	fb_3d_paras.display_3d_enable = dis_3d_paras->display_3d_enable;
	fb_3d_paras.eInputFormat = dis_3d_paras->eInputSource;
	fb_3d_paras.mode_2d_to_3d = dis_3d_paras->mode_2d_to_3d;
	fb_3d_paras.red_blue = dis_3d_paras->red_blue;
	fb_3d_paras.side_by_side = dis_3d_paras->side_by_side;
	fb_3d_paras.top_and_bottom = dis_3d_paras->top_and_bottom;

	ioctrl_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
	ioctrl_pars.param = (uint32)&fb_3d_paras;
		
	if (ioctl(handle->fd_vpo, VPO_DISPLAY_3D_ENABLE, &ioctrl_pars)) {
        return DIS_ERR_IOCTLFAILED;
    }
    return DIS_ERR_NONE;
}

/**
 * Function Name: alisldis_get_boot_media_status
 * @brief         check boot show media if finish
 *
 * @param[in] hdl Device handle
 * @param[out] show media status  0:show media not finish, 1:show media finish
 *
 * @return        alisl_retcode
 *
 * @author        Vedic.Fu <vedic.fu@alitech.com>
 * @date          2/6/2015, Created
 *
 * @note just support HD/DEN handler
 */
alisl_retcode alisldis_get_boot_media_status(alisl_handle hdl, int *status)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    CHECK_HANDLE(hdl);
    if (NULL == status) {
        SL_ERR("status point is NULL!\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
	memset(&ioctrl_pars, 0, sizeof(ioctrl_pars));
    ioctrl_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_BOOT_LOGO_STATUS, &ioctrl_pars);/* VPO_IO_* is used by driver,app used VPO_* */

	SL_DBG("\n before status = %d\n", *status);
    *status = ioctrl_pars.param;
	SL_DBG("\nafter status = %d\n", *status);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}

/*
 * @set afd parameters
 * @param[in] hdl shared lib handle
 * @param[in] dis afd parameters
 * @return alisl_retcode
 * @ Note:
 */ 
alisl_retcode alisldis_set_afd(alisl_handle hdl, struct dis_afd_attr *dis_afd_paras) 
{
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
	struct ali_vpo_afd_pars ioctrl_pars;

    if (NULL == hdl || NULL == dis_afd_paras) {
        SL_ERR("hdl/dis_afd_paras point is NULL!\n");
        return DIS_ERR_INVAL;
    }
	
	memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
	pthread_mutex_lock(handle->mutex);
	ioctrl_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
	ioctrl_pars.afd_param.afd_solution = dis_afd_paras->afd_solution;
	ioctrl_pars.afd_param.b_swscale_afd = dis_afd_paras->bSwscaleAfd;
	ioctrl_pars.afd_param.protect_mode_enable = dis_afd_paras->protect_mode_enable;
		
	if (ioctl(handle->fd_vpo, VPO_AFD_CONFIG, &ioctrl_pars)) {
		SL_ERR("ioctl VPO_AFD_CONFIG Error!\n");		
		pthread_mutex_unlock(handle->mutex);
		return DIS_ERR_IOCTLFAILED;
	}
	
	pthread_mutex_unlock(handle->mutex);
    return DIS_ERR_NONE;
}

/**
 * Function Name: alisldis_set_enhance_osd
 * @brief         Set enhancement of display osd layer
 *
 * @param[in] hdl Device handle
 * @param[in] enhance Enhance paramter
 *
 * @return        alisl_retcode
 *
 * @author        Vedic.fu <Vedic.Fu@alitech.com>
 * @date          7/8/2015, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_enhance_osd(alisl_handle        hdl,
                     struct dis_enhance *enhance)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    struct ali_fb_video_enhance_pars osd_enhance;
    alisl_retcode    ret  = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);

    if (!enhance) {
        SL_ERR("Invlidate enhance\n");
        return DIS_ERR_INVAL;
    }
	dis_enhance_type type = DIS_ENHANCE_ALL & enhance->type;

	if (DIS_SD_DEV == dev->dev_id) {
		return DIS_ERR_NONE;
	}

    pthread_mutex_lock(dev->mutex);
#if 0
    SL_DBG("dev_id = %d, brightness = %d, saturation = %d, constrast = %d, sharpeness = %d, hue = %d\n", dev->dev_id, enhance->brightness,
		enhance->saturation, enhance->contrast, enhance->sharpeness,
		enhance->hue);
#endif
#define enhance_ioctl_osd(enhance_type, vpo_type, val)                      \
    do {                                                                \
        if (type & enhance_type) {                                      \
            osd_enhance.changed_flag = vpo_type;   \
            osd_enhance.grade = val % 100;          \
                                                                        \
            if (ioctl(dev->fd_gma1, FBIO_VIDEO_ENHANCE, &osd_enhance) < 0) { \
                SL_ERR("ioctl failed\n");          \
                ret = DIS_ERR_IOCTLFAILED;                              \
                goto quit;                                              \
            }                                                           \
        }                                                               \
    } while(0)

    enhance_ioctl_osd(DIS_ENHANCE_BRIGHTNESS, FBIO_SET_ENHANCE_BRIGHTNESS, enhance->brightness);
    enhance_ioctl_osd(DIS_ENHANCE_SATURATION, FBIO_SET_ENHANCE_SATURATION, enhance->saturation);
    enhance_ioctl_osd(DIS_ENHANCE_CONTRAST, FBIO_SET_ENHANCE_CONTRAST, enhance->contrast);
    enhance_ioctl_osd(DIS_ENHANCE_SHARPNESS, FBIO_SET_ENHANCE_SHARPNESS, enhance->sharpeness);
    enhance_ioctl_osd(DIS_ENHANCE_HUE, FBIO_SET_ENHANCE_HUE, enhance->hue);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}

alisl_retcode alisldis_exchange_video_layer(alisl_handle hdl)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
	struct ali_vpo_ioctrl_pars ioctrl_pars;
    alisl_retcode ret = DIS_ERR_NONE;

    CHECK_HANDLE(hdl);
    memset(&ioctrl_pars, 0, sizeof(ioctrl_pars));
    ioctrl_pars.hd_dev = 1;//can only support HD right now
    
    pthread_mutex_lock(dev->mutex);

    VPO_IOCTL(VPO_EXCHANGE_VIDEO_LAYER, &ioctrl_pars);

quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;
}


alisl_retcode
alisldis_deo_control_onoff(alisl_handle hdl, bool on)
{
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
	struct ali_vpo_ioctrl_pars ioctrl_pars;

    if (NULL == hdl) {
        SL_ERR("hdl is NULL!\n");
        return DIS_ERR_INVAL;
    }
	
	memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
	pthread_mutex_lock(handle->mutex);
	ioctrl_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
	ioctrl_pars.param = on ? 1 : 0;
		
	if (ioctl(handle->fd_vpo, VPO_CLOSE_DEO, &ioctrl_pars)) {
		SL_ERR("ioctl VPO_CLOSE_DEO Error!\n");		
		pthread_mutex_unlock(handle->mutex);
		return DIS_ERR_IOCTLFAILED;
	}
	
	pthread_mutex_unlock(handle->mutex);
    return DIS_ERR_NONE;
}

alisl_retcode alisldis_set_enhance_max_value(alisl_handle hdl, unsigned int max)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode ret = DIS_ERR_NONE;
    CHECK_HANDLE(hdl);

    //range can't not be 0, the driver which store lenth of range is 16bits
    if (max == 0 || max > 0xFFFF) {
        return DIS_ERR_INVAL;
    }
        
    dev->video_enhance_max = max;    
    return ret;
}

alisl_retcode
alisldis_get_mp_source_rect(alisl_handle     hdl,
                            struct dis_rect *ret_rect)
{
    struct alisldis_dev *dev = (struct alisldis_dev *)hdl;
    alisl_retcode        ret = DIS_ERR_NONE;
    struct ali_vpo_screem_rect_pars screem_rect_pars;

    CHECK_HANDLE(hdl);
    if (!ret_rect) {
        SL_ERR("invalidate ret_rect\n");
        return DIS_ERR_INVAL;
    }

    pthread_mutex_lock(dev->mutex);
    screem_rect_pars.hd_dev = (dev->dev_id == DIS_HD_DEV) ? 1 : 0;
    VPO_IOCTL(VPO_GET_MP_SOURCE_RECT, &screem_rect_pars);

    ret_rect->x = screem_rect_pars.mp_screem_rect.u_start_x;
    ret_rect->y = screem_rect_pars.mp_screem_rect.u_start_y;
    ret_rect->w = screem_rect_pars.mp_screem_rect.u_width;
    ret_rect->h = screem_rect_pars.mp_screem_rect.u_height;
			    
quit:
    pthread_mutex_unlock(dev->mutex);
    return ret;

}

alisl_retcode
alisldis_set_afd_scale_mode(alisl_handle  hdl,
                            unsigned char enable)
{
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    if (NULL == hdl) {
        SL_ERR("hdl is NULL!\n");
        return DIS_ERR_INVAL;
    }

    memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
    pthread_mutex_lock(handle->mutex);
    ioctrl_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
    ioctrl_pars.param = enable ? 1 : 0;

    if (ioctl(handle->fd_vpo, VPO_SELECT_SCALE_MODE, &ioctrl_pars)) {
        SL_ERR("ioctl VPO_SELECT_SCALE_MODE Error!\n");		
        pthread_mutex_unlock(handle->mutex);
        return DIS_ERR_IOCTLFAILED;
    }

    pthread_mutex_unlock(handle->mutex);
    return DIS_ERR_NONE;
}

alisl_retcode
alisldis_set_cgms_aps(alisl_handle  hdl,
                      struct dis_cgms_aps_info *cgms_aps)
{
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
    struct ali_vpo_cgms_info_pars cgms_info_pars;
    struct ali_vpo_ioctrl_pars ioctrl_pars;

    if (NULL == hdl || cgms_aps == NULL) {
        SL_ERR("hdl/cgms_aps point is NULL!\n");
        return DIS_ERR_INVAL;
    }
    
    memset((void *)&cgms_info_pars, 0, sizeof(cgms_info_pars));
    memset((void *)&ioctrl_pars, 0, sizeof(ioctrl_pars));
    
    pthread_mutex_lock(handle->mutex);
    cgms_info_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
    cgms_info_pars.cgms_info.cgms = cgms_aps->cgms_a;
    cgms_info_pars.cgms_info.aps = cgms_aps->aps;
    
    ioctrl_pars.hd_dev = (handle->dev_id == DIS_HD_DEV) ? 1 : 0;
    ioctrl_pars.param = 1;

    if (ioctl(handle->fd_vpo, VPO_ALWAYS_OPEN_CGMS_INFO, &ioctrl_pars)) {
        SL_ERR("ioctl VPO_ALWAYS_OPEN_CGMS_INFO Error!\n");		
        pthread_mutex_unlock(handle->mutex);
        return DIS_ERR_IOCTLFAILED;
    }
    
    if (ioctl(handle->fd_vpo, VPO_SET_CGMS_INFO, &cgms_info_pars)) {
        SL_ERR("ioctl VPO_SET_CGMS_INFO Error!\n");		
        pthread_mutex_unlock(handle->mutex);
        return DIS_ERR_IOCTLFAILED;
    }
   
    pthread_mutex_unlock(handle->mutex);
    return DIS_ERR_NONE;
}                      

/**
 * Function Name: alisldis_get_dac_configure
 * @brief         get dac configured
 *
 * @param[in] hdl Device handle
 * @param[in] dis_dac_info : all configured dac information
 * @param[in] 
 *
 * @return        alisl_retcode
 *
 * @author       
 * @date          
 *
 * @note
 */
alisl_retcode
alisldis_get_dac_configure(alisl_handle hdl,
                  dis_dac_info *p_dac_info)
{
    struct alisldis_dev *handle = (struct alisldis_dev *)hdl;
    struct ali_vpo_dac_state_pars dac_pars;

    if (NULL == hdl || p_dac_info == NULL) {
        SL_ERR("hdl p_dac_info point is NULL!\n");
        return DIS_ERR_INVAL;
    }
    memset(&dac_pars, 0 ,sizeof(struct ali_vpo_dac_state_pars));
    dac_pars.hd_dev = handle->dev_id;
    pthread_mutex_lock(handle->mutex);
    if (ioctl(handle->fd_vpo,  VPO_GET_DAC_INFO, &dac_pars)) {
        SL_ERR("ioctl VPO_GET_DAC_INFO Error!\n");		
        pthread_mutex_unlock(handle->mutex);
        return DIS_ERR_IOCTLFAILED;
    }
    SL_DBG("dac_pars.dac0_signal_type = %d\n",dac_pars.dac_state.dac0_signal_type);
    SL_DBG("dac_pars.dac1_signal_type = %d\n",dac_pars.dac_state.dac1_signal_type);
    SL_DBG("dac_pars.dac2_signal_type = %d\n",dac_pars.dac_state.dac2_signal_type);
    SL_DBG("dac_pars.dac3_signal_type = %d\n",dac_pars.dac_state.dac3_signal_type);
    if(1 == dac_pars.dac_state.dac0_enabled) {
        p_dac_info->dac_type[0] = vpo_dac_type_convert(dac_pars.dac_state.dac0_signal_type);
    } else {
        SL_DBG("This dac is not registered. dac_pars.dac0_signal_type = %d\n",dac_pars.dac_state.dac0_signal_type);
        p_dac_info->dac_type[0] = DIS_TYPE_NONE;
    }

    if(1 == dac_pars.dac_state.dac1_enabled) {
        p_dac_info->dac_type[1] = vpo_dac_type_convert(dac_pars.dac_state.dac1_signal_type);
    } else {
        SL_DBG("This dac is not registered. dac_pars.dac0_signal_type = %d\n",dac_pars.dac_state.dac1_signal_type);
        p_dac_info->dac_type[1] = DIS_TYPE_NONE;
    }

    if(1 == dac_pars.dac_state.dac2_enabled) {
        p_dac_info->dac_type[2] = vpo_dac_type_convert(dac_pars.dac_state.dac2_signal_type);
    } else {
        SL_DBG("This dac is not registered. dac_pars.dac0_signal_type = %d\n",dac_pars.dac_state.dac2_signal_type);
        p_dac_info->dac_type[2] = DIS_TYPE_NONE;
    }

    if(1 == dac_pars.dac_state.dac3_enabled) {
        p_dac_info->dac_type[3] = vpo_dac_type_convert(dac_pars.dac_state.dac3_signal_type);
    } else {
        SL_DBG("This dac is not registered. dac_pars.dac0_signal_type = %d\n",dac_pars.dac_state.dac3_signal_type);
        p_dac_info->dac_type[3] = DIS_TYPE_NONE;
    }
    pthread_mutex_unlock(handle->mutex);

    return DIS_ERR_NONE;
}


