#include "internal.h"
#include "alipltflog.h"
#include "alipltfintf.h"

#undef INT64_MAX
#undef VDEC_IO_SET_MODULE_INFO

#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>

#include <ali_video_common.h>
#include <alidefinition/adf_decv.h>

static struct alislvdec_dev *m_dev[VDEC_NB_VIDEO] = {0};// remove

//default MP fullvidew, PIP 1/4 preview to avoid de underrun for multi process
static struct vdec_display_setting m_display_setting[VDEC_DIS_LAYER_MAX] = {
							[0].out_mode = VDEC_OUT_FULLVIEW,
							[0].srect.x = 0,
							[0].srect.y = 0,
							[0].srect.w = 720,
							[0].srect.h = 2880,
							[0].drect.x = 0,
							[0].drect.y = 0,
							[0].drect.w = 720,
							[0].drect.h = 2880,
							[1].out_mode = VDEC_OUT_PREVIEW,
							[1].srect.x = 0,
							[1].srect.y = 0,
							[1].srect.w = 720,
							[1].srect.h = 2880,
							[1].drect.x = 0,
							[1].drect.y = 0,
							[1].drect.w = 360,
							[1].drect.h = 1440};
static int m_parse_afd = -1;//default 0: not parse afd, see boot config
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER; // remove
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
typedef struct dev_vdec {
	enum vdec_video_id     id;
	const char      *path;
} dev_vdec_t;
struct dev_vdec dev_video[] = {
	{VDEC_ID_VIDEO_0, "/dev/ali_video0"},
	{VDEC_ID_VIDEO_1, "/dev/ali_video1"}//{VDEC_ID_VIDEO_1, "/dev/ali_video1"}
};

static int  sl_decv_add_epoll_fd(struct alislvdec_dev *dev);
static void sl_decv_del_epoll_fd(struct alislvdec_dev *dev);

//#define VDEC_DEV_PATH "/dev/ali_video0"
//#define VDEC_FD_INVAL -1

#define CHECK_HANDLE(hdl)                                               \
    do {                                                                \
        if (!(hdl)) {   \
            SL_ERR("invalidate handle, handle = 0x%08x\n", hdl); \
            return VDEC_ERR_INVALHANDLE;                                \
        }                                                               \
    } while(0)

static uint32_t
vdec_attr_2_iocmd(enum vdec_attr_type attr)
{
	switch (attr) {
		case VDEC_ATTR_CONTINUE_ON_ERROR :
			return VDEC_IO_CONTINUE_ON_ERROR;
		case VDEC_ATTR_FIRST_I_FREERUN   :
			return VDEC_IO_FIRST_I_FREERUN;
		case VDEC_ATTR_SAR_ENABLE        :
			return VDEC_IO_SAR_ENABLE;
		case VDEC_ATTR_PARSING_ENABLE    :
			return VDEC_DTVCC_PARSING_ENABLE;
		case VDEC_ATTR_SET_DMA_CHANNEL   :
			return VDEC_SET_DMA_CHANNEL;
		case VDEC_ATTR_SHOW_COLORBAR     :
			return VDEC_IO_COLORBAR;
		case VDEC_ATTR_GET_RAW_STATUS    :
			return VDECIO_GET_STATUS;
		default:
			SL_ERR("invalidate attr = 0x%08x\n", attr);
			return -1;
	}
}


static enum tvsystem
slvdec_tvsys(enum dis_tvsys tvsys)
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
		default:
			SL_ERR("invalidate tvsys = 0x%08x\n", tvsys);
			return -1;
	}
}


static enum dis_tvsys
vdec_tvsys_convert(enum tvsystem tvsys)
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
		default:
			SL_ERR("invalidate tvsys = 0x%08x\n", tvsys);
			return -1;
	}
}

/* convert share library playback dir to driver playback dir */
static enum vdec_playback_dir
to_drv_vdec_playback_dir(enum vdec_playback_direction drv_dir)
{
	switch (drv_dir) {
		case VDEC_PLAYBACK_FORWARD :
				return VDEC_PLAY_FORWARD;
		case VDEC_PLAYBACK_BACKWARD    :
			return VDEC_PLAY_BACKWARD;
		default:
			SL_ERR("invalidate vdec_playback_direction\n");
			return -1;
	}
}


static enum vdec_playback_direction
to_vdec_playback_direction(enum vdec_playback_dir dir)
{
	switch(dir) {
		case VDEC_PLAY_FORWARD:
				return VDEC_PLAYBACK_FORWARD;
		case VDEC_PLAY_BACKWARD:
			return VDEC_PLAYBACK_BACKWARD;
		default:
			SL_ERR("invalidate direction!\n");
			return -1;
	}
}

static enum vdec_aspect
to_vdec_aspect(enum asp_ratio drv_aspect)

{

	switch(drv_aspect) {
		case DAR_FORBIDDEN :
				return VDEC_ASPECT_FORBIDDEN;
		case SAR           :
			return VDEC_ASPECT_SAR;
		case DAR_4_3       :
			return VDEC_ASPECT_4_3;
		case DAR_16_9      :
			return VDEC_ASPECT_16_9;
		case DAR_221_1     :
			return VDEC_ASPECT_DAR_221_1;
		default:
			SL_ERR("Invalidate aspect\n");
			return -1;
	}
}


static enum vdec_state
to_vdec_state(enum vdec_status drv_state)
{
	switch (drv_state) {
		case VDEC_STARTED :
				return VDEC_STATE_STARTED;
		case VDEC_STOPPED :
			return VDEC_STATE_STOPPED;
		case VDEC_PAUSED  :
			return VDEC_STATE_PAUSE;
		default:
			SL_ERR("Invalidat status\n");
			return -1;
	}
}

static enum vdec_out_mode
to_vdec_out(enum vdec_output_mode drv_mode)
{
	switch (drv_mode) {
		case VDEC_FULL_VIEW :
				return VDEC_OUT_FULLVIEW;
		case VDEC_PREVIEW   :
			return VDEC_OUT_PREVIEW;
		case VDEC_SW_PASS   :
			return VDEC_OUT_SWPASS;
		default:
			SL_ERR("Invalidate output mode, %d\n", drv_mode);
			return -1;
	}
}


static enum vdec_out_mode
to_drv_vdec_rate(enum vdec_playback_speed speed)
{
	switch (speed) {
		case VDEC_PLAYBACK_SPEED_1_2  :
				return VDEC_RATE_1_2;
		case VDEC_PLAYBACK_SPEED_1_4  :
			return VDEC_RATE_1_4;
		case VDEC_PLAYBACK_SPEED_1_8  :
			return VDEC_RATE_1_8;
		case VDEC_PLAYBACK_SPEED_STEP :
			return VDEC_RATE_STEP;
		case VDEC_PLAYBACK_SPEED_1    :
			return VDEC_RATE_1;
		case VDEC_PLAYBACK_SPEED_2    :
			return VDEC_RATE_2;
		case VDEC_PLAYBACK_SPEED_4    :
			return VDEC_RATE_4;
		case VDEC_PLAYBACK_SPEED_8    :
			return VDEC_RATE_8;
        case VDEC_PLAYBACK_SPEED_16    :
            return VDEC_RATE_16;
        case VDEC_PLAYBACK_SPEED_32    :
            return VDEC_RATE_32;
		default:
			SL_ERR("Invalidate output playback rate\n");
			return -1;
	}
}

static enum vdec_playback_speed
to_vdec_playback_speed(enum vdec_playback_rate rate)
{
	switch (rate) {
		case VDEC_RATE_1_2  :
				return VDEC_PLAYBACK_SPEED_1_2;
		case VDEC_RATE_1_4  :
			return VDEC_PLAYBACK_SPEED_1_4;
		case VDEC_RATE_1_8  :
			return VDEC_PLAYBACK_SPEED_1_8;
		case VDEC_RATE_STEP :
			return VDEC_PLAYBACK_SPEED_STEP;
		case VDEC_RATE_1    :
			return VDEC_PLAYBACK_SPEED_1;
		case VDEC_RATE_2    :
			return VDEC_PLAYBACK_SPEED_2;
		case VDEC_RATE_4    :
			return VDEC_PLAYBACK_SPEED_4;
		case VDEC_RATE_8    :
			return VDEC_PLAYBACK_SPEED_8;
		case VDEC_RATE_16    :
			return VDEC_PLAYBACK_SPEED_16;
		case VDEC_RATE_32    :
			return VDEC_PLAYBACK_SPEED_32;            
		default:
			SL_ERR("Invalidate output playback rate\n");
			return -1;
	}
}

static enum vdec_avsync_mode
to_drv_vdec_sync_mode(enum vdec_av_sync_mode mode)
{
	switch (mode) {
		case VDEC_AV_SYNC_PTS:
				return VDEC_AVSYNC_PTS;
		case VDEC_AV_SYNC_FREERUN:
			return VDEC_AVSYNC_FREERUN;
		default:
			SL_ERR("Invalidate AVSYNC mode\n");
			return -1;
	}
}

static alisl_retcode vdec_set_output_rect(alisl_handle      hdl,
										  int               cmd,
										  struct vdec_rect *srect,
										  struct vdec_rect *drect)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;

	struct vdec_display_rect display_rect;

	CHECK_HANDLE(hdl);

	if (!srect || !drect) {
		SL_ERR("invalidate srect = 0x%08x, drect = 0x%08x\n", srect, drect);
		return VDEC_ERR_INVAL;
	}

	display_rect.src_x = srect->x;
	display_rect.src_y = srect->y;
	display_rect.src_w = srect->w;
	display_rect.src_h = srect->h;

	display_rect.dst_x = drect->x;
	display_rect.dst_y = drect->y;
	display_rect.dst_w = drect->w;
	display_rect.dst_h = drect->h;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME: 0x%x, dev->fd: %d, src(%d, %d, %d, %d) -> dst(%d, %d, %d, %d)\n",cmd,dev->fd,display_rect.src_x,display_rect.src_y,display_rect.src_w,
        display_rect.src_h,display_rect.dst_x,display_rect.dst_y,display_rect.dst_w,display_rect.dst_h);
	if (ioctl(dev->fd, cmd, &display_rect)) {
		SL_ERR("ioctl %d failed!\n",cmd);
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


static alisl_retcode
vdec_set_frame_type(alisl_handle          hdl,
					int                   cmd,
					enum vdec_frame_type frame_type)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME: 0x%x,dev->fd: %d, frame_type: %d\n",cmd,dev->fd,frame_type);
	if (ioctl(dev->fd, cmd, frame_type)) {
		SL_ERR("ioctl %d failed\n",cmd);
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

static alisl_retcode vdec_reg_callback(struct alislvdec_dev *dev,
                                       enum vdec_cbtype     type,
                                       vdec_callback       new_cb)
{
	alisl_retcode           ret = VDEC_ERR_NOERR;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("dev->fd: %d, type: %d\n",dev->fd,type);
    if (new_cb) {
       SL_DBG("IOCTL_NAME:VDECIO_REG_CALLBACK\n");
    	if (ioctl(dev->fd, VDECIO_REG_CALLBACK, type)) {
    		SL_ERR("ioctl VDECIO_REG_CALLBACK failed\n");
    		ret = VDEC_ERR_IOCTLFAILED;
    		goto quit;
    	}
	} else {
	SL_DBG("IOCTL_NAME:VDECIO_UNREG_CALLBACK\n");
    	if (ioctl(dev->fd, VDECIO_UNREG_CALLBACK, type)) {
    		SL_ERR("ioctl VDECIO_UNREG_CALLBACK failed\n");
    		ret = VDEC_ERR_IOCTLFAILED;
    		goto quit;
    	}
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


static void vdec_set_attrs_before_decoding(struct alislvdec_dev *dev)
{
	if (-1 != dev->variable_resolution)
		alislvdec_set_variable_resolution(dev, dev->variable_resolution);
	if (-1 != dev->continue_on_dec_err)
		alislvdec_set_continue_on_error(dev, dev->continue_on_dec_err);
}

static void vdec_reg_callback_before_playing(struct alislvdec_dev *dev)
{

	if (dev->cb_first_showed.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_FIRST_SHOWED, dev->cb_first_showed.callback_func);
	}
	if (dev->cb_mode_switch_ok.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_MODE_SWITCH_OK, dev->cb_mode_switch_ok.callback_func);
	}
	if (dev->cb_backward_restart_gop.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_BACKWARD_RESTART_GOP, dev->cb_backward_restart_gop.callback_func);
	}
	if (dev->cb_first_head_parsed.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_FIRST_HEAD_PARSED, dev->cb_first_head_parsed.callback_func);
	}
	if (dev->cb_first_i_frame_decoded.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_FIRST_I_DECODED, dev->cb_first_i_frame_decoded.callback_func);
	}
	if (dev->cb_user_data_parsed.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_MONITOR_USER_DATA_PARSED, dev->cb_user_data_parsed.callback_func);
	}
	if (dev->cb_info_changed.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_INFO_CHANGE, dev->cb_info_changed.callback_func);
	}
	if (dev->cb_status_changed.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_STATE_CHANGED, dev->cb_status_changed.callback_func);
	}
	if (dev->cb_decoder_error.callback_func) {
		vdec_reg_callback(dev, VDEC_CB_ERROR, dev->cb_decoder_error.callback_func);
	}
}

