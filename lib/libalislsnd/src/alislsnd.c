/** @file     alislsnd.c
 *  @brief    this is a share libarary about audio, maybe called sound
 *            or snd module. Uplayer functions in HLD or other places
 *            can call functions here to control snd driver in kernel.
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            beacuse deca and sound is the same device in kernel, and
 *            audio device doesn't support open twice or more. but in
 *            hld and share library, they are two different muodules,
 *            so use a extern device, then deca and sound can use
 *            the same device.
 */

/* system headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/* kernel headers */
#include <ali_audio_common.h>
#include <ali_basic_common.h>
//#include <ali_stc_common.h>
#include <dvb_audio.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/mman.h>
/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alislevent.h>
#include <alislsnd.h>
#include <bits_op.h>

#include <sys/epoll.h>

/* local headers */
#include "internal.h"
#include "error.h"

#define PCM_HDR_LEN (32)
#define SHIFTBITS (4)
#if 1
#define pcm_cap_dbg(fmt, args...) SL_INFO("\033[0;31m %s@%d: \033[0m"fmt, __func__, __LINE__, ##args)
#else
#define pcm_cap_dbg(...)
#endif

#if 0
#define print_ko(fmt, args...) printf("\033[0;31m "fmt"\033[0m", ##args)
#else
#define print_ko(...) 
#endif
#if 0
#define print_ok(fmt, args...) printf("\033[0;32m "fmt"\033[0m", ##args)
#else
#define print_ok(...) 
#endif

struct sound_private *snd_dev = NULL;
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_mutex_snd_cb = PTHREAD_MUTEX_INITIALIZER;
//#define PCM_CAP_APP_DBG
#ifdef PCM_CAP_APP_DBG
#define USE_FILE
#define BACKUP_PCMDATA_FILE "/mnt/usb/sda1/pcmcaph"
#define BACKUP_BUF_LEN 0x100000
static FILE *backup_file = NULL;
static char *backup_buf = NULL;
static int backup_buf_wr = 0;
int backup_pcm_data_open() 
{
    backup_buf = (char *)malloc(BACKUP_BUF_LEN);
    if (!backup_buf) {
        pcm_cap_dbg("malloc fail\n");
        return -1;
    }
    backup_file = fopen(BACKUP_PCMDATA_FILE, "wb");
    if (!backup_file) {
        free(backup_buf);
        backup_buf = NULL;
        pcm_cap_dbg("fopen fail\n");
        return -1;
    }
    return 0;
}
int backup_pcm_data_close() 
{
    #ifndef USE_FILE
    pcm_cap_dbg("backup_buf_wr: %d\n", backup_buf_wr);
    fwrite(backup_buf, 1, backup_buf_wr, backup_file);
    #endif
    fsync(fileno(backup_file));
    fclose(backup_file);
    free(backup_buf);
    backup_buf = NULL;
    backup_buf_wr = 0;
    backup_file = NULL;
    return 0;
}
static int backup_pcm_data(struct sound_private *priv, struct pcm_capture_buff *info) {
    #ifndef USE_FILE
    if (!backup_buf)
        return 0;
    #else
    if (!backup_file)
        return 0;
    #endif
    if ((info->buff_rd < info->buff_wt) || (info->buff_wt_skip)) {
#ifndef USE_FILE
        if (backup_buf_wr + (info->buff_wt - info->buff_rd)*16 < BACKUP_BUF_LEN) {
            if (info->buff_wt_skip) {
                memcpy(backup_buf+backup_buf_wr, priv->pcm_cap_buf+info->buff_rd*16, (info->buff_wt_skip - info->buff_rd)*16);
                backup_buf_wr += (info->buff_wt_skip - info->buff_rd)*16;
            } else { 
                memcpy(backup_buf+backup_buf_wr, priv->pcm_cap_buf+info->buff_rd*16, (info->buff_wt - info->buff_rd)*16);
                backup_buf_wr += (info->buff_wt - info->buff_rd)*16;
            }
        }
#else
        if (info->buff_wt_skip)
            fwrite(priv->pcm_cap_buf+info->buff_rd*16, 1, (info->buff_wt_skip - info->buff_rd)*16, backup_file);
        else
            fwrite(priv->pcm_cap_buf+info->buff_rd*16, 1, (info->buff_wt - info->buff_rd)*16, backup_file);
#endif
        return (info->buff_wt_skip?(info->buff_wt_skip - info->buff_rd)*16:(info->buff_wt - info->buff_rd)*16);
    } else if (info->buff_wt < info->buff_rd) {
#ifndef USE_FILE
        if (backup_buf_wr + (info->buff_len - info->buff_rd + info->buff_wt)*16 < BACKUP_BUF_LEN) {
            memcpy(backup_buf+backup_buf_wr, priv->pcm_cap_buf+info->buff_rd*16, (info->buff_len - info->buff_rd)*16);
            memcpy(backup_buf+backup_buf_wr+(info->buff_len - info->buff_rd)*16, 
            priv->pcm_cap_buf, info->buff_wt*16);
            backup_buf_wr += (info->buff_len - info->buff_rd + info->buff_wt)*16;
        }
#else
        fwrite(priv->pcm_cap_buf+info->buff_rd*16, 1, (info->buff_len - info->buff_rd)*16, backup_file);
        fwrite(priv->pcm_cap_buf, 1, info->buff_wt*16, backup_file);
#endif
        return (info->buff_len - info->buff_rd + info->buff_wt)*16;
    }
    return 0;
}
#endif

typedef struct frm_header {
    char foo[16];
    short buff_len;
    short buff_wt;
    short buff_rd;
    short d1;
    short d2;
    short s1;
    short s2;
    short index;
}frm_header;

static int get_pcm_frame_cnt(struct sound_private *priv, struct pcm_capture_buff *p_buff)
{
    unsigned int total_len = 0;
    
    if ((p_buff->buff_wt > p_buff->buff_rd) || (p_buff->buff_wt_skip)) {
        if (p_buff->buff_wt_skip) {
            //  w-----------rd--------w_skip----
            total_len = (p_buff->buff_wt_skip-p_buff->buff_rd)<<SHIFTBITS;
        } else {
            // --------------rd----------wt-----
            total_len = (p_buff->buff_wt-p_buff->buff_rd)<<SHIFTBITS;
        }
    } else {
        //    ---------------wt---------rd-----
        total_len = (p_buff->buff_len-p_buff->buff_rd+p_buff->buff_wt)<<SHIFTBITS;
    }
    pcm_cap_dbg("info: %u, %u, %u, %u, %u\n", p_buff->buff_len, p_buff->buff_rd, p_buff->buff_wt, p_buff->buff_wt_skip, total_len);

#ifdef PCM_CAP_APP_DBG
    backup_pcm_data(priv, p_buff);
#endif
    int frm_cnt = 0;
    unsigned int parsed_len = 0;
    char *frm_buf_start = NULL;
    unsigned int cur_frm_len = 0;
    UINT32 tmp_rd = p_buff->buff_rd;
    /* frm_header structure is in section 7.1.1 of 
        \\alinet\zh\STB\STB_Project\C3505\2.Implement\Design Document\
        Digital\AUDIO\c3505_Audio_DataSheet.doc
        I2SO_Frame_len[15:0]:
        Note: 1. Unit is 128-bit
             2. Must be 256-bit aligned
             3. Length include Frame_Header(2*128-bit) and Sample Data.
    */
    while (parsed_len < total_len) {
        frm_buf_start = priv->pcm_cap_buf + (tmp_rd<<SHIFTBITS);
        cur_frm_len = ((frm_buf_start[1]<<8) |frm_buf_start[0])<<SHIFTBITS;
        /*
        1152*2*4+32 = 9248;
        576*2*4+32 = 4640;
        384*2*4+32 = 3104
        */
        if (((cur_frm_len != 9248)&&(cur_frm_len!=4640)&&(cur_frm_len!=3104))) {
            /*
            Temporary solution to handle exception.
            */
            if ((9248 == priv->last_pcm_frm_size)||
                (4640 == priv->last_pcm_frm_size)||
                    (3104 == priv->last_pcm_frm_size))
                cur_frm_len = priv->last_pcm_frm_size;
            else
                cur_frm_len = 9248;
        } else
            priv->last_pcm_frm_size = cur_frm_len;
#if 0 //def PCM_CAP_APP_DBG
        if (cur_frm_len != 9248) {
            int i=0;
            print_ko("ko the %d frm dump header:\n", frm_cnt+1);
            for(i=0;i<PCM_HDR_LEN;i++) {
                print_ko("%02x ", frm_buf_start[i]);
                if (i==15)
                    print_ko("\n");
            }
            print_ko("\ndump header end\n");
        } else {
            print_ok("ok the %d frm dump header:\n", frm_cnt+1);
            #if 0
            int i=0;
            for(i=0;i<PCM_HDR_LEN;i++) {
                print_ok("%02x ", frm_buf_start[i]);
                if (i==15)
                    print_ok("\n");
            }
            #else
            frm_header *p_frm_hdr = (frm_header *)frm_buf_start;
            print_ok("%d, %d, %d, %d, %d, %d, %d, %d\n", p_frm_hdr->buff_len, p_frm_hdr->buff_wt,
            p_frm_hdr->buff_rd, p_frm_hdr->d1, p_frm_hdr->d2, p_frm_hdr->s1, p_frm_hdr->s2,
            p_frm_hdr->index);
            #endif
            print_ok("\ndump header end\n");
        }
#endif
        frm_cnt++;
        parsed_len += cur_frm_len;
        tmp_rd += (cur_frm_len>>SHIFTBITS);
        tmp_rd %= (priv->pcm_cap_buf_len>>SHIFTBITS);
    }
    if (parsed_len != total_len) {
        pcm_cap_dbg("warning %u, %u\n", parsed_len, total_len);
    }
    pcm_cap_dbg("frm_cnt: %d\n", frm_cnt);
    return frm_cnt;
}

