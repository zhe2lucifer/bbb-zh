/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: videotest.c
 *
 *  Description:
 *
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.02.22       Dylan.Yang     0.1.000      Add Video Demo
 ****************************************************************************/
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
#include <sys/socket.h>
#include <sys/epoll.h>
#include <linux/netlink.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "fb.h"
#include "video.h"
#include <linux/Ali_DmxLibInternal.h>
#include <ali_dmx_common.h>
#include "audio.h"
#include <ctype.h>
#define VPKT_HDR_SBM_IDX        0
#define PIP_VPKT_HDR_SBM_IDX    1
#define VPKT_DATA_SBM_IDX       2
#define PIP_VPKT_DATA_SBM_IDX   3
#define DISP_IN_SBM_IDX         4
#define PIP_DISP_IN_SBM_IDX     5

static int play_exit = 0;


static int video_play_stopped = 0;
static int audio_play_stopped = 0;


#define APKT_HDR_SBM_IDX        4
#define APKT_DATA_SBM_IDX       5


ali_audio_codec_id audio_codec_id = 0x15002;
int32_t audio_sample_rate = 48000;


static int file_is_exist(const char *fname)
{
    FILE *file = NULL;

    file = fopen(fname, "rb");

    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}


static void signal_handler(int signal, siginfo_t *siginfo, void *context)
{
    printf("Receive SIGNAL: %d\n", signal);
    switch(signal) {
        case SIGINT:
            play_exit = 1;
            break;
        default:
            break;
    }
}

