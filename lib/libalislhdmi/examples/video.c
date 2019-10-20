#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <signal.h>
#include "video.h"

struct ali_decoder_private *ali_video_priv;

int ali_video_vpo_onoff(int32_t layer, int32_t onoff)
{
    struct ali_vpo_win_status_pars win_status_pars;
    int ret, fb1_fd;

    fb1_fd = open("/dev/fb1", O_RDWR);
    if(fb1_fd <= 0) {
        log_info("open /dev/fb1 fail\n");
        return -1;
    }

    memset((void *)&win_status_pars, 0, sizeof(win_status_pars));

    win_status_pars.hd_dev = 1;
    win_status_pars.on = onoff;
    win_status_pars.layer = (layer == VPO_PIPWIN) ? VPO_LAYER_AUXP : VPO_LAYER_MAIN;
    ret = ioctl(fb1_fd, VPO_SET_WIN_ONOFF_EX, &win_status_pars);
    if(ret < 0) {
        log_info("close vpo layer %d fail", layer);
    }

    close(fb1_fd);

    return ret;
}

#if 0
/** Get current value of the STC timer.
 *  \param[out] val_out 90kHz, 33 bit
 *  \returns 0 on success, negative on error
 */

int ali_stc_open(ali_stc_id *stc_out)
{
    struct ali_stc_private *stcp = NULL;

    if(*stc_out) {
        log_info("stc has been inited\n");
        return 0;
    }

    stcp = malloc(sizeof(struct ali_stc_private));
    if(stcp == NULL) {
        log_info("malloc stc private fail\n");
        goto fail;
    }

    stcp->stc0_fd = open(STCDEV0, O_RDWR);
    stcp->stc1_fd = open(STCDEV1, O_RDWR);

    *stc_out = (ali_stc_id)stcp;

    return 0;

fail:
    if(stcp->stc0_fd > 0)
        close(stcp->stc0_fd);
    if(stcp->stc1_fd > 0)
        close(stcp->stc1_fd);
    if(stcp)
        free(stcp);

    *stc_out = 0;

    return -1;
}

void ali_stc_close(ali_stc_id stc)
{
    struct ali_stc_private *stcp = (struct ali_stc_private *)stc;

    if(stcp == NULL) {
        log_info("stc has been released\n");
        return;
    }

    if(stcp->stc0_fd > 0)
        close(stcp->stc0_fd);
    if(stcp->stc1_fd > 0)
        close(stcp->stc1_fd);

    free(stcp);
}

int ali_stc_get(ali_stc_id stc, uint8_t id, uint64_t *val_out)
{
    struct ali_stc_private *stcp = (struct ali_stc_private *)stc;
    unsigned int stc_value, read_size;
    int stc_fd, ret = 0;


    if(id == 0)
        stc_fd = stcp->stc0_fd;
    else
        stc_fd = stcp->stc1_fd;

    if(stc_fd <= 0) {
        log_info("open stc%d fail\n", id);
        return -1;
    }

    read_size = read(stc_fd, &stc_value, sizeof(unsigned int));
    if(read_size == sizeof(unsigned int)) {
        *val_out = stc_value;
    } else {
        ret = -1;
        log_info("read stc%d fail\n", id);
    }

    return ret;
}

/** Set current value of the STC timer.
 *  \param val 90kHz, 33 bit
 *  \returns 0 on success, negative on error
 */
int ali_stc_set(ali_stc_id stc, uint8_t id, uint64_t val)
{
    struct ali_stc_private *stcp = (struct ali_stc_private *)stc;
    unsigned int stc_value = val, write_size;
    int stc_fd, ret = 0;

    if(id == 0)
        stc_fd = stcp->stc0_fd;
    else
        stc_fd = stcp->stc1_fd;

    if(stc_fd <= 0) {
        log_info("open stc%d fail\n", id);
        return -1;
    }

    write_size = write(stc_fd, &stc_value, sizeof(unsigned int));
    if(write_size != sizeof(unsigned int)) {
        ret = -1;
        log_info("write stc%d fail\n", id);
    } else {
        ioctl(stc_fd, STCIO_SET_VALID, 1);
    }

    return ret;
}

