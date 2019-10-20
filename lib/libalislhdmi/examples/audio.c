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
#include "audio.h"

int ali_audio_get_memory(int32_t video_id, ali_audio_decoder_mem *decoder_mem)
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


    decoder_mem->mp_mem_addr = (uint32_t)video_mem_info.mp_mem_start;
    decoder_mem->mp_mem_size = video_mem_info.mp_mem_size;
    log_info("decoder use mem: mp 0x%x 0x%x\n",
             decoder_mem->mp_mem_addr, decoder_mem->mp_mem_size);

    close(video_fd);

    return 0;
}

/** Open video decoder with given codec.
 *  \param decoder_init  codec to be used
 *  \param[out] decoder_out returned decoder instance
 *  \returns 0 on success, negative on error
 */
int ali_audio_open(ali_audio_decoder_init *decoder_init, ali_audio_decoder_id *decoder_out)
{
    struct ali_aduio_decoder_private *decp = NULL;
    struct sbm_config sbm_info;
    struct audio_config audio_init;

    ali_audio_decoder_buffer *decoder_buffer = &decoder_init->dec_buf;
    //int pkt_sbm_idx, dec_out_sbm_idx, display_sbm_idx;
    int ret = 0, reset = 0;
    char dev_name[24];

    if(*decoder_out) {
        log_info("decoder has been inited\n");
        return ret;
    }

    decp = malloc(sizeof(struct ali_aduio_decoder_private));
    if(decp == NULL) {
        log_info("malloc decoder private fail\n");
        goto fail;
    }

    memset(decp, 0, sizeof(struct ali_aduio_decoder_private));
    decp->decode_mode = decoder_init->decode_mode;
    decp->codec_id = decoder_init->codec_id;
    decp->audio_id = decoder_init->audio_id;

    memset(&sbm_info, 0, sizeof(sbm_info));
    memset(&audio_init, 0, sizeof(struct audio_config));

    sprintf(dev_name, "/dev/ali_sbm%d", decoder_init->pkt_data_sbm);
    decp->pkt_data_fd = open(dev_name, O_WRONLY);
    if(decp->pkt_data_fd <= 0) {
        log_info("open %s fail\n", dev_name);
        goto fail;
    }
    sbm_info.buffer_addr = decoder_buffer->pkt_data_addr;
    sbm_info.buffer_size = decoder_buffer->pkt_data_size;
    sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = 128*1024;
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
        sbm_info.reserve_size = 10*sizeof(struct av_packet);
        sbm_info.wrap_mode = SBM_MODE_NORMAL;
        sbm_info.lock_mode = SBM_SPIN_LOCK;
        ret = ioctl(decp->pkt_hdr_fd, SBMIO_CREATE_SBM, &sbm_info);
        if(ret < 0) {
            log_info("decore create sbm fail\n");
            goto fail;
        }
    }

    audio_init.decode_mode = decoder_init->decode_mode;
    audio_init.sync_mode   = 0;
    audio_init.sync_unit   = 0;

    audio_init.deca_input_sbm = (decoder_init->pkt_hdr_sbm<<16)|decoder_init->pkt_data_sbm;
    audio_init.deca_output_sbm = -1;
    audio_init.snd_input_sbm = -1;

    audio_init.codec_id = decoder_init->codec_id;
    audio_init.extradata_size = decoder_init->extradata_size;
    memcpy(audio_init.extra_data, decoder_init->extradata, decoder_init->extradata_size);

    audio_init.bits_per_coded_sample = 32;
    audio_init.sample_rate = decoder_init->samplerate;
    audio_init.channels = decoder_init->channels;
    audio_init.channels = (audio_init.channels>2)?2:audio_init.channels;
    audio_init.bit_rate = 1;
    audio_init.pcm_buf_size = 0;
    audio_init.pcm_buf = (UINT32)NULL;
    audio_init.block_align = 1;
    audio_init.codec_frame_size = 1;

    ret = ali_audio_ioctl(DECA_DECORE_INIT, &audio_init, &reset);
    if(ret != 0) {
        printf("deca decore init fail %d\n", ret);
        goto fail;
    }

    *decoder_out = (ali_audio_decoder_id)decp;

    return 0;

fail:
    if(decp->pkt_hdr_fd > 0)
        close(decp->pkt_hdr_fd);
    if(decp->pkt_data_fd > 0)
        close(decp->pkt_data_fd);
    if(decp)
        free(decp);

    *decoder_out = 0;

    return -1;
}

/** Close given decoder instance.
 *  \param decoder  decoder instance created with \e ali_video_decoder_open()
 */
void ali_audio_close(ali_audio_decoder_id decoder)
{
    struct ali_aduio_decoder_private *decp = (struct ali_aduio_decoder_private *)decoder;

    if(decp == NULL) {
        log_info("decore has been released\n");
        return;
    }

    ali_audio_ioctl(DECA_DECORE_RLS, NULL, NULL);

    if(decp->pkt_hdr_fd > 0)
        close(decp->pkt_hdr_fd);
    if(decp->pkt_data_fd > 0)
        close(decp->pkt_data_fd);

    decp->codec_id = 0;
    free(decp);
}