void *test_audio_playback(void * ctx)
{
    ali_audio_decoder_init decoder_init;
    ali_audio_codec_id codec_id = audio_codec_id; //0x15002;
    ali_audio_decoder_id decoder_out = 0;
    ali_audio_decoder_status decore_status;
    ali_audio_decoder_mem decoder_mem;
    ali_audio_decoder_buffer *decoder_buffer = NULL;
    ali_audio_decoder_buffer_status buffer_status;

    char *file_name = (char *)ctx;
    struct av_packet pkt;
    //  FILE *pts_fp = NULL;
    FILE *pkt_fp = NULL;
    FILE *pktsize_fp = NULL;
    FILE *extradata_fp = NULL;
    //uint64_t file_size = 0;
    uint32_t run_addr = 0, run_size = 0, frame_size = 0, extra_data_size = 0;
    uint32_t free_size = 0, valid_size = 0, total_size = 0;
    uint8_t *frame = NULL, *extra_data = NULL;
    uint8_t file_name_len = 0;
    char pts_name[128] = {0x0};
    char pkt_name[128] = {0x0};
    char pktsize_name[128] = {0x0};
    char extradata_name[128] = {0x0};
    int32_t read_size = 0, need_read = 1, ret = 0, flags = 0;
    //int32_t i = 0;
    int32_t audio_id = 0;

    decoder_out = 0;

    memset(&pkt, 0, sizeof(pkt));
    memset(&decoder_init, 0, sizeof(decoder_init));
    memset(&decoder_mem, 0, sizeof(decoder_mem));
    memset(&buffer_status, 0, sizeof(buffer_status));


    memset(pts_name, 0x0, sizeof(pts_name));
    memset(pkt_name, 0x0, sizeof(pkt_name));
    memset(pktsize_name, 0x0, sizeof(pktsize_name));
    memset(extradata_name, 0x0, sizeof(extradata_name));

    file_name_len = strlen(file_name);


    strncpy(pts_name, file_name, file_name_len);
    strncat(pts_name, ".audio0.pts", strlen(".audio0.pts"));
    if (file_is_exist(pts_name) == 0) {
        printf("The %s does not exit!!\n", pts_name);
        goto exit;
    }


    strncpy(pkt_name, file_name, file_name_len);
    strncat(pkt_name, ".audio0.pkt", strlen(".audio0.pkt"));
    if (file_is_exist(pkt_name) == 0) {
        printf("The %s does not exit!!\n", pkt_name);
        goto exit;
    }

    strncpy(pktsize_name, file_name, file_name_len);
    strncat(pktsize_name, ".audio0.pktsize", strlen(".audio0.pktsize"));
    if (file_is_exist(pktsize_name) == 0) {
        printf("The %s does not exit!!\n", pktsize_name);
        goto exit;
    }

    strncpy(extradata_name, file_name, file_name_len);
    strncat(extradata_name, ".audio0.extradata", strlen(".audio0.extradata"));
    if (file_is_exist(extradata_name) == 0) {
        printf("The %s does not exit!!\n", extradata_name);
        goto exit;
    }

    pkt_fp = fopen(pkt_name, "rb");
    if(pkt_fp == NULL) {
        printf("open pkt file %s fail\n", pkt_name);
        goto exit;
    }

    pktsize_fp = fopen(pktsize_name, "rb");
    if(pktsize_fp == NULL) {
        printf("open pkt size file %s fail\n", pktsize_name);
        goto exit;
    }

    frame = (uint8_t *)malloc(0x100000);
    if(frame == NULL) {
        printf("malloc frame buffer fail\n");
        goto exit;
    }

    extra_data = (uint8_t *)malloc(0x400);
    if(extra_data == NULL) {
        printf("malloc extra data buffer fail\n");
        goto exit;
    } else {
        extradata_fp = fopen(extradata_name, "rb");
        if(extradata_fp) {
            fseek(extradata_fp, 0, SEEK_END);
            extra_data_size = ftell(extradata_fp);
            fseek(extradata_fp, 0, SEEK_SET);
            fflush(extradata_fp);

            if(extra_data_size <= 0) {
                extra_data_size = 0;
            }

            read_size = fread(extra_data, 1, extra_data_size, extradata_fp);
            if(read_size != extra_data_size) {
                extra_data_size = 0;
            }
            fclose(extradata_fp);
        } else {
            printf("no extra data file %s found\n", extradata_name);
            goto exit;
        }
    }

    ret = ali_audio_get_memory(0, &decoder_mem);
    if(ret < 0) {
        printf("get video decoder memory fail\n");
        goto exit;
    }

    decoder_buffer = &(decoder_init.dec_buf);

    run_addr = decoder_mem.mp_mem_addr + decoder_mem.mp_mem_size;
    run_size = decoder_mem.mp_mem_size;

    /* allocate decoder buffers */
    decoder_buffer->pkt_hdr_size  = 4096 * sizeof(struct av_packet);
    decoder_buffer->pkt_data_size = 0x300000;

    if(decoder_buffer->pkt_hdr_size + decoder_buffer->pkt_data_size> run_size) {
        printf("decore not enough memory %d\n", run_size);
        goto exit;
    }

    decoder_buffer->pkt_data_addr = run_addr - decoder_buffer->pkt_data_size;
    run_addr -= decoder_buffer->pkt_data_size;
    run_size -= decoder_buffer->pkt_data_size;

    decoder_buffer->pkt_hdr_addr = (decoder_buffer->pkt_hdr_size == 0) ? 0 : (run_addr - decoder_buffer->pkt_hdr_size);
    run_addr -= decoder_buffer->pkt_hdr_size;
    run_size -= decoder_buffer->pkt_hdr_size;


    /* init decoder */
    decoder_init.decode_mode = 1; // 1 media player;
    decoder_init.codec_id    = codec_id;

    decoder_init.pkt_hdr_sbm = APKT_HDR_SBM_IDX;
    decoder_init.pkt_data_sbm = (decoder_buffer->pkt_hdr_size == 0) ? APKT_HDR_SBM_IDX : APKT_DATA_SBM_IDX;
    decoder_init.extradata    = extra_data;
    decoder_init.extradata_size = extra_data_size;
    decoder_init.channels     = 2;
    decoder_init.samplerate   = audio_sample_rate; //48000;
    decoder_init.audio_id = audio_id;
    ret = ali_audio_open(&decoder_init, &decoder_out);
    if(ret < 0) {
        printf("open audio decoder fail\n");
        goto exit;
    }

    usleep(1000);

    while(1) {
        ali_audio_get_buffer_status(decoder_out, &buffer_status);
        total_size = buffer_status.total_size;
        valid_size = buffer_status.valid_size;
        free_size = buffer_status.free_size;

        if(need_read) {
            /* parse a complete frame */
            read_size = fread(&frame_size, 1, 4, pktsize_fp);
            if(frame_size & 0x80000000) {
                frame_size &= ~0x80000000;
                flags = AV_PKT_FLAG_CODEC_DATA;
            } else {
                flags = 0;
            }

            read_size = fread(frame, 1, frame_size, pkt_fp);
        }

        if(read_size == frame_size) {
            if(free_size > read_size && valid_size < total_size*4/5) {
                pkt.pts = AV_NOPTS_VALUE;
                pkt.dts = AV_NOPTS_VALUE;
                pkt.size = read_size;
                pkt.flags = flags;

                do {
                    ret = ali_audio_write_header(decoder_out, &pkt);
                    if(ret != 0) {
                        usleep(100*1000);
                    }

                    if(play_exit) {
                        break;
                    }
                } while(ret != 0);

                do {
                    ret = ali_audio_write(decoder_out, frame, read_size);
                    if(ret != 0) {
                        usleep(100*1000);
                    }

                    if(play_exit) {
                        break;
                    }
                } while(ret != 0);

                need_read = 1;
            } else {
                memset(&decore_status, 0, sizeof(decore_status));
                //ali_video_ioctl(decoder_out, VDECIO_MP_GET_STATUS, &decore_status);
                //printf("video status %d %d %d %d %d\n",decore_status.pic_width,decore_status.pic_height,
                //decore_status.frame_rate,decore_status.buffer_size,decore_status.buffer_used);
                usleep(100*000);
                need_read = 0;
            }

            if(play_exit) {
                break;
            }
        } else {
            printf("read frame error %d %d\n", read_size, frame_size);
            break;
        }
    }

    while(1) {
        if(play_exit) {
            break;
        }

        ali_audio_get_buffer_status(decoder_out, &buffer_status);

        if(buffer_status.valid_size < 1024)
            break;
        else
            sleep(1);
    }



exit:

    if (decoder_out != 0) {
        ali_audio_close(decoder_out);
        decoder_out = 0;
    }

    if (pkt_fp) {
        fclose(pkt_fp);
        pkt_fp = NULL;
    }

    if (pktsize_fp) {
        fclose(pktsize_fp);
        pktsize_fp = NULL;
    }

    if(frame) {
        free(frame);
        frame = NULL;
    }

    if(extra_data) {
        free(extra_data);
        extra_data = NULL;
    }

    play_exit = 1;
    audio_play_stopped = 1;

    return NULL;
}