/** Pause/unpause the timer.
 *  When paused, \a ali_stc_get will alywas return the same value.
 *  Pausing STC should also pause the video display / audio playback.
 *  \param pause  pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_stc_pause(ali_stc_id stc, uint8_t id, int32_t pause)
{
    struct ali_stc_private *stcp = (struct ali_stc_private *)stc;
    int stc_fd, ret = 0;

    if(id == 0)
        stc_fd = stcp->stc0_fd;
    else
        stc_fd = stcp->stc1_fd;

    if(stc_fd <= 0) {
        log_info("open stc%d fail\n", id);
        return -1;
    }

    ret = ioctl(stc_fd, STCIO_PAUSE_STC, pause);
    if(ret < 0) {
        log_info("pause stc%d fail\n", id);
        ret = -1;
    }

    return ret;
}

/** Change speed of the STC timer.
 *  This is for drift correction.
 *  All consecutive corrections are accumulative.
 *  \param ppm correction to be made, in 1/1000000 units.
 *  \returns 0 on success, negative on error
 */
int ali_stc_change_speed(ali_stc_id stc, uint8_t id, int ppm)
{
    struct ali_stc_private *stcp = (struct ali_stc_private *)stc;
    unsigned int devisor = ppm;
    int stc_fd, ret = 0;

    if(id == 0)
        stc_fd = stcp->stc0_fd;
    else
        stc_fd = stcp->stc1_fd;

    if(stc_fd <= 0) {
        log_info("open stc%d fail\n", id);
        return -1;
    }

    ret = ioctl(stc_fd, STCIO_SET_DIVISOR, devisor);
    if(ret < 0) {
        log_info("change stc%d speed to %d fail\n", id, devisor);
        ret = -1;
    }

    return ret;
}
#endif

static int ali_video_select_decoder(ali_video_decoder_id decoder, int preview)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_codec_param codec_param;
    enum video_decoder_type decoder_type = MPEG2_DECODER;
    int ret = -1;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->codec_id == h264)
        decoder_type = H264_DECODER;
    else if(decp->codec_id == mpg2)
        decoder_type = MPEG2_DECODER;
    else if(decp->codec_id == hevc)
        decoder_type = H265_DECODER;
    else
        decoder_type = INVALID_DECODER;

    if(decp->video_fd <= 0) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    codec_param.type = decoder_type;
    codec_param.preview = preview;
    ret = ioctl(decp->video_fd, VDECIO_SELECT_DECODER, &codec_param);
    if(ret < 0) {
        log_info("select video decoder fail\n");
        ret = -1;
    }

    return ret;
}

static int ali_video_start(ali_video_decoder_id decoder)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    int ret = -1;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->codec_id != h264 && decp->codec_id != mpg2
            && decp->codec_id != hevc)
        return 0;

    if(decp->video_fd <= 0) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    ret = ioctl(decp->video_fd, VDECIO_START, 0);
    if(ret < 0) {
        log_info("start video decoder fail\n");
        ret = -1;
    }

    return ret;
}

static int ali_video_stop(ali_video_decoder_id decoder, int bclosevp, int bfillblack)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_stop_param stop_param;
    int ret= -1;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->codec_id != h264 && decp->codec_id != mpg2
            && decp->codec_id != hevc)
        return 0;

    if(decp->video_fd <= 0) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    stop_param.close_display = bclosevp;
    stop_param.fill_black = bfillblack;
    ret = ioctl(decp->video_fd, VDECIO_STOP, &stop_param);
    if(ret < 0) {
        log_info("stop video decoder fail\n");
        ret = -1;
    }

    return ret;
}

int ali_video_get_memory(int32_t video_id, ali_video_decoder_mem *decoder_mem)
{
    struct ali_video_mem_info video_mem_info;
    char dev_name[24];
    int video_fd;

    sprintf(dev_name, "/dev/ali_video%d", video_id);
    video_fd = open(dev_name, O_RDWR);
    if(video_fd <= 0) {
        log_info("open %s fail\n", dev_name);
        return -1;
    }
    ioctl(video_fd, VDECIO_GET_MEM_INFO, (int)&video_mem_info);

    decoder_mem->main_mem_addr = (uint32_t)video_mem_info.mem_start;
    decoder_mem->main_mem_size = video_mem_info.mem_size;
    decoder_mem->priv_mem_addr = (uint32_t)video_mem_info.priv_mem_start;
    decoder_mem->priv_mem_size = video_mem_info.priv_mem_size;
    decoder_mem->mp_mem_addr = (uint32_t)video_mem_info.mp_mem_start;
    decoder_mem->mp_mem_size = video_mem_info.mp_mem_size;
    log_info("decoder use mem: cpu 0x%x 0x%x, see 0x%x 0x%x, mp 0x%x 0x%x, vbv 0x%x 0x%x\n",
             decoder_mem->main_mem_addr, decoder_mem->main_mem_size,
             decoder_mem->priv_mem_addr, decoder_mem->priv_mem_size,
             decoder_mem->mp_mem_addr, decoder_mem->mp_mem_size,
             video_mem_info.vbv_mem_start, video_mem_info.vbv_mem_size);

    close(video_fd);

    return 0;
}

