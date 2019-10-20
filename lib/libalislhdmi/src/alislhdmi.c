/**@file
*  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
*  All rights reserved
*
*  @file           alislhdmi.c
*  @brief          hdmi device function wrapping
*
*  @version        1.0
*  @date           Fri 27 Sep 2013 10:18:36 AM CST
*  @revision       none
*
*  @author         Ze Hong <ze.hong@alitech.com>
*/

#include <ali_hdmi_common.h>
/* global valualbe */

/* System headerfiles */
#include <malloc.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <linux/version.h> 

/* Shared library headerfiles */
#include <alipltflog.h>
#include <alipltfintf.h>
#include <alislhdmi.h>

/* Local headerfiles */
#include "internal.h"

/* ALi common headerfiles */
#include <ali_media_common.h>
#include <ali_video_common.h>
#include <hdmi_io_common.h>

#define HDMI_EDID_BLOCK_LENGTH 128
#define EDID_BLOCK_NUM_MAX 8

#define MAX_KUMSG_SIZE 1024

#define ALISL_HDCP14_MEM_KEY_LENGTH 286
#define ALISL_HDCP22_MEM_KEY_LENGTH 402
#define ALISL_HDCP22_CE_KEY_LENGTH 416

static alisl_hdmi_3d_descriptor hdmi_desc;
static pthread_mutex_t       hdmi_alisl_mutex      = PTHREAD_MUTEX_INITIALIZER;
static uint32_t              hdmi_open_count       = 0;
static st_hdmi_device       *hdmi_alisl_dev        = NULL;
//static char netlink_msg_buffer[NLMSG_SPACE(MAX_NETLINK_MSG_SIZE)] = {0};
static unsigned char alisl_hdcp_key[286];


static inline alisl_retcode check_ptr(void *ptr)
{
    return (ptr == NULL) ? 0 : 1;
}
/*
static void message_convert(st_msg_desc *msg,
                            uint8_t *data)
{
    msg->type = data[0];
    msg->len = (data[1] << 8) | data[2];
    msg->data = &data[3];
}
*/
static void *hdmi_netlink_daemon(void *handle)//(alisl_handle handle)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    //st_msg_desc msg;

    uint8_t  msg_type   = 0;
    uint16_t msg_length = 0;
    uint8_t *msg_data   = 0;


    //while(1) {
        //uint8_t *data = (unsigned char *)netlink_receive_msg(&pdev->nl);
        unsigned char data[MAX_KUMSG_SIZE] = {0};
        //if (NULL == data) {
        if (read(pdev->kumsgfd, data, MAX_KUMSG_SIZE) < 0) {
            //usleep(100);
        } else {
            msg_type   = data[0];
            msg_length = (data[1] << 8) | data[2];
            msg_data   = &(data[3]);
            switch (msg_type) {
                case ALI_HDMI_MSG_EDID:
                    if (check_ptr(pdev) && check_ptr(pdev->edid_ready)) {
                        pdev->edid_ready();
                    } else {
                        SL_DBG("HDMI_ERROR_BADCB");
                    }
                    break;
                case ALI_HDMI_MSG_PLUGIN:
                    /*
                     * unknown callback for ShangHai shall be called
                     */
                    if (check_ptr(pdev) && check_ptr(pdev->hotplug_in)) {
                        pdev->hotplug_in();
                    } else {
                        SL_DBG("HDMI_ERROR_BADCB");
                    }
                    break;
                case ALI_HDMI_MSG_PLUGOUT:
                    /*
                     * unknown callback for ShangHai shall be called
                     */
                    if (check_ptr(pdev) && check_ptr(pdev->hotplug_out)) {
                        pdev->hotplug_out();
                    } else {
                        SL_DBG("HDMI_ERROR_BADCB");
                    }
                    break;
                case ALI_HDMI_MSG_CEC:
                    if (check_ptr(pdev) && check_ptr(pdev->cec_receive)) {
                        pdev->cec_receive(msg_data, (uint8_t)msg_length);
                    } else {
                        SL_DBG("HDMI_ERROR_BADCB");
                    }
                    break;
                case ALI_HDMI_MSG_HDCP: //fawn++
                    if (check_ptr(pdev) && check_ptr(pdev->hdcp_fail)) {
                        pdev->hdcp_fail(msg_data, (uint8_t)msg_length);
                    } else {
                        SL_DBG("HDMI_ERROR_BADCB");
                    }
                    break;
            }
        }
        return 0;
    //}
}

static alisl_retcode hdmi_daemon_monitor_create(alisl_handle handle)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
#if 0
    alisl_retcode retcode = pthread_create(&(pdev->tid),
                                           NULL,
                                           hdmi_netlink_daemon,
                                           (void *)handle);
    return retcode;
#else
    if (-1 == alislevent_open(&pdev->alislevent_hld)) {
        SL_ERR("hdmi_daemon_monitor_create error: alislevent_open fail\n");
        return 1;
    }
    //SL_DBG("add hdmi kumsgfd: %d\n", pdev->kumsgfd);
    pdev->slev.fd = pdev->kumsgfd;
    pdev->slev.events = EPOLLIN;;
    pdev->slev.cb = hdmi_netlink_daemon;
    pdev->slev.data = (void *)pdev;
    alislevent_add(pdev->alislevent_hld, &pdev->slev);
    return 0;
#endif
}