static sl_snd_capture_buffer * extract_pcm_frame_by_cnt(struct sound_private *priv, struct pcm_capture_buff *p_buff, int cnt)
{
    sl_snd_capture_buffer *p_pcm_buf = (sl_snd_capture_buffer *)malloc(cnt*sizeof(sl_snd_capture_buffer));
    if (!p_pcm_buf) {
        pcm_cap_dbg("malloc fail\n");
        return NULL;
    }
    unsigned int total_len = 0;
    if ((p_buff->buff_wt > p_buff->buff_rd) || (p_buff->buff_wt_skip)) {
        /*
        when the space left is not enough for a complete pcm frame, driver will set 
        buff_wt_skip to 1, then write pcm frame from the begin of pcm buffer
        */
        if (p_buff->buff_wt_skip) {
            // w------------rd--------w_skip----
            total_len = (p_buff->buff_wt_skip-p_buff->buff_rd)<<SHIFTBITS;
        } else {
            // --------------rd----------wt-----
            total_len = (p_buff->buff_wt-p_buff->buff_rd)<<SHIFTBITS;
        }
    } else {  //---------------wt---------rd-----
        total_len = (p_buff->buff_len - p_buff->buff_rd + p_buff->buff_wt)<<SHIFTBITS;
    }
    int frm_cnt = 0;
    unsigned int parsed_len = 0;
    char *frm_buf_start = NULL;
    unsigned int cur_frm_len = 0, cur_frm_data_len=0;
    UINT32 tmp_rd = p_buff->buff_rd;
    //Driver will make sure that the space left is enough for a complete pcm frame
    while ((parsed_len < total_len) && (frm_cnt<cnt)) {
        frm_buf_start = priv->pcm_cap_buf + (tmp_rd<<SHIFTBITS);
        cur_frm_len = ((frm_buf_start[1]<<8) |frm_buf_start[0])<<SHIFTBITS;
        if (((cur_frm_len != 9248)&&(cur_frm_len!=4640)&&(cur_frm_len!=3104)))
            cur_frm_len = priv->last_pcm_frm_size;
        cur_frm_data_len = cur_frm_len - PCM_HDR_LEN; //32byte header
        
#if 0 //def PCM_CAP_APP_DBG
        if (cur_frm_len != 9248) {
            int i=0;
            print_ko("ko the %d frm dump header:\n", frm_cnt+1);
            for(i=0;i<PCM_HDR_LEN;i++) {
                print_ko("%02x ", frm_buf_start[i]);
                if (i==15)
                    print_ko("\n");
            }
            print_ko("\ndump header end\n");
        } else {
            print_ok("ok the %d frm dump header:\n", frm_cnt+1);
            int i=0;
            for(i=0;i<PCM_HDR_LEN;i++) {
                print_ok("%02x ", frm_buf_start[i]);
                if (i==15)
                    print_ok("\n");
            }
            print_ok("\ndump header end\n");
        }
#endif
        p_pcm_buf[frm_cnt].pv_buffer_data = frm_buf_start+PCM_HDR_LEN;
        p_pcm_buf[frm_cnt].ul_buffer_length = cur_frm_data_len;
        p_pcm_buf[frm_cnt].ul_sample_rate = p_buff->sample_rate;
        frm_cnt++;
        pcm_cap_dbg("%d frm info: len: %u, sr: %u\n", frm_cnt, cur_frm_data_len, p_buff->sample_rate);
        parsed_len += cur_frm_len;
        tmp_rd += (cur_frm_len>>SHIFTBITS);
        tmp_rd %= (priv->pcm_cap_buf_len>>SHIFTBITS);
    }
    if ((parsed_len != total_len) ||(cnt != frm_cnt)) {
        pcm_cap_dbg("warning %u, %u\n", parsed_len, total_len);
    }
    pcm_cap_dbg("frm_cnt: %d\n", frm_cnt);
    return p_pcm_buf;
}

static int pcm_capture_buf_ret(struct sound_private *priv) {
    unsigned int total_size = (priv->last_buff_rd<<SHIFTBITS);
    alisl_retcode ret = 0;
    unsigned int i = 0;
    
    if ((0 == priv->pcm_cap_init_flag) ||(NULL == priv->pcm_buf_array)) {
        pcm_cap_dbg("already return pcmcap buf\n");
        return 0;
    }
    for (i=0; i<priv->pcm_buf_array_size; i++)
        total_size += PCM_HDR_LEN + priv->pcm_buf_array[i].ul_buffer_length;
    free(priv->pcm_buf_array);
    priv->pcm_buf_array = NULL;
    priv->pcm_buf_array_size = 0;

    if (ioctl(priv->fd, SND_SET_PCM_CAPTURE_BUFF_RD, (total_size%priv->pcm_cap_buf_len)>>SHIFTBITS)) {
        pcm_cap_dbg("SND_SET_PCM_CAPTURE_BUFF_RD %d fail\n", (total_size%priv->pcm_cap_buf_len)>>SHIFTBITS);
        ret = ERROR_IOCTL;
    } else {
        pcm_cap_dbg("update rd %d %d %d ok\n", (total_size%priv->pcm_cap_buf_len)>>SHIFTBITS, priv->last_buff_wt, priv->last_buff_wt_skip);
    }
    return ret;
}

static void sl_snd_handle_msg(void *msg)
{
    uint8_t  *msg_data = NULL;
    uint32_t  msg_type = 0;
    snd_deca_cb_type type = 0;
    uint32_t callback_info = 0;

    pthread_mutex_lock(&m_mutex_snd_cb);

    msg_data = (uint8 *)msg;
    msg_type = msg_data[0];
//SL_DBG will do nothing if disable DEBUG in Mini build,thus the warning that 'msg_len set but not used' occur
#ifdef ENABLE_DEBUG
	uint32_t  msg_len  = 0;
    msg_len  = msg_data[1];
    SL_DBG("msg_type: %u, msg_len: %u\n", msg_type, msg_len);
#endif
    switch(msg_type) {
        case MSG_FIRST_FRAME_OUTPUT: 
        {
            type = SL_DECA_FIRST_FRAME;
            break;
        }
        case MSG_DECA_MONITOR_NEW_FRAME:
        {
            type = SL_DECA_MONITOR_NEW_FRAME;
            break;
        }
        case MSG_DECA_MONITOR_START:
        {
            type = SL_DECA_MONITOR_START;
            break;
        }
        case MSG_DECA_MONITOR_STOP:
        {
            type = SL_DECA_MONITOR_STOP;
            break;
        }
        case MSG_DECA_MONITOR_DECODE_ERR:
        {
            type = SL_DECA_MONITOR_DECODE_ERR;
            break;
        }
        case MSG_DECA_MONITOR_OTHER_ERR:
        {
            type = SL_DECA_MONITOR_OTHER_ERR;
            break;
        }
        case MSG_SND_MONITOR_OUTPUT_DATA_END:
        {
            type = SL_SND_MONITOR_OUTPUT_DATA_END;
            break;
        }
        case MSG_SND_MONITOR_SBM_MIX_END:
        {
            if (msg_data[2] == 1) {
                callback_info = SL_SND_MIX_PAUSED;
            } else {
                callback_info = SL_SND_MIX_END_NORMAL;
            }
            type = SL_SND_MONITOR_MIX_DATA_END;
            break;
        }
        case MSG_SND_MONITOR_ERRORS_OCCURED:
        {
            type = SL_SND_MONITOR_ERRORS_OCCURED;
            break;
        }
        case MSG_SND_MONITOR_REMAIN_DATA_BELOW_THRESHOLD:
        {
            type = SL_SND_MONITOR_REMAIN_DATA_BELOW_THRESHOLD;
            break;
        }
        case MSG_DECA_STATE_CHANGED:
        default:
        {
            type = SL_DECA_CB_TYPE_MAX;
            SL_ERR("unsupported msg_type: %u\n", msg_type);
            pthread_mutex_unlock(&m_mutex_snd_cb);
            return;
        }
    }
    SL_DBG("type: %u, deca_cb: %p, snd_cb: %p\n", type, snd_dev->deca_cb, snd_dev->snd_cb);
    
    if((type < SL_SND_INDEX)&&(type >= SL_DECA_INDEX))
    {
        if(snd_dev->deca_cb)
        {
            snd_dev->deca_cb(type, 0, 0);
        }
    }
    else if((type >= SL_SND_INDEX)&&(type < SL_DECA_CB_TYPE_MAX))
    {
        if(snd_dev->snd_cb)
        {
            snd_dev->snd_cb(type, callback_info, 0);
        }
    }
    else
    {
        SL_ERR("it should not happen.\n", msg_type);
    }
    pthread_mutex_unlock(&m_mutex_snd_cb);
}

static void* sl_snd_poll_cb(void *param)
{
    unsigned char msg[MAX_KUMSG_SIZE] = {0};
    int ret = 0;
    //printf("************** %s ***********\n", __FUNCTION__);

    if (!snd_dev) {
        SL_ERR("device has been destroyed\n");
        return NULL;
    }

    ret = read(snd_dev->kumsg_fd, msg, MAX_KUMSG_SIZE);

    if (ret > 0)
        sl_snd_handle_msg(msg);
    
    return NULL;
}

static int sl_snd_add_poll_fd(struct sound_private *dev)
{
    int flags = O_CLOEXEC;

    if((dev->kumsg_fd = ioctl(dev->fd, AUDIO_GET_KUMSGQ, &flags)) < 0) {
        SL_ERR("AUDIO_GET_KUMSGQ fail(%d)\n", dev->kumsg_fd);
        return -1;
    }
    //create_fd
    dev->snd_cb_event.cb = sl_snd_poll_cb;
    dev->snd_cb_event.data = (void *)dev;
    dev->snd_cb_event.events = EPOLLIN;
    dev->snd_cb_event.fd = dev->kumsg_fd;

    if(alislevent_open(&dev->snd_event_handle))
    {
        SL_ERR("alislevent_open fail\n");
        return -1;
    }

    if(alislevent_add(dev->snd_event_handle, &dev->snd_cb_event))
    {
        SL_ERR("alislevent_add fail\n");
        alislevent_close(dev->snd_event_handle);
        return -1;
    }
    
    return 0;
}

static int sl_snd_del_poll_fd(struct sound_private *dev)
{
    alislevent_del(dev->snd_event_handle, &dev->snd_cb_event);
    memset(&dev->snd_cb_event, 0, sizeof(dev->snd_cb_event));
    alislevent_close(dev->snd_event_handle);
    dev->snd_event_handle = NULL;
    //printf("************** %s ***********\n", __FUNCTION__);
    close(dev->kumsg_fd);
    dev->kumsg_fd = -1;   
    return 0;
}