#define MAX_EVENTS           10
#define EPOLL_TIMEOUT        -1//3000
#define MAX_MSG_SIZE         1024
#define NETLINK_ALITRANSPORT 31

typedef struct {
    int argc;
    char file_name[128];
    char codec_name[128];
    int32_t width;
    int32_t height;
    int32_t frame_rate;
    int32_t video_id;
} video_play_ctx_t;

/* for media player test */
//static int test_playback(int argc, char *argv[])

static void *test_video_playback(void *param)
{
    ali_video_decoder_init decoder_init;
    ali_video_codec_id codec_id = h264;
    ali_video_decoder_id decoder_out = 0;
    ali_video_decoder_mem decoder_mem;
    ali_video_decoder_buffer *decoder_buffer = NULL;
    ali_video_decoder_buffer_status buffer_status;
    struct vdec_display_param display_param;
    struct vdec_pip_param pip_param;
    struct vdec_decore_status decore_status;
    struct av_packet pkt;
    FILE *pkt_fp = NULL;
    FILE *pktsize_fp = NULL;
    FILE *extradata_fp = NULL;
    uint32_t run_addr = 0, run_size = 0, frame_size = 0, extra_data_size = 0;
    uint32_t free_size = 0, valid_size = 0, total_size = 0, last_buffer_used = 0;
    uint8_t *frame = NULL, *extra_data = NULL;
    char *file_name = NULL;
    int file_name_len = 0;
    char pts_name[128] = {0x0};
    char pkt_name[128] = {0x0};
    char pktsize_name[128] = {0x0};
    char extradata_name[128] = {0x0};
    char *codec_name = "h264";
    int32_t width = 0, height = 0, frame_rate = 0, preview = 0, flags = 0;
    int32_t read_size = 0, need_read = 1, ret = 0;
    int32_t i = 0;
    int32_t avsync_fd = -1;
    int32_t video_id = 0;
    int32_t pkt_hdr_sbm = -1;
    int32_t layer = VPO_MAINWIN;

    video_play_ctx_t *ctx = (video_play_ctx_t *) param;

    if (!ctx) {
        return NULL;
    }

    printf("[%s] [%s] [%d] [%d] [%d] [%d]\n", ctx->file_name, ctx->codec_name, ctx->width, ctx->height, ctx->frame_rate, ctx->video_id);

    avsync_fd = open(AVSYNCDEV, O_RDWR);

    decoder_out = 0;

    memset(&pkt, 0, sizeof(pkt));
    memset(&decoder_init, 0, sizeof(decoder_init));
    memset(&decoder_mem, 0, sizeof(decoder_mem));
    memset(&buffer_status, 0, sizeof(buffer_status));

    memset(pts_name, 0x0, sizeof(pts_name));
    memset(pkt_name, 0x0, sizeof(pkt_name));
    memset(pktsize_name, 0x0, sizeof(pktsize_name));
    memset(extradata_name, 0x0, sizeof(extradata_name));

    //file_name = argv[0];
    file_name = ctx->file_name;
    file_name_len = strlen(file_name);


    strncpy(pts_name, file_name, file_name_len);
    strncat(pts_name, ".video0.pts", strlen(".video0.pts"));
    if (file_is_exist(pts_name) == 0) {
        printf("The %s does not exit!!\n", pts_name);
        goto exit;
    }

    strncpy(pkt_name, file_name, file_name_len);
    strncat(pkt_name, ".video0.pkt", strlen(".video0.pkt"));
    if (file_is_exist(pkt_name) == 0) {
        printf("The %s does not exit!!\n", pkt_name);
        goto exit;
    }

    strncpy(pktsize_name, file_name, file_name_len);
    strncat(pktsize_name, ".video0.pktsize", strlen(".video0.pktsize"));
    if (file_is_exist(pktsize_name) == 0) {
        printf("The %s does not exit!!\n", pktsize_name);
        goto exit;
    }

    strncpy(extradata_name, file_name, file_name_len);
    strncat(extradata_name, ".video0.extradata", strlen(".video0.extradata"));
    if (file_is_exist(extradata_name) == 0) {
        printf("The %s does not exit!!\n", extradata_name);
        goto exit;
    }

    //codec_name = argv[2];
    codec_name = ctx->codec_name;

    if(strcmp(codec_name, "mpg2") == 0) {
        codec_id = mpg2;
    } else if(strcmp(codec_name, "h264") == 0) {
        codec_id = h264;
    } else if(strcmp(codec_name, "h265") == 0) {
        codec_id = hevc;
    } else if(strcmp(codec_name, "wmv3") == 0) {
        codec_id = wx3;
    } else if(strcmp(codec_name, "wvc1") == 0) {
        codec_id = wvc1;
    } else if(strcmp(codec_name, "xvid") == 0) {
        codec_id = xvid;
    } else if(strcmp(codec_name, "vp8") == 0) {
        codec_id = vp8;
    } else if(strcmp(codec_name, "rmvb") == 0) {
        codec_id = rmvb;
    } else if(strcmp(codec_name, "mjpg") == 0) {
        codec_id = mjpg;
    } else {
        codec_id = h264;
    }

    printf("video codec is [%c%c%c%c] !!\n",
           (uint8_t)codec_id, (uint8_t)(codec_id>>8), (uint8_t)(codec_id>>16), (uint8_t)(codec_id>>24));
    /*
        width  = (int32_t)atoi(argv[3]);
        height = (int32_t)atoi(argv[4]);
        frame_rate = (int32_t)atoi(argv[5]) * 1000;
        video_id = (int32_t)atoi(argv[1]);
      */
    width  = ctx->width;
    height = ctx->height;
    frame_rate = ctx->frame_rate * 1000;
    video_id = ctx->video_id;


    layer = VPO_MAINWIN;
    preview = 0;

    if(width <= 0) {
        width = 3840;
    }

    if(height <= 0) {
        height = 2160;
    }

    if(frame_rate <= 0) {
        frame_rate = 25000;
    }

    if(video_id < 0) {
        video_id = 0;
    }
    if(preview < 0) {
        preview = 0;
    }

    layer = (video_id == 0) ? VPO_MAINWIN : VPO_PIPWIN;


    pkt_hdr_sbm = (video_id == 0) ? VPKT_HDR_SBM_IDX : PIP_VPKT_HDR_SBM_IDX;

    pkt_fp = fopen(pkt_name, "rb");
    if(pkt_fp == NULL) {
        printf("open pkt file %s fail\n", pkt_name);
        goto exit;
    }

    pktsize_fp = fopen(pktsize_name, "rb");
    if(pktsize_fp == NULL) {
        printf("open pkt size file %s fail\n", pktsize_name);
        goto exit;
    }

    frame = (uint8_t *)malloc(0x400000);
    if(frame == NULL) {
        printf("malloc frame buffer fail\n");
        goto exit;
    }

    extra_data = (uint8_t *)malloc(0x4000);
    if(extra_data == NULL) {
        printf("malloc extra data buffer fail\n");
        goto exit;
    } else {
        extradata_fp = fopen(extradata_name, "rb");
        if(extradata_fp) {
            fseek(extradata_fp, 0, SEEK_END);
            extra_data_size = ftell(extradata_fp);
            fseek(extradata_fp, 0, SEEK_SET);
            fflush(extradata_fp);

            if(extra_data_size <= 0) {
                extra_data_size = 0;
            }

            read_size = fread(extra_data, 1, extra_data_size, extradata_fp);
            if(read_size != extra_data_size) {
                extra_data_size = 0;
            }
            fclose(extradata_fp);
        } else {
            printf("no extra data file %s found\n", extradata_name);
            goto exit;
        }
    }

    ret = ali_video_get_memory(video_id, &decoder_mem);
    if(ret < 0) {
        printf("get video decoder memory fail\n");
        goto exit;
    }

    decoder_buffer = &(decoder_init.dec_buf);

    run_addr = decoder_mem.mp_mem_addr;
    run_size = decoder_mem.mp_mem_size;

    /* allocate decoder buffers */
    decoder_buffer->frm_buf_addr = decoder_mem.main_mem_addr;
    decoder_buffer->frm_buf_size = decoder_mem.main_mem_size;

    decoder_buffer->pkt_hdr_size  = 0;//4096 * sizeof(struct av_packet);
    decoder_buffer->pkt_data_size = 0x500000;
    decoder_buffer->dec_out_size  = 0; /* dont need frame out to user */
    decoder_buffer->disp_in_size  = 0;//256 * sizeof(struct av_frame);

    if(decoder_buffer->pkt_hdr_size + decoder_buffer->pkt_data_size
            + decoder_buffer->dec_out_size + decoder_buffer->disp_in_size > run_size) {
        printf("decore not enough memory %d\n", run_size);
        goto exit;;
    }

    decoder_buffer->pkt_data_addr = run_addr;
    run_addr += decoder_buffer->pkt_data_size;
    run_size -= decoder_buffer->pkt_data_size;

    decoder_buffer->pkt_hdr_addr = (decoder_buffer->pkt_hdr_size == 0) ? 0 : run_addr;
    run_addr += decoder_buffer->pkt_hdr_size;
    run_size -= decoder_buffer->pkt_hdr_size;

    decoder_buffer->dec_out_addr = (decoder_buffer->dec_out_size == 0) ? 0 : run_addr;
    run_addr += decoder_buffer->dec_out_size;
    run_size -= decoder_buffer->dec_out_size;

    decoder_buffer->disp_in_addr = (decoder_buffer->disp_in_size == 0) ? 0 : run_addr;
    run_addr += decoder_buffer->disp_in_size;
    run_size -= decoder_buffer->disp_in_size;

    /* init decoder */
    decoder_init.input_type  = ALI_VIDEO_INPUT_USER;
    decoder_init.decode_mode = VDEC_MODE_SBM;
    decoder_init.codec_id    = codec_id;
    decoder_init.pic_width   = width;
    decoder_init.pic_height  = height;
    decoder_init.sar_width   = 1;
    decoder_init.sar_height  = 1;
    decoder_init.frame_rate  = frame_rate;
    decoder_init.pkt_hdr_sbm = pkt_hdr_sbm;
    decoder_init.pkt_data_sbm = (decoder_buffer->pkt_hdr_size == 0) ? pkt_hdr_sbm : VPKT_DATA_SBM_IDX;
    decoder_init.dec_out_sbm = (decoder_buffer->dec_out_size == 0) ? -1 : -1;//DECV_OUT_SBM_IDX;
    decoder_init.disp_in_sbm = (decoder_buffer->disp_in_size == 0) ? -1 : DISP_IN_SBM_IDX;
    decoder_init.extradata   = (int8_t *)extra_data;
    decoder_init.extradata_size = extra_data_size;
    decoder_init.video_id = video_id;
    ret = ali_video_open(&decoder_init, &decoder_out);
    if(ret < 0) {
        printf("open video decoder fail\n");
        goto exit;
    }

    memset(&pip_param, 0, sizeof(pip_param));
    pip_param.layer = layer;//(video_id == 0) ? VPO_MAINWIN : VPO_PIPWIN;
    ali_video_ioctl(decoder_out, VDECIO_SET_PIP_PARAM, (uint32_t)&pip_param);

    memset(&display_param, 0, sizeof(display_param));
    display_param.rect.src_x = 0;
    display_param.rect.src_y = 0;
    display_param.rect.src_w = 720;
    display_param.rect.src_h = 2880;
    if (preview || pip_param.layer == VPO_PIPWIN) {
        display_param.rect.dst_x = 0;
        display_param.rect.dst_y = 0;
        display_param.rect.dst_w = 720 / 2;
        display_param.rect.dst_h = 2880 / 2;
        display_param.mode = VDEC_PREVIEW;
    } else {
        display_param.rect.dst_x = 0;
        display_param.rect.dst_y = 0;
        display_param.rect.dst_w = 720;
        display_param.rect.dst_h = 2880;
        display_param.mode = VDEC_FULL_VIEW;
    }
    ali_video_ioctl(decoder_out, VDECIO_SET_DISPLAY_MODE, (uint32_t)&display_param);

    //ali_video_ioctl(decoder_out, VDECIO_MP_SET_QUICK_MODE, 1);
    ali_video_ioctl(decoder_out, VDECIO_MP_DYNAMIC_FRAME_ALLOC, 1);

    usleep(1000);

    while(1) {
        ali_video_get_buffer_status(decoder_out, &buffer_status);
        total_size = buffer_status.total_size;
        valid_size = buffer_status.valid_size;
        free_size = buffer_status.free_size;

        if(need_read) {
            /* parse a complete frame */
            read_size = fread(&frame_size, 1, 4, pktsize_fp);
            if(frame_size & 0x80000000) {
                frame_size &= ~0x80000000;
                flags = AV_PKT_FLAG_CODEC_DATA;
            } else {
                flags = 0;
            }

            read_size = fread(frame, 1, frame_size, pkt_fp);
        }

        if(read_size == frame_size) {
            if(free_size > read_size && valid_size < total_size*4/5) {
                pkt.pts = AV_NOPTS_VALUE;
                pkt.dts = AV_NOPTS_VALUE;
                pkt.size = read_size;
                pkt.flags = flags;

                do {
                    ret = ali_video_write_header(decoder_out, &pkt);
                    if(ret != 0) {
                        usleep(100*1000);
                    }

                    if(play_exit) {
                        break;
                    }
                } while(ret != 0);

                do {
                    ret = ali_video_write(decoder_out, frame, read_size);
                    if(ret != 0) {
                        usleep(100*1000);
                    }

                    if(play_exit) {
                        break;
                    }
                } while(ret != 0);

                need_read = 1;
            } else {
                memset(&decore_status, 0, sizeof(decore_status));
                //ali_video_ioctl(decoder_out, VDECIO_MP_GET_STATUS, &decore_status);
                //prinf("video status %d %d %d %d %d\n",decore_status.pic_width,decore_status.pic_height,
                //decore_status.frame_rate,decore_status.buffer_size,decore_status.buffer_used);
                usleep(100*000);
                need_read = 0;
            }

            if(play_exit) {
                break;
            }
        } else {
            printf("mzhu read frame error %d %d\n", read_size, frame_size);
            break;
        }
    }

    i = 0;

    while(1) {
        if(play_exit) {
            break;
        }

        memset(&decore_status, 0, sizeof(decore_status));
        ali_video_ioctl(decoder_out, VDECIO_MP_GET_STATUS, (uint32_t)(&decore_status));

        if(decore_status.buffer_used == last_buffer_used) {
            if(++i > 5)
                break;
        } else {
            i = 0;
        }
        last_buffer_used = decore_status.buffer_used;
        sleep(1);
    }



exit:


    if (decoder_out != 0) {
        ali_video_ioctl(decoder_out, VDECIO_MP_GET_STATUS, (uint32_t)&decore_status);
        ali_video_close(decoder_out);
        ali_video_vpo_onoff(decore_status.layer, 0);
        decoder_out = 0;
    }

    if (pkt_fp) {
        fclose(pkt_fp);
        pkt_fp = NULL;
    }

    if (pktsize_fp) {
        fclose(pktsize_fp);
        pktsize_fp = NULL;
    }

    if(frame) {
        free(frame);
        frame = NULL;
    }

    if(extra_data) {
        free(extra_data);
        extra_data = NULL;
    }

    if(avsync_fd > 0) {
        close(avsync_fd);
    }

    video_play_stopped = 1;
    play_exit = 1;
    return 0;
}