alisl_retcode alislhdmi_callback_register(alisl_handle handle,
                                          e_hdmi_callback_type type,
                                          alisl_handle cb_func)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;

    switch (type) {
        case HDMI_CALLBACK_EDID_READY:
            pdev->edid_ready = (hdmi_edid_callback)cb_func;
            break;
        case HDMI_CALLBACK_HOT_PLUGIN:
            pdev->hotplug_in = (hdmi_hotplug_callback)cb_func;
            break;
        case HDMI_CALLBACK_HOT_PLUGOUT:
            pdev->hotplug_out = (hdmi_hotplug_callback)cb_func;
            break;
        case HDMI_CALLBACK_CEC_MSGRCV:
            pdev->cec_receive = (hdmi_cec_callback)cb_func;
            break;
        case HDMI_CALLBACK_HDCP_FAIL:
            pdev->hdcp_fail = (hdmi_hdcp_fail_callback)cb_func;
            break;
        default:
            SL_ERR("unknown callback type.\n");
            return HDMI_ERROR_INVAL;
    }

    return HDMI_ERROR_NONE;
}

alisl_retcode alislhdmi_callback_unregister(alisl_handle handle,
                                            e_hdmi_callback_type type)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;

    switch (type) {
        case HDMI_CALLBACK_EDID_READY:
            pdev->edid_ready = NULL;
            break;
        case HDMI_CALLBACK_HOT_PLUGIN:
            pdev->hotplug_in = NULL;
            break;
        case HDMI_CALLBACK_HOT_PLUGOUT:
            pdev->hotplug_out = NULL;
            break;
        case HDMI_CALLBACK_CEC_MSGRCV:
            pdev->cec_receive = NULL;
            break;
        case HDMI_CALLBACK_HDCP_FAIL:
            pdev->hdcp_fail = NULL;
            break;
        default:
            SL_ERR("unknown callback type.\n");
            return HDMI_ERROR_INVAL;
    }

    return HDMI_ERROR_NONE;
}

alisl_retcode alislhdmi_open(alisl_handle *handle,
                             uint8_t *hdcp_key)
{
    alisl_retcode retcode = HDMI_ERROR_NONE;
    int hdmi_fd = -1;
//    int port_id = -1;
    st_hdmi_device *pdev;
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(hdmi_alisl_dev)
	    goto already_opened;

    pdev = malloc(sizeof(st_hdmi_device));
    if (NULL == pdev) {
            goto FUN_NOMEM;
    }
    memset(pdev, 0, sizeof(st_hdmi_device));

    hdmi_fd = open(ALI_HDMI_DEVICE, O_RDWR);
    if (hdmi_fd < 0) {
	    SL_DBG("open %s failed\n", ALI_HDMI_DEVICE);
	    retcode = HDMI_ERROR_OPEN;
            goto FUNC_CLOSE_EXIT;
    }

    pdev->handle = hdmi_fd;
    memcpy(alisl_hdcp_key, hdcp_key, 286);
    if (!ioctl(hdmi_fd, HDMI_IOCINI_HDMIHW, alisl_hdcp_key)) {
        pdev->handle = hdmi_fd;
    } else {
        retcode = HDMI_ERROR_IOCTL;
        SL_ERR("HDMI_IOCINI_HDMIHW fail.\n");
        goto FUNC_CLOSE_EXIT;
    }
    int flags = O_CLOEXEC;
    pdev->kumsgfd = ioctl(pdev->handle, HDMI_IOCT_GET_KUMSGQ, &flags);
    if (pdev->kumsgfd == 0){
        retcode = ERROR_IOCTL;
        SL_ERR("HDMI_IOCT_GET_KUMSGQ failed.\n");
        goto FUNC_CLOSE_EXIT;
    }
    //SL_DBG("hdmi kumsgfd: %d\n", pdev->kumsgfd);

    if (HDMI_ERROR_NONE != hdmi_daemon_monitor_create(pdev)) {
        SL_ERR("create daemon fail.\n");
        retcode = HDMI_ERROR_DEAMON;
        goto FUNC_CLOSE_EXIT;
    }
    hdmi_alisl_dev = (alisl_handle)pdev;

already_opened:
    *handle = (alisl_handle)hdmi_alisl_dev;
    hdmi_open_count++;

    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;

FUNC_CLOSE_EXIT:
    if (-1 != hdmi_fd) {
        close(hdmi_fd);
        hdmi_fd = -1;
    }
    free(pdev);
FUN_NOMEM:
    pthread_mutex_unlock(&hdmi_alisl_mutex);
    return retcode;
}

alisl_retcode alislhdmi_close(alisl_handle handle)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    alisl_short_cea_desc	*p_tmp = NULL;
    alisl_short_cea_desc	*p_last = NULL;
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if (!hdmi_open_count) {
	    SL_ERR("already close.\n");
	    retcode = HDMI_ERROR_INVAL;
	    goto FUNC_EXIT;
    }

    hdmi_open_count --;

    free(pdev->hdcp_key);
#if 0
    retcode = pthread_cancel(pdev->tid);
    if (retcode) {
        goto FUNC_EXIT;
    }
    pthread_join(pdev->tid, NULL);
#else
    alislevent_del(pdev->alislevent_hld, &pdev->slev);
    alislevent_close(pdev->alislevent_hld);
#endif
    if(pdev->kumsgfd)
	    close(pdev->kumsgfd);

    close(pdev->handle);
    free((void *)pdev);
    hdmi_alisl_dev = NULL;

    p_last = hdmi_desc.short_hdmi_vic_desc;
    while(NULL != p_last) {
	    p_tmp = p_last->next;
	    free(p_last);
	    p_last = p_tmp;
    }
    p_last = hdmi_desc.short_3d_desc;
    while(NULL != p_last) {
	    p_tmp = p_last->next;
	    free(p_last);
	    p_last = p_tmp;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);
    return retcode;
}

alisl_retcode alislhdmi_get_edid_video_format(alisl_handle handle,
                                              enum alisl_video_picfmt *format)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;

    HDMI_ioctl_video_format_t cmd;

    if(handle == NULL) {
        return HDMI_ERROR_INVAL;
    }
    if(ioctl(pdev->handle, HDMI_IOCT_GET_VIDEO_FORMAT, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_VIDEO_FORMAT failed.\n");
        return HDMI_ERROR_IOCTL;
    } else {
        *format = cmd.format;
        return HDMI_ERROR_NONE;
    }
}