alisl_retcode alislsnd_reg_callback(alisl_handle hdl, audio_regcb_param_s *p_regcb_param, alislsnd_callback cb)
{
    struct sound_private *priv = (struct sound_private *)hdl;
    alisl_retcode ret = 0;
    struct audio_callback_register_param register_param;
    if((!p_regcb_param) || (!priv))
    {
        SL_ERR("invalid arg\n");
        return ERROR_INVAL;
    }
    memset(&register_param, 0, sizeof(struct audio_callback_register_param));
    switch(p_regcb_param->e_cb_type) {
        //deca
        case SL_DECA_MONITOR_NEW_FRAME:
            register_param.e_cb_type = DECA_MSG_CB_DECODE_NEW_FRAME;
            break;
        case SL_DECA_MONITOR_START:
            register_param.e_cb_type = DECA_MSG_CB_DECODE_START;
            break;
        case SL_DECA_MONITOR_STOP:
            register_param.e_cb_type = DECA_MSG_CB_DECODE_STOP;
            break;
        case SL_DECA_MONITOR_DECODE_ERR:
            register_param.e_cb_type = DECA_MSG_CB_DECODE_FAIL;
            break;
        case SL_DECA_MONITOR_OTHER_ERR:
            register_param.e_cb_type = DECA_MSG_CB_DECODE_DATA_INVALID;
            break;
        case SL_DECA_FIRST_FRAME:
            register_param.e_cb_type = SND_MSG_CB_FIRST_FRM_OUTPUT;
            break;
        case SL_DECA_STATE_CHANGED:
            register_param.e_cb_type = DECA_MSG_CB_DECODE_STATE_SWITCH;
            break;
        case SL_SND_MONITOR_OUTPUT_DATA_END:
            register_param.e_cb_type = SND_MSG_CB_OUTPUT_DATA_END;
            break;
        case SL_SND_MONITOR_MIX_DATA_END:
            register_param.e_cb_type = SND_MONITOR_SBM_MIX_END;
            break;
        default:
            return ERROR_FAILED;
    }
    register_param.monitor_rate = p_regcb_param->monitor_rate;
    register_param.reversed = p_regcb_param->reversed;
    register_param.timeout_threshold = p_regcb_param->timeout_threshold;
    pthread_mutex_lock(&m_mutex);
    if (priv->kumsg_fd == -1) {
        if (sl_snd_add_poll_fd(priv))
        {
            SL_ERR("sl_snd_add_poll_fd fail\n");
            pthread_mutex_unlock(&m_mutex);
            return ERROR_FAILED;
        }
    }
    SL_DBG("type: %d, cb: %p\n", p_regcb_param->e_cb_type, cb);
    if(cb)
    {
        if((p_regcb_param->e_cb_type < SL_SND_INDEX)&&(p_regcb_param->e_cb_type >= SL_DECA_INDEX))
        {
            priv->deca_cb = cb;
        }
        else if((p_regcb_param->e_cb_type < SL_DECA_CB_TYPE_MAX)&&(p_regcb_param->e_cb_type >= SL_SND_INDEX))
        {
            priv->snd_cb = cb;
        }
        if(ioctl(priv->fd, AUDIO_CB_RPC_REGISTER, &register_param) < 0) 
        {
            SL_ERR("AUDIO_CB_RPC_REGISTER fail!\n");
            ret = ERROR_IOCTL;
        }
    }
    else
    {       
        if (ioctl(priv->fd, AUDIO_CB_RPC_UNREGISTER, &register_param) < 0)
        {
            SL_ERR("AUDIO_CB_RPC_UNREGISTER fail!\n");
            ret = ERROR_IOCTL;
        }
    }
    pthread_mutex_unlock(&m_mutex);
    return ret;
}


static bool check_io_port(enum SndIoPort io_port)
{
    if(io_port > MAX_IO_PORT)
        return false;
    return true;
}

/**
 * @brief         sound device construct, malloc memory and init params
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
static alisl_retcode alislsnd_construct(void **handle)
{
    struct sound_private *priv ;

    priv = malloc(sizeof(*priv));
    if (priv == NULL)
    {
        SL_ERR("SND malloc memory failed!\n");
        return ERROR_NOMEM;
    }

    memset(priv, 0, sizeof(*priv));
    priv->fd = -1;
    priv->kumsg_fd = -1;
    bit_set(priv->status, SND_STATUS_CONSTRUCT);
    *handle = priv;
    return 0;
}


/**
 * @brief         sound device destruct, free memory, close device handle.
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
static int alislsnd_destruct(void **handle)
{
    free(*handle);
    *handle = NULL;

    return 0;
}

/**
*    @brief        snd open, just get ali_m36_audio0 handle priv->fd, then
*                  can use this handle, do not need to do other hardware ini,
*                  because it is done by deca module.
*    @author       ze.hong
*    @date         2013-6-19
*    @param[in]    handle is sound device pointer
*    @param[out]
*    @return       alisl_retcode
*    @note         none -> open|stop           
*/
alisl_retcode alislsnd_open(void **handle)
{
    struct sound_private *priv = NULL;
    
    if (NULL == handle) {
        SL_ERR("NULL SND private data structure!\n");
        return ERROR_INVAL;
    }
    pthread_mutex_lock(&m_mutex);
    if (snd_dev == NULL)
    {
        alislsnd_construct((void **)&priv);
        if(NULL == priv)
        {
            pthread_mutex_unlock(&m_mutex);
            return ERROR_NOMEM;
        }
        if ((priv->fd = open("/dev/ali_m36_audio0", O_RDWR)) < 0)
        {
            SL_ERR("SND device open failed!\n");
            alislsnd_destruct((void **)&priv);
            pthread_mutex_unlock(&m_mutex);
            return ERROR_OPEN;
        }

        bit_set(priv->status, SND_STATUS_OPEN | SND_STATUS_STOP);
        bit_clear(priv->status, SND_STATUS_CLOSE);
        snd_dev = priv;
    }
    snd_dev->open_cnt++;
    *handle = snd_dev;
    SL_DBG("%p snd_dev->fd:%d\n", snd_dev,snd_dev->fd);

    pthread_mutex_unlock(&m_mutex);
    return 0;
}



/**
 * @brief         sound device close, close snd handle priv->fd.
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_close(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;


    if (priv == NULL)
    {
        SL_ERR("NULL SND private data structure!\n");
        return ERROR_INVAL;
    }
    pthread_mutex_lock(&m_mutex);

    if(--priv->open_cnt)
    {
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }
    
    if(priv->kumsg_fd != -1)
    {   
        SL_DBG("sl_snd_del_poll_fd\n");
        sl_snd_del_poll_fd(priv);
        priv->snd_cb = NULL;
        priv->deca_cb = NULL;
    }
    
    close(priv->fd);
    priv->fd = -1;
    bit_clear(priv->status, SND_STATUS_STOP | SND_STATUS_OPEN);
    bit_set(priv->status, SND_STATUS_CLOSE);
    alislsnd_destruct((void **)&priv);
    snd_dev = NULL;
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

/**
 * @brief         set sound mute
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note          stop -> start
 *
 */
alisl_retcode alislsnd_start(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if(priv == NULL)
    {
        SL_ERR("NULL private data structure!\n");
        return ERROR_INVAL;
    }

    SL_DBG("snd_dev:%p snd_dev->fd:%d\n", snd_dev, snd_dev->fd);

    if(bit_test_any(priv->status, SND_STATUS_START)) {
        SL_DBG("Snd already start!\n");
        return 0;
    }
    
    if(!bit_test_any(priv->status, SND_STATUS_OPEN)) {
        SL_ERR("Snd not opened!\n");
        return ERROR_STATUS;
    }
    
    //if not in stop, don`t permit start
    if(!bit_test_any(priv->status, SND_STATUS_STOP)) {
        SL_ERR("Snd should stop first!\n");
        return ERROR_STATUS;
    }
    
    if (0 == ioctl(priv->fd, AUDIO_PLAY, 0))
    {
        bit_set(priv->status, SND_STATUS_START);
        bit_clear(priv->status, SND_STATUS_STOP);
        return 0;
    }
    else
    {
        SL_ERR("SND AUDIO_PLAY io control failed!\n");
        return ERROR_IOCTL;
    }

}


/**
 * @brief         set sound mute
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note          resume|pause|start -> stop
 *
 */