/** Open video decoder with given codec.
 *  \param decoder_init  codec to be used
 *  \param[out] decoder_out returned decoder instance
 *  \returns 0 on success, negative on error
 */
int ali_video_open(ali_video_decoder_init *decoder_init, ali_video_decoder_id *decoder_out)
{
    struct ali_decoder_private *decp = NULL;
    struct sbm_config sbm_info;
    struct vdec_mp_init_param init_param;
    struct vdec_mp_sbm_param sbm_param;
    struct vdec_mp_extra_data extra_data_param;
    struct vdec_display_rect display_rect;
    ali_video_decoder_buffer *decoder_buffer = &decoder_init->dec_buf;
    // int pkt_sbm_idx, dec_out_sbm_idx, display_sbm_idx;
    int ret = 0;
    char dev_name[24];

    if(*decoder_out) {
        log_info("decoder has been inited\n");
        return ret;
    }

    decp = malloc(sizeof(struct ali_decoder_private));
    if(decp == NULL) {
        log_info("malloc decoder private fail\n");
        goto fail;
    }

    memset(decp, 0, sizeof(struct ali_decoder_private));
    decp->decode_mode = decoder_init->input_type;
    decp->codec_id = decoder_init->codec_id;
    decp->video_id = decoder_init->video_id;

    sprintf(dev_name, "/dev/ali_video%d", decoder_init->video_id);
    decp->video_fd = open(dev_name, O_RDWR);
    if(decp->video_fd <= 0) {
        log_info("open %s fail\n", dev_name);
        goto fail;
    }

    if(decp->decode_mode == ALI_VIDEO_INPUT_USER) {
        memset(&sbm_info, 0, sizeof(sbm_info));
        memset(&init_param, 0, sizeof(init_param));

        sprintf(dev_name, "/dev/ali_sbm%d", decoder_init->pkt_data_sbm);
        decp->pkt_data_fd = open(dev_name, O_WRONLY);
        if(decp->pkt_data_fd <= 0) {
            log_info("open %s fail\n", dev_name);
            goto fail;
        }
        sbm_info.buffer_addr = decoder_buffer->pkt_data_addr;
        sbm_info.buffer_size = decoder_buffer->pkt_data_size;
        sbm_info.block_size = 0x20000;
        sbm_info.reserve_size = 1024;
        sbm_info.wrap_mode = SBM_MODE_PACKET;
        sbm_info.lock_mode = SBM_SPIN_LOCK;
        ret = ioctl(decp->pkt_data_fd, SBMIO_CREATE_SBM, &sbm_info);
        if(ret < 0) {
            log_info("decore create sbm fail\n");
            goto fail;
        }

        sprintf(dev_name, "/dev/ali_sbm%d", decoder_init->pkt_hdr_sbm);
        decp->pkt_hdr_fd = open(dev_name, O_WRONLY);
        if(decp->pkt_hdr_fd <= 0) {
            log_info("open %s fail\n", dev_name);
            goto fail;
        }
        if(decoder_init->pkt_data_sbm != decoder_init->pkt_hdr_sbm) {
            sbm_info.buffer_addr = decoder_buffer->pkt_hdr_addr;
            sbm_info.buffer_size = decoder_buffer->pkt_hdr_size;
            sbm_info.reserve_size = sizeof(struct av_packet);
            sbm_info.wrap_mode = SBM_MODE_NORMAL;
            sbm_info.lock_mode = SBM_SPIN_LOCK;
            ret = ioctl(decp->pkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
            if(ret < 0) {
                log_info("decore create sbm fail\n");
                goto fail;
            }
        }

        if(decoder_buffer->dec_out_addr && decoder_buffer->dec_out_size
                && decoder_init->dec_out_sbm > 0) {
            sprintf(dev_name, "/dev/ali_sbm%d", decoder_init->dec_out_sbm);
            decp->dec_out_fd = open(dev_name, O_RDONLY);
            if(decp->dec_out_fd <= 0) {
                log_info("open %s fail\n", dev_name);
                goto fail;
            }
            sbm_info.buffer_addr = decoder_buffer->dec_out_addr;
            sbm_info.buffer_size = decoder_buffer->dec_out_size;
            sbm_info.reserve_size = sizeof(struct av_frame);
            sbm_info.wrap_mode = SBM_MODE_NORMAL;
            sbm_info.lock_mode = SBM_SPIN_LOCK;
            ret = ioctl(decp->disp_in_fd, SBMIO_CREATE_SBM, &sbm_info);
            if(ret < 0) {
                log_info("decore create sbm fail\n");
                goto fail;
            }
        }

        if(decoder_buffer->disp_in_addr&& decoder_buffer->disp_in_size
                && decoder_init->disp_in_sbm > 0) {
            sprintf(dev_name, "/dev/ali_sbm%d", decoder_init->disp_in_sbm);
            decp->disp_in_fd = open(dev_name, O_RDONLY);
            if(decp->disp_in_fd <= 0) {
                log_info("open %s fail\n", dev_name);
                goto fail;
            }
            sbm_info.buffer_addr = decoder_buffer->disp_in_addr;
            sbm_info.buffer_size = decoder_buffer->disp_in_size;
            sbm_info.reserve_size = sizeof(struct av_frame);
            sbm_info.wrap_mode = SBM_MODE_NORMAL;
            sbm_info.lock_mode = SBM_SPIN_LOCK;
            ret = ioctl(decp->disp_in_fd, SBMIO_CREATE_SBM, &sbm_info);
            if(ret < 0) {
                log_info("decore create sbm fail\n");
                goto fail;
            }
        }

        ali_video_select_decoder((ali_video_decoder_id)decp, 0);
        ret = ali_video_start((ali_video_decoder_id)decp);
        if(ret < 0 ) {
            log_info("decore start video fail\n");
            goto fail;
        }

        init_param.decode_mode    = decoder_init->decode_mode;
        init_param.codec_tag      = decoder_init->codec_id;
        init_param.frame_rate     = decoder_init->frame_rate; /* param parser from container */
        init_param.pic_width      = decoder_init->pic_width;
        init_param.pic_height     = decoder_init->pic_height;
        init_param.pixel_aspect_x = decoder_init->sar_width;
        init_param.pixel_aspect_y = decoder_init->sar_height;
        init_param.dec_buf_addr   = decoder_init->dec_buf.frm_buf_addr;
        init_param.dec_buf_size   = decoder_init->dec_buf.frm_buf_size;
        if(decoder_init->codec_id == h264 && decoder_init->extradata_size > 0 && decoder_init->extradata) {
            init_param.decoder_flag |= VDEC_FLAG_AVC1_FORMAT;
        }
        ret = ali_video_ioctl((ali_video_decoder_id)decp, VDECIO_MP_INITILIZE, (uint32_t)&init_param);
        if(ret < 0) {
            log_info("decore init fail\n");
            goto fail;
        }

        sbm_param.packet_header = decoder_init->pkt_hdr_sbm;
        sbm_param.packet_data   = decoder_init->pkt_data_sbm;
        sbm_param.decode_output = decoder_init->dec_out_sbm;
        sbm_param.display_input = decoder_init->disp_in_sbm;
        ret = ali_video_ioctl((ali_video_decoder_id)decp, VDECIO_MP_SET_SBM_IDX, (uint32_t)&sbm_param);
        if(ret < 0) {
            log_info("decore set video sbm fail\n");
            goto fail;
        }

        if(decoder_init->extradata_size > 0 && decoder_init->extradata) {
            log_info("decode extradata: 0x%x %d\n", (uint32_t)decoder_init->extradata, decoder_init->extradata_size);
            extra_data_param.extra_data = (uint8_t *)decoder_init->extradata;
            extra_data_param.extra_data_size = decoder_init->extradata_size;
            ret = ali_video_ioctl((ali_video_decoder_id)decp, VDECIO_MP_EXTRA_DATA, (uint32_t)&extra_data_param);
            if(ret != 0 ) {
                log_info("decore extradata fail: %d\n",ret);
            }
        }

        if(decoder_init->preview) {
            log_info("set display rect, src<%d %d %d %d>, dst<%d %d %d %d>\n",
                     decoder_init->src_rect.x, decoder_init->src_rect.y, decoder_init->src_rect.w, decoder_init->src_rect.h,
                     decoder_init->dst_rect.x, decoder_init->dst_rect.y, decoder_init->dst_rect.w, decoder_init->dst_rect.h);
            display_rect.src_x = decoder_init->src_rect.x;
            display_rect.src_y = decoder_init->src_rect.y;
            display_rect.src_w = decoder_init->src_rect.w;
            display_rect.src_h = decoder_init->src_rect.h;
            display_rect.dst_x = decoder_init->dst_rect.x;
            display_rect.dst_y = decoder_init->dst_rect.y;
            display_rect.dst_w = decoder_init->dst_rect.w;
            display_rect.dst_h = decoder_init->dst_rect.h;
            ret = ali_video_ioctl((ali_video_decoder_id)decp, VDECIO_MP_SET_DISPLAY_RECT, (uint32_t)&display_rect);
            if(ret < 0) {
                log_info("decore set display rect fail\n");
            }
        }
    } else {
        ali_video_select_decoder((ali_video_decoder_id)decp, decoder_init->preview);
        ret = ali_video_start((ali_video_decoder_id)decp);
        if(ret < 0 ) {
            log_info("decore start video fail\n");
            goto fail;
        }
    }

    *decoder_out = (ali_video_decoder_id)decp;
    ali_video_priv = decp;

    return 0;

fail:
    if(decp->pkt_hdr_fd > 0)
        close(decp->pkt_hdr_fd);
    if(decp->pkt_data_fd > 0)
        close(decp->pkt_data_fd);
    if(decp->dec_out_fd > 0)
        close(decp->dec_out_fd);
    if(decp->disp_in_fd > 0)
        close(decp->disp_in_fd);
    if(decp)
        free(decp);

    *decoder_out = 0;

    return -1;
}

/** Close given decoder instance.
 *  \param decoder  decoder instance created with \e ali_video_decoder_open()
 */
void ali_video_close(ali_video_decoder_id decoder)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_mp_rls_param rls_param;
    int ret;

    if(decp == NULL) {
        log_info("decore has been released\n");
        return;
    }

    memset(&rls_param, 0, sizeof(rls_param));

    ali_video_stop(decoder, 0, 0);

    if(decp->decode_mode) {
        ret = ali_video_ioctl(decoder, VDECIO_MP_RELEASE, (uint32_t)&rls_param);
        if(ret < 0) {
            log_info("decore release fail\n");
        }

        if(decp->pkt_hdr_fd > 0)
            close(decp->pkt_hdr_fd);
        if(decp->pkt_data_fd > 0)
            close(decp->pkt_data_fd);
        if(decp->dec_out_fd > 0)
            close(decp->dec_out_fd);
        if(decp->disp_in_fd > 0)
            close(decp->disp_in_fd);
    }

    decp->codec_id = 0;//mpg2;
    ali_video_select_decoder(decoder, 0);
    close(decp->video_fd);

    free(decp);
    ali_video_priv = NULL;
}