alisl_retcode alislhdmi_get_edid_nativeres(alisl_handle handle,
                                           enum alisl_hdmi_res *res)
{
    int edid_status;
    int ret;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    enum HDMI_API_RES native;//used for native resolution.

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    ret = ioctl(pdev->handle, HDMI_IOCG_EDIDRDY, (void *)&edid_status);
    if (ret != 0 || edid_status != ALISL_HDMI_EDID_READY) {
        SL_ERR("HDMI_IOCG_EDIDRDY fail.\n");
        retcode = HDMI_ERROR_GETRES;
                  goto FUNC_EXIT;
    } else {
        ret = ioctl(pdev->handle, HDMI_IOCG_NATIVERES, (void*)&native);
        if (ret != 0) {
            SL_ERR("HDMI_IOCG_NATIVERES fail.\n");
            ret = HDMI_ERROR_GETRES;
        } else {
            *res = native;
        }

    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_edid_allres(alisl_handle handle,
                                        uint32_t *native_res_index,
                                        enum alisl_hdmi_res *video_res)
{
    int edid_status;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    int ret;
    HDMI_ioctl_edid_res_list_t cmd;
    st_hdmi_device *pdev = (st_hdmi_device *)handle;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    ret = ioctl(pdev->handle, HDMI_IOCG_EDIDRDY, (void *)&edid_status);
    if (ret != 0 || edid_status != ALISL_HDMI_EDID_READY) {
        SL_ERR("HDMI_IOCG_EDIDRDY fail.\n");
        retcode = HDMI_ERROR_GETRES;
        goto FUNC_EXIT;
    } else {

        ret = ioctl(pdev->handle, HDMI_IOCG_GET_ALL_VID_RES, &cmd);
        if (ret != 0) {
            SL_ERR("HDMI_IOCG_GET_ALL_VID_RES fail.\n");
            retcode = HDMI_ERROR_GETRES;
            goto FUNC_EXIT;
        } else {

            *native_res_index = cmd.native_res_index;
            memcpy(video_res, cmd.video_res_list, sizeof(enum HDMI_API_RES) * HDMI_API_RES_SUPPORT_NUM);
        }
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_edid_audio_prefercodetype(alisl_handle handle,
                                                      enum alisl_audio_coding_type *aud_fmt)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    uint16_t aud_fmt_code = 0x0000;
    alisl_retcode retcode = HDMI_ERROR_NONE;

    pthread_mutex_lock(&hdmi_alisl_mutex);
	*aud_fmt = 0;
    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if (ioctl(pdev->handle, HDMI_IOCG_HDMIAUDIO, (void*)&aud_fmt_code)) {
        SL_ERR("HDMI_IOCG_HDMIAUDIO fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
    	SL_DBG("aud_fmt_code %d \n", aud_fmt_code);
        if ((aud_fmt_code & EDID_AUDIO_LPCM))
            *aud_fmt = ALISL_EDID_AUDIO_LPCM;
		else{
			SL_DBG("don't find PCM format!\n");
			retcode = HDMI_ERROR_INVAL;
			goto FUNC_EXIT;
		}

    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_edid_audio_allcodetype(alisl_handle handle,
                                                   unsigned short *aud_fmt)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    uint16_t aud_fmt_code = 0x0000;
    alisl_retcode retcode = HDMI_ERROR_NONE;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCG_HDMIAUDIO, (void*)&aud_fmt_code)) {
        SL_ERR("HDMI_IOCG_HDMIAUDIO fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *aud_fmt = aud_fmt_code;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_vic_num(alisl_handle handle,
                                         unsigned int *vic_num)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    unsigned int num = 0;
    alisl_retcode retcode = HDMI_ERROR_NONE;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCG_GET_VIDEO_CEA_NUM, (void*)&num)) {
        SL_ERR("HDMI_IOCG_GET_VIDEO_CEA_NUM fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *vic_num = num;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_aud_num(alisl_handle handle,
                                         unsigned int *aud_num)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    unsigned int num = 0;
    alisl_retcode retcode = HDMI_ERROR_NONE;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCG_GET_AUDIO_CEA_NUM, (void*)&num)) {
        SL_ERR("HDMI_IOCG_GET_AUDIO_CEA_NUM fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *aud_num = num;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_edid_CEA_item(alisl_handle handle,
                                        uint8_t index,
                                        enum alisl_hdmi_cea_db_type cea_db_type,
                                        void * item)
{
    alisl_retcode retcode = HDMI_ERROR_NONE;
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    HDMI_ioctl_short_audio_descriptor_t audio_cea_item;
    HDMI_ioctl_short_video_descriptor_t video_cea_item;

    pthread_mutex_lock(&hdmi_alisl_mutex);
    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    memset(&audio_cea_item, 0, sizeof(HDMI_ioctl_short_audio_descriptor_t));
    memset(&video_cea_item, 0, sizeof(HDMI_ioctl_short_video_descriptor_t));
    switch(cea_db_type) {
        case ALISL_HDMI_CEA_AUD_TYPE:
            audio_cea_item.audio_format_code = index;
            if (ioctl(pdev->handle, HDMI_IOCG_GET_AUDIO_CEA, &audio_cea_item)){
                SL_ERR("HDMI_IOCG_GET_AUDIO_CEA fail.\n");
                retcode = HDMI_ERROR_INVAL;
                goto FUNC_EXIT;
            } else {
                /*SL_DBG("HDMI_IOCG_GET_AUDIO_CEA (%d,%d,%d,%d)\n",
                                audio_cea_item.audio_format_code,
                                audio_cea_item.max_num_audio_channels,
                                audio_cea_item.audio_sampling_rate,
                                audio_cea_item.max_audio_bit_rate);*/

                memcpy(item, &audio_cea_item, sizeof(HDMI_ioctl_short_audio_descriptor_t));
            }
            break;
        case ALISL_HDMI_CEA_VIDEO_TYPE:
            video_cea_item.native_indicator = index;
            if (ioctl(pdev->handle, HDMI_IOCG_GET_VIDEO_CEA, &video_cea_item)){
                SL_ERR("HDMI_IOCG_GET_VIDEO_CEA fail.\n");
                retcode = HDMI_ERROR_INVAL;
                goto FUNC_EXIT;
            } else {
                SL_DBG("HDMI_IOCG_GET_AUDIO_CEA (%d,%d,%d,%d)\n",
                                audio_cea_item.audio_format_code,
                                audio_cea_item.max_num_audio_channels,
                                audio_cea_item.audio_sampling_rate,
                                audio_cea_item.max_audio_bit_rate);

                memcpy(item, &video_cea_item, sizeof(HDMI_ioctl_short_video_descriptor_t));
            }
            break;
            break;
        case ALISL_HDMI_CEA_VSD_TYPE:
        case ALISL_HDMI_CEA_SPK_TYPE:
            break;
    }


FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_hdcp_support(alisl_handle handle,
                                                   unsigned char *hdcp_support)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_hdcp_cap_t cap;
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle,  HDMI_IOCT_GET_HDCP_CAP, &cap)) {
        SL_ERR("HDMI_IOCT_GET_HDCP_CAP fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *hdcp_support = cap.hdcp_cap;
        SL_DBG("HDMI_IOCT_GET_HDCP_CAP ok: %uc\n", *hdcp_support);
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}



alisl_retcode alislhdmi_set_switch_status(alisl_handle handle,
                                          unsigned int on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_sw_onoff_state_t cmd;
    cmd.hdmi_status = on_off;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_SET_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_switch_status(alisl_handle handle,
                                          unsigned int *status)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_sw_onoff_state_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *status = cmd.hdmi_status;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_hdcp_status(alisl_handle handle,
                                        unsigned int on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_hdcp_state_t cmd;
    cmd.hdcp_status = on_off;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_HDCP_SET_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_HDCP_SET_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_hdcp_status(alisl_handle handle,
                                        unsigned int *status)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL || status == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    /* get the authentication status of HDCP, don't get the flag set by
     * HDMI_IOCT_HDCP_SET_ONOFF
     */
    *status = ioctl(pdev->handle, HDMI_IOCT_HDCP_GET_STATUS, NULL);

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_cec_status(alisl_handle handle,
                                       unsigned int on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    HDMI_ioctl_cec_state_t cmd;
    cmd.cec_status = on_off;
    alisl_retcode retcode = HDMI_ERROR_NONE;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_CEC_SET_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_CEC_SET_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_cec_status(alisl_handle handle,
                                       unsigned int *status)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    HDMI_ioctl_cec_state_t cmd;
    alisl_retcode retcode = HDMI_ERROR_NONE;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_CEC_GET_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_CEC_GET_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *status = cmd.cec_status;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_audio_status(alisl_handle handle,
                                         unsigned int on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_hdmi_audio_state_t cmd;
    cmd.hdmi_audio_status = on_off;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_SET_HDMI_AUDIO_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_HDMI_AUDIO_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_audio_status(alisl_handle handle,
                                         unsigned int *status)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_hdmi_audio_state_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_HDMI_AUDIO_ONOFF, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_HDMI_AUDIO_ONOFF fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *status = cmd.hdmi_audio_status;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_mem_sel(alisl_handle handle,
                                unsigned int mem_sel)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_mem_sel_t cmd;
    cmd.mem_sel = mem_sel;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_HDCP_MEM_SEL, &cmd)) {
        SL_ERR("HDMI_IOCT_HDCP_MEM_SEL fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_hdcpkey(alisl_handle handle,
                                    unsigned char *key,
                                    unsigned int length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    pthread_mutex_lock(&hdmi_alisl_mutex);
    if(handle == NULL) {
		SL_ERR("handle is null\n");
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if(key == NULL) {
        SL_ERR("key is NULL.\n");
        retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
    }
    if( ALISL_HDCP14_MEM_KEY_LENGTH == length) {
        memcpy(alisl_hdcp_key, key, length);
	    HDMI_ioctl_hdcp_key_info_t cmd;
        cmd.scramble = *key;
        memcpy(cmd.hdcp_ksv, (key+1), 5);
        memcpy(cmd.encrypted_hdcp_keys, (key+6), 280);
	    if (ioctl(pdev->handle, HDMI_IOCT_HDCP_SET_KEY_INFO, &cmd)) {
            SL_ERR("HDMI_IOCT_HDCP_SET_KEY_INFO error\n");
            retcode = -1;
        }
	} else if ( ALISL_HDCP22_MEM_KEY_LENGTH == length) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
	    HDMI_ioctl_hdcp22_key_info_t cmd22;
        cmd22.header = *key;
        memcpy(cmd22.key_n, (key+1), 384);
        memcpy(cmd22.key_e, (key+385), 1);
		memcpy(cmd22.lc, (key+386), 16);
		if (ioctl(pdev->handle, HDMI_IOCT_HDCP22_SET_KEY_INFO, &cmd22)) {
            SL_ERR("HDMI_IOCT_HDCP22_SET_KEY_INFO error\n");
            retcode = -1;
        }
#else
	    SL_WARN("no support hdcp22\n");
#endif
	} else if(ALISL_HDCP22_CE_KEY_LENGTH == length) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
		HDMI_ioctl_hdcp22_ce_key_info_t  info;
		memset(&info, 0x0, sizeof(HDMI_ioctl_hdcp22_ce_key_info_t));
		memcpy(&info.ce_key[0], key, sizeof(info.ce_key));
		retcode = ioctl(pdev->handle, HDMI_IOCT_HDCP22_SET_CE_KEY_INFO, &info);
		if (retcode) {
			SL_ERR("HDMI_IOCT_HDCP22_SET_CE_KEY_INFO error\n");
			retcode = -1;
		}
#else
		SL_WARN("no support hdcp22\n");
#endif
	} else {
        SL_ERR("length mismatch\n");
		retcode = -1;
	}
    //pthread_mutex_lock(&hdmi_alisl_mutex);

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);
    return retcode;
}

alisl_retcode alislhdmi_transmit_cec(alisl_handle handle,
                                     uint8_t *message,
                                     uint8_t message_length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_cec_msg_t cmd;
    memcpy(cmd.message, message, message_length);
    cmd.message_length = message_length;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_CEC_TRANSMIT, &cmd)) {
        SL_ERR("HDMI_IOCT_CEC_TRANSMIT fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        if(!cmd.ret) {
			SL_ERR("cmd.ret fail.\n");
			retcode = HDMI_ERROR_GETMSG;
        }
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_cec_get_pa(alisl_handle handle,
                                   unsigned short *physical_address)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_cec_addr_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_CEC_GET_PA, &cmd)) {
        SL_ERR("HDMI_IOCT_CEC_GET_PA fail.\n");
        retcode = HDMI_ERROR_IOCTL;
        *physical_address = 0xFFFF;
    } else {
        if(cmd.ret) {
            *physical_address = cmd.cec_addr;
        }
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_cec_set_la(alisl_handle handle,
                                   uint8_t logical_address)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_cec_addr_t cmd;
    cmd.cec_addr = logical_address;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_CEC_SET_LA, &cmd)) {
        SL_ERR("HDMI_IOCT_CEC_SET_LA fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        if(!cmd.ret) {
			SL_ERR("HDMI_IOCT_CEC_GET_PA return value is invalid.\n");
			retcode = HDMI_ERROR_INVAL;
        }
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_cec_get_la(alisl_handle handle,
                                   uint8_t *logical_address)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_cec_addr_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_CEC_GET_LA, &cmd)) {
        SL_ERR("HDMI_IOCT_CEC_GET_LA fail.\n");
        retcode = HDMI_ERROR_IOCTL;
        *logical_address = 0xFF;
    } else {
        if(cmd.ret) {
            *logical_address = cmd.cec_addr;
        }
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_cec_set_res(alisl_handle handle,
                                    enum alisl_hdmi_res *res)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_video_res_t cmd;
    cmd.video_res = *res;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_SET_VID_RES, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_VID_RES fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_cec_get_res(alisl_handle handle,
                                    enum alisl_hdmi_res *res)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_video_res_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_VID_RES, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_VID_RES fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *res = cmd.video_res;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_vendor_name(alisl_handle handle,
                                        unsigned char *vendor_name,
                                        unsigned char length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_vendor_name_t cmd;

    if (length > 8)
        length = 8;
    memcpy(cmd.vendor_name, vendor_name, length);
    cmd.length = length;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_SET_VENDOR_NAME, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_VENDOR_NAME fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);
    return retcode;
}

