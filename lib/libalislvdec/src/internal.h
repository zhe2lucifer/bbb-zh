#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <alipltflog.h>
#include <alislevent.h>
#include <bits_op.h>
#include <flag_op.h>

#include "alislvdec.h"
#undef INT64_MAX
#include <ali_video_common.h>

#include <stdio.h>

#define MAX_KUMSG_SIZE      1024

#define VDEC_STATUS_NONE         (0)
#define VDEC_STATUS_ALL          (0xFFFFFFFF)
#define VDEC_STATUS_CONSTRUCT    (1 << 0)
#define VDEC_STATUS_OPEN         (1 << 1)
#define VDEC_STATUS_START        (1 << 2)
#define VDEC_STATUS_PAUSE        (1 << 3)
#define VDEC_STATUS_STOP         (1 << 4)

enum alislvdec_errcode
{
    VDEC_ERR_NOERR               = ERROR_NONE,
    VDEC_ERR_INVAL               = ERROR_INVAL,
    VDEC_ERR_INVALHANDLE         = ERROR_INVAL,
    VDEC_ERR_FILEOPENFAILED      = ERROR_OPEN,
    VDEC_ERR_PTHREADCREATEFAILED = ERROR_PTCREATE,
    VDEC_ERR_NOMEM               = ERROR_NOMEM,
    VDEC_ERR_IOCTLFAILED         = ERROR_IOCTL,
    VDEC_ERR_FAILED              = ERROR_FAILED,
};

typedef struct vdec_callback_info {
    vdec_callback callback_func;
    void *user_data;
}vdec_callback_info;

struct alislvdec_dev {
	enum vdec_video_id             id;
	int fd;
	uint32_t open_cnt;
	/* callbacks */
	vdec_callback_info cb_first_showed;
	vdec_callback_info cb_mode_switch_ok;
	vdec_callback_info cb_backward_restart_gop;
	vdec_callback_info cb_first_head_parsed;
	vdec_callback_info cb_first_i_frame_decoded;
	vdec_callback_info cb_user_data_parsed;
	vdec_callback_info cb_info_changed;
	vdec_callback_info cb_status_changed;
	vdec_callback_info cb_decoder_error;
	vdec_callback_info cb_frame_displayed;
	vdec_callback_info cb_monitor_gop;

	struct vdec_information status_info;
	bool first_showed;
	bool invalid_dma;
	pthread_t       callback_thread;
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_callback;
	struct flag             status;

	enum vdec_dis_layer dis_layer;//can not swap right now, store dis layer in SL
    uint32_t decode_error_cnt;  /** used to statistics decode error count */
	//check whether register decode error cb &frame_displayed cb for trick play
	bool if_reg_cb_for_trick;
	bool if_frm_dis;
	bool if_cb_err;

	struct alislevent vdec_event;
	alisl_handle vdec_event_handle;
	int kumsg_fd;/**< used for kernel-userspace messaging */
	int continue_on_dec_err;
	int variable_resolution;
    int es_flag;//A sign to distinguish between LIVE and mediaplay's DYNAMIC_FRAME_ALLOC,1:mediaplay 0:live 
};

#ifdef __cplusplus
}
#endif

#endif    /* _INTERNAL_H_ */
