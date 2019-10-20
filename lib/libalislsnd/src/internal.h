/** @file     internal.h
 *  @brief    alislsnd internal.h
 *  @author   ze.hong
 *  @date     2013-7-8
 *  @version  1.0.0
 *  @note     Ali Corp. All Rights Reserved. 2013-2999 Copyright (C)
 *            input file detail description here
 *            input file detail description here
 *            input file detail description here
 */

#ifndef __ALISLSND_INTERNEL_H__
#define __ALISLSND_INTERNEL_H__

#include <alislsnd.h>
#include <alipltflog.h>
#include <alislevent.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SND_STATUS_NONE         (0)
#define SND_STATUS_ALL          (0xFFFFFFFF)
#define SND_STATUS_CONSTRUCT    (1 << 0)
#define SND_STATUS_OPEN         (1 << 1)
#define SND_STATUS_START        (1 << 2)
#define SND_STATUS_STOP         (1 << 3)
#define SND_STATUS_CLOSE        (1 << 4)
#define SND_STATUS_PAUSE        (1 << 5)

#define STACKSIZE               0x1000
#define MAX_IO_PORT 4

#define MAX_KUMSG_SIZE 1024

typedef struct sound_private 
{
	int                   fd;
	uint32_t              status;
	enum Snd_decoder_type decode_type;
	enum SndSyncMode 	  sync_mode;
	enum SndTrackMode 	  trackmode[MAX_IO_PORT];
	enum SndOutFormat 	  OutputFormat[MAX_IO_PORT];
	uint8_t               volume[MAX_IO_PORT];
	bool 				  mute_state[MAX_IO_PORT];
	unsigned int open_cnt;
    alislsnd_callback deca_cb;
    alislsnd_callback snd_cb;

    int kumsg_fd;/**< used for kernel-userspace messaging */
    struct alislevent snd_cb_event;
    alisl_handle snd_event_handle;
    int pcm_cap_init_flag;
    char *pcm_cap_buf;  //mmaped pcm captuer buffer address
    unsigned int pcm_cap_buf_len; //mmaped pcm captuer buffer length unit in bytes.
    sl_snd_capture_buffer *pcm_buf_array; //array storing pcm data without header
    unsigned int pcm_buf_array_size;  //array size
    UINT16  last_buff_rd; //last  read pointer index of mmaped pcm capture buffer
    UINT16  last_buff_wt; //last write pointer index of mmaped pcm capture buffer
    UINT16 last_buff_wt_skip; //the start index of buffer can`t storing a complete pcm frame.
    UINT16 last_pcm_frm_size; //the last pcm frame size unit in bytes.
} sound_private_t;

#ifdef __cplusplus
}
#endif

#endif