/** Push next PES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_video_write_header(ali_video_decoder_id decoder, const struct av_packet *pkt_hdr)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    uint32_t write_size;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        write_size = write(decp->pkt_hdr_fd, pkt_hdr, sizeof(struct av_packet));
        if(write_size == sizeof(struct av_packet)) {
            return 0;
        } else {
            return -1;
        }
    }

    return 0;
}

/** Push next fragment of PES packet to video decoder.
 *  \param buf  buffer with fragment of one PES packet
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_video_write(ali_video_decoder_id decoder, const uint8_t *buf, uint32_t size)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    uint32_t write_size, update_size = 0;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        write_size = write(decp->pkt_data_fd, buf, size);
        if(write_size == size) {
            return 0;
        } else {
            return -1;
        }
    } else {
        while(1) {
            write_size = write(decp->video_fd, buf+update_size, size);
            if(write_size > 0) {
                size -= write_size;
                update_size += write_size;
                if(size == 0) {
                    break;
                }
            }
            usleep(10000);
        }
    }

    return 0;
}

/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_video_pause(ali_video_decoder_id decoder, int32_t pause)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_mp_pause_param pause_param;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        pause_param.pause_decode  = 0xFF;
        pause_param.pause_display = pause;
        ret = ali_video_ioctl(decoder, VDECIO_MP_PAUSE, (uint32_t)&pause_param);
        if(ret < 0) {
            log_info("decore pause video fail\n");
            return -1;
        }
    } else {
        if(pause) {
            ret = ioctl(decp->video_fd, VDECIO_PAUSE, 0);
        } else {
            ret = ioctl(decp->video_fd, VDECIO_RESUME, 0);
        }

        if(ret < 0) {
            log_info("pause video fail\n");
            return -1;
        }
    }

    return 0;
}

/** Enable/disable STC sync.
 *  With sync enabled, decoder displays video frames according to pts/stc difference.
 *  With sync disabled, decoder displays video frames according to vsync.
 *  \param enable  enable/disable STC sync
 *  \returns 0 on success, negative on error
 */