static alisl_retcode alislvdec_construct(alisl_handle *handle)
{
	struct alislvdec_dev *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		SL_ERR("malloc memory failed!\n");
		return VDEC_ERR_NOMEM;
	}

	memset(dev, 0, sizeof(*dev));

	flag_init(&dev->status, VDEC_STATUS_CONSTRUCT);

	*handle = dev;

	return 0;
}
/**
 * function Name: alislvdec_open
 *
 * @brief         Init VDEC device, and return a handle for further control of the device.
 *
 * @param[out] hdl Return handle for further control.
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
alislvdec_open(alisl_handle *hdl, enum vdec_video_id id)
{
	struct alislvdec_dev *dev;
	int i;
	for (i = 0; i < ARRAY_SIZE(dev_video); i++) {
		if (id == dev_video[i].id) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev_video) || hdl == NULL) {
		SL_ERR("Invalidate video id!\n");
		return VDEC_ERR_INVAL;
	}
	/* lock open to deal with multiple open simultaneously */
	pthread_mutex_lock(&m_mutex);

	if (m_dev[i] == NULL) {
		if (alislvdec_construct((alisl_handle *)&dev)) {
			goto no_mem;
		}
	} else {
		dev = m_dev[i];
	}

	if (flag_bit_test_any(&dev->status, VDEC_STATUS_OPEN)) {
		//SL_INFO("Already opened!\n");
		goto exit_success;
	}

	dev->first_showed = false;
	dev->open_cnt     = 0;
	dev->invalid_dma  = false;
	dev->dis_layer = VDEC_DIS_MAIN_LAYER;
	pthread_mutex_init(&dev->mutex, NULL);
	pthread_mutex_init(&dev->mutex_callback, NULL);
	dev->id = id;
	dev->variable_resolution = -1;
	dev->continue_on_dec_err = -1;
	dev->fd = open(dev_video[i].path, O_RDWR);
	if (dev->fd < 0) {
		SL_ERR("FATAL: %s opened failed!\n", dev_video[i].path);
		goto event_add_failed;
	}

		//ioctl(dev->fd, ALIVIDEOIO_ENABLE_DBG_LEVEL, 2);

	if (!sl_decv_add_epoll_fd(dev)) {
		SL_ERR("can't add callback event to epoll\n");
		goto event_add_failed;
	}
	flag_bit_set(&dev->status, VDEC_STATUS_OPEN);

exit_success:
	*hdl = (alisl_handle)dev;
	dev->open_cnt++;
	m_dev[i] = dev;
	pthread_mutex_unlock(&m_mutex);

	return VDEC_ERR_NOERR;

event_add_failed:
	free(dev);
no_mem:
	*hdl = NULL;
	pthread_mutex_unlock(&m_mutex);
	return VDEC_ERR_NOMEM;
}

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
alislvdec_close(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	unsigned int i;

	CHECK_HANDLE(hdl);

	for (i = 0; i < ARRAY_SIZE(dev_video); i++) {
		if (dev->id == dev_video[i].id) {
			break;
		}
	}

	if (i == ARRAY_SIZE(dev_video)) {
		SL_ERR("Invalidate vdec id!\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&m_mutex);
	if (--dev->open_cnt) {
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_OPEN)) {
		SL_WARN("Video device not opened!\n");
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}

	sl_decv_del_epoll_fd(dev);
	close(dev->fd);
	pthread_mutex_destroy(&dev->mutex);
	pthread_mutex_destroy(&dev->mutex_callback);
	flag_delete(&dev->status);
	dev->decode_error_cnt = 0;
	free((void *)dev);

	dev = NULL;
	if (i < ARRAY_SIZE(dev_video)) {//fix bug from bug detective
	    m_dev[i] = NULL;
	}
	pthread_mutex_unlock(&m_mutex);
	return VDEC_ERR_NOERR;
}

/*
static alisl_retcode
alislvdec_dev_fd(alisl_handle hdl, int *devfd)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;

	CHECK_HANDLE(hdl);

	if (NULL == devfd) {
		SL_ERR("invalidate devfd = 0x%08x\n", devfd);
		return VDEC_ERR_INVAL;
	}

	*devfd = dev->fd;

quit:
	return VDEC_ERR_NOERR;
}
*/

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
alislvdec_start(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);
	pthread_mutex_lock(&dev->mutex);

	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_OPEN)) {
		SL_ERR("Not opened!\n");
		ret = VDEC_ERR_INVALHANDLE;
		goto quit;
	}

	if (flag_bit_test_any(&dev->status, VDEC_STATUS_START)) {
		SL_WARN("Already started!\n");
		ret = VDEC_ERR_NOERR;
		goto quit;
	}

	pthread_mutex_unlock(&dev->mutex);

	if (dev->dis_layer < VDEC_DIS_LAYER_MAX) {
		if (m_display_setting[dev->dis_layer].out_mode != VDEC_OUT_LAST) {
		    ret = alislvdec_set_display_mode(hdl, &m_display_setting[dev->dis_layer]);
		}
	}

	// 75695(old) fix bug for PVR no first show callback, reg callback in SL before start
	vdec_reg_callback_before_playing(dev);
	/*
	App may set some attributes before setting video format, and these attributes
	are stored and would be configured again internally to driver after setting video format.
	*/
	vdec_set_attrs_before_decoding(dev);
	pthread_mutex_lock(&dev->mutex);
	
    SL_DBG("IOCTL_NAME:VDECIO_PARSE_AFD, dev->id: %d, m_parse_afd: %d\n",dev->id,m_parse_afd);
	if (m_parse_afd >= 0) {
		//SL_DBG("m_parse_afd: %d\n", m_parse_afd);
		if (ioctl(dev->fd, VDECIO_PARSE_AFD, m_parse_afd)) {
			SL_ERR("ioctl VDECIO_PARSE_AFD failed!\n");
		}
	}
	
	SL_DBG("VDECIO_START[%d]\n", dev->id);
	if (ioctl(dev->fd, VDECIO_START, 0)) {
		SL_ERR("ioctl VDECIO_START failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

	bzero((void *) & (dev->status_info), sizeof(dev->status_info));
	flag_bit_clear(&dev->status, VDEC_STATUS_STOP);
	flag_bit_set(&dev->status, VDEC_STATUS_START);
quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
			   bool         fill_black)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_stop_param  stop_param;

	CHECK_HANDLE(hdl);

	stop_param.close_display = close_display ? 1 : 0;
	stop_param.fill_black    = fill_black    ? 1 : 0;

	pthread_mutex_lock(&dev->mutex);

	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_START)) {
		SL_ERR("VDEC do not started yet!\n");
		ret = VDEC_ERR_NOERR;
		goto quit;
	}
    SL_DBG("IOCTL_NAME:VDECIO_STOP, dev->id: %d, close_display: %d, fill_black: %d\n",dev->id,
        stop_param.close_display,stop_param.fill_black);
	if (ioctl(dev->fd, VDECIO_STOP, &stop_param)) {
		SL_ERR("ioctl VDECIO_STOP failed: close_display = 0x%08x, fill_black = 0x%08x!\n",
			   close_display, fill_black);
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

	flag_bit_clear(&dev->status, VDEC_STATUS_START);
	flag_bit_set(&dev->status, VDEC_STATUS_STOP);
	dev->invalid_dma = false;
	dev->decode_error_cnt = 0;
	dev->continue_on_dec_err = -1;
    //#fix BUG 94299, don't reset this value while decv stop
	//dev->variable_resolution = -1;
quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

alisl_retcode
alislvdec_set_vbv_buf_mode(alisl_handle hdl, enum vdec_vbv_buf_mode vbv_buf_mode)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
    int vbv_buf_working_mode;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);

    //To avoid the case: Default not set vbv_buf_mode in aui_decv_open()
    vbv_buf_working_mode = (vbv_buf_mode == VBV_BUF_BLOCK_FULL_MODE) ? 0 : 1;
    SL_DBG("IOCTL_NAME:VDECIO_VBV_BUFFER_OVERFLOW_RESET, dev->id: %d, vbv_buf_working_mode: %d\n",dev->id,vbv_buf_working_mode);
    if (ioctl(dev->fd, VDECIO_VBV_BUFFER_OVERFLOW_RESET, vbv_buf_working_mode))
    {
        SL_ERR("ioctl VDECIO_VBV_BUFFER_OVERFLOW_RESET failed!\n");
        ret = VDEC_ERR_IOCTLFAILED;
        goto quit;
    }

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}