alisl_retcode alislhdmi_set_product_desc(alisl_handle handle,
                                         unsigned char *product_desc,
                                         unsigned char length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_product_desc_t cmd;

    if (length > 16)
        length = 8;
    memcpy(cmd.product_desc, product_desc, length);
    cmd.length = length;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_SET_PRODUCT_DESC, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_PRODUCT_DESC fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);
    return retcode;
}

alisl_retcode alislhdmi_get_vendor_name(alisl_handle handle,
                                        unsigned char *vendor_name,
                                        unsigned char *length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_vendor_name_t cmd;
	memset(&cmd, 0, sizeof(HDMI_ioctl_vendor_name_t));

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_VENDOR_NAME, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_VENDOR_NAME fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
		memcpy(vendor_name, cmd.vendor_name, cmd.length);
        *length = cmd.length;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_monitor_name(alisl_handle handle,
                                        unsigned char *monitor_name,
                                        unsigned char *length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_edid_vendor_t cmd;
	memset(&cmd, 0, sizeof(HDMI_ioctl_edid_vendor_t));

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_MONITOR, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_EDID_MONITOR fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
    	*length = sizeof(cmd.monitor);
		memcpy(monitor_name, cmd.monitor, sizeof(cmd.monitor));
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_manufacturer_name(alisl_handle handle,
                                        unsigned char *manufacturer_name,
                                        unsigned char *length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_edid_vendor_t cmd;
	memset(&cmd, 0, sizeof(HDMI_ioctl_edid_vendor_t));

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_MANUFACTURER, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_EDID_MANUFACTURER fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
    	*length = sizeof(cmd.manufacturer);
		memcpy(manufacturer_name, cmd.manufacturer, sizeof(cmd.manufacturer));
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_product_desc(alisl_handle handle,
                                         unsigned char *product_desc,
                                         unsigned char *length)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_product_desc_t cmd;
	memset(&cmd, 0, sizeof(HDMI_ioctl_product_desc_t));

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if (ioctl(pdev->handle, HDMI_IOCT_GET_PRODUCT_DESC, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_PRODUCT_DESC fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        memcpy(product_desc, cmd.product_desc, cmd.length);
		*length = cmd.length;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);
    return retcode;
}

alisl_retcode alislhdmi_get_link_status(alisl_handle handle,
                                        unsigned char *link_status)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_link_status_t cmd;
	memset(&cmd, 0, sizeof(HDMI_ioctl_link_status_t));

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_LINK_ST, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_LINK_ST fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *link_status = cmd.link_status;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_3d_present(alisl_handle handle,
                                       int *present)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_3d_status_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_3D_PRESENT, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_3D_PRESENT fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *present = cmd.present;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_color_space(alisl_handle handle,
                                      enum alisl_video_picfmt color_space)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
	HDMI_ioctl_color_space_t cmd;
    alisl_retcode retcode = HDMI_ERROR_NONE;
	switch(color_space)
	{
	case ALISL_Video_PicFmt_RGB_MODE1: cmd.color_space = HDMI_RGB; break;
	case ALISL_Video_PicFmt_RGB_MODE2: cmd.color_space = HDMI_RGB; break;
	case ALISL_Video_PicFmt_YCBCR_422: cmd.color_space = HDMI_YCBCR_422; break;
	case ALISL_Video_PicFmt_YCBCR_444: cmd.color_space = HDMI_YCBCR_444; break;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
	case ALISL_Video_PicFmt_YCBCR_420: cmd.color_space = HDMI_YCBCR_420; break;
#endif
	default:SL_ERR("unknown color space.\n"); break;
	}
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if (ioctl(pdev->handle, HDMI_IOCT_SET_COLOR_SPACE, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_COLOR_SPACE fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;

}

alisl_retcode alislhdmi_get_color_space(alisl_handle handle,
                                      enum alisl_video_picfmt *color_space)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_color_space_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_COLOR_SPACE, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_COLOR_SPACE fail.\n");
        retcode = HDMI_ERROR_IOCTL;
        *color_space = 0xFF;
    } else {
    	switch(cmd.color_space)
    	{
		case HDMI_RGB: *color_space = ALISL_Video_PicFmt_RGB_MODE1; break; // use ALISL_Video_PicFmt_RGB_MODE1 default
		case HDMI_YCBCR_422: *color_space = ALISL_Video_PicFmt_YCBCR_422; break;
		case HDMI_YCBCR_444: *color_space = ALISL_Video_PicFmt_YCBCR_444;break;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
		case HDMI_YCBCR_420: *color_space = ALISL_Video_PicFmt_YCBCR_420; break;
#endif
		default: SL_ERR("unknown color space.\n"); break;
		}
    }
FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}


alisl_retcode alislhdmi_set_deep_color(alisl_handle handle,
                                      enum alisl_hdmi_deepcolor deep_mode)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
	HDMI_ioctl_deep_color_t cmd;
    alisl_retcode retcode = HDMI_ERROR_NONE;

	switch(deep_mode)
	{
	case ALISL_HDMI_DEEPCOLOR_24: cmd.dp_mode = HDMI_DEEPCOLOR_24; break;
	case ALISL_HDMI_DEEPCOLOR_30: cmd.dp_mode = HDMI_DEEPCOLOR_30; break;
	case ALISL_HDMI_DEEPCOLOR_36: cmd.dp_mode = HDMI_DEEPCOLOR_36; break;
	case ALISL_HDMI_DEEPCOLOR_48: cmd.dp_mode = HDMI_DEEPCOLOR_48; break;
	default:SL_ERR("unknown deep mode.\n"); break;
	}

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if (ioctl(pdev->handle, HDMI_IOCT_SET_DEEP_COLOR, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_DEEP_COLOR fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_deep_color(alisl_handle handle,
                                      enum alisl_hdmi_deepcolor *deep_mode)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_deep_color_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_DEEP_COLOR, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_DEEP_COLOR fail.\n");
        retcode = HDMI_ERROR_IOCTL;
        *deep_mode = 0xFF;
    } else {
    	switch(cmd.dp_mode)
    	{
		case HDMI_DEEPCOLOR_24: *deep_mode = ALISL_HDMI_DEEPCOLOR_24; break;// use ALISL_Video_PicFmt_RGB_MODE1 default
		case HDMI_DEEPCOLOR_30: *deep_mode = ALISL_HDMI_DEEPCOLOR_30; break;
		case HDMI_DEEPCOLOR_36: *deep_mode = ALISL_HDMI_DEEPCOLOR_36; break;
		case HDMI_DEEPCOLOR_48: *deep_mode = ALISL_HDMI_DEEPCOLOR_48; break;
		default: SL_ERR("unknown deep mode.\n"); break;
		}
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}


alisl_retcode alislhdmi_get_edid_deep_color(alisl_handle handle,
                                      int *dc_fmt)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_deep_color_t cmd;
	int edid_rdy;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCG_EDIDRDY, (void*)&edid_rdy)) {
        SL_ERR("HDMI_IOCG_EDIDRDY fail.\n");
        retcode = HDMI_ERROR_IOCTL;
        *dc_fmt = 0xFF;
    } else {
		if (0 == edid_rdy){
			SL_ERR("edid_rdy is 0.\n");
			retcode = HDMI_ERROR_INVAL;
			*dc_fmt = 0xFF;
		}else{
			if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_DEEP_COLOR, &cmd)) {
				SL_ERR("HDMI_IOCT_GET_EDID_DEEP_COLOR fail.\n");
				retcode = HDMI_ERROR_IOCTL;
				*dc_fmt = 0xFF;
			} else {
				*dc_fmt = cmd.dp_mode;
			}
    	}
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_edid_total_length(alisl_handle handle,unsigned int *datalen)
{
	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL || datalen == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}

	cmd.length = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_LENGTH, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_LENGTH fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}

	*datalen = cmd.length;

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_block(alisl_handle handle,unsigned char *p_ediddata,
											unsigned int *datalen, unsigned int block_idx)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_edid_block_data_t cmd;


    pthread_mutex_lock(&hdmi_alisl_mutex);

	if(block_idx >=8 ){
		SL_ERR("out of idx.\n");
		retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
	}

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

	cmd.block_num = block_idx;

    if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
        retcode = HDMI_ERROR_IOCTL;
        goto FUNC_EXIT;
    }

	memcpy(p_ediddata, cmd.block, HDMI_EDID_BLOCK_LENGTH);
	*datalen = HDMI_EDID_BLOCK_LENGTH; // driver use 128 Byte EDID_BLOCK_SIZE

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_set_av_blank(alisl_handle handle,
                                      unsigned int on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
	HDMI_ioctl_av_state_t cmd;
    alisl_retcode retcode = HDMI_ERROR_NONE;

	cmd.av_blank_status = on_off;
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if (ioctl(pdev->handle, HDMI_IOCT_SET_AV_BLANK, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_AV_BLANK fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_av_blank(alisl_handle handle,
                                         unsigned int *on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_av_state_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_AV_BLANK, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_AV_BLANK fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *on_off = cmd.av_blank_status;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}


alisl_retcode alislhdmi_set_av_mute(alisl_handle handle,
                                      unsigned int on_off)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
	HDMI_ioctl_av_mute_t cmd;
    alisl_retcode retcode = HDMI_ERROR_NONE;

	cmd.av_mute = on_off;
    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }
    if (ioctl(pdev->handle, HDMI_IOCT_SET_AV_MUTE, &cmd)) {
        SL_ERR("HDMI_IOCT_SET_AV_MUTE fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

/* hdmi_type=1 means HDMI device, hdmi_type=0 means DVI device*/
alisl_retcode alislhdmi_get_hdmi_type(alisl_handle handle,
                                         unsigned int *hdmi_type)
{
    st_hdmi_device *pdev = (st_hdmi_device *)handle;
    alisl_retcode retcode = HDMI_ERROR_NONE;
    HDMI_ioctl_edid_hdmi_mode_t cmd;

    pthread_mutex_lock(&hdmi_alisl_mutex);

    if(handle == NULL) {
        retcode = HDMI_ERROR_INVAL;
        goto FUNC_EXIT;
    }

    if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_HDMI_MODE, &cmd)) {
        SL_ERR("HDMI_IOCT_GET_EDID_HDMI_MODE fail.\n");
        retcode = HDMI_ERROR_IOCTL;
    } else {
        *hdmi_type = cmd.hdmi_mode;
    }

FUNC_EXIT:
    pthread_mutex_unlock(&hdmi_alisl_mutex);

    return retcode;
}

alisl_retcode alislhdmi_get_prefer_video_formt(alisl_handle handle,
                                         unsigned int *video_formt, unsigned char idx)
{
	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
		byte 0x26 ~ 0X35 :
		Standard timing information. Up to 8 2-byte fields describing standard display modes.
		Unused fields are filled with 01 01
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	// video_formt should point to the struct #aui_hdmi_edid_video_format
	//Take the  fields
	video_formt[0] = (unsigned int)((cmd.block[0x26 + idx * 2]+31)*8);  //video_format
	video_formt[1] = (unsigned int)((cmd.block[0x27 + idx * 2]&0x3F)+60);  //field_rate
	video_formt[2] = (unsigned int)((cmd.block[0x27 + idx * 2]&0xC0)>>6);  //aspect_ratio

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_3d_descriptor(alisl_handle handle,
										 alisl_hdmi_3d_descriptor *desc_data)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;
	alisl_short_cea_desc	*tmp_last = NULL;
	alisl_short_cea_desc	*tmp_head_vic = NULL;   //short_hdmi_vic_descriptor  Link Header
	alisl_short_cea_desc	*tmp_head_3d = NULL;	 //short_3d_descriptor  Link Header
	alisl_short_cea_desc	*p_tmp = NULL;
	alisl_short_cea_desc	*short_descriptor = NULL;

	unsigned int i = 0,j = 0;
	unsigned char *pdata = NULL;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	// desc_data should point to the struct #aui_hdmi_3d_descriptor
	for( i = 1;i < EDID_BLOCK_NUM_MAX;i++) {
		cmd.block_num = i;
		if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
			SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
			retcode = HDMI_ERROR_IOCTL;
			goto FUNC_EXIT;
		}
		/*  CEA-861B
		    	byte 0x04
			bit 7..5: Block Type Tag (1 is audio, 2 is video, 3 is vendor specific, 4 is speaker
		 			 allocation, all other values Reserved)
			bit 4..0: Total number of bytes in this block following this byte

			byte 0x17 ~ byte (0x17+hdmi_vic_len) :  hdmi video descriptor
			byte (0x17+hdmi_vic_len) ~ byte (0x17+hdmi_vic_len+hdmi_3d_len) :  hdmi 3d descriptor
		*/
		if( 0x03 == (cmd.block[0x04] >> 5)) { //data block collection,Vendor Specific Data Block
			hdmi_desc.hdmi_3d_multi_present = (cmd.block[4+13]&0x60)>>5; 	//hdmi_3d_multi_present
			hdmi_desc.hdmi_vic_len = (cmd.block[4+14]&0xe0)>>5; // hdmi_vic_len
   			hdmi_desc.hdmi_3d_len = (cmd.block[4+14]&0x1f);   // hdmi_3d_len

			// get short_hdmi_vic_descriptor, link form of storage
   			pdata = &(cmd.block[4+15]);    //The first vic descriptor data address
			if(NULL == hdmi_desc.short_hdmi_vic_desc) {  //It is judged whether or not there is data
				tmp_head_vic = (alisl_short_cea_desc *)malloc(sizeof(alisl_short_cea_desc));
				if (NULL == tmp_head_vic) {
					retcode = HDMI_ERROR_INVAL;
					goto FUNC_EXIT;
				}
				hdmi_desc.short_hdmi_vic_desc = tmp_head_vic;
			} else { //No need to allocate memory
				tmp_head_vic = hdmi_desc.short_hdmi_vic_desc;
			}
			tmp_last = tmp_head_vic;
			tmp_last->cea_data = (*(pdata));
			tmp_last->next = NULL;
			for(j = 1; j < hdmi_desc.hdmi_vic_len; j++) {  //Creates a linked list of stored data
				if(NULL == tmp_last->next){//you need to determine whether it has been allocated
					short_descriptor = (alisl_short_cea_desc *)malloc(sizeof(alisl_short_cea_desc));
					if (NULL == short_descriptor) {
						while(NULL != tmp_head_vic) { //Memory allocation failed to release
							p_tmp = tmp_head_vic->next;
							free(tmp_head_vic);
							tmp_head_vic = p_tmp;
						}
						retcode = HDMI_ERROR_INVAL;
						goto FUNC_EXIT;
					}
					short_descriptor->next = NULL; //The new pointer is NULL
				} else {
					short_descriptor = tmp_last->next;
				}
				short_descriptor->cea_data = (*(pdata+j));
				tmp_last->next = short_descriptor;
				tmp_last = short_descriptor;
				short_descriptor = NULL;
			}
			tmp_last = NULL;

			// get short_hdmi_3d_descriptor, similar to short_hdmi_vic_descriptor
			pdata = &(cmd.block[4+15+hdmi_desc.hdmi_vic_len]);  //The first 3d descriptor data address
			if(NULL == hdmi_desc.short_3d_desc) {
				tmp_head_3d = (alisl_short_cea_desc *)malloc(sizeof(alisl_short_cea_desc));
				if (NULL == tmp_head_3d) {
					retcode = HDMI_ERROR_INVAL;
					goto FUNC_EXIT;
				}
				hdmi_desc.short_3d_desc = tmp_head_3d;
			} else {
				tmp_head_3d = hdmi_desc.short_3d_desc;
			}
			tmp_last = tmp_head_3d;
			tmp_last->cea_data = (*(pdata));
			tmp_last->next = NULL;
			for(j = 1; j < hdmi_desc.hdmi_3d_len; j++) {
				if(NULL == tmp_last->next){
					short_descriptor=(alisl_short_cea_desc *)malloc(sizeof(alisl_short_cea_desc));
					if (NULL == short_descriptor) {
						while(NULL != tmp_head_vic) {
							p_tmp = tmp_head_vic->next;
							free(tmp_head_vic);
							tmp_head_vic = p_tmp;
						}
						while(NULL != tmp_head_3d) {
							p_tmp = tmp_head_3d->next;
							free(tmp_head_3d);
							tmp_head_3d = p_tmp;
						}
						retcode = HDMI_ERROR_INVAL;
						goto FUNC_EXIT;
					}
					short_descriptor->next = NULL;
				} else {
					short_descriptor = tmp_last->next;
				}
				short_descriptor->cea_data = (*(pdata+j));
				tmp_last->next = short_descriptor;
				tmp_last = short_descriptor;
				short_descriptor = NULL;
			}
			tmp_last = NULL;
			break;          // only one Vendor Specific Data Block
		}
	}
	//Transfer structure aui_hdmi_3d_descriptor
	memcpy(desc_data,&hdmi_desc,sizeof(alisl_hdmi_3d_descriptor));

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_product_id(alisl_handle handle,
										 unsigned short *product_id)
{
 	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
		byte 0x0a ~ 0X0b : Manufacturer product code. 16-bit number, little-endian.
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	*product_id = (unsigned short)(cmd.block[0x0B]<<8|cmd.block[0x0A]);

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_serial_number(alisl_handle handle,
										 unsigned long *serial_number)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
		byte 0x1C ~ 0x1F : Serial number. 32 bits, little endian.
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	*serial_number = (unsigned long)(cmd.block[0x0F]<<24|cmd.block[0x0E]<<16|cmd.block[0x0D]<<8|cmd.block[0x0C]);

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_week_manufacturer(alisl_handle handle,
										 unsigned char *w_manufacture)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
		byte 0x10 : Week of manufacture. Week numbering is not consistent between manufacturers.
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	*w_manufacture = (cmd.block[0x10]);

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_year_manufacturer(alisl_handle handle,
										 unsigned short *y_manufacture)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
	  	byte 0x11 : Year of manufacture, less 1990. (1990¡§C2245). If week=255, it is the model year instead.
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	*y_manufacture = (unsigned short)(cmd.block[0x11]+1990);

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_version(alisl_handle handle,
										 unsigned short *version)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
		byte 0x12 : EDID version, usually 1 (for 1.3)
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	*version = (unsigned short)(cmd.block[0x12]);

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}

alisl_retcode alislhdmi_get_edid_revision(alisl_handle handle,
										 unsigned short *revision)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*	EDID-1.3
		byte 0x13: EDID revision, usually 3 (for 1.3)
	*/
	cmd.block_num = 0;

	if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
		SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
		retcode = HDMI_ERROR_IOCTL;
		goto FUNC_EXIT;
	}
	*revision = (unsigned short)(cmd.block[0x13]);

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}