alisl_retcode alislsnd_stop(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if(priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(bit_test_any(priv->status, SND_STATUS_STOP)) {
        SL_DBG("Snd already stopped!\n");
        return 0;
    }

    if(!bit_test_any(priv->status, SND_STATUS_OPEN)) {
        SL_ERR("Snd should open first!\n");
        return ERROR_STATUS;
    }

    if (0 == ioctl(priv->fd, AUDIO_STOP, 0))
    {
        bit_set(priv->status, SND_STATUS_STOP);
        bit_clear(priv->status, SND_STATUS_START | SND_STATUS_PAUSE);
        return 0;
    }
    else
    {
        SL_ERR("SND AUDIO_STOP io control failed!\n");
        return ERROR_IOCTL;
    }

}

/**
 * @brief         set sound mute
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note          start -> pause
 *
 */
alisl_retcode alislsnd_pause(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if(priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(bit_test_any(priv->status, SND_STATUS_PAUSE)) {
        SL_DBG("Snd already pause!\n");
        return 0;
    }
    
    if(!bit_test_any(priv->status, SND_STATUS_START)) {
        SL_ERR("deca should start first!\n");
        return ERROR_STATUS;
    }
    
    if(0 == ioctl(priv->fd, AUDIO_PAUSE, 0))
    {
        bit_set(priv->status, SND_STATUS_PAUSE);
        //we should clear start state
        bit_clear(priv->status, SND_STATUS_START);
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }

}

/**
 * @brief         init ASE
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_init_ase(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    if(0==ioctl(priv->fd, AUDIO_ASE_INIT, 0))
    {
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }
}

/**
 * @brief         resume after pause
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note          pause -> resume
 *
 */
alisl_retcode alislsnd_resume(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if(priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    if(bit_test_any(priv->status, SND_STATUS_START)) {
        SL_DBG("Snd already play!\n");
        return 0;
    }
    if(!bit_test_any(priv->status, SND_STATUS_PAUSE)) {
        SL_ERR("deca should pause first!\n");
        return ERROR_STATUS;
    }
    
    if(0 == ioctl(priv->fd, AUDIO_PLAY, 0))
    {
        //should set start status
        bit_set(priv->status, SND_STATUS_START);
        //should clear pause
        bit_clear(priv->status, SND_STATUS_PAUSE);
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }
}

static enum audio_stream_type sl_to_drv_type(enum Snd_decoder_type decoder_type) {
    enum audio_stream_type audio_type = AUDIO_INVALID;
    switch (decoder_type){
        case SND_TYPE_MPEG1:  
            audio_type = AUDIO_MPEG1;
            break;
        case SND_TYPE_MPEG2:
            audio_type = AUDIO_MPEG2;
            break;
        case SND_TYPE_MPEG_AAC:
            audio_type = AUDIO_MPEG_AAC;
            break;
        case SND_TYPE_AC3:  
            audio_type = AUDIO_AC3;
            break;
        case SND_TYPE_DTS: 
            audio_type = AUDIO_DTS;
            break;
        case SND_TYPE_PPCM: 
            audio_type = AUDIO_PPCM;
            break;
        case SND_TYPE_LPCM_V:
            audio_type = AUDIO_LPCM_V;
            break;
        case SND_TYPE_LPCM_A:
            audio_type = AUDIO_LPCM_A;
            break;
        case SND_TYPE_PCM: 
            audio_type = AUDIO_PCM;
            break;
        case SND_TYPE_WMA:
            audio_type = AUDIO_BYE1;
            break;
        case SND_TYPE_RA8:
            audio_type = AUDIO_RA8;
            break;
        case SND_TYPE_MP3: 
            audio_type = AUDIO_MP3_2;
            break;
        case SND_TYPE_MPEG_ADTS_AAC:
            audio_type = AUDIO_MPEG_ADTS_AAC;
            break;
        case SND_TYPE_OGG:
            audio_type = AUDIO_OGG;
            break;
        case SND_TYPE_EC3:
            audio_type = AUDIO_EC3;
            break;
        default:
            break;     
    }
    return audio_type;
}
/**
 * @brief         set audio decoder type
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_decoder_type(void *handle,enum Snd_decoder_type decoder_type)
{
    struct sound_private *priv = (struct sound_private *)handle;
    enum audio_stream_type audio_type = AUDIO_INVALID;
    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    audio_type= sl_to_drv_type(decoder_type);

    if (0 == ioctl(priv->fd, AUDIO_SET_STREAMTYPE, audio_type))
    {
        priv->decode_type = decoder_type;
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }
}

/**
 * @brief         get audio running status
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_status(void *handle,enum snd_status *status)
{
    struct sound_private *priv = (struct sound_private *)handle;
    struct ali_audio_ioctl_command io_param;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    io_param.ioctl_cmd = DECA_GET_DECA_STATE;
    io_param.param = (unsigned int)status;
    if    (0==ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param))
    {
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }


}


/**
 * @brief         get audio decoder type
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]    decoder_type
 * @return        decoder_type
 * @note
 *
 */
alisl_retcode alislsnd_get_decoder_type(void *handle,enum Snd_decoder_type *decoder_type)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    *decoder_type = priv->decode_type;
    return 0;
}

/**
 * @brief         set sound mute
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[in]     mute is mute or unmute flag,
 * @param[in]     io_port is to control witch io port,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_mute(void *handle, bool mute,enum SndIoPort io_port)
{
    struct sound_private *priv = (struct sound_private *)handle;
    struct ali_audio_ioctl_command io_param;

    int ret = -1;
    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }


    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }
    switch(io_port)
    {
        case SND_IO_ALL:
            ret = ioctl(priv->fd, AUDIO_SET_MUTE, mute);
            priv->mute_state[SND_IO_ALL] = mute;
            priv->mute_state[SND_IO_RCA] = mute;
            priv->mute_state[SND_IO_HDMI] = mute;
            priv->mute_state[SND_IO_SPDIF] = mute;
            break;
        case SND_IO_RCA:
            io_param.ioctl_cmd = SND_I2S_OUT;
            io_param.param = mute;
            ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
            priv->mute_state[io_port] = mute;
            break;
        case SND_IO_HDMI:
            io_param.ioctl_cmd = SND_HDMI_OUT;
            io_param.param = mute;
            ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
            priv->mute_state[io_port] = mute;
            break;
        case SND_IO_SPDIF:
            io_param.ioctl_cmd = SND_SPDIF_OUT;
            io_param.param = mute;
            ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
            priv->mute_state[io_port] = mute;
            break;
        default:
            break;
    }
    if (0 == ret)
        return 0;
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }

}

/**
 * @brief         get mute state
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[in]     mute is mute or unmute flag,
 * @param[in]     io_port is to control witch io port,
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_mute_state(void *handle,enum SndIoPort io_port,bool *mute_state)
{
    struct sound_private *priv = (struct sound_private *)handle;
    int ret = -1;
    
    if ((priv == NULL) || (NULL == mute_state))
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }

    if (!bit_test_any(priv->status, SND_STATUS_OPEN))
    {
        SL_ERR("SND not open!\n");
        return ERROR_STATUS;
    }
    if (io_port != SND_IO_ALL) {
        //Can`t get separate i2s/spdif/hdmi mute status from driver, so use local status.
        *mute_state = priv->mute_state[io_port];
    } else {
        //if mute return 0, if un_mute return 1, return -1 if ioctl fail.
        ret = alislsnd_ioctl(handle, SND_IS_SND_MUTE, 0);
        //printf("%s -> ret: %d\n", __func__, ret);
        if (ret > 0) {
            *mute_state = 0;
        } else if (ret == 0) {
            *mute_state = 1;
        } else {
            return -1;
        }
    }
    return 0;
}


/**
 * @brief         set sound device volume
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer, volume is value to be set
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_volume(void *handle, uint8_t volume,enum SndIoPort io_port)
{

    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }
    SL_DBG("%s snd_dev:%p snd_dev->fd:%d\n",__FUNCTION__,snd_dev,snd_dev->fd);
    if (0 == ioctl(priv->fd, AUDIO_SET_VOLUME, volume))
    {
        priv->volume[io_port] = volume;
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }

}


/**
 * @brief         get  volume
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[in]     io_port
 * @param[out]    volume
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_volume(void *handle,enum SndIoPort io_port,
                                            uint8_t *volume)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }
    //printf("SND io control to get volume \n");
    if (0 == ioctl(priv->fd, AUDIO_GET_VOLUME, volume))
    {
        //printf("ioctl get volume : %d\n", (int)(*volume));
        //priv->volume[io_port] = volume;
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }
    //*volume = priv->volume[io_port];
    return 0;
}

/**
 * @brief         get  underrun times
 * @date          2017-1-9
 * @param[in]     handle is sound device pointer
 * @param[in]     io_port
 * @param[out]    underrun_times
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_underrun_times(void *handle,enum SndIoPort io_port,
                                            uint8_t *underrun_times)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if((priv == NULL) || (underrun_times == NULL)){
        SL_DBG("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port)){
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }
    
    if (0 == ioctl(priv->fd, AUDIO_GET_UNDERRUN_TIMES, underrun_times)){
        return 0;
    } 
    else{
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }
    return 0;
}


/**
 * @brief         set sound channel to duplicate
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     channel indicates which channel be selected
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_trackmode(void *handle, enum SndTrackMode trackmode,
                                                enum SndIoPort io_port)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }

    if(0==ioctl(priv->fd, AUDIO_CHANNEL_SELECT, trackmode))
    {
        priv->trackmode[io_port] = trackmode;
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }
}

/**
 * @brief         set sound channel to duplicate
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     channel indicates which channel be selected
 * @param[in]     io_port
 * @param[out]    trackmode
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_trackmode(void *handle,enum SndIoPort io_port,
                                     enum SndTrackMode *trackmode)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }

    *trackmode = priv->trackmode[io_port];
    return 0;
}

/**
 * @brief         set data output format, select pcm or undecoder audio output
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     type is spdif type, pcm or undecoder audio
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_output_format(void *handle, enum SndOutFormat output_format,
                                                    enum SndIoPort io_port)
{
    struct sound_private *priv = (struct sound_private *)handle;
    struct ali_audio_ioctl_command io_param;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }
    io_param.ioctl_cmd = FORCE_SPDIF_TYPE;
    io_param.param = output_format;
    if(0 == ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param))
    {
        priv->OutputFormat[io_port] = output_format;

        if ((SND_OUT_FORMAT_BS == output_format) || (SND_OUT_FORMAT_FORCE_DD == output_format)){
            io_param.ioctl_cmd = DECA_EMPTY_BS_SET;
            io_param.param = 0;
            if (ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param) != 0) {
                SL_ERR("empty BS set error.\n");
                return ERROR_IOCTL;
            }

            io_param.ioctl_cmd = DECA_ADD_BS_SET;
            io_param.param = AUDIO_AC3;
            if (ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param) != 0) {
                SL_ERR("add AC3 to BS set error.\n");
                return ERROR_IOCTL;
            }

            io_param.param = AUDIO_EC3;
            if (ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param) != 0) {
                SL_ERR("add EC3 to BS set error.\n");
                return ERROR_IOCTL;
            }

            io_param.param = AUDIO_DTS;
            if (ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param) != 0) {
                SL_ERR("add DTS to BS set error.\n");
                return ERROR_IOCTL;
            }
        }

        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }


}

/**
 * @brief         get data output format, select pcm or undecoder audio output
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     type is spdif type, pcm or undecoder audio
 * @param[in]     io_port
 * @param[out]    output_format
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_output_format(void *handle,enum SndIoPort io_port,
                                         enum SndOutFormat *output_format)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(false == check_io_port(io_port))
    {
        SL_ERR("IO port is not right\n");
        return ERROR_INVAL;
    }

    *output_format = priv->OutputFormat[io_port];
    return 0;
}

/**
 * @brief         set audio sync mode with video
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[in]    sync_mode
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_av_sync_mode(void *handle, enum SndSyncMode sync_mode)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if (0 == ioctl(priv->fd, AUDIO_SET_AV_SYNC, sync_mode))
    {
        priv->sync_mode = sync_mode;
        return 0;
    }
    else
    {
        SL_ERR("SND io control failed!\n");
        return ERROR_IOCTL;
    }

}

/**
 * @brief         set audio sync mode with video
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer,
 * @param[out]    sync_mode
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_get_av_sync_mode(void *handle, enum SndSyncMode *sync_mode)
{
    struct sound_private *priv = (struct sound_private *)handle;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    *sync_mode = priv->sync_mode;
    return 0;
}

/**
 * @brief         snd decore io control
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cmd is io control command from uplayer
 * @param[in]    param2 param2_description
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_decore_ioctl(void *handle, enum SndIoCmd cmd, void *param1, void *param2)
{
    struct sound_private *priv = (struct sound_private *)handle;
    struct ali_audio_rpc_pars rpc_pars;
    int io_cmd = -1;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));

    switch(cmd)
    {
        case SND_DECORE_INIT:
            io_cmd = DECA_DECORE_INIT;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = sizeof(struct audio_config);
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 4;
            break;
        case SND_DECORE_RLS:
            io_cmd = DECA_DECORE_RLS;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 1;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 0;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        case SND_DECORE_FLUSH:
            io_cmd = DECA_DECORE_FLUSH;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 1;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 0;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        case SND_DECORE_SET_BASE_TIME:
            io_cmd = DECA_DECORE_SET_BASE_TIME;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
            break;
        case SND_DECORE_SET_QUICK_MODE:
            io_cmd = DECA_DECORE_SET_QUICK_MODE;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        case SND_DECORE_PAUSE_DECODE:
            io_cmd = DECA_DECORE_PAUSE_DECODE;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 4;
            break;
        case SND_DECORE_GET_PCM_TRD:
            io_cmd = DECA_DECORE_GET_PCM_TRD;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 3;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = 4;
            rpc_pars.arg[1].out = 1;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 4;
            rpc_pars.arg[2].out = 1;
            break;
        case SND_DECORE_GET_STATUS:
            io_cmd = DECA_DECORE_GET_STATUS;
            rpc_pars.API_ID = RPC_AUDIO_DECORE_IOCTL;
            rpc_pars.arg_num = 2;
            rpc_pars.arg[0].arg = (void *)&io_cmd;
            rpc_pars.arg[0].arg_size = sizeof(io_cmd);
            rpc_pars.arg[1].arg = (void *)param1;
            rpc_pars.arg[1].arg_size = sizeof(struct audio_decore_status);;
            rpc_pars.arg[1].out = 1;
            rpc_pars.arg[2].arg = (void *)param2;
            rpc_pars.arg[2].arg_size = 0;
            break;
        default:
            return ERROR_NOPRIV;
    }

    return ioctl(priv->fd, AUDIO_DECORE_COMMAND, &rpc_pars);
}

static int alislsnd_enable_rawdata_output(struct sound_private *priv)
{
    struct ali_audio_ioctl_command io_param;

    if(NULL == priv)
    {
        SL_ERR("%s -> NULL parameter\n", __FUNCTION__);
        return -1;
    }
    memset(&io_param, 0, sizeof(io_param));
    io_param.ioctl_cmd = DECA_ADD_BS_SET;
    io_param.param = AUDIO_AC3;
    if (ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param) != 0) {
        SL_ERR("%s -> add AC3 to BS set error.\n", __FUNCTION__);
        return -1;
    }

    io_param.param = AUDIO_EC3;
    if (ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param) != 0) {
        SL_ERR("%s -> add EC3 to BS set error.\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

static unsigned long alislsnd_convert_deca_io_command(enum SndIoCmd cmd)
{
    unsigned long io_command = -1;
    switch (cmd){
        case SND_DECA_EMPTY_BS_SET:
            io_command = DECA_EMPTY_BS_SET;
            break;
        case SND_DECA_ADD_BS_SET:
            io_command = DECA_ADD_BS_SET;
            break;
        case SND_DECA_DEL_BS_SET:
            io_command = DECA_DEL_BS_SET;
            break;
        case SND_DECA_IS_BS_MEMBER:
            io_command = DECA_IS_BS_MEMBER;
            break;
        case SND_SET_PASS_CI:
            io_command = SET_PASS_CI;
            break;
        case SND_DECA_SET_DOLBY_ONOFF:
            io_command = DECA_SET_DOLBY_ONOFF;
            break;
        case SND_DECA_HDD_PLAYBACK:
            io_command = DECA_HDD_PLAYBACK;
            break;
        case SND_DECA_SET_PLAY_SPEED:
            io_command = DECA_SET_PLAY_SPEED;
            break;
        case SND_DECA_SET_DECODER_COUNT:
            io_command = DECA_SET_DECODER_COUNT;
            break;
        case SND_DECA_SET_AC3_MODE:
            io_command = DECA_SET_AC3_MODE;
            break;
        case SND_DECA_DOLBYPLUS_CONVERT_ONOFF:
            io_command = DECA_DOLBYPLUS_CONVERT_ONOFF;
            break;
        case SND_DECA_SYNC_BY_SOFT:
            io_command = DECA_SYNC_BY_SOFT;
            break;
        case SND_DECA_SYNC_NEXT_HEADER:
            io_command = DECA_SYNC_NEXT_HEADER;
            break;
        case SND_DECA_SOFTDEC_JUMP_TIME2:
            io_command = DECA_SOFTDEC_JUMP_TIME2;
            break;
        case SND_DECA_SOFTDEC_IS_PLAY_END2:
            io_command = DECA_SOFTDEC_IS_PLAY_END2;
            break;
        case SND_DECA_INDEPENDENT_DESC_ENABLE:
            io_command = DECA_INDEPENDENT_DESC_ENABLE;
            break;
        case SND_DECA_RESET_BS_BUFF:
            io_command = DECA_RESET_BS_BUFF;
            break;
        case SND_DECA_DOLBYPLUS_DEMO_ONOFF:
            io_command = DECA_DOLBYPLUS_DEMO_ONOFF;
            break;
        case SND_DECA_SET_BUF_MODE:
            io_command = DECA_SET_BUF_MODE;
            break;
        case SND_DECA_DO_DDP_CERTIFICATION:
            io_command = DECA_DO_DDP_CERTIFICATION;
            break;
        case SND_DECA_DYNAMIC_SND_DELAY:
            io_command = DECA_DYNAMIC_SND_DELAY;
            break;
        case SND_DECA_SET_AC3_COMP_MODE:
            io_command = DECA_SET_AC3_COMP_MODE;
            break;
        case SND_DECA_SET_AC3_STEREO_MODE:
            io_command = DECA_SET_AC3_STEREO_MODE;
            break;
        case SND_DECA_CONFIG_BS_BUFFER:
            io_command = DECA_CONFIG_BS_BUFFER;
            break;
        case SND_DECA_CONFIG_BS_LENGTH:
            io_command = DECA_CONFIG_BS_LENGTH;
            break;
        case SND_DECA_BS_BUFFER_RESUME:
            io_command = DECA_BS_BUFFER_RESUME;
            break;
        case SND_DECA_DOLBY_SET_VOLUME_DB:
            io_command = DECA_DOLBY_SET_VOLUME_DB;
            break;
        case SND_DECA_GET_STR_TYPE:
            io_command = DECA_GET_STR_TYPE;
            break;
        case SND_DECA_GET_HIGHEST_PTS:
            io_command = DECA_GET_HIGHEST_PTS;
            break;
        case SND_DECA_GET_AC3_BSMOD:
            io_command = DECA_GET_AC3_BSMOD;
            break;
        case SND_DECA_CHECK_DECODER_COUNT:
            io_command = DECA_CHECK_DECODER_COUNT;
            break;
        case SND_DECA_GET_DESC_STATUS:
            io_command = DECA_GET_DESC_STATUS;
            break;
        case SND_DECA_GET_DECODER_HANDLE:
            io_command = DECA_GET_DECODER_HANDLE;
            break;
        case SND_DECA_GET_DECA_STATE:
            io_command = DECA_GET_DECA_STATE;
            break;
        case SND_DECA_SOFTDEC_GET_ELAPSE_TIME2:
            io_command = DECA_SOFTDEC_GET_ELAPSE_TIME2;
            break;
        case SND_DECA_DOLBYPLUS_CONVERT_STATUS:
            io_command = DECA_DOLBYPLUS_CONVERT_STATUS;
            break;
        case SND_DECA_GET_BS_FRAME_LEN:
            io_command = DECA_GET_BS_FRAME_LEN;
            break;
        case SND_DECA_GET_DDP_INMOD:
            io_command = DECA_GET_DDP_INMOD;
            break;
        case SND_DECA_GET_AUDIO_INFO:
            io_command = DECA_GET_AUDIO_INFO;
            break;
        case SND_DECA_SET_REVERB:
            io_command = DECA_SET_REVERB;
            break;
        case SND_DECA_SET_PL_II:
            io_command = DECA_SET_PL_II;
            break;
        case SND_DECA_SOFTDEC_INIT:
            io_command = DECA_SOFTDEC_INIT;
            break;
        case SND_DECA_SOFTDEC_CLOSE:
            io_command = DECA_SOFTDEC_CLOSE;
            break;
        case SND_DECA_SOFTDEC_SET_TIME:
            io_command = DECA_SOFTDEC_SET_TIME;
            break;
        case SND_DECA_SOFTDEC_JUMP_TIME:
            io_command = DECA_SOFTDEC_JUMP_TIME;
            break;
        case SND_DECA_SOFTDEC_IS_PLAY_END:
            io_command = DECA_SOFTDEC_IS_PLAY_END;
            break;
        case SND_DECA_SOFTDEC_INIT2:
            io_command = DECA_SOFTDEC_INIT2;
            break;
        case SND_DECA_SOFTDEC_CLOSE2:
            io_command = DECA_SOFTDEC_CLOSE2;
            break;
        case SND_DECA_SOFTDEC_CAN_DECODE2:
            io_command = DECA_SOFTDEC_CAN_DECODE2;
            break;
        case SND_DECA_GET_PLAY_PARAM:
            io_command = DECA_GET_PLAY_PARAM;
            break;
        case SND_DECA_GET_ES_BUFF_STATE:
            io_command = DECA_GET_ES_BUFF_STATE;
            break;
        case SND_DECA_CHANGE_AUD_TRACK:
            io_command = DECA_CHANGE_AUD_TRACK;
            break;
        default:
            break;
    }
    return io_command;
}

static unsigned long alislsnd_convert_snd_io_command(enum SndIoCmd cmd)
{
    unsigned long io_command = -1;
    switch (cmd) {
        case SND_IS_SND_RUNNING:
            io_command = IS_SND_RUNNING;
            break;
        case SND_IS_SND_MUTE:
            io_command = IS_SND_MUTE;
            break;
        case SND_SND_CC_MUTE:
            io_command = SND_CC_MUTE;
            break;
        case SND_SND_DAC_MUTE:
            io_command = SND_DAC_MUTE;
            break;
        case SND_SND_CC_MUTE_RESUME:
            io_command = SND_CC_MUTE_RESUME;
            break;
        case SND_SND_SPO_ONOFF:
            io_command = SND_SPO_ONOFF;
            break;
        case SND_SND_SET_FADE_SPEED:
            io_command = SND_SET_FADE_SPEED;
            break;
        case SND_IS_PCM_EMPTY:
            io_command = IS_PCM_EMPTY;
            break;
        case SND_SND_BYPASS_VCR:
            io_command = SND_BYPASS_VCR;
            break;
        case SND_SND_CHK_SPDIF_TYPE:
            io_command = SND_CHK_SPDIF_TYPE;
            break;
        case SND_SND_PAUSE_MUTE:
            io_command = SND_PAUSE_MUTE;
            break;
        case SND_SND_SET_DESC_VOLUME_OFFSET:
            io_command = SND_SET_DESC_VOLUME_OFFSET;
            break;
        case SND_SND_SET_BS_OUTPUT_SRC:
            io_command = SND_SET_BS_OUTPUT_SRC;
            break;
        case SND_SND_SECOND_DECA_ENABLE:
            io_command = SND_SECOND_DECA_ENABLE;
            break;
        case SND_SND_DO_DDP_CERTIFICATION:
            io_command = SND_DO_DDP_CERTIFICATION;
            break;
        case SND_SND_CHK_DAC_PREC:
            io_command = SND_CHK_DAC_PREC;
            break;
        case SND_SND_POST_PROCESS_0:
            io_command = SND_POST_PROCESS_0;
            break;
        case SND_SND_SPECIAL_MUTE_REG:
            io_command = SND_SPECIAL_MUTE_REG;
            break;
        case SND_SND_SET_SYNC_DELAY:
            io_command = SND_SET_SYNC_DELAY;
            break;
        case SND_SND_SET_SYNC_LEVEL:
            io_command = SND_SET_SYNC_LEVEL;
            break;
        case SND_SND_SET_MUTE_TH:
            io_command = SND_SET_MUTE_TH;
            break;
        case SND_SND_AUTO_RESUME:
            io_command = SND_AUTO_RESUME;
            break;
        case SND_SND_REQ_REM_DATA:
            io_command = SND_REQ_REM_DATA;
            break;
        case SND_SND_GET_TONE_STATUS:
            io_command = SND_GET_TONE_STATUS;
            break;
        case SND_SND_CHK_PCM_BUF_DEPTH:
            io_command = SND_CHK_PCM_BUF_DEPTH;
            break;
        case SND_SND_GET_SAMPLES_REMAIN:
            io_command = SND_GET_SAMPLES_REMAIN;
            break;
        case SND_SND_REQ_REM_PCM_DATA:
            io_command = SND_REQ_REM_PCM_DATA;
            break;
        case SND_SND_REQ_REM_PCM_DURA:
            io_command = SND_REQ_REM_PCM_DURA;
            break;
        case SND_SND_GET_MUTE_TH:
            io_command = SND_GET_MUTE_TH;
            break;
        case SND_SND_SPECTRUM_START:
            io_command = SND_SPECTRUM_START;
            break;
        case SND_SND_SPECTRUM_STEP_TABLE:
            io_command = SND_SPECTRUM_STEP_TABLE;
            break;
        case SND_SND_SPECTRUM_STOP:
            io_command = SND_SPECTRUM_STOP;
            break;
        case SND_SND_SPECTRUM_CLEAR:
            io_command = SND_SPECTRUM_CLEAR;
            break;
        case SND_SND_SPECTRUM_VOL_INDEPEND:
            io_command = SND_SPECTRUM_VOL_INDEPEND;
            break;
        case SND_SND_SPECTRUM_CAL_COUNTER:
            io_command = SND_SPECTRUM_CAL_COUNTER;
            break;
        case SND_SND_SET_SPDIF_SCMS:
            io_command = SND_SET_SPDIF_SCMS;
            break;
        case SND_SND_SET_SYNC_PARAM:
            io_command = SND_SET_SYNC_PARAM;
            break;
        case SND_SND_IO_PAUSE_SND:
            io_command = SND_IO_PAUSE_SND;
            break;
        case SND_SND_IO_RESUME_SND:
            io_command = SND_IO_RESUME_SND;
            break;
        case SND_SND_IO_DDP_SPO_INTF_CFG:
            io_command = SND_IO_DDP_SPO_INTF_CFG;
            break;
        case SND_SND_IO_SPO_INTF_CFG:
            io_command = SND_IO_SPO_INTF_CFG;
            break;
        case SND_SND_ONLY_SET_SPDIF_DELAY_TIME:
            io_command = SND_ONLY_SET_SPDIF_DELAY_TIME;
            break;
        case SND_SND_ONLY_GET_SPDIF_DELAY_TIME:
            io_command = SND_ONLY_GET_SPDIF_DELAY_TIME;
            break;
        case SND_SND_IO_GET_PLAY_PTS:
            io_command = SND_IO_GET_PLAY_PTS;
            break;
        default:
            break;
    }
    return io_command;
}

static void alislsnd_get_deca_state(unsigned long *state)
{
    switch (*state) {
        case DECA_STATE_DETACH:
            *state = DECA_STATUS_DETACH;
            break;
        case DECA_STATE_ATTACH:
            *state = DECA_STATUS_ATTACH;
            break;
        case DECA_STATE_IDLE:
            *state = DECA_STATUS_IDLE;
            break;
        case DECA_STATE_PLAY:
            *state = DECA_STATUS_PLAY;
            break;
        case DECA_STATE_PAUSE:
            *state = DECA_STATUS_PAUSE;
            break;    
        default:
            *state = DECA_STATUS_DETACH;
            break;
    }
    return;
}

static void alislsnd_get_deca_status(struct Snd_stream_info * info, struct cur_stream_info *info_ori)
{
    info->bit_depth = info_ori->bit_depth;
    info->bit_depth = info_ori->bit_depth;
    info->sample_rate = info_ori->sample_rate;
    info->samp_num = info_ori->samp_num;
    info->chan_num = info_ori->chan_num;
    info->frm_cnt = info_ori->frm_cnt;
    info->reserved1 = info_ori->reserved1;
    info->reserved2 = info_ori->reserved2;
    info->input_ts_cnt = info_ori->input_ts_cnt;
    info->sync_error_cnt = info_ori->sync_error_cnt;
    info->sync_success_cnt = info_ori->sync_success_cnt;
    info->sync_frm_len = info_ori->sync_frm_len;
    info->decode_error_cnt = info_ori->decode_error_cnt;
    info->decode_success_cnt = info_ori->decode_success_cnt;
    info->cur_frm_pts = info_ori->cur_frm_pts;
    switch(info_ori->str_type) {
        case AUDIO_MPEG1:
            info->str_type = SND_TYPE_MPEG1;
            break;
        case AUDIO_MPEG2:
            info->str_type = SND_TYPE_MPEG2;
            break;    
        case AUDIO_MPEG_AAC:
            info->str_type = SND_TYPE_MPEG_AAC;
            break;
        case AUDIO_AC3:
            info->str_type = SND_TYPE_AC3;
            break;
        case AUDIO_DTS:
            info->str_type = SND_TYPE_DTS;
            break;
        case AUDIO_PPCM:
            info->str_type = SND_TYPE_PPCM;
            break;
        case AUDIO_LPCM_V:
            info->str_type = SND_TYPE_LPCM_V;
            break;
        case AUDIO_LPCM_A:
            info->str_type = SND_TYPE_LPCM_A;
            break;
        case AUDIO_PCM:
            info->str_type = SND_TYPE_PCM;
            break;
        case AUDIO_BYE1:
            info->str_type = SND_TYPE_WMA;
            break;
        case AUDIO_RA8:
            info->str_type = SND_TYPE_RA8;
            break;
        case AUDIO_MP3_2:
            info->str_type = SND_TYPE_MP3;
            break;
        case AUDIO_MPEG_ADTS_AAC:
            info->str_type = SND_TYPE_MPEG_ADTS_AAC;
            break;
        case AUDIO_OGG:
            info->str_type = SND_TYPE_OGG;
            break;
        case AUDIO_EC3:
            info->str_type = SND_TYPE_EC3;
            break;
        default:
            info->str_type = SND_TYPE_INVALID;
            break;
    }
    return;
}

static void alislsnd_get_deca_es_buffer_status(struct snd_deca_buf_info *info,
    struct deca_buf_info *info_ori)
{
    info->buf_base_addr = info_ori->buf_base_addr;
    info->buf_len = info_ori->buf_len;   
    info->used_len = info_ori->used_len;  
    info->remain_len = info_ori->remain_len;
    info->cb_rd = info_ori->cb_rd;     
    info->cb_wt = info_ori->cb_wt;     
    info->es_rd = info_ori->es_rd;     
    info->es_wt = info_ori->es_wt;
    return;
}

/**
 * @brief         snd  io control
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     cmd is io control command from uplayer
 * @param[in]    param2 param2_description
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_ioctl(void *handle, enum SndIoCmd cmd, unsigned int param)
{
    struct sound_private *priv = (struct sound_private *)handle;
    struct ali_audio_rpc_pars rpc_pars;
    int ret = -1;
    struct ali_audio_ioctl_command io_param;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    if(cmd < SND_DECORE_INIT)
    {
        switch(cmd)
        {
            case SND_DECA_BEEP_INTERVAL:
                ret = ioctl(priv->fd,AUDIO_ASE_DECA_BEEP_INTERVAL,param);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @SND_DECA_BEEP_INTERVAL fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            case SND_DECA_STR_STOP:
            {
                ret = ioctl(priv->fd,AUDIO_ASE_STR_STOP,param);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @SND_DECA_STR_STOP fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            }
            case SND_DECA_STR_PLAY:
            {
                memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
                rpc_pars.arg_num = 1;
                rpc_pars.arg[0].arg = (void *)param;
                rpc_pars.arg[0].arg_size = sizeof(struct ase_str_play_param);
                ret = ioctl(priv->fd,AUDIO_ASE_STR_PLAY,&rpc_pars);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_ASE_STR_PLAY fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            }         
            case SND_DECA_SET_STR_TYPE:
                ret=ioctl(priv->fd, AUDIO_SET_STREAMTYPE, param);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_SET_STREAMTYPE fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            case SND_DECA_GET_DECA_STATE:
                io_param.ioctl_cmd = DECA_GET_DECA_STATE;
                io_param.param = param;
                ret = ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param);
                alislsnd_get_deca_state((unsigned long *)param);
                break;
            case SND_DECA_GET_PLAY_PARAM: {
                    struct cur_stream_info deca_play_info;
                    memset(&deca_play_info, 0, sizeof(struct cur_stream_info));
                    io_param.ioctl_cmd = DECA_GET_PLAY_PARAM;
                    io_param.param = (unsigned long)&deca_play_info;
                    ret = ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param);
                    alislsnd_get_deca_status((struct Snd_stream_info *)param, &deca_play_info);
                }
                break;
            case SND_DECA_GET_ES_BUFF_STATE: {
                    struct deca_buf_info deca_es_buff_info;
                    memset(&deca_es_buff_info, 0, sizeof(struct deca_buf_info));
                    io_param.ioctl_cmd = DECA_GET_ES_BUFF_STATE;
                    io_param.param = (unsigned long)&deca_es_buff_info;
                    ret = ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param);
                    alislsnd_get_deca_es_buffer_status((struct snd_deca_buf_info *)param, &deca_es_buff_info);
                }
                break;
            case SND_DECA_GET_AUDIO_INFO: {
                    struct AUDIO_INFO audio_info;
                    memset(&audio_info, 0, sizeof(audio_info));
                    io_param.ioctl_cmd = DECA_GET_AUDIO_INFO;
                    io_param.param = (unsigned long)&audio_info;
                    ret = ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param);      
                    *(unsigned int *)param = audio_info.mode;                    
                }
                break;
            case SND_DECA_ADD_BS_SET: {
                enum audio_stream_type audio_type = sl_to_drv_type((enum Snd_decoder_type)param);
                io_param.ioctl_cmd = DECA_ADD_BS_SET;
                io_param.param = audio_type;
                ret = ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param);
                break;
            }
            case SND_DECA_EMPTY_BS_SET:
            case SND_DECA_DEL_BS_SET:
            case SND_DECA_IS_BS_MEMBER:
            case SND_SET_PASS_CI:
            case SND_DECA_SET_DOLBY_ONOFF:
            case SND_DECA_HDD_PLAYBACK:
            case SND_DECA_SET_PLAY_SPEED:
            case SND_DECA_SET_DECODER_COUNT:
            case SND_DECA_SET_AC3_MODE:
            case SND_DECA_DOLBYPLUS_CONVERT_ONOFF:
            case SND_DECA_SYNC_BY_SOFT:
            case SND_DECA_SYNC_NEXT_HEADER:
            case SND_DECA_SOFTDEC_JUMP_TIME2:
            case SND_DECA_SOFTDEC_IS_PLAY_END2:
            case SND_DECA_INDEPENDENT_DESC_ENABLE:
            case SND_DECA_RESET_BS_BUFF:
            case SND_DECA_DOLBYPLUS_DEMO_ONOFF:
            case SND_DECA_SET_BUF_MODE:
            case SND_DECA_DO_DDP_CERTIFICATION:
            case SND_DECA_DYNAMIC_SND_DELAY:
            case SND_DECA_SET_AC3_COMP_MODE:
            case SND_DECA_SET_AC3_STEREO_MODE:
            case SND_DECA_CONFIG_BS_BUFFER:
            case SND_DECA_CONFIG_BS_LENGTH:
            case SND_DECA_BS_BUFFER_RESUME:
            case SND_DECA_DOLBY_SET_VOLUME_DB:
            case SND_DECA_GET_STR_TYPE:
            case SND_DECA_GET_HIGHEST_PTS:
            case SND_DECA_GET_AC3_BSMOD:
            case SND_DECA_CHECK_DECODER_COUNT:
            case SND_DECA_GET_DESC_STATUS:
            case SND_DECA_GET_DECODER_HANDLE:        
            case SND_DECA_SOFTDEC_GET_ELAPSE_TIME2:
            case SND_DECA_DOLBYPLUS_CONVERT_STATUS:
            case SND_DECA_GET_BS_FRAME_LEN:
            case SND_DECA_GET_DDP_INMOD:            
            case SND_DECA_SET_REVERB:
            case SND_DECA_SET_PL_II:
            case SND_DECA_SOFTDEC_INIT:
            case SND_DECA_SOFTDEC_CLOSE:
            case SND_DECA_SOFTDEC_SET_TIME:
            case SND_DECA_SOFTDEC_JUMP_TIME:
            case SND_DECA_SOFTDEC_IS_PLAY_END:
            case SND_DECA_SOFTDEC_INIT2:
            case SND_DECA_SOFTDEC_CLOSE2:
            case SND_DECA_SOFTDEC_CAN_DECODE2:        
            case SND_DECA_CHANGE_AUD_TRACK:
                io_param.ioctl_cmd = alislsnd_convert_deca_io_command(cmd);
                io_param.param = param;
                ret = ioctl(priv->fd, AUDIO_DECA_IO_COMMAND, &io_param);
                break;
            default:
                break;
        }
    }
    else if (cmd < SND_SND_START)
    {
        switch(cmd)
        {
            case SND_IS_SND_RUNNING:
            case SND_IS_SND_MUTE:
            case SND_SND_CC_MUTE:
            case SND_SND_DAC_MUTE:
            case SND_SND_CC_MUTE_RESUME:
            case SND_SND_SPO_ONOFF:
            case SND_SND_SET_FADE_SPEED:
            case SND_IS_PCM_EMPTY:
            case SND_SND_BYPASS_VCR:
            case SND_SND_CHK_SPDIF_TYPE:
            case SND_SND_PAUSE_MUTE:
            case SND_SND_SET_DESC_VOLUME_OFFSET:
            case SND_SND_SET_BS_OUTPUT_SRC:
            case SND_SND_SECOND_DECA_ENABLE:
            case SND_SND_DO_DDP_CERTIFICATION:
            case SND_SND_CHK_DAC_PREC:
            case SND_SND_POST_PROCESS_0:
            case SND_SND_SPECIAL_MUTE_REG:
            case SND_SND_SET_SYNC_DELAY:
            case SND_SND_SET_SYNC_LEVEL:
            case SND_SND_SET_MUTE_TH:
            case SND_SND_AUTO_RESUME:
            case SND_SND_REQ_REM_DATA:
            case SND_SND_GET_TONE_STATUS:
            case SND_SND_CHK_PCM_BUF_DEPTH:
            case SND_SND_GET_SAMPLES_REMAIN:
            case SND_SND_REQ_REM_PCM_DATA:
            case SND_SND_REQ_REM_PCM_DURA:
            case SND_SND_GET_MUTE_TH:
            case SND_SND_SPECTRUM_START:
            case SND_SND_SPECTRUM_STEP_TABLE:
            case SND_SND_SPECTRUM_STOP:
            case SND_SND_SPECTRUM_CLEAR:
            case SND_SND_SPECTRUM_VOL_INDEPEND:
            case SND_SND_SPECTRUM_CAL_COUNTER:
            case SND_SND_SET_SPDIF_SCMS:
            case SND_SND_SET_SYNC_PARAM:
            case SND_SND_IO_PAUSE_SND:
            case SND_SND_IO_RESUME_SND:
            case SND_SND_IO_DDP_SPO_INTF_CFG:
            case SND_SND_IO_SPO_INTF_CFG:
            case SND_SND_ONLY_SET_SPDIF_DELAY_TIME:
            case SND_SND_ONLY_GET_SPDIF_DELAY_TIME:
            {
                io_param.ioctl_cmd = alislsnd_convert_snd_io_command(cmd);
                io_param.param=param;
                if((SND_IO_SPO_INTF_CFG == io_param.ioctl_cmd)
                    ||(SND_IO_DDP_SPO_INTF_CFG == io_param.ioctl_cmd))
                {
                    //if set ec3/ac3 not pcm output, we should output the ec3/ac3 raw data.
                    if(param != 0)
                    {
                        if(alislsnd_enable_rawdata_output(priv))
                        {
                            printf("%s -> alislsnd_enable_rawdata_output fail\n", __FUNCTION__);
                        }
                    }                    
                }
                ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
                break;
            }
            case SND_SND_IO_GET_PLAY_PTS:
            {
                struct snd_get_pts_param pts_param;
                memset(&pts_param, 0, sizeof(pts_param));
                io_param.ioctl_cmd = alislsnd_convert_snd_io_command(cmd);
                io_param.param = (unsigned int)&pts_param;
                ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
                if (0 == ret)
                {
                    *(unsigned int *)param = pts_param.pts;
                }
                else
                {
                    *(unsigned int *)param = 0;
                }
                break;
            }
            default:
                break;
        }

    } else {
        switch(cmd)
        {
            case SND_DECA_INIT_TONE_VOICE:
                ret = ioctl(priv->fd, AUDIO_INIT_TONE_VOICE, 0);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_INIT_TONE_VOICE fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            case SND_DECA_GEN_TONE_VOICE:
                ret = ioctl(priv->fd, AUDIO_GEN_TONE_VOICE, param);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_GEN_TONE_VOICE fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            case SND_DECA_STOP_TONE_VOICE:
                ret = ioctl(priv->fd, AUDIO_STOP_TONE_VOICE, 0);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_STOP_TONE_VOICE fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            case SND_SND_STOP:
                ret = ioctl(priv->fd, AUDIO_SND_STOP, 0);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_SND_STOP fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;
            case SND_SND_START:
                ret = ioctl(priv->fd, AUDIO_SND_START, 0);
                if(ret)
                {
                    printf("WARNING %s -> cmd%d @AUDIO_SND_START fail, ret: %d\n", __FUNCTION__, cmd, ret);
                }
                break;    
            default:
                break;
        }
    }
    if(ret < 0)
    {
        printf("WARNING %s -> cmd: %d fail, ret: %d\n", __FUNCTION__, cmd, ret);
    }
    return ret;
}

/**
 * @brief         init decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[in]     config audio config info
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_init(void *handle, struct snd_audio_config *config)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    int reset = 0;
    struct audio_config audio_config_info;
    if (priv == NULL || config == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    audio_config_info.decode_mode = config->decode_mode;
    audio_config_info.sync_mode = config->sync_mode;
    audio_config_info.sync_unit = config->sync_unit;
    audio_config_info.deca_input_sbm = config->deca_input_sbm;
    audio_config_info.deca_output_sbm = config->deca_output_sbm;
    audio_config_info.snd_input_sbm = config->snd_input_sbm;
    audio_config_info.pcm_sbm = config->pcm_sbm;
    audio_config_info.codec_id = config->codec_id;
    if (audio_config_info.codec_id == SND_CODEC_ID_OGG) {
        audio_config_info.codec_id = 0x4f4747;//MKBETAG( 0 ,'O','G','G'),refer to enum codec_id in avplay_sbm.h
    }
    //printf("audio_config_info.codec_id: 0x%x\n", audio_config_info.codec_id);
    audio_config_info.bits_per_coded_sample = config->bits_per_coded_sample;
    audio_config_info.sample_rate = config->sample_rate;
    audio_config_info.channels = config->channels;
    audio_config_info.bit_rate = config->bit_rate;
    audio_config_info.pcm_buf = config->pcm_buf;
    audio_config_info.pcm_buf_size = config->pcm_buf_size;
    audio_config_info.block_align = config->block_align;
    memcpy(audio_config_info.extra_data, config->extra_data, 512);
    audio_config_info.codec_frame_size = config->codec_frame_size;
    audio_config_info.extradata_size = config->extradata_size;
    audio_config_info.extradata_mode = config->extradata_mode;
    audio_config_info.cloud_game_prj = config->cloud_game_prj;
    audio_config_info.encrypt_mode = config->encrypt_mode;
    SL_DBG("{cmd(SND_DECORE_INIT) decode_mode:%d,sync_mode:%d,codec_id:0x%x,bits_per_coded_sample:%d,sample_rate:%d,"\
        "channels:%d,bit_rate:%d,pcm_buf:%d,pcm_buf_size:%d,block_align:%d,codec_frame_size:%d,extradata_size:%d,deca_input_sbm:0x%x}\n",
        audio_config_info.decode_mode,audio_config_info.sync_mode,audio_config_info.codec_id,audio_config_info.bits_per_coded_sample,
        audio_config_info.sample_rate,audio_config_info.channels,audio_config_info.bit_rate,audio_config_info.pcm_buf,audio_config_info.pcm_buf_size,
        audio_config_info.block_align,audio_config_info.codec_frame_size,audio_config_info.extradata_size,audio_config_info.deca_input_sbm);

    ret = alislsnd_decore_ioctl(handle, SND_DECORE_INIT, &audio_config_info, &reset);
    return ret;
}

/**
 * @brief         release decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_release(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    SL_DBG("{cmd(SND_DECORE_RLS)}\n");
    ret = alislsnd_decore_ioctl(handle, SND_DECORE_RLS, NULL, NULL);
    return ret;
}

/**
 * @brief         pause decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_pause(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    int pause = 1;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    SL_DBG("{cmd(SND_DECORE_PAUSE_DECODE) pause:%d}\n",pause);
    ret = alislsnd_decore_ioctl(handle, SND_DECORE_PAUSE_DECODE, &pause, &pause);
    return ret;
}

/**
 * @brief         resume decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_resume(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    int pause = 0;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    SL_DBG("{cmd(SND_DECORE_PAUSE_DECODE) pause:%d}\n",pause);
    ret = alislsnd_decore_ioctl(handle, SND_DECORE_PAUSE_DECODE, &pause, &pause);
    return ret;
}

/**
 * @brief         flush decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_flush(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    SL_DBG("{cmd(SND_DECORE_FLUSH)}\n");
    ret = alislsnd_decore_ioctl(handle, SND_DECORE_FLUSH, NULL, NULL);
    return ret;
}

/**
 * @brief         flush decore in mp mode
 * @author        ze.hong
 * @date          2013-7-8
 * @param[in]     handle is sound device pointer
 * @param[out]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_mp_decore_get_status(void *handle,
    struct snd_audio_decore_status *status)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    struct audio_decore_status audio_status;

    if (priv == NULL || status == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    ret = alislsnd_decore_ioctl(handle, SND_DECORE_GET_STATUS, &audio_status, NULL);

    status->sample_rate = audio_status.sample_rate;
    status->channels = audio_status.channels;
    status->bits_per_sample = audio_status.bits_per_sample;
    status->first_header_got = audio_status.first_header_got;
    status->first_header_parsed = audio_status.first_header_parsed;
    status->frames_decoded = audio_status.frames_decoded;

    return ret;
}

alisl_retcode alislsnd_audio_description_enable(void *handle, unsigned char enable)
{
    struct sound_private *priv = (struct sound_private *)handle;
    int ret = -1;
    struct ali_audio_ioctl_command io_param;
    unsigned long param = (enable == 0) ? 0 : 1;

    memset(&io_param, 0, sizeof(struct ali_audio_ioctl_command));
    io_param.ioctl_cmd = SND_SET_AD_DYNAMIC_EN;
    io_param.param = param;   

    ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
    return ret;
}

alisl_retcode alislsnd_set_mix_balance(void *handle, int balance)
{
    struct sound_private *priv = (struct sound_private *)handle;
    int ret = -1;
    struct ali_audio_ioctl_command io_param;  

    memset(&io_param, 0, sizeof(struct ali_audio_ioctl_command));
    io_param.ioctl_cmd = SND_SET_DESC_VOLUME_OFFSET_NEW;
    io_param.param = balance;

    ret = ioctl(priv->fd, AUDIO_SND_IO_COMMAND, &io_param);
    return ret;
}

/**
 * @brief         set audio mix info
 * @author        tom.xie
 * @date          2016-4-28
 * @param[in]     
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_audio_mix_info(void *handle, struct snd_audio_mix_info *p_mix_info)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    int try_time = 5;
    struct snd_mix_info mix_info;
    struct snd_mix_info mix_stop_info;

    if (priv == NULL) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    
    memset(&mix_info, 0, sizeof(mix_info));
    mix_info.cmd = AUD_MIX_START;
    mix_info.ch_num = p_mix_info->ch_num;
    mix_info.sample_rate = p_mix_info->sample_rate;
    mix_info.bit_per_samp = p_mix_info->bit_per_samp;
    mix_info.sbm_id = p_mix_info->sbm_id;
    memset(&mix_stop_info, 0, sizeof(mix_stop_info));

    while (ioctl(priv->fd, AUDIO_SND_SET_MIX_INFO, &mix_info)) {
        if (ioctl(priv->fd, AUDIO_SND_SET_MIX_INFO, &mix_stop_info)) {
            SL_ERR("audio ioctl(AUDIO_SND_SET_MIX_END) fail\n");
            ret = ERROR_IOCTL;
            break;
        }

        usleep(10*1000);//wait for the task exited
        try_time --;

        if (0 >= try_time) {
            SL_ERR("audio ioctl(AUDIO_SND_SET_MIX_INFO) fail, try 5 times\n");
            ret = ERROR_IOCTL;
            break;
        }
    }
    
    return ret;
}

/**
 * @brief         set audio mix info
 * @author        tom.xie
 * @date          2016-4-28
 * @param[in]
 * @return        alisl_retcode
 * @note
 *
 */
alisl_retcode alislsnd_set_audio_mix_end(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;

    if (priv == NULL)
    {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    if(ioctl(priv->fd, AUDIO_SND_SET_MIX_END, 0)) {
        SL_ERR("audio ioctl(AUDIO_SND_SET_MIX_END) fail\n");
        ret = ERROR_IOCTL;
    }
    return ret;
}

alisl_retcode alislsnd_audio_mix_stop(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    struct snd_mix_info mix_stop_info;

    if (priv == NULL) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    
    memset(&mix_stop_info, 0, sizeof(mix_stop_info));
    mix_stop_info.cmd = AUD_MIX_STOP;
    
    if (ioctl(priv->fd, AUDIO_SND_SET_MIX_INFO, &mix_stop_info)) {
        SL_ERR("stop mix audio ioctl(AUDIO_SND_SET_MIX_INFO) fail\n");
        ret = ERROR_IOCTL;
    }
  
    return ret;
}

alisl_retcode alislsnd_audio_mix_pause(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    struct snd_mix_info mix_pause_info;

    if (priv == NULL) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    
    memset(&mix_pause_info, 0, sizeof(mix_pause_info));
    mix_pause_info.cmd = AUD_MIX_PAUSE;
    
    if (ioctl(priv->fd, AUDIO_SND_SET_MIX_INFO, &mix_pause_info)) {
        SL_ERR("pause mix audio ioctl(AUDIO_SND_SET_MIX_INFO) fail\n");
        ret = ERROR_IOCTL;
    }
    
    return ret;
}

alisl_retcode alislsnd_audio_mix_resume(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    alisl_retcode ret = 0;
    struct snd_mix_info mix_resume_info;

    if (priv == NULL) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    
    memset(&mix_resume_info, 0, sizeof(mix_resume_info));
    mix_resume_info.cmd = AUD_MIX_RESUME;
    
    if (ioctl(priv->fd, AUDIO_SND_SET_MIX_INFO, &mix_resume_info)) {
        SL_ERR("resume mix audio ioctl(AUDIO_SND_SET_MIX_INFO) fail\n");
        ret = ERROR_IOCTL;
    }
    
    return ret;
}

alisl_retcode alislsnd_pcm_capture_start(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    if (NULL == priv) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    struct pcm_capture_buff info;
    memset(&info, 0, sizeof(info));
    if (priv->pcm_cap_init_flag)
        return 0;
    if (ioctl(priv->fd, SND_GET_PCM_CAPTURE_BUFF_INFO, &info)) {
        pcm_cap_dbg("cap ioctl fail\n");
        SL_ERR("SND_GET_PCM_CAPTURE_BUFF_INFO fail\n");
        return -1;
    }
    pcm_cap_dbg("%p, %u, %u, %u, %u, %u\n", info.buff_base, info.buff_len,
        info.buff_rd, info.buff_wt, info.sample_rate, info.status);
    priv->pcm_cap_buf_len = info.buff_len<<SHIFTBITS; //1<<17; //
    priv->pcm_cap_buf = mmap(NULL, info.buff_len<<SHIFTBITS, PROT_READ,MAP_SHARED,priv->fd,0);
    if (((void *)-1) == priv->pcm_cap_buf) {
        pcm_cap_dbg("mmap cap buf fail\n");
        SL_ERR("mmap pcmcap buf fail\n");
        return -1;
    }
    priv->pcm_cap_init_flag = 1;
    return 0;
}

alisl_retcode alislsnd_pcm_capture_buf_info_get(void *handle, sl_snd_capture_buffer **sl_info, int *p_cnt)
{
    struct sound_private *priv = (struct sound_private *)handle;
    if ((NULL == priv) ||(NULL == sl_info) ||(NULL == p_cnt)) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }

    *sl_info = NULL;
    *p_cnt = 0;
    struct pcm_capture_buff info;
    memset(&info, 0, sizeof(info));
    if (ioctl(priv->fd, SND_GET_PCM_CAPTURE_BUFF_INFO, &info)) {
        SL_ERR("SND_GET_PCM_CAPTURE_BUFF_INFO fail\n");
        return -1;
    }
    if ((0 == info.buff_wt_skip) && (info.buff_rd == info.buff_wt)) {
        return 0;
    }

    int cnt = get_pcm_frame_cnt(priv, &info);
    *sl_info = extract_pcm_frame_by_cnt(priv, &info, cnt);
    if (NULL == *sl_info) {
        pcm_cap_dbg("no any pcm frame\n");
        return -1;
    } else {
        *p_cnt = cnt;
        priv->pcm_buf_array = *sl_info;
        priv->pcm_buf_array_size = cnt;
        priv->last_buff_rd = info.buff_rd;
        priv->last_buff_wt = info.buff_wt;
        priv->last_buff_wt_skip = info.buff_wt_skip;
        return 0;
    }
}
alisl_retcode alislsnd_pcm_capture_buf_rd_update(void *handle, unsigned long rd)
{
    struct sound_private *priv = (struct sound_private *)handle;
    if (NULL == priv) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    if (rd != priv->pcm_buf_array_size) {
        pcm_cap_dbg("%u vs %lu\n", priv->pcm_buf_array_size, rd);
        return -1;
    }
    return pcm_capture_buf_ret(priv);
}

alisl_retcode alislsnd_pcm_capture_stop(void *handle)
{
    struct sound_private *priv = (struct sound_private *)handle;
    if (NULL == priv) {
        SL_ERR("SND NULL private data structure!\n");
        return ERROR_INVAL;
    }
    if (priv->pcm_cap_init_flag) {
        pcm_capture_buf_ret(priv);
        if (priv->pcm_cap_buf != ((void *)-1)) {
            if (-1 == munmap(priv->pcm_cap_buf, priv->pcm_cap_buf_len))
                SL_ERR("pcm cap munmap fail"); 
        }
        priv->pcm_cap_init_flag=0;
    }
    return 0;
}