int ali_video_set_sync(ali_video_decoder_id decoder, int32_t enable)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_mp_sync_param mp_sync_param;
    struct vdec_sync_param sync_param;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        mp_sync_param.sync_mode = enable ? AV_SYNC_EXTERNAL : AV_SYNC_NONE;
        mp_sync_param.sync_unit = 0;
        ret = ali_video_ioctl(decoder, VDECIO_MP_SET_SYNC_MODE, (uint32_t)&mp_sync_param);
        if(ret < 0) {
            log_info("decore set sync mode fail: %d\n",ret);
            return -1;
        }
    } else {
        sync_param.sync_mode = enable ? VDEC_SYNC_PTS : VDEC_SYNC_FREERUN;
        ret = ioctl(decp->video_fd, VDECIO_SET_SYNC_MODE, &sync_param);
        if(ret < 0) {
            log_info("video set sync mode fail\n");
            return -1;
        }
    }

    return 0;
}

/** Choose what frames are to be decoded.
 *  \param types see \e ali_video_frame_types
 *  \returns 0 on success, negative on error
 */
int ali_video_set_frame_types(ali_video_decoder_id decoder, ali_video_frame_types types)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        ret = ali_video_ioctl(decoder, VDECIO_MP_SET_DEC_FRM_TYPE, types);
        if(ret < 0) {
            log_info("decore set decode mode fail: %d\n",ret);
            return -1;
        }
    } else {
        ret = ioctl(decp->video_fd, VDECIO_SET_DEC_FRM_TYPE, types);
        if(ret < 0) {
            log_info("video set frame types fail\n");
            return -1;
        }
    }

    return 0;
}

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
int ali_video_get_status(ali_video_decoder_id decoder, ali_video_decoder_status *status)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_decore_status decore_status;
    struct vdec_information vdec_stat;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    memset(&decore_status, 0, sizeof(decore_status));

    if(decp->decode_mode) {
        ret = ali_video_ioctl(decoder, VDECIO_MP_GET_STATUS, (uint32_t)&decore_status);
        if(ret < 0) {
            log_info("decore get status fail: %d\n",ret);
            return -1;
        }

        status->width = decore_status.pic_width;
        status->height = decore_status.pic_height;
        status->fps = decore_status.frame_rate;
        status->interlaced = decore_status.interlaced_frame;
        status->top_field_first = decore_status.top_field_first;
        status->frames_decoded = decore_status.frames_decoded;
        status->frames_displayed = decore_status.frames_displayed;
        status->last_pts = decore_status.frame_last_pts;
        status->buffer_size = decore_status.buffer_size;
        status->buffer_used = decore_status.buffer_used;
    } else {
        ret = ioctl(decp->video_fd, VDECIO_GET_STATUS, (uint32_t)&vdec_stat);
        if(ret < 0) {
            log_info("video get status fail\n");
            return -1;
        }

        status->width = vdec_stat.pic_width;
        status->height = vdec_stat.pic_height;
        status->fps = vdec_stat.frame_rate;
        status->interlaced = vdec_stat.interlaced_frame;
        status->top_field_first = decore_status.top_field_first;
        status->frames_decoded = vdec_stat.frames_decoded;
        status->frames_displayed = vdec_stat.frames_displayed;
        status->last_pts = vdec_stat.frame_last_pts;
        status->buffer_size = vdec_stat.buffer_size;
        status->buffer_used = vdec_stat.buffer_used;
    }

    return 0;
}