alisl_retcode alislhdmi_get_raw_edid(alisl_handle handle,
										 unsigned int length,unsigned char *p_ediddata)
{
  	st_hdmi_device *pdev = (st_hdmi_device *)handle;
	alisl_retcode retcode = HDMI_ERROR_NONE;
	HDMI_ioctl_edid_block_data_t cmd;
	unsigned long nblock = 0;
	unsigned int i = 0;

	pthread_mutex_lock(&hdmi_alisl_mutex);

	if(handle == NULL) {
		retcode = HDMI_ERROR_INVAL;
		goto FUNC_EXIT;
	}
	/*
		get all EDID Block
	*/

	nblock = length / HDMI_EDID_BLOCK_LENGTH;//get  EDID Block number

	for (i = 0; i<nblock; i++) {
		cmd.block_num = i;
		if (ioctl(pdev->handle, HDMI_IOCT_GET_EDID_BLOCK, &cmd)) {
			SL_ERR("HDMI_IOCT_GET_EDID_BLOCK fail.\n");
			retcode = HDMI_ERROR_IOCTL;
			goto FUNC_EXIT;
		}
		memcpy(&(p_ediddata[0+HDMI_EDID_BLOCK_LENGTH*i]), &(cmd.block[0]), HDMI_EDID_BLOCK_LENGTH);
	}

FUNC_EXIT:
	pthread_mutex_unlock(&hdmi_alisl_mutex);

	return retcode;
}