static int print_usage(void)
{
    //-n /mnt/usb/sda1/decoder/1920x1080_16r9.mkv -i 0 -t xvid -w 1920 -h 1080 -f 30 -at mp3 -ar 44100
    printf("./alislhdmikit --video [video_file_path] [video_id] [video_codec] [video_width] [video_heigh] [video_frame_rate] [audio_codec] [audio_sample_rate]");
    return 0;


}


static pthread_t g_audi_play_thid;
static pthread_t g_video_play_thid;
static unsigned int is_initialed  = 0;
video_play_ctx_t g_video_ctx;

//int main(int argc, char *argv[])
int videotest(int argc, char *argv[], unsigned int b_wait_finish)
{
    struct sigaction action;
    int res = 0;
    char *audio_codec_name = "CODEC_ID_AAC";
    char * file_name = NULL;

    if (argc != 8) {
        printf("There are error setting for parameters!! (argc = %d)\n", argc);
        print_usage();
        return -1;
    }

    file_name = argv[0];
    /*
    	if (file_is_exist(file_name) == 0)
    	{
    		printf("The %s does not exit!!\n", file_name);
    		return -1;
    	}
    */
    audio_play_stopped = 0;
    video_play_stopped = 0;

    play_exit = 0;

    if(b_wait_finish == 1) {
        action.sa_sigaction = signal_handler;
        action.sa_flags = SA_SIGINFO;
        sigemptyset(&action.sa_mask);
        sigaction(SIGINT, &action, NULL);
    }

    audio_codec_name = argv[6];

    if(strcmp(audio_codec_name, "mp3") == 0) {
        audio_codec_id = 0x15001;
    } else if(strcmp(audio_codec_name, "aac") == 0) {
        audio_codec_id = 0x15002;
    } else if(strcmp(audio_codec_name, "ac3") == 0) {
        audio_codec_id = 0x15003;
    } else {
        audio_codec_id = 0x15002;
    }

    audio_sample_rate = atoi(argv[7]);
    pthread_create(&g_audi_play_thid, NULL, test_audio_playback, file_name);

    memset(&g_video_ctx, 0x0, sizeof(video_play_ctx_t));
    strncpy(g_video_ctx.file_name, argv[0], strlen(argv[0]));
    strncpy(g_video_ctx.codec_name, argv[2], strlen(argv[2]));
    g_video_ctx.width  = (int32_t)atoi(argv[3]);
    g_video_ctx.height = (int32_t)atoi(argv[4]);
    g_video_ctx.frame_rate = (int32_t)atoi(argv[5]);
    g_video_ctx.video_id = (int32_t)atoi(argv[1]);
    pthread_create(&g_video_play_thid, NULL, test_video_playback, &g_video_ctx);


    if(b_wait_finish == 1) {
        struct timeval timeout;

        memset(&timeout, 0x0 ,sizeof(struct timeval));
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        while(1) {
            if ((audio_play_stopped == 1) && (video_play_stopped == 1)) {
                break;
            }
            select(1, NULL, NULL, NULL, &timeout);
        }

        pthread_join(g_video_play_thid, NULL);
        pthread_join(g_audi_play_thid, NULL);

        printf("Exit videotest !!\n");
    }
    is_initialed = 1;
    return res;
}

int videotest_stop(void)
{
    if (is_initialed) {
        play_exit = 1;
        while(1) {
            if ((audio_play_stopped == 1) && (video_play_stopped == 1)) {
                break;
            }
            usleep(500);

        }
        pthread_join(g_video_play_thid, NULL);
        pthread_join(g_audi_play_thid, NULL);
        printf("Stop videotest!!\n");
        is_initialed = 0;
    }
    return 0;
}