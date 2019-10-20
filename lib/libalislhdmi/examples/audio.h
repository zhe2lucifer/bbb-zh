#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <ali_avsync_common.h>
#include <ali_video_common.h>
#include <ali_audio_common.h>
#include <ali_sbm_common.h>
#include <alidefinition/adf_deca.h>

//#define log_info(fmt, ...)  do { fprintf(stderr, "TEST INFO-" fmt "\n", ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)  do { } while (0)
#define log_error(fmt, ...) do { fprintf(stderr, "TEST ERROR-" fmt "\n", ##__VA_ARGS__); } while (0)

#define WRITE_HEADER      0
#define WRITE_DATA        1
#define WRITE_DONE        2

/** E.g. mpeg2, h264, mpeg4p2, vc1, etc.
 *  Values can be decided by ALi.
 */
typedef uint32_t ali_audio_codec_id;
typedef uint32_t ali_audio_decoder_id;


/** STC timer id. *  Values can be decided by ALi.
 *  May be, that there is only one timer.
 */
typedef uint32_t ali_stc_id;

struct ali_audio_decoder_buffer_status_s {
    uint32_t total_size;
    uint32_t valid_size;
    uint32_t free_size;
};
typedef struct ali_audio_decoder_buffer_status_s  ali_audio_decoder_buffer_status;

typedef struct audio_decore_status  ali_audio_decoder_status;

struct ali_audio_decoder_mem_s {
    uint32_t mp_mem_addr;
    uint32_t mp_mem_size;
};
typedef struct ali_audio_decoder_mem_s  ali_audio_decoder_mem;

struct ali_audio_decoder_buffer_s {
    uint32_t pkt_hdr_addr;
    uint32_t pkt_hdr_size;
    uint32_t pkt_data_addr;
    uint32_t pkt_data_size;
};
typedef struct ali_audio_decoder_buffer_s  ali_audio_decoder_buffer;


struct ali_audio_decoder_init_s {
    ali_audio_codec_id codec_id;
    int32_t decode_mode;
    uint8_t *extradata;
    int32_t extradata_size;
    int32_t pkt_hdr_sbm;
    int32_t pkt_data_sbm;
    int32_t channels;
    int32_t samplerate;
    ali_audio_decoder_buffer dec_buf;
    int32_t audio_id;
};
typedef struct ali_audio_decoder_init_s  ali_audio_decoder_init;

struct ali_aduio_decoder_private {
    int pkt_hdr_fd;
    int pkt_data_fd;
    int audio_fd;
    int decode_mode;  /* 0: live play, 1: media player */
    unsigned int codec_id;
    int audio_id;
};

/** Open video decoder with given codec.
 *  \param codec_id  codec to be used
 *  \param type input from demux or user
 *  \param[out] decoder_out returned decoder instance
 *  \returns 0 on success, negative on error
 */
int ali_audio_open(ali_audio_decoder_init *decoder_init, ali_audio_decoder_id *decoder_out);

/** Close given decoder instance.
 *  \param decoder  decoder instance created with \e ali_video_decoder_open()
 */
void ali_audio_close(ali_audio_decoder_id decoder);

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
int ali_audio_get_status(ali_audio_decoder_id decoder, ali_audio_decoder_status *status);

/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_audio_pause(ali_audio_decoder_id decoder, int32_t pause);


/** Push next PES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_audio_write_header(ali_audio_decoder_id decoder, const struct av_packet *pkt_hdr);

/** Push next fragment of PES packet to video decoder.
 *  \param buf  buffer with fragment of one PES packet
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_audio_write(ali_audio_decoder_id decoder, const uint8_t* buf, uint32_t size);

/** Get memory used by decoder and media player.
 *  \param[out] decoder_mem  returned memory decoder using
 *  \returns 0 on success, negative on error
 */
int ali_audio_get_memory(int32_t video_id, ali_audio_decoder_mem *decoder_mem);

/** Get current buffer status.
 *  \param[out] buffer_status  returned current buffer status
 *  \returns 0 on success, negative on error
 */
int ali_audio_get_buffer_status(ali_audio_decoder_id decoder, ali_audio_decoder_buffer_status *buffer_status);

int ali_audio_ioctl(int cmd, void *param1, void *param2);

#endif