/** Get current buffer status of decoder.
 *  \param[out] buffer_status  returned buffer status
 *  \returns 0 on success, negative on error
 */
int ali_video_get_buffer_status(ali_video_decoder_id decoder, ali_video_decoder_buffer_status *buffer_status)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_information vdec_stat;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    memset(&vdec_stat, 0, sizeof(vdec_stat));

    if(decp->decode_mode) {
        ioctl(decp->pkt_data_fd, SBMIO_SHOW_TOTAL_SIZE, &buffer_status->total_size);
        ioctl(decp->pkt_data_fd, SBMIO_SHOW_VALID_SIZE, &buffer_status->valid_size);
        ioctl(decp->pkt_data_fd, SBMIO_SHOW_FREE_SIZE, &buffer_status->free_size);
    } else {
        ret = ali_video_ioctl(decoder, VDECIO_GET_STATUS, (uint32_t)&vdec_stat);
        if(ret < 0) {
            log_info("video get status fail\n");
            return -1;
        }

        buffer_status->total_size = vdec_stat.buffer_size;
        buffer_status->valid_size = vdec_stat.buffer_used;
        buffer_status->free_size  = buffer_status->total_size - buffer_status->valid_size;
    }

    return 0;
}

int ali_video_ioctl(ali_video_decoder_id decoder, int cmd, uint32_t param)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    static uint32_t decore_init = 0;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        if(decp->codec_id != h264 && decp->codec_id != mpg2
                && decp->codec_id != xvid && decp->codec_id != flv1
                && decp->codec_id != wvc1 && decp->codec_id != wx3
                && decp->codec_id != rmvb && decp->codec_id != mjpg
                && decp->codec_id != vp8  && decp->codec_id != hevc) {
            log_info("video decore ioctl not support %x\n",decp->codec_id);
            return -1;
        }

        if(decp->video_fd <= 0) {
            log_info("decoder has not been opened\n");
            return -1;
        }

        if(cmd == VDECIO_MP_INITILIZE) {
            decore_init = 1;
        }

        if(decore_init == 0 && cmd != VDEC_CMD_INIT) {
            log_info("video decore has not been inited %d\n", cmd);
            return -1;
        }

        ret = ioctl(decp->video_fd, cmd, param);
        if(ret < 0) {
            log_info("video decore ioctl fail\n");
            ret = -1;
        }

        if(cmd == VDECIO_MP_RELEASE) {
            decore_init = 0;
        }
    } else {
        ret = ioctl(decp->video_fd, cmd, param);
    }

    return ret;
}