alisl_retcode
alislvdec_pause(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);

	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_START)) {
		SL_ERR("VDEC do not started yet!\n");
		ret = VDEC_ERR_FAILED;
		goto quit;
	}
    SL_DBG("IOCTL_NAME:VDECIO_PAUSE, dev->id: %d\n", dev->id);
	if (ioctl(dev->fd, VDECIO_PAUSE, 0)) {
		SL_ERR("ioctl VDECIO_PAUSE fail\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
alislvdec_buffer_reset(alisl_handle hdl, unsigned int reset_buffer)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);

	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_START)) {
		SL_ERR("VDEC do not started yet!\n");
		ret = VDEC_ERR_FAILED;
		goto quit;
	}
	//printf("%s -> before VDECIO_RESTART: %u\n",  __func__, reset_buffer);
	SL_DBG("IOCTL_NAME:VDECIO_RESTART, dev->id: %d, reset_buffer: %d\n",dev->id,reset_buffer);
	if (ioctl(dev->fd, VDECIO_RESTART, reset_buffer)) {
		SL_ERR("ioctl VDECIO_RESTART fail\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
	//printf("%s -> after VDECIO_RESTART: %u\n",  __func__, reset_buffer);
quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

alisl_retcode
alislvdec_set_colorbar(alisl_handle hdl, unsigned long color_bar_addr)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_DRAW_COLOR_BAR, dev->id: %d, color_bar_addr: 0x%x\n",dev->id,color_bar_addr);
	if (ioctl(dev->fd, VDECIO_DRAW_COLOR_BAR, color_bar_addr)) {
		SL_ERR("ioctl VDECIO_DRAW_COLOR_BAR fail\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


alisl_retcode
alislvdec_resume(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);

	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_START)) {
		SL_ERR("VDEC do not started yet!\n");
		ret = VDEC_ERR_FAILED;
		goto quit;
	}
    SL_DBG("IOCTL_NAME:VDECIO_RESUME, dev->id: %d\n",dev->id);
	if (ioctl(dev->fd, VDECIO_RESUME, 0)) {
		SL_ERR("ioctl VDECIO_RESUME fail\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

/**
 * Function Name:   alislvdec_set_output
 * @brief           Set VDEC's output mode
 *
 * @param[in]       hdl Device handl
 * @param[in]       out_mode VDEC output settings
 * @param[out]      mpcb Opaque MP callback data set to DIS if not null
 * @param[out]      pipcb Opaque PIP callback data set to DIS if not null
 *
 * @return          alisl_retcode
 *
 * @author          Glen Dai <glen.dai@alitech.com>
 * @date            31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_output(alisl_handle             hdl,
					 struct vdec_out_setting *setting,
					 uint32_t                *mpcb,
					 uint32_t                *pipcb)
{
	static struct mpsource_call_back s_mpcb;
	static struct pipsource_call_back s_pipcb;

	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;

	struct vdec_output_param output_param;
	enum vdec_output_mode vdec_out_mode;

	CHECK_HANDLE(hdl);

	if (!setting) {
		SL_ERR("Invalidate paremeters set\n");
		return VDEC_ERR_INVAL;
	}

	switch (setting->out_mode) {
		case VDEC_OUT_FULLVIEW :
			vdec_out_mode = VDEC_FULL_VIEW;
			break;
		case VDEC_OUT_PREVIEW  :
			vdec_out_mode = VDEC_PREVIEW;
			break;
		case VDEC_OUT_SWPASS   :
			vdec_out_mode = VDEC_SW_PASS;
			break;
		default:
			SL_ERR("invalidate out_mode = 0x%08x\n", setting->out_mode);
			return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	output_param.output_mode   = vdec_out_mode;
	output_param.progressive   = setting->progressive ? 1 : 0;
	output_param.tv_sys        = slvdec_tvsys(setting->out_tvsys);
	output_param.smooth_switch = setting->smooth_switch_en;
	output_param.mp_callback   = s_mpcb;
	output_param.pip_callback  = s_pipcb;
    SL_DBG("IOCTL_NAME:VDECIO_SET_OUTPUT_MODE, dev->id: %d, output_mode: %d, progressive: %d, tv_sys: %d,"\
        "smooth_switch: %d\n",dev->id,output_param.output_mode,output_param.progressive,output_param.tv_sys
        ,output_param.smooth_switch);
	if (ioctl(dev->fd, VDECIO_SET_OUTPUT_MODE, &output_param)) {
		SL_ERR("ioctl VDECIO_SET_OUTPUT_MODE failed");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

	if (mpcb)
		memcpy(mpcb, &s_mpcb, sizeof(s_mpcb));

	if (pipcb)
		memcpy(pipcb, &s_pipcb, sizeof(s_pipcb));

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
						enum vdec_av_sync_mode sync_mode)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_sync_param  sync_param;

	CHECK_HANDLE(hdl);

	sync_param.sync_mode = to_drv_vdec_sync_mode(sync_mode);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME: VDECIO_SET_SYNC_MODE, dev->id: %d, sync_mode: %d\n",dev->id,sync_param.sync_mode);
	if (ioctl(dev->fd, VDECIO_SET_SYNC_MODE, &sync_param)) {
		SL_ERR("ioctl VDECIO_SET_SYNC_MODE failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
						 uint32_t     delay)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_SET_SYNC_DELAY, dev->id: %d, delay: %d\n",dev->id,delay);
	if (ioctl(dev->fd, VDECIO_SET_SYNC_DELAY, &delay)) {
		SL_ERR("ioctl VDECIO_SET_SYNC_DELAY failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
							 bool         enable)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;

	int freerun  = enable ? 1 : 0;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_FIRST_I_FREERUN, dev->id: %d, freerun: %d\n",dev->id,freerun);
	if (ioctl(dev->fd, VDECIO_FIRST_I_FREERUN, &freerun)) {
		SL_ERR("ioctl VDECIO_FIRST_I_FREERUN failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
						  struct vdec_rect *drect)
{

	alisl_retcode ret = VDEC_ERR_NOERR;
	ret = vdec_set_output_rect(hdl, VDECIO_SET_OUTPUT_RECT, srect, drect);
	return ret;
}

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
						struct vdec_frame_cap *cap)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	struct vdec_picture pic_cap;

	CHECK_HANDLE(hdl);

	if (!cap) {
		SL_ERR("invalid cap parameter\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	pic_cap.type              = VDEC_PIC_YV12;
	pic_cap.out_data_buf      = cap->buf_addr;
	pic_cap.out_data_buf_size = cap->buf_sz;
    SL_DBG("IOCTL_NAME:VDECIO_CAPTURE_DISPLAYING_FRAME, dev->id: %d, type: %d, out_data_buf: 0x%x, out_data_buf_size: %d\n",
        dev->id,pic_cap.type,pic_cap.out_data_buf,pic_cap.out_data_buf_size);
	if (ioctl(dev->fd, VDECIO_CAPTURE_DISPLAYING_FRAME, &pic_cap)) {
		SL_ERR("ioctl VDECIO_CAPTURE_DISPLAYING_FRAME failed, you may have set the wrong cap paramter!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

	cap->cap_buf_sz     = pic_cap.out_data_valid_size;
	cap->cap_pic_height = pic_cap.pic_height;
	cap->cap_pic_width  = pic_cap.pic_width;

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


/**
 * Function Name:	alislvdec_set_decoder
 * @brief			Set current decoder type
 *
 * @param[in]		hdl Device handl
 * @param[in]		type Decoder type
 * @param[in]		preview Boolean value to tell VDEC to change to preview mode
 *
 * @return			alisl_retcode
 *
 * @author			Glen Dai <glen.dai@alitech.com>
 * @date			31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_decoder(alisl_handle           hdl,
					  enum vdec_decoder_type type,
					  bool                   preview)
{
	struct alislvdec_dev    *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode            ret = VDEC_ERR_NOERR;
	struct vdec_codec_param  codec_param;
    (void)preview;    
	CHECK_HANDLE(hdl);
	SL_DBG("type: %d\n", type);
	pthread_mutex_lock(&dev->mutex);
	switch(type)
	{
		case VDEC_DECODER_MPEG: codec_param.type = VDEC_MPEG; break;
		case VDEC_DECODER_AVC:  codec_param.type = VDEC_AVC; break;
		case VDEC_DECODER_AVS:  codec_param.type = VDEC_AVS; break;
		case VDEC_DECODER_HEVC:  codec_param.type = VDEC_HEVC; break;
		default: SL_ERR("set invalid video format\n");
		// for other fomat for media player just return success
		ret = VDEC_ERR_INVAL;
		goto quit;
	}
	codec_param.preview = -1;
    SL_DBG("IOCTL_NAME:VDECIO_SELECT_DECODER, dev->id: %d, type: %d, preview: %d\n",dev->id,
        codec_param.type,codec_param.preview);
	if (ioctl(dev->fd, VDECIO_SELECT_DECODER, &codec_param)) {
		SL_ERR("ioctl VDECIO_SELECT_DECODER failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
	dev->es_flag = 0;//set up to 0,live
	/** reg decode error callback, used to statistics decode error count, for aui use */
	dev->decode_error_cnt = 0;
    SL_DBG("IOCTL_NAME:VDECIO_REG_CALLBACK\n");
	if (ioctl(dev->fd, VDECIO_REG_CALLBACK, VDEC_CB_ERROR)) {
		SL_ERR("ioctl VDECIO_REG_CALLBACK return failed\n");
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

/**
 * Function Name: alislvdec_get_decoder
 * @brief Get current decoder.
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
					  enum vdec_decoder_type *type)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	enum vdec_type        cur_type;

	CHECK_HANDLE(hdl);

	if (!type) {
		SL_ERR("invalidate type\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	if (ioctl(dev->fd, VDECIO_GET_CUR_DECODER, &cur_type)) {
		SL_ERR("ioctl VDECIO_GET_CUR_DECODER failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
    SL_DBG("IOCTL_NAME:VDECIO_GET_CUR_DECODER, dev->id: %d, cur_type: %d\n",dev->id,cur_type);
	switch(cur_type) {
		case VDEC_AVC:
			*type = VDEC_DECODER_AVC;
			break;
		case VDEC_MPEG:
			*type = VDEC_DECODER_MPEG;
			break;
		case VDEC_AVS:
			*type = VDEC_DECODER_AVS;
			break;
		case VDEC_HEVC:
			*type = VDEC_DECODER_HEVC;
			break;
		default:
			SL_ERR("invlidate codec type : 0x%08x\n", cur_type);
			ret = VDEC_ERR_INVAL;
			break;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
					   vdec_callback           *old_cb)
{
    struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
    alisl_retcode         ret = VDEC_ERR_NOERR;

    CHECK_HANDLE(hdl);
    SL_DBG("dev->id: %d, type: %d, new_cb: %p, user_data: %p\n",dev->id,type,new_cb,user_data);
    switch(type) {
        case SL_VDEC_CB_FIRST_SHOWED:
            if (old_cb)
                *old_cb = dev->cb_first_showed.callback_func;
                
            dev->cb_first_showed.callback_func = new_cb;
            dev->cb_first_showed.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_FIRST_SHOWED, new_cb);
            break;

        case SL_VDEC_CB_MODE_SWITH_OK:
            if (old_cb)
                *old_cb = dev->cb_mode_switch_ok.callback_func;
                
            dev->cb_mode_switch_ok.callback_func = new_cb;
            dev->cb_mode_switch_ok.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_MODE_SWITCH_OK, new_cb);
            break;

        case SL_VDEC_CB_BACKWARD_RESTART_GOP:
            if (old_cb)
                *old_cb = dev->cb_backward_restart_gop.callback_func;
            dev->cb_backward_restart_gop.callback_func = new_cb;
            dev->cb_backward_restart_gop.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_BACKWARD_RESTART_GOP, new_cb);
            break;

        case SL_VDEC_CB_FIRST_HEAD_PARSED:
            if (old_cb)
                *old_cb = dev->cb_first_head_parsed.callback_func;
                
            dev->cb_first_head_parsed.callback_func = new_cb;
            dev->cb_first_head_parsed.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_FIRST_HEAD_PARSED, new_cb);
            break;

        case SL_VDEC_CB_FRIST_I_FRAME_DECODED:
            if (old_cb)
                *old_cb = dev->cb_first_i_frame_decoded.callback_func;

            dev->cb_first_i_frame_decoded.callback_func = new_cb;
            dev->cb_first_i_frame_decoded.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_FIRST_I_DECODED, new_cb);
            break;

        case SL_VDEC_CB_STATUS_CHANGED:
            if (old_cb)
                *old_cb = dev->cb_status_changed.callback_func;

            dev->cb_status_changed.callback_func = new_cb;
            dev->cb_status_changed.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_STATE_CHANGED, new_cb);
            break;

        case SL_VDEC_CB_ERROR:
            if (old_cb)
                *old_cb = dev->cb_decoder_error.callback_func;
                
            dev->cb_decoder_error.callback_func = new_cb;
            dev->cb_decoder_error.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_ERROR, new_cb);
            break;

        case SL_VDEC_CB_USER_DATA_PARSED:
            if (old_cb)
                *old_cb = dev->cb_user_data_parsed.callback_func;

            dev->cb_user_data_parsed.callback_func = new_cb;
            dev->cb_user_data_parsed.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_MONITOR_USER_DATA_PARSED, new_cb);
            break;
        case SL_VDEC_CB_INFO_CHANGED:
            if (old_cb)
                *old_cb = dev->cb_info_changed.callback_func;

            dev->cb_info_changed.callback_func = new_cb;
            dev->cb_info_changed.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_INFO_CHANGE, new_cb);
            break;
        case SL_VDEC_CB_FRAME_DISPLAYED:
            if (old_cb)
                *old_cb = dev->cb_frame_displayed.callback_func;

            dev->cb_frame_displayed.callback_func = new_cb;
            dev->cb_frame_displayed.user_data = user_data;
            if((0 == dev->if_reg_cb_for_trick)&&(NULL == new_cb)) {
                ret = vdec_reg_callback(dev, VDEC_CB_FRAME_DISPLAYED, new_cb);
            }
            break;
        case SL_VDEC_CB_MONITOR_GOP:		
            if (old_cb)
                *old_cb = dev->cb_monitor_gop.callback_func;

            dev->cb_monitor_gop.callback_func = new_cb;
            dev->cb_monitor_gop.user_data = user_data;
            ret = vdec_reg_callback(dev, VDEC_CB_MONITOR_GOP, new_cb);			
            break;
        default:
            break;
    }
    
    return ret;
}


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
					   enum vdec_playback_speed     speed)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	struct vdec_playback_param playback_param;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);

	playback_param.direction = to_drv_vdec_playback_dir(dir);
	playback_param.rate      = to_drv_vdec_rate(speed);
    SL_DBG("IOCTL_NAME:VDECIO_SET_PLAY_MODE, dev->id: %d, direction: %d, rate: %d\n",dev->id,
        playback_param.direction,playback_param.rate);
	if (ioctl(dev->fd, VDECIO_SET_PLAY_MODE, &playback_param)) {
		SL_ERR("ioctl VDECIO_SET_PLAY_MODE failed: dir = 0x%08x, speed = 0x%08x!\n", dir, speed);
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

/**
 * Function Name: alislvdec_set_trickmode
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
alislvdec_set_trickmode(alisl_handle                 hdl,
					   enum vdec_playback_direction dir,
					   enum vdec_playback_speed     speed,
					   enum vdec_playback_method    mode)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	struct vdec_playback_param playback_param;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
	#if 0
	enum vdec_type        cur_type=VDEC_MPEG;
	if (ioctl(dev->fd, VDECIO_GET_CUR_DECODER, &cur_type)) {
		SL_ERR("ioctl failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
	#endif
	playback_param.direction = to_drv_vdec_playback_dir(dir);
	playback_param.rate = to_drv_vdec_rate(speed);
	playback_param.mode = (mode == VDEC_PLAYBACK_METHOD_CONTINUOUS) ? 0 : 1;
    SL_DBG("IOCTL_NAME:VDECIO_SET_TRICK_MODE, dev->id: %d, direction: %d, rate: %d, mode: %d\n",
        dev->id,playback_param.direction,playback_param.rate,playback_param.mode);
	if (ioctl(dev->fd, VDECIO_SET_TRICK_MODE, &playback_param)) {
		SL_ERR("ioctl VDECIO_SET_TRICK_MODE failed: dir = 0x%08x, speed = 0x%08x!\n", dir, speed);
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
	if((VDEC_PLAYBACK_METHOD_NONCONTINUOUS == mode) &&(0 == dev->if_reg_cb_for_trick)) {
		#if 0
		if ((VDEC_AVC==cur_type)&&(ioctl(dev->fd, VDECIO_REG_CALLBACK, VDEC_CB_ERROR))) {
    		SL_ERR("reg error_cb failed\n");
    		ret = VDEC_ERR_IOCTLFAILED;
    		goto quit;
		}
		#endif
        SL_DBG("IOCTL_NAME:VDECIO_REG_CALLBACK\n");
		if (ioctl(dev->fd, VDECIO_REG_CALLBACK, VDEC_CB_FRAME_DISPLAYED)) {
    		SL_ERR("ioctl VDECIO_REG_CALLBACK failed\n");
    		ret = VDEC_ERR_IOCTLFAILED;
    		goto quit;
		}
		dev->if_reg_cb_for_trick = 1;
	}
	else if((VDEC_PLAYBACK_METHOD_CONTINUOUS == mode) &&(1 == dev->if_reg_cb_for_trick)) {
		#if 0
		//In continuous mode, all exceptions will be handled by vdec driver.
		if(NULL == dev->cb_decoder_error) {
			if (((VDEC_AVC==cur_type))&&ioctl(dev->fd, VDECIO_UNREG_CALLBACK, VDEC_CB_ERROR)) {
	    		SL_ERR("unreg error_cb failed\n");
	    		ret = VDEC_ERR_IOCTLFAILED;
	    		goto quit;
			}
		}
		#endif
		if(NULL == dev->cb_frame_displayed.callback_func) {
            SL_DBG("IOCTL_NAME:VDECIO_UNREG_CALLBACK\n");
			if (ioctl(dev->fd, VDECIO_UNREG_CALLBACK, VDEC_CB_FRAME_DISPLAYED)) {
	    		SL_ERR("ioctl VDECIO_UNREG_CALLBACK failed\n");
	    		ret = VDEC_ERR_IOCTLFAILED;
	    		goto quit;
			}
		}
		dev->if_reg_cb_for_trick = 0;
	}
	dev->if_cb_err = 0;
	dev->if_frm_dis = 0;
quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}
alisl_retcode alislvdec_get_trick_info(alisl_handle hdl, int *if_cb_err, int *if_frm_dis) {
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);
	if((!if_cb_err) || (!if_frm_dis)) {
		SL_ERR("if_hw_err or if_frm_dis null pointer\n");
		return VDEC_ERR_INVAL;
	}
	pthread_mutex_lock(&dev->mutex);
	*if_cb_err = dev->if_cb_err;
	*if_frm_dis = dev->if_frm_dis;
	if(dev->if_cb_err){
		//printf("%s -> if_cb_err is 1\n",__func__);
		dev->if_cb_err = 0;
	}
	if(dev->if_frm_dis) {
		//printf("%s -> if_cb_dis is 1\n",__func__);
		dev->if_frm_dis = 0;
	}
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}
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
					 struct vdec_color *color)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	struct vdec_yuv_color yuv_color;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
	yuv_color.y = color->y;
	yuv_color.u = color->u;
	yuv_color.v = color->v;
    SL_DBG("IOCTL_NAME:VDECIO_FILL_FRAME, dev->id: %d, yuv_color(y,u,v):(%d,%d,%d)\n",dev->id,yuv_color.y,yuv_color.u,yuv_color.v);
	if (ioctl(dev->fd, VDECIO_FILL_FRAME, &yuv_color)) {
		SL_ERR("ioctl VDECIO_FILL_FRAME failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
					 struct vdec_pvr_config_param *param)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	struct vdec_pvr_param pvr_param;

	CHECK_HANDLE(hdl);

	if (!param) {
		SL_ERR("Invalidate param!\n");
		return VDEC_ERR_INVAL;
	}

	pvr_param.is_scrambled = param->is_scrambled;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_SET_PVR_PARAM, dev->id: %d, is_scrambled: %d\n",dev->id,pvr_param.is_scrambled);
	if (ioctl(dev->fd, VDECIO_SET_PVR_PARAM, &pvr_param)) {
		SL_ERR("ioctl VDECIO_SET_PVR_PARAM failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
alislvdec_pause_pvr(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_PAUSE, dev->id: %d\n",dev->id);
	if (ioctl(dev->fd, VDECIO_PAUSE, 0)) {
		SL_ERR("ioctl VDECIO_PAUSE failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
alislvdec_resume_pvr(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_RESUME, dev->id: %d\n",dev->id);
	if (ioctl(dev->fd, VDECIO_RESUME, 0)) {
		SL_ERR("ioctl VDECIO_RESUME failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
alislvdec_step_pvr(alisl_handle hdl)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_STEP, dev->id: %d\n",dev->id);
	if (ioctl(dev->fd, VDECIO_STEP, 0)) {
		SL_ERR("ioctl VDECIO_STEP failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}


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
				   struct vdec_info *info)
{
	struct alislvdec_dev    *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode            ret = VDEC_ERR_NOERR;
	struct vdec_information  drv_info;

	CHECK_HANDLE(hdl);

	if (!info) {
		SL_ERR("invalidate info\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_GET_STATUS, dev->id: %d\n",dev->id);
	if (ioctl(dev->fd, VDECIO_GET_STATUS, &drv_info)) {
		SL_ERR("ioctl VDECIO_GET_STATUS failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

	info->state = to_vdec_state(drv_info.status);
	SL_DBG("state %d\n", info->state);

	info->first_header_parsed = drv_info.first_header_parsed;
	info->first_pic_decoded   = drv_info.first_pic_decoded;
	info->first_pic_showed    = drv_info.first_pic_showed;
	info->pic_width           = drv_info.pic_width;
	info->pic_height          = drv_info.pic_height;
	info->aspect              = to_vdec_aspect(drv_info.aspect_ratio);
	info->frames_displayed    = drv_info.frames_displayed;
	info->frames_decoded      = drv_info.frames_decoded;
	info->frame_last_pts      = drv_info.frame_last_pts;
	info->show_frame          = drv_info.show_frame;
	info->queue_count         = drv_info.queue_count;
	info->buffer_size         = drv_info.buffer_size;
	info->buffer_used         = drv_info.buffer_used;
	info->frame_rate          = drv_info.frame_rate;
	info->interlaced_frame    = drv_info.interlaced_frame;
	info->top_field_first     = drv_info.top_field_first;
	info->hw_dec_error        = drv_info.hw_dec_error;
	info->is_support          = drv_info.is_support;
	info->sar_height          = drv_info.sar_height;
	info->sar_width           = drv_info.sar_width;
	info->active_format       = drv_info.active_format;
	info->out_mode            = to_vdec_out(drv_info.output_mode);
	info->playback_dir        =
		to_vdec_playback_direction(drv_info.playback_param.direction);
	info->playback_speed      =
		to_vdec_playback_speed(drv_info.playback_param.rate);
	info->api_playback_dir    =
		to_vdec_playback_direction(drv_info.api_playback_param.direction);
	info->api_playback_speed  =
		to_vdec_playback_speed(drv_info.api_playback_param.rate);
	if (drv_info.layer == VPO_MAINWIN) {
		info->layer = VDEC_DIS_MAIN_LAYER;
	} else if (drv_info.layer == VPO_PIPWIN){
		info->layer = VDEC_DIS_AUX_LAYER;
	} else {
		info->layer = VDEC_DIS_LAYER_MAX;
	}
	/** registered callback in aliplatform layer, statistical decode error count */
	info->decode_error_cnt    = dev->decode_error_cnt;
	//store info for tvsys and stream progressive
	/** trick mode 3.1. */
	info->ff_mode = drv_info.ff_mode;
	info->rect_switch_done = drv_info.rect_switch_done;
    info->max_width = drv_info.max_width;
    info->max_height = drv_info.max_height;
    info->max_frame_rate = drv_info.max_frame_rate;
    
	memcpy(&(dev->status_info), (void *)&drv_info, sizeof(drv_info));

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
					enum dis_tvsys *ret_tvsys)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	enum tvsystem vpo_tvsys = -1;

	struct vdec_information *status_info = &(dev->status_info);

	int frame_rate;
	int source_width;

	CHECK_HANDLE(hdl);

	if (!ret_tvsys) {
		SL_ERR("invalidate tvsys\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	if(!status_info->first_pic_showed) {
		SL_ERR("do not have first showed\n");
		ret = VDEC_ERR_FAILED;
		goto quit;
	}

	frame_rate   = status_info->frame_rate;
	source_width = status_info->pic_width;

	switch( (frame_rate + 999) / 1000 ) {
		case 25:
		case 50:
			if(source_width <= 720)
				vpo_tvsys = PAL;
			else if(source_width <= 1280)
				vpo_tvsys = LINE_720_25;
			else
				vpo_tvsys = LINE_1080_25;
			break;
		case 24:
		case 30:
		case 60:
			if(source_width <= 720)
				vpo_tvsys = NTSC;
			else if(source_width <= 1280)
				vpo_tvsys = LINE_720_30;
			else
				vpo_tvsys = LINE_1080_30;
			break;

		default:
			ret = VDEC_ERR_FAILED;
			break;
	}

	*ret_tvsys = vdec_tvsys_convert(vpo_tvsys);

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
								bool         *progressive)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	CHECK_HANDLE(hdl);

	if (!progressive) {
		SL_ERR("invlidate progressive\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	if(!dev->status_info.first_pic_showed) {
		ret = VDEC_ERR_INVAL;
		SL_DBG("Not first showed\n");
		goto quit;
	}

	*progressive = dev->status_info.interlaced_frame ? false : true;

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
				 uint32_t      buf_sz)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;

	int              write_sz;

	CHECK_HANDLE(hdl);

	if ((!buf) || (!buf_sz)) {
		SL_ERR("invalid buffer parameter\n");
		return VDEC_ERR_INVAL;
	}
	pthread_mutex_lock(&dev->mutex);
	/* set 0xFF: search start code by software, only set once,
	   some the status of video decoder will reset when set it*/
	if (!dev->invalid_dma) {
        SL_DBG("IOCTL_NAME:VDECIO_SET_DMA_CHANNEL, dev->id: %d\n",dev->id);
		if (ioctl(dev->fd, VDECIO_SET_DMA_CHANNEL, 0xFF)) {
			SL_WARN("ioctl VDECIO_SET_DMA_CHANNEL failed!\n");
		} else {
			dev->invalid_dma = true;
		}
	}
	pthread_mutex_unlock(&dev->mutex);
	while (1) {
		pthread_mutex_lock(&dev->mutex);

		write_sz = write(dev->fd, buf, buf_sz);

		if (write_sz > 0) {
			buf_sz -= write_sz;
			if (buf_sz == 0) {
				pthread_mutex_unlock(&dev->mutex);
				break;
			}
		} else {
		    pthread_mutex_unlock(&dev->mutex);
		    ret = VDEC_ERR_IOCTLFAILED;
			break;
		}

		pthread_mutex_unlock(&dev->mutex);
		usleep(10000);
	}

	return ret;
}


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
				   uint32_t            attr_val)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode         ret = VDEC_ERR_NOERR;
	uint32_t iocmd;
	CHECK_HANDLE(hdl);


	pthread_mutex_lock(&dev->mutex);
	iocmd = vdec_attr_2_iocmd(attr_type);
	if (-1 == iocmd) {
		ret = VDEC_ERR_INVAL;
		SL_ERR("invalid attr_type\n");
		goto quit;
	}
    SL_DBG("IOCTL_NAME: %d, dev->id: %d, attr_val: %d\n",iocmd,dev->id,attr_val);
	if (ioctl(dev->fd, iocmd, attr_val)) {
		SL_ERR("ioctl %d failed \n",iocmd);
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
						 enum vdec_frame_type frame_type)
{
	alisl_retcode ret = VDEC_ERR_NOERR;
	ret = vdec_set_frame_type(hdl, VDECIO_SET_DEC_FRM_TYPE, frame_type);
	return ret;
}

static inline void handle_msg(struct alislvdec_dev *dev, void *msg)
{
	uint8_t  *msg_data = (uint8_t *)msg;
	uint32_t  msg_type = msg_data[0];
//	uint32_t  msg_len  = msg_data[1];
//	int i = 0;
	struct vdec_info_from_cb vinfo;
	struct vdec_info_cb_param *infocb = NULL;
	enum vdec_decoder_status status = 0xff;
	enum vdec_error_code error_code = 0xff;
	int oldtype;

	pthread_cleanup_push((void *)pthread_mutex_unlock, &dev->mutex_callback);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);

	pthread_mutex_lock(&dev->mutex_callback);
	switch(msg_type) {
		case MSG_FIRST_SHOWED: {
			struct vdec_information pars;
			pthread_mutex_lock(&dev->mutex);
			ioctl(dev->fd, VDECIO_GET_STATUS, &pars);
            SL_DBG("IOCTL_NAME:VDECIO_GET_STATUS, dev->id: %d, first_header_parsed: %d, first_pic_decoded: %d\n"\
                "first_pic_showed: %d, pic_width: %d, pic_height: %d, aspect_ratio: %d\n"\
                " frames_displayed: %d, frames_decoded: %d, frame_last_pts: %d\n"\
                "buffer_size: %d, buffer_used: %d, frame_rate: %d, interlaced_frame: %d\n"\
                "top_field_first: %d, hw_dec_error: %d, is_support: %d, sar_height: %d\n"\
                "sar_width: %d, active_format: %d, output_mode: %d\n",dev->id,
                pars.first_header_parsed,pars.first_pic_decoded,pars.first_pic_showed,pars.pic_width,
                pars.pic_height,pars.aspect_ratio,pars.frames_displayed,pars.frames_decoded,pars.frame_last_pts,
                pars.buffer_size,pars.buffer_used,pars.frame_rate,pars.interlaced_frame,pars.top_field_first,
                pars.hw_dec_error,pars.is_support,pars.sar_height,pars.sar_width,pars.active_format,pars.output_mode);
			
			memcpy(&(dev->status_info), (void *)&pars, sizeof(pars));
			pthread_mutex_unlock(&dev->mutex);
			
			SL_INFO("%s: MSG_FIRST_SHOWED: %d\n", __func__, dev->id);
			if (dev->cb_first_showed.callback_func) {
				dev->cb_first_showed.callback_func(dev->cb_first_showed.user_data, SL_VDEC_CB_FIRST_SHOWED, 0, 0);
			}
			dev->first_showed = true;
		}
		break;

		case MSG_MODE_SWITCH_OK:
			if (dev->cb_mode_switch_ok.callback_func) {
				dev->cb_mode_switch_ok.callback_func(dev->cb_mode_switch_ok.user_data, SL_VDEC_CB_MODE_SWITH_OK, msg_data[2], msg_data[3]);
			}
			break;

		case MSG_BACKWARD_RESTART_GOP:
			if (dev->cb_backward_restart_gop.callback_func) {
				dev->cb_backward_restart_gop.callback_func(dev->cb_backward_restart_gop.user_data, SL_VDEC_CB_BACKWARD_RESTART_GOP, msg_data[2], msg_data[3]);
			}
			break;

		case MSG_FIRST_HEADRE_PARSED:
			if (dev->cb_first_head_parsed.callback_func) {
				dev->cb_first_head_parsed.callback_func(dev->cb_first_head_parsed.user_data, SL_VDEC_CB_FIRST_HEAD_PARSED, msg_data[2], msg_data[3]);
			}
			break;

		case MSG_FIRST_I_DECODED:
			if (dev->cb_first_i_frame_decoded.callback_func)
				dev->cb_first_i_frame_decoded.callback_func(dev->cb_first_i_frame_decoded.user_data, SL_VDEC_CB_FRIST_I_FRAME_DECODED, msg_data[2], msg_data[3]);
			break;

		case MSG_USER_DATA_PARSED:
			if (dev->cb_user_data_parsed.callback_func && !flag_bit_test_any(&dev->status, VDEC_STATUS_STOP)) {
				dev->cb_user_data_parsed.callback_func(dev->cb_user_data_parsed.user_data, SL_VDEC_CB_USER_DATA_PARSED, (uint32_t)msg_data[2],(uint32_t)(msg_data + 4));
			}
			break;
		case MSG_INFO_CHANGED:
			if (dev->cb_info_changed.callback_func) {
				memset(&vinfo, 0, sizeof(vinfo));
				infocb = (struct vdec_info_cb_param *)(msg_data + 4);
				if (infocb->info_change_flags & VDEC_CHANGE_DIMENSIONS) {
					memset(&vinfo, 0, sizeof(vinfo));
					vinfo.flag = eVDEC_CHANGE_DIMENSIONS;
					vinfo.pic_width = infocb->pic_width;
					vinfo.pic_height = infocb->pic_height;
					dev->cb_info_changed.callback_func(dev->cb_info_changed.user_data, SL_VDEC_CB_INFO_CHANGED, (uint32_t)(&vinfo), 0);
				}
				if (infocb->info_change_flags & VDEC_CHANGE_FRAMERATE) {
					memset(&vinfo, 0, sizeof(vinfo));
					vinfo.flag = eVDEC_CHANGE_FRAMERATE;
					vinfo.frame_rate = infocb->frame_rate;
					dev->cb_info_changed.callback_func(dev->cb_info_changed.user_data, SL_VDEC_CB_INFO_CHANGED, (uint32_t)(&vinfo), 0);
				}
				if (infocb->info_change_flags & VDEC_CHANGE_AFD) {
					memset(&vinfo, 0, sizeof(vinfo));
					vinfo.flag = eVDEC_CHANGE_AFD;
					vinfo.active_format = infocb->active_format;
					dev->cb_info_changed.callback_func(dev->cb_info_changed.user_data, SL_VDEC_CB_INFO_CHANGED, (uint32_t)(&vinfo), 0);
				}
				if (infocb->info_change_flags & VDEC_CHANGE_SAR) {
					memset(&vinfo, 0, sizeof(vinfo));
					vinfo.flag = eVDEC_CHANGE_SAR;
					vinfo.sar_height = infocb->sar_height;
					vinfo.sar_width = infocb->sar_width;
					dev->cb_info_changed.callback_func(dev->cb_info_changed.user_data, SL_VDEC_CB_INFO_CHANGED, (uint32_t)(&vinfo), 0);
				}
				if (0 == infocb->info_change_flags) {
					SL_ERR("maybe error! callback funtion: info_change_flags == 0\n");
				}
			}
			break;
		case MSG_STATE_CHANGED:
			switch ((uint32_t)msg_data[2]) {
				case VDEC_STATE_NODATA:
					status = VDEC_DECODER_STATUS_NODATA;
					break;
				case VDEC_STATE_DECODING:
					status = VDEC_DECODER_STATUS_DECODING;
					break;
				default:
					SL_ERR("unknow status: %d\n", msg_data[2]);
					break;
			}
			if (dev->cb_status_changed.callback_func) {
				dev->cb_status_changed.callback_func(dev->cb_status_changed.user_data, SL_VDEC_CB_STATUS_CHANGED, status, 0);
			}
			break;
		case MSG_ERROR:
			switch ((uint32_t)msg_data[2]) {
				case VDEC_ERROR_NODATA:
					error_code = VDEC_ERROR_CODE_NODATA;
					break;
				case VDEC_ERROR_HARDWARE:
					error_code = VDEC_ERROR_CODE_HARDWARE;
					dev->decode_error_cnt++;  /** used to statistics decode error count */
					//SL_ERR("VDEC_ERROR_CODE_HARDWARE\n");
					break;
				case VDEC_ERROR_SYNC:
					error_code = VDEC_ERROR_CODE_SYNC;
					break;
				case VDEC_ERROR_FRAMEDROP:
					error_code = VDEC_ERROR_CODE_FRAMEDROP;
					break;
				case VDEC_ERROR_FRAMEHOLD:
					error_code = VDEC_ERROR_CODE_FRAMEHOLD;
					break;
				case VDEC_ERROR_GIVEUPSEQ:
					error_code = VDEC_ERROR_CODE_GIVEUPSEQ;
					break;
				default:
					error_code = VDEC_ERROR_CODE_INVDATA;
					//SL_ERR("VDEC_ERROR_INVDATA\n");
					break;
			}
			if (dev->cb_decoder_error.callback_func) {
				dev->cb_decoder_error.callback_func(dev->cb_decoder_error.user_data, SL_VDEC_CB_ERROR, error_code, 0);
			}
			#if 0
			pthread_mutex_lock(&dev->mutex);
			if(dev->if_reg_cb_for_trick) {
				if((error_code == VDEC_ERROR_CODE_HARDWARE) ||(error_code == VDEC_ERROR_CODE_INVDATA)) {
					//printf("%s -> set cb err 1\n",__func__);
					dev->if_cb_err = 1;
				}
			}
			pthread_mutex_unlock(&dev->mutex);
			#endif
			break;
		case MSG_FRAME_DISPLAYED:
			if (dev->cb_frame_displayed.callback_func) {
				dev->cb_frame_displayed.callback_func(dev->cb_frame_displayed.user_data, SL_VDEC_CB_FRAME_DISPLAYED, 0, 0);
			}
			pthread_mutex_lock(&dev->mutex);
			if (dev->if_reg_cb_for_trick) {
				//printf("%s -> set frm dis 1\n",__func__);
				dev->if_frm_dis=1;
			}
			pthread_mutex_unlock(&dev->mutex);
			break;
        case MSG_MONITOR_GOP:
            if (dev->cb_monitor_gop.callback_func) {
                dev->cb_monitor_gop.callback_func(dev->cb_monitor_gop.user_data, SL_VDEC_CB_MONITOR_GOP, 0, 0);
            }
            break;				
		default:
			break;
	}

	pthread_mutex_unlock(&dev->mutex_callback);
	pthread_setcanceltype(oldtype, NULL);
	pthread_cleanup_pop(0);
}

static void *sl_decv_callback_epool_cb(void *argv)
{
    int ret = 0;
    struct alislvdec_dev *dev = (struct alislvdec_dev *)argv;
    unsigned char msg[MAX_KUMSG_SIZE] = {0};
    //printf("************** %s ***********\n", __FUNCTION__);

    if (!dev) {
        SL_ERR("device has been destroyed\n");
        return NULL;
    }

    ret = read(dev->kumsg_fd, msg, MAX_KUMSG_SIZE);

    if (ret > 0) {
        handle_msg(dev, msg);
    }

    return (void *) NULL;
}

static void sl_decv_del_epoll_fd(struct alislvdec_dev *dev)
{
	alislevent_del(dev->vdec_event_handle, &dev->vdec_event);
	alislevent_close(dev->vdec_event_handle);
	dev->vdec_event_handle = NULL;
    //printf("************** %s ***********\n", __FUNCTION__);
	close(dev->kumsg_fd);
	dev->kumsg_fd = -1;
}

static int sl_decv_add_epoll_fd(struct alislvdec_dev *dev)
{
	int flags = O_CLOEXEC;
  
	if((dev->kumsg_fd = ioctl(dev->fd, VDECIO_GET_KUMSGQ, &flags)) < 0) {
		return -1;
	}
	dev->vdec_event.cb = sl_decv_callback_epool_cb;
    dev->vdec_event.data = (void *)dev;
    dev->vdec_event.events = EPOLLIN;
    dev->vdec_event.fd = dev->kumsg_fd;

    if(alislevent_open(&dev->vdec_event_handle)) {
        SL_ERR("%s -> alislevent_open fail\n", __FUNCTION__);
        return -1;
    }

    if(alislevent_add(dev->vdec_event_handle, &dev->vdec_event)) {
        SL_ERR("%s -> alislevent_add fail\n", __FUNCTION__);
        alislevent_close(dev->vdec_event_handle);
        return -1;
    }

	return 1;

}

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
					   struct vdec_mem_info *mem_info)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct ali_video_mem_info mem_info_param;

	CHECK_HANDLE(hdl);

	if (!mem_info) {
		SL_ERR("invalidate mem_info = 0x%08x\n", mem_info);
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	memset(&mem_info_param, 0, sizeof(mem_info_param));
	if (ioctl(dev->fd, VDECIO_GET_MEM_INFO, &mem_info_param)) {
		SL_ERR("ioctl VDECIO_GET_MEM_INFO failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
    SL_DBG("IOCTL_NAME:VDECIO_GET_MEM_INFO, dev->id: %d,mem_start: 0x%x, mem_size: %ld\n"\
        "priv_mem_start: 0x%x, priv_mem_size: %ld, mp_mem_start: 0x%x, mp_mem_size: %ld\n"\
        "vbv_mem_start: 0x%x, vbv_mem_size: %ld\n",dev->id,mem_info_param.mem_start,
        mem_info_param.mem_size,mem_info_param.priv_mem_start,mem_info_param.priv_mem_size,
        mem_info_param.mp_mem_start,mem_info_param.mp_mem_size,mem_info_param.vbv_mem_start,
        mem_info_param.vbv_mem_size);
	mem_info->mem_start = mem_info_param.mem_start;
	mem_info->mem_size = mem_info_param.mem_size;
	mem_info->priv_mem_start = mem_info_param.priv_mem_start;
	mem_info->priv_mem_size = mem_info_param.priv_mem_size;
	mem_info->mp_mem_start = mem_info_param.mp_mem_start;
	mem_info->mp_mem_size = mem_info_param.mp_mem_size;
	mem_info->vbv_mem_size = mem_info_param.vbv_mem_size;
	mem_info->vbv_mem_start = mem_info_param.vbv_mem_start;

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}
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
alislvdec_get_av_frame_struct_size(unsigned int *size)
{
	if (size) {
		*size = sizeof(struct av_frame);
		return VDEC_ERR_NOERR;
	} else
		return VDEC_ERR_INVAL;
}
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
				  struct vdec_mp_init_config *init_config)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_init_param init_param;
	uint32_t codec_tag;
	uint32_t decode_mode;

	CHECK_HANDLE(hdl);
    memset(&init_param, 0, sizeof(init_param));
	if (!init_config) {
		SL_ERR("invalid init_config = 0x%08x\n", init_config);
		return VDEC_ERR_INVAL;
	}
	switch(init_config->codec_id) {
		case VDEC_ID_H264:
			codec_tag = h264;
			break;
		case VDEC_ID_XVID:
			codec_tag = xvid;
			break;
		case VDEC_ID_MPG2:
			codec_tag = mpg2;
			break;
		case VDEC_ID_FLV1:
			codec_tag = flv1;
			break;
		case VDEC_ID_VP8:
			codec_tag = vp8;
			break;
		case VDEC_ID_WVC1:
			codec_tag = wvc1;
			break;
		case VDEC_ID_WMV3:
			codec_tag = wx3;
			break;
		case VDEC_ID_WX3:
			codec_tag = wx3;
			break;
		case VDEC_ID_RMVB:
			codec_tag = rmvb;
			break;
		case VDEC_ID_MJPG:
			codec_tag = mjpg;
			break;
		case VDEC_ID_VC1:
			codec_tag = vc1;
			break;
		case VDEC_ID_XD:
			codec_tag = xvid;
			break;
		case VDEC_ID_HEVC:
			codec_tag = hevc;
			break;
		default:
			codec_tag = 0;
			break;
	}

	switch(init_config->decode_mode) {
		case VDEC_WORK_MODE_VOL:
			decode_mode = VDEC_MODE_VOL;
			break;
		case VDEC_WORK_MODE_HEADER:
			decode_mode = VDEC_MODE_HEADER;
			break;
		case VDEC_WORK_MODE_SKIP_B_FRAME:
			decode_mode = VDEC_MODE_SKIP_B_FRAME;
			break;
		case VDEC_WORK_MODE_SKIP_B_P_FRAME:
			decode_mode = VDEC_MODE_SKIP_B_P_FRAME;
			break;
		case VDEC_WORK_MODE_SBM:
			decode_mode = VDEC_MODE_SBM;
			break;
		case VDEC_WORK_MODE_SBM_STREAM:
			decode_mode = VDEC_MODE_SBM_STREAM;
			break;
		default:
			decode_mode = VDEC_MODE_NORMAL;
			break;
	}

	init_param.codec_tag = codec_tag;
	init_param.decode_mode = decode_mode;
	init_param.decoder_flag = init_config->decoder_flag;
	init_param.preview = init_config->preview;
	/**driver use frame rate * 10000 to caculte the frame rate when 
 	there is no frame rate info in the video data, we use frame rate * 1000 as default, 
	so we need to *10 as default **/
	init_param.frame_rate = init_config->frame_rate * 10;
	init_param.pic_width = init_config->pic_width;
	init_param.pic_height = init_config->pic_height;
	init_param.pixel_aspect_x = init_config->pixel_aspect_x;
	init_param.pixel_aspect_y = init_config->pixel_aspect_y;
	init_param.dec_buf_addr = init_config->dec_buf_addr;
	init_param.dec_buf_size = init_config->dec_buf_size;
	//add encrypt_mode for vmx ott project
    init_param.encrypt_mode = init_config->encrypt_mode;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_INITILIZE, dev->id: %d, codec_tag: %d, decode_mode: %d, decoder_flag: %d\n"\
        "preview: %d, frame_rate: %d, pic_width: %d, pic_height: %d, pixel_aspect_x: %d\n"\
        "pixel_aspect_y: %d, dec_buf_addr: 0x%x, dec_buf_size: %d, encrypt_mode: %d\n",
        dev->id,init_param.codec_tag,init_param.decode_mode,init_param.decoder_flag,init_param.preview,
        init_param.frame_rate,init_param.pic_width,init_param.pic_height,init_param.pixel_aspect_x,
        init_param.pixel_aspect_y,init_param.dec_buf_addr,init_param.dec_buf_size, init_param.encrypt_mode);
	if (ioctl(dev->fd, VDECIO_MP_INITILIZE, &init_param)) {
		SL_ERR("ioctl VDECIO_MP_INITILIZE failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
    dev->es_flag = 1;
	/** reg decode error callback, used to statistics decode error count, for aui use */
	dev->decode_error_cnt = 0;
    SL_DBG("IOCTL_NAME:VDECIO_REG_CALLBACK\n");
	if (ioctl(dev->fd, VDECIO_REG_CALLBACK, VDEC_CB_ERROR)) {
		SL_ERR("ioctl VDECIO_REG_CALLBACK return failed\n");
	}
quit:
	pthread_mutex_unlock(&dev->mutex);

	if (dev->dis_layer < VDEC_DIS_LAYER_MAX) {
		//set dis player layer after VDECIO_MP_INITILIZE
		alislvdec_set_display_layer(hdl, dev->dis_layer);
		
		if (m_display_setting[dev->dis_layer].out_mode != VDEC_OUT_LAST) {
			ret = alislvdec_set_display_mode(hdl, &m_display_setting[dev->dis_layer]);
		}
	}

	vdec_reg_callback_before_playing(dev);
	/*
	App may set some attributes before setting video format, and these attributes
	are stored and would be configured again internally to driver after setting video format.
	*/
	vdec_set_attrs_before_decoding(dev);
	return ret;
}

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
alislvdec_mp_release(alisl_handle hdl)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_rls_param rls_param;

	CHECK_HANDLE(hdl);

	memset(&rls_param, 0, sizeof(rls_param));

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_RELEASE, dev->id: %d\n",dev->id);
	if (ioctl(dev->fd, VDECIO_MP_RELEASE, &rls_param)) {
		SL_ERR("ioctl VDECIO_MP_RELEASE failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
	dev->decode_error_cnt = 0;
	dev->continue_on_dec_err = -1;
	dev->variable_resolution = -1;
quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
alislvdec_mp_pause(alisl_handle hdl)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_pause_param pause_param;

	CHECK_HANDLE(hdl);

	pause_param.pause_decode = 1;
	pause_param.pause_display = 1;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_PAUSE, dev->id: %d, pause_decode: %d, pause_display: %d\n",dev->id,
        pause_param.pause_decode,pause_param.pause_display);
	if (ioctl(dev->fd, VDECIO_MP_PAUSE, &pause_param)) {
		SL_ERR("ioctl VDECIO_MP_PAUSE failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}
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
alislvdec_mp_resume(alisl_handle hdl)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_pause_param pause_param;

	CHECK_HANDLE(hdl);

	pause_param.pause_decode = 0;
	pause_param.pause_display = 0;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_PAUSE, dev->id: %d, pause_decode: %d, pause_display: %d\n",dev->id,
        pause_param.pause_decode,pause_param.pause_display);
	if (ioctl(dev->fd, VDECIO_MP_PAUSE, &pause_param)) {
		SL_ERR("ioctl VDECIO_MP_PAUSE failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
				   enum vdec_playback_mode playback_mode)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_flush_param flush_param;

	CHECK_HANDLE(hdl);

	if (playback_mode == VDEC_PLAYBACK_MODE_NORMAL)
		flush_param.flush_flag = 1;
	else
		flush_param.flush_flag = 3;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_FLUSH, dev->id: %d, flush_flag: %d\n",dev->id,flush_param.flush_flag);
	if (ioctl(dev->fd, VDECIO_MP_FLUSH, &flush_param)) {
		SL_ERR("ioctl VDECIO_MP_FLUSH failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
					 struct vdec_mp_sbm_config *sbm_config)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_sbm_param sbm_param;

	CHECK_HANDLE(hdl);

	if (!sbm_config) {
		SL_ERR("invalidate sbm_config = 0x%08x\n", sbm_config);
		return VDEC_ERR_INVAL;
	}
	sbm_param.packet_header = sbm_config->packet_header;
	sbm_param.packet_data = sbm_config->packet_data;
	sbm_param.decode_output = sbm_config->decode_output;
	sbm_param.display_input = sbm_config->display_input;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_SET_SBM_IDX, dev->id: %d, packet_header: %d\n packet_data: %d,"\
        "decode_output: %d, display_input: %d\n", dev->id,sbm_param.packet_header,
        sbm_param.packet_data,sbm_param.decode_output,sbm_param.display_input);
	if (ioctl(dev->fd, VDECIO_MP_SET_SBM_IDX, &sbm_param)) {
		SL_ERR("ioctl VDECIO_MP_SET_SBM_IDX failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
							struct vdec_mp_extra_data_config *data_config)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_extra_data extra_data_param;

	CHECK_HANDLE(hdl);

	if (!data_config) {
		SL_ERR("invalidate data_config = 0x%08x\n", data_config);
		return VDEC_ERR_INVAL;
	}
	extra_data_param.extra_data = data_config->extra_data;
	extra_data_param.extra_data_size = data_config->extra_data_size;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_EXTRA_DATA, dev->id: %d, extra_data: %p, extra_data_size: %d\n",dev->id,
        extra_data_param.extra_data,extra_data_param.extra_data_size);
	if (ioctl(dev->fd, VDECIO_MP_EXTRA_DATA, &extra_data_param)) {
		SL_ERR("ioctl VDECIO_MP_EXTRA_DATA failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

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
						   struct vdec_mp_sync_config *sync_config)
{
	struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode           ret = VDEC_ERR_NOERR;
	struct vdec_mp_sync_param sync_param;

	CHECK_HANDLE(hdl);

	if (!sync_config) {
		SL_ERR("invalidate sync_config = 0x%08x\n", sync_config);
		return VDEC_ERR_INVAL;
	}

	if (sync_config->sync_mode == VDEC_AV_SYNC_FREERUN) {
		sync_param.sync_mode = AV_SYNC_NONE;
	} else {
		sync_param.sync_mode = AV_SYNC_AUDIO;
	}
	sync_param.sync_unit = sync_config->sync_unit;

	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_MP_SET_SYNC_MODE, dev->id: %d, sync_mode: %d, sync_unit: %d\n",dev->id,
        sync_param.sync_mode,sync_param.sync_unit);
	if (ioctl(dev->fd, VDECIO_MP_SET_SYNC_MODE, &sync_param)) {
		SL_ERR("ioctl VDECIO_MP_SET_SYNC_MODE failed\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

/**
 * Function Name: alislvdec_mp_set_frame_type
 * @brief         Set VDEC's frame type in mp mode
 *
 * @param[in] hdl Device handl
 * @param[in] frame_type Frame type to set to mp VDEC device
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
							enum vdec_frame_type frame_type)
{
	alisl_retcode ret = VDEC_ERR_NOERR;
	ret = vdec_set_frame_type(hdl, VDECIO_MP_SET_DEC_FRM_TYPE, frame_type);
	return ret;
}

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
							 struct vdec_rect *drect)
{
	alisl_retcode ret = VDEC_ERR_NOERR;
	ret = vdec_set_output_rect(hdl, VDECIO_MP_SET_DISPLAY_RECT, srect, drect);
	return ret;
}

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
alislvdec_mp_get_info(alisl_handle      hdl,
					  struct vdec_mp_info *info)
{
	struct alislvdec_dev       *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode              ret = VDEC_ERR_NOERR;
	struct vdec_decore_status  dec_status;

	CHECK_HANDLE(hdl);

	if (!info) {
		SL_ERR("invalid info\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&dev->mutex);

	if (ioctl(dev->fd, VDECIO_MP_GET_STATUS, &dec_status)) {
		SL_ERR("ioctl VDECIO_MP_GET_STATUS failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
		goto quit;
	}
    SL_DBG("IOCTL_NAME:VDECIO_MP_GET_STATUS, dev->id: %d, decode_status: %d, pic_width: %d, pic_height: %d\n"\
        "sar_width: %d, sar_height: %d, frame_rate: %d, interlaced_frame: %d, top_field_first: %d\n"\
        "first_header_got: %d, first_pic_showed: %d, frames_decoded: %d, frames_displayed: %d, frame_last_pts: %d\n"\
        "buffer_size: %d, buffer_used: %d, decode_error: %d, decoder_feature: %d, under_run_cnt: %d\n"\
        "first_pic_decoded: %d, output_mode: %d, frame_angle: %d,layer: %d\n",dev->id,dec_status.decode_status,
        dec_status.pic_width,dec_status.pic_height,dec_status.sar_width,dec_status.sar_height,dec_status.frame_rate,
        dec_status.interlaced_frame,dec_status.top_field_first,dec_status.first_header_got,dec_status.first_pic_showed,
        dec_status.frames_decoded,dec_status.frames_displayed,dec_status.frame_last_pts,dec_status.buffer_size,
        dec_status.buffer_used,dec_status.decode_error,dec_status.decoder_feature,dec_status.under_run_cnt,
        dec_status.first_pic_decoded,dec_status.output_mode,dec_status.frame_angle,dec_status.layer);    
	info->decode_status = dec_status.decode_status;
	info->pic_width = dec_status.pic_width;
	info->pic_height = dec_status.pic_height;
	info->sar_width = dec_status.sar_width;
	info->sar_heigth = dec_status.sar_height;
	info->frame_rate = dec_status.frame_rate;
	info->interlaced_frame = dec_status.interlaced_frame;
	info->top_field_first = dec_status.top_field_first;
	info->first_header_got = dec_status.first_header_got;
	info->first_pic_showed = dec_status.first_pic_showed;
	info->frames_decoded = dec_status.frames_decoded;
	info->frames_displayed = dec_status.frames_displayed;
	info->frame_last_pts = dec_status.frame_last_pts;
	info->buffer_size = dec_status.buffer_size;
	info->buffer_used = dec_status.buffer_used;
	info->decode_error = dec_status.decode_error;
	info->decoder_feature = dec_status.decoder_feature;
	info->under_run_cnt = dec_status.under_run_cnt;
	info->first_pic_decoded = dec_status.first_pic_decoded;
	info->output_mode = dec_status.output_mode;
	info->frame_angle = dec_status.frame_angle;
	if (dec_status.layer == VPO_MAINWIN) {
		info->layer = VDEC_DIS_MAIN_LAYER;
	} else if (dec_status.layer == VPO_PIPWIN){
		info->layer = VDEC_DIS_AUX_LAYER;
	} else {
		info->layer = VDEC_DIS_LAYER_MAX;
	}
	/** registered callback in aliplatform layer, statistical decode error count */
	info->decode_error_cnt    = dev->decode_error_cnt;

quit:
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

/**
 * Function Name: alislvdec_set_variable_resolution
 * @brief         Set VDEC to surport variable resolution
 * @param[in]     hdl Device handle
 * @param[in]     enable Enable for disable
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_variable_resolution(alisl_handle  hdl,
                                  bool          enable)
{
    struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
    CHECK_HANDLE(hdl);
    unsigned long vdec_enable = enable ? 1 : 0;
    dev->variable_resolution = enable;
    pthread_mutex_lock(&dev->mutex);
    if(dev->es_flag) {
        SL_DBG("IOCTL_NAME:VDECIO_MP_DYNAMIC_FRAME_ALLOC, dev->id: %d, vdec_enable: %ld\n",dev->id,vdec_enable);
        if (ioctl(dev->fd, VDECIO_MP_DYNAMIC_FRAME_ALLOC, vdec_enable) < 0) {
            //ret = VDEC_ERR_IOCTLFAILED; //ignore the return value
            SL_ERR("ioctl VDECIO_MP_DYNAMIC_FRAME_ALLOC fail!\n");
            }
    } else {
        SL_DBG("IOCTL_NAME:VDECIO_DYNAMIC_FRAME_ALLOC\n");
        if (ioctl(dev->fd, VDECIO_DYNAMIC_FRAME_ALLOC, vdec_enable) < 0) {
            //ret = VDEC_ERR_IOCTLFAILED; //ignore the return value
            SL_ERR("ioctl VDECIO_DYNAMIC_FRAME_ALLOC fail!\n");
        }
    }
    pthread_mutex_unlock(&dev->mutex);

    return VDEC_ERR_NOERR;
}

/**
 * Function Name: alislvdec_set_display_mode
 * @brief         a new interface integrates setting output and setting output rect
 * @param[in]     hdl Device handle
 * @param[in]     out put mode and rect
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          15/08/2014, Created
 *
 * @note
 */
alisl_retcode
alislvdec_set_display_mode(alisl_handle                 hdl,
                           struct vdec_display_setting  *setting)
{
    struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
    struct vdec_display_param display_param;
    enum vdec_output_mode vdec_out_mode;
    alisl_retcode ret = VDEC_ERR_NOERR;

    CHECK_HANDLE(hdl);
	if (!setting) {
		SL_ERR("Invalid parameters set\n");
		return VDEC_ERR_INVAL;
	}
	if (dev->dis_layer < VDEC_DIS_LAYER_MAX)
		memcpy(&m_display_setting[dev->dis_layer], setting, sizeof(struct vdec_display_setting));
	switch (setting->out_mode) {
		case VDEC_OUT_FULLVIEW :
			vdec_out_mode = VDEC_FULL_VIEW;
			break;
		case VDEC_OUT_PREVIEW  :
			vdec_out_mode = VDEC_PREVIEW;
			break;
		case VDEC_OUT_SWPASS   :
			vdec_out_mode = VDEC_SW_PASS;
			break;
		default:
			SL_ERR("invalid out_mode = 0x%08x\n", setting->out_mode);
			return VDEC_ERR_INVAL;
	}
	display_param.mode = vdec_out_mode;
	display_param.rect.src_x = setting->srect.x;
	display_param.rect.src_y = setting->srect.y;
	display_param.rect.src_w = setting->srect.w;
	display_param.rect.src_h = setting->srect.h;
	display_param.rect.dst_x = setting->drect.x;
	display_param.rect.dst_y = setting->drect.y;
	display_param.rect.dst_w = setting->drect.w;
	display_param.rect.dst_h = setting->drect.h;
    pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_SET_DISPLAY_MODE, dev->id: %d,dis_layer: %d, mode: %d, src(%d, %d, %d, %d) -> dst(%d, %d, %d, %d)\n",
        dev->id,dev->dis_layer,display_param.mode,display_param.rect.src_x,display_param.rect.src_y,display_param.rect.src_w,display_param.rect.src_h,
        display_param.rect.dst_x,display_param.rect.dst_y,display_param.rect.dst_w,display_param.rect.dst_h);
    if (ioctl(dev->fd, VDECIO_SET_DISPLAY_MODE, &display_param)) {
        SL_ERR("ioctl VDECIO_SET_DISPLAY_MODE fail!\n");
        ret = VDEC_ERR_IOCTLFAILED;
    }
    pthread_mutex_unlock(&dev->mutex);

    return ret;
}

alisl_retcode
alislvdec_set_continue_on_error(alisl_handle  hdl,
                                bool          enable)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);
	unsigned long vdec_enable = enable ? 1:0;
	dev->continue_on_dec_err = enable;
	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_CONTINUE_ON_ERROR, dev->id: %d, vdec_enable: %d\n",dev->id,vdec_enable);
	if (ioctl(dev->fd, VDECIO_CONTINUE_ON_ERROR, vdec_enable)) {
		SL_ERR("ioctl VDECIO_CONTINUE_ON_ERROR failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

alisl_retcode
alislvdec_set_parse_afd(alisl_handle  hdl, bool enable)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);
	unsigned long vdec_enable = enable ? 1:0;
	m_parse_afd = vdec_enable;
	pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_PARSE_AFD, dev->id: %d, vdec_enable: %d\n",dev->id,vdec_enable);
	if (ioctl(dev->fd, VDECIO_PARSE_AFD, vdec_enable)) {
		SL_ERR("ioctl VDECIO_PARSE_AFD failed!\n");
		ret = VDEC_ERR_IOCTLFAILED;
	}
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

alisl_retcode alislvdec_get_id(alisl_handle  hdl, enum vdec_video_id *id)
{
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	alisl_retcode ret = VDEC_ERR_NOERR;
	CHECK_HANDLE(hdl);
	if (!id) {
		SL_ERR("invalid id\n");
		return VDEC_ERR_INVAL;
	}
	*id = dev->id;
	return ret;
}

alisl_retcode alislvdec_get_decoder_by_id(enum vdec_video_id id, alisl_handle *hdl)
{
	struct alislvdec_dev *dev;
	int i;
	if (!hdl) {
		SL_ERR("invalid hdl\n");
		return VDEC_ERR_INVAL;
	}

	for (i = 0; i < ARRAY_SIZE(dev_video); i++) {
		if (id == dev_video[i].id) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev_video)) {
		SL_ERR("Invalidate video id!\n");
		return VDEC_ERR_INVAL;
	}

	pthread_mutex_lock(&m_mutex);

	if (m_dev[i] == NULL) {
		pthread_mutex_unlock(&m_mutex);
		return VDEC_ERR_INVAL;
	} else {
		dev = m_dev[i];
	}

	if (flag_bit_test_any(&dev->status, VDEC_STATUS_OPEN)) {
		SL_DBG("Already opened!\n");
		*hdl = (alisl_handle)dev;
		pthread_mutex_unlock(&m_mutex);
		return VDEC_ERR_NOERR;
	} else {
		pthread_mutex_unlock(&m_mutex);
		return VDEC_ERR_INVAL;
	}
}
alisl_retcode alislvdec_set_display_layer(alisl_handle hdl, enum vdec_dis_layer layer)
{
	alisl_retcode         ret = VDEC_ERR_NOERR;
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
	struct vdec_pip_param pip_param;
	CHECK_HANDLE(hdl);
	pthread_mutex_lock(&dev->mutex);
	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_OPEN)) {
		pthread_mutex_unlock(&dev->mutex);
		SL_ERR("Not opened!\n");
		ret = VDEC_ERR_INVALHANDLE;
		return ret;
	}
	if (layer == VDEC_DIS_AUX_LAYER)
		pip_param.layer = VPO_PIPWIN;
	else
		pip_param.layer = VPO_MAINWIN;
	dev->dis_layer = layer;
    SL_DBG("IOCTL_NAME: VDECIO_SET_PIP_PARAM, dev->id: %d, layer: %d\n",dev->id,pip_param.layer);
	ret = ioctl(dev->fd, VDECIO_SET_PIP_PARAM, &pip_param);
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

alisl_retcode alislvdec_get_display_layer(alisl_handle hdl, enum vdec_dis_layer* layer)
{
	alisl_retcode         ret = VDEC_ERR_NOERR;
	struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
//	struct vdec_pip_param pip_param;
	CHECK_HANDLE(hdl);
	if (!layer) {
		SL_ERR("invalid layer\n");
		return VDEC_ERR_INVAL;
	}
	pthread_mutex_lock(&dev->mutex);
	if (!flag_bit_test_any(&dev->status, VDEC_STATUS_OPEN)) {
		pthread_mutex_unlock(&dev->mutex);
		SL_ERR("Not opened!\n");
		ret = VDEC_ERR_INVALHANDLE;
		return ret;
	}
	SL_DBG("layer %d\n", dev->dis_layer);
	*layer = dev->dis_layer;
	pthread_mutex_unlock(&dev->mutex);
	return ret;
}

alisl_retcode
alislvdec_store_display_rect(enum vdec_dis_layer layer,
                             struct vdec_display_setting  *setting)
{
    alisl_retcode ret = VDEC_ERR_NOERR;

	if (layer >= VDEC_DIS_LAYER_MAX || setting == NULL) {
		SL_ERR("Invalidate paremeters set\n");
		return VDEC_ERR_INVAL;
	}
	memcpy(&m_display_setting[layer], setting, sizeof(struct vdec_display_setting));
    return ret;
}

alisl_retcode
alislvdec_get_display_rect(enum vdec_dis_layer layer, struct vdec_display_setting  *setting)
{
	alisl_retcode ret = VDEC_ERR_NOERR;

	if (layer >= VDEC_DIS_LAYER_MAX || setting == NULL) {
		SL_ERR("Invalidate paremeters set\n");
		return VDEC_ERR_INVAL;
	}
	memcpy(setting, &m_display_setting[layer], sizeof(struct vdec_display_setting));
    return ret;
}

alisl_retcode
alislvdec_show_last_frame(alisl_handle hdl, vdec_show_last_frame_mode mode)
{
    struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
    alisl_retcode           ret = VDEC_ERR_NOERR;
    unsigned long flush = (mode == VDEC_SHOW_LAST_FRAM_IMMEDIATE) ? 1 : 0;

    CHECK_HANDLE(hdl);

    pthread_mutex_lock(&dev->mutex);
    SL_DBG("IOCTL_NAME:VDECIO_FLUSH, dev->id: %d, flush: %ld\n",dev->id,flush);
    if (ioctl(dev->fd, VDECIO_FLUSH, flush)) {
        SL_ERR("ioctl VDECIO_FLUSH failed\n");
        ret = VDEC_ERR_IOCTLFAILED;
        goto quit;
    }

quit:
    pthread_mutex_unlock(&dev->mutex);
    return ret;
}

alisl_retcode
alislvdec_set_user_data_type(alisl_handle hdl, vdec_user_data_type type)
{
    struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
    alisl_retcode           ret = VDEC_ERR_NOERR;
    unsigned long get_all = 0;//VDEC_USER_DATA_DEFAULT
    unsigned long get_user_data_info = 0;//VDEC_USER_DATA_DEFAULT

    CHECK_HANDLE(hdl);

    pthread_mutex_lock(&dev->mutex);

    if (type == VDEC_USER_DATA_ALL) {
        get_all = 1;
        get_user_data_info = 1;
    }

    if (ioctl(dev->fd, VDECIO_GET_ALL_USER_DATA, get_all)) {
        SL_ERR("ioctl VDECIO_GET_ALL_USER_DATA failed\n");
        ret = VDEC_ERR_IOCTLFAILED;
        goto quit;
    }
    SL_DBG("IOCTL_NAME:VDECIO_GET_ALL_USER_DATA, dev->id: %d, get_all: %ld\n",dev->id,get_all);
    if (ioctl(dev->fd, VDECIO_GET_USER_DATA_INFO, get_user_data_info)) {
        SL_ERR("ioctl VDECIO_GET_USER_DATA_INFO failed\n");
        ret = VDEC_ERR_IOCTLFAILED;
        goto quit;
    }

quit:
    pthread_mutex_unlock(&dev->mutex);
    return ret;
}

alisl_retcode
alislvdec_set_sync_repeat_interval(alisl_handle hdl, unsigned int interval)
{
    struct alislvdec_dev   *dev = (struct alislvdec_dev *)hdl;
    alisl_retcode           ret = VDEC_ERR_NOERR;

    CHECK_HANDLE(hdl);

    pthread_mutex_lock(&dev->mutex);

    if (ioctl(dev->fd, VDECIO_SET_SYNC_REPEAT_INTERVAL, interval)) {
        SL_ERR("ioctl failed\n");
        ret = VDEC_ERR_IOCTLFAILED;
    }

    pthread_mutex_unlock(&dev->mutex);
    return ret;
}

alisl_retcode
alislvdec_get_decoder_ext(alisl_handle            hdl,
					  enum vdec_decoder_type *type)
{
    struct alislvdec_dev *dev = (struct alislvdec_dev *)hdl;
    alisl_retcode         ret = VDEC_ERR_NOERR;
    enum video_decoder_type  cur_type;

    CHECK_HANDLE(hdl);

    if (!type) {
        SL_ERR("invalidate type\n");
        return VDEC_ERR_INVAL;
    }

    pthread_mutex_lock(&dev->mutex);

    if (ioctl(dev->fd, VDECIO_GET_CUR_DECODER, &cur_type)) {
        SL_ERR("ioctl VDECIO_GET_CUR_DECODER failed!\n");
        ret = VDEC_ERR_IOCTLFAILED;
        goto quit;
    }
    
    SL_DBG("IOCTL_NAME:VDECIO_GET_CUR_DECODER, dev->id: %d, cur_type: %d\n",dev->id,cur_type);

    switch(cur_type) {
        case MPEG2_DECODER:
            *type = VDEC_DECODER_MPEG; //both ts and mp
            break;
        case H264_DECODER:
            *type = VDEC_DECODER_AVC; //both ts and mp
            break;
        case AVS_DECODER:
            *type = VDEC_DECODER_AVS; //both ts and mp
            break;
        case H265_DECODER:
            *type = VDEC_DECODER_HEVC; //both ts and mp
            break;
        case VC1_DECODER:
            *type = VDEC_DECODER_VC1;
            break;
        case MPEG4_DECODER:
            *type = VDEC_DECODER_MPEG4;
            break;
        case VP8_DECODER:
            *type = VDEC_DECODER_VP8;
            break;
        case RV_DECODER:
            *type = VDEC_DECODER_RV;
            break;	
        case MJPG_DECODER:
            *type = VDEC_DECODER_MJPG;
            break;	
        default:
            SL_WARN("invlidate codec type : 0x%08x\n", cur_type);
            *type = cur_type;
            ret = VDEC_ERR_INVAL;
            break;
    }

quit:
    pthread_mutex_unlock(&dev->mutex);
    return ret;
}