/** Push next PES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_audio_write_header(ali_audio_decoder_id decoder, const struct av_packet *pkt_hdr)
{
    struct ali_aduio_decoder_private *decp = (struct ali_aduio_decoder_private *)decoder;
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
int ali_audio_write(ali_audio_decoder_id decoder, const uint8_t *buf, uint32_t size)
{
    struct ali_aduio_decoder_private *decp = (struct ali_aduio_decoder_private *)decoder;
    uint32_t write_size;

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
    }

    return 0;
}

/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_audio_pause(ali_audio_decoder_id decoder, int32_t pause)
{
    struct ali_aduio_decoder_private *decp = (struct ali_aduio_decoder_private *)decoder;
    int pause_decode = 0,  pause_output = 0;
    int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        pause_decode = pause;
        pause_output = pause;
        ret = ali_audio_ioctl(DECA_DECORE_PAUSE_DECODE, &pause_decode, &pause_output);
        if(ret < 0) {
            log_info("decore pause audio fail\n");
            return -1;
        }
    }

    return 0;
}

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
int ali_audio_get_status(ali_audio_decoder_id decoder, ali_audio_decoder_status *status)
{

    return 0;
}

/** Get current buffer status of decoder.
 *  \param[out] buffer_status  returned buffer status
 *  \returns 0 on success, negative on error
 */
int ali_audio_get_buffer_status(ali_audio_decoder_id decoder, ali_audio_decoder_buffer_status *buffer_status)
{
    struct ali_aduio_decoder_private *decp = (struct ali_aduio_decoder_private *)decoder;
    //int ret;

    if(decp == NULL) {
        log_info("decoder has not been opened\n");
        return -1;
    }

    if(decp->decode_mode) {
        ioctl(decp->pkt_data_fd, SBMIO_SHOW_TOTAL_SIZE, &buffer_status->total_size);
        ioctl(decp->pkt_data_fd, SBMIO_SHOW_VALID_SIZE, &buffer_status->valid_size);
        ioctl(decp->pkt_data_fd, SBMIO_SHOW_FREE_SIZE, &buffer_status->free_size);
    }

    return 0;
}

int ali_audio_ioctl(int cmd, void *param1, void *param2)
{
    struct ali_audio_rpc_pars rpc_pars;
    static unsigned int decore_init = 0;
    int audio_fd, ret = 0;

    if(cmd == DECA_DECORE_INIT) {
        decore_init = 1;
    }

    if(decore_init == 0 && cmd != DECA_DECORE_INIT) {
        printf("audio decore has not been inited %d\n", cmd);
        return -1;
    }

    audio_fd = open("/dev/ali_m36_audio0", O_RDWR);
    if(audio_fd <= 0) {
        printf("open audio fail\n");
        return -1;
    }

    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));

    rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
    rpc_pars.arg_num = 3;

    switch(cmd) {
        case DECA_DECORE_INIT:
            rpc_pars.arg[0].arg = (void *)&cmd;
            rpc_pars.arg[0].arg_size = sizeof(cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = sizeof(struct audio_config);
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 4;
            break;
        case DECA_DECORE_RLS:
        case DECA_DECORE_FLUSH:
            rpc_pars.arg[0].arg = (void *)&cmd;
            rpc_pars.arg[0].arg_size = sizeof(cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 0;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        case DECA_DECORE_SET_BASE_TIME:
        case DECA_DECORE_SET_QUICK_MODE:
            rpc_pars.arg[0].arg = (void *)&cmd;
            rpc_pars.arg[0].arg_size = sizeof(cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        case DECA_DECORE_PAUSE_DECODE:
        case DECA_DECORE_SET_SYNC_MODE:
            rpc_pars.arg[0].arg = (void *)&cmd;
            rpc_pars.arg[0].arg_size = sizeof(cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 4;
            break;
        case DECA_DECORE_GET_PCM_TRD:
            rpc_pars.arg[0].arg = (void *)&cmd;
            rpc_pars.arg[0].arg_size = sizeof(cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[1].out = 1;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 4;
            rpc_pars.arg[2].out = 1;
            break;
        case DECA_DECORE_GET_STATUS:
            rpc_pars.arg[0].arg = (void *)&cmd;
            rpc_pars.arg[0].arg_size = sizeof(cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = sizeof(struct audio_decore_status);
            rpc_pars.arg[1].out = 1;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        default:
            ret = -1;
            break;
    }

    ret = ioctl(audio_fd, AUDIO_DECORE_COMMAND, &rpc_pars);
    if(ret < 0) {
        printf("audio decore ioctl fail\n");
        ret = -1;
    }

    close(audio_fd);

    if(cmd == DECA_DECORE_RLS) {
        decore_init = 0;
    }

    return ret;
}