/** Connect video decoder with demux channel.
 *  \param channel input source for video decoder's PES parser
 *  \returns 0 on success, negative on error
 */
/*
int ali_video_set_demux_channel(ali_video_decoder_id decoder, ali_demux_channel_id channel)
{
    struct ali_decoder_private *decp = (struct ali_decoder_private *)decoder;
    struct vdec_mp_rls_param rls_param;
    int ret;

    if(decp->codec_id != mpg2 && decp->codec_id != h264) {
        log_info("video ioctl not support %x\n",decp->codec_id);
        return -1;
    }

    if(decp->decode_mode) {
        memset(&rls_param, 0, sizeof(rls_param));

        ali_video_stop(decoder, 0, 0);

        ret = ali_video_ioctl(decoder, VDECIO_MP_RELEASE, &rls_param);
        if(ret < 0){
            log_info("decore release fail\n");
            return -1;
        }

        close(decp->pkt_hdr_fd);
    	close(decp->pkt_data_fd);
        close(decp->dec_out_fd);
        close(decp->disp_in_fd);

        decp->decode_mode = 0;

        ali_video_select_decoder(decoder, 0);
        ret = ali_video_start(decoder);
        if(ret < 0 ){
            log_info("decore start video fail\n");
            return -1;
        }
    }

    return 0;
}
*/
