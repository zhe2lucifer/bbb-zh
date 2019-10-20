#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <ali_avsync_common.h>
#include <ali_video_common.h>
#include <ali_sbm_common.h>
//#include <ali_stc_common.h>
//#include <ali_soc_common.h>
#include <alidefinition/adf_decv.h>
#include <alidefinition/adf_vpo.h>

//#define log_info(fmt, ...)  do { fprintf(stderr, "TEST INFO-" fmt "\n", ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)  do { } while (0)
#define log_error(fmt, ...) do { fprintf(stderr, "TEST ERROR-" fmt "\n", ##__VA_ARGS__); } while (0)

#define AVSYNCDEV         "/dev/ali_avsync0"
#define VIDEODEV          "/dev/ali_video0"
#define VIDEODEV0         "/dev/ali_video0"
#define VIDEODEV1         "/dev/ali_video1"
#define FBDEV             "/dev/fb0"
#define FBDEV0            "/dev/fb0"
#define FBDEV1            "/dev/fb1"
#define FBDEV2            "/dev/fb2"
#define STCDEV0           "/dev/ali_stc0"
#define STCDEV1           "/dev/ali_stc1"
#define SOCDEV            "/dev/ali_soc"

#define WRITE_HEADER      0
#define WRITE_DATA        1
#define WRITE_DONE        2

/** E.g. mpeg2, h264, mpeg4p2, vc1, etc.
 *  Values can be decided by ALi.
 */
typedef uint32_t ali_video_codec_id;
typedef uint32_t ali_video_decoder_id;


/** STC timer id. *  Values can be decided by ALi.
 *  May be, that there is only one timer.
 */
typedef uint32_t ali_stc_id;

enum ali_video_frame_types_e {
    ali_video_frame_types_all,  /// decode all frames (I, P and B)
    ali_video_frame_types_IP,   /// decode only I and P frames  ==> ALi not support this mode
    ali_video_frame_types_I,    /// decode only I frames
};
typedef enum ali_video_frame_types_e  ali_video_frame_types;

enum ali_video_input_types_e {
    ALI_VIDEO_INPUT_DMX,
    ALI_VIDEO_INPUT_USER,
};
typedef enum ali_video_input_types_e  ali_video_input_types;

struct ali_video_decoder_buffer_status_s {
    uint32_t total_size;
    uint32_t valid_size;
    uint32_t free_size;
};
typedef struct ali_video_decoder_buffer_status_s  ali_video_decoder_buffer_status;

struct ali_video_decoder_status_s {
    uint32_t buffer_size; /// size of internal buffer used for PES/ES gathering
    uint32_t buffer_used; /// used space in internal buffer, app will use it to control speed of pushing data
    int64_t last_pts;     /// 90kHz, 33 bits, -1 if unknown
    uint32_t frames_decoded;
    uint32_t frames_displayed;
    int32_t width;      /// -1 if unknown
    int32_t height;     /// -1 if unknown
    int32_t fps;        /// -1 if unknown, units can be decided by ALi
    int8_t  interlaced; /// -1 if unknown
    int8_t  top_field_first; /// -1 if unknown
};
typedef struct ali_video_decoder_status_s  ali_video_decoder_status;

struct ali_video_decoder_mem_s {
    uint32_t main_mem_addr;
    uint32_t main_mem_size;
    uint32_t priv_mem_addr;
    uint32_t priv_mem_size;
    uint32_t mp_mem_addr;
    uint32_t mp_mem_size;
};
typedef struct ali_video_decoder_mem_s  ali_video_decoder_mem;

struct ali_video_decoder_buffer_s {
    uint32_t pkt_hdr_addr;
    uint32_t pkt_hdr_size;
    uint32_t pkt_data_addr;
    uint32_t pkt_data_size;
    uint32_t dec_out_addr;
    uint32_t dec_out_size;
    uint32_t disp_in_addr;
    uint32_t disp_in_size;
    uint32_t frm_buf_addr;
    uint32_t frm_buf_size;
};
typedef struct ali_video_decoder_buffer_s  ali_video_decoder_buffer;

struct ali_video_decoder_rect_s {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
};
typedef struct ali_video_decoder_rect_s  ali_video_decoder_rect;

struct ali_video_decoder_init_s {
    ali_video_input_types input_type;
    ali_video_codec_id codec_id;
    int32_t decode_mode;
    int32_t pic_width;
    int32_t pic_height;
    int32_t sar_width;
    int32_t sar_height;
    int32_t frame_rate;
    int8_t *extradata;
    int32_t extradata_size;
    int32_t preview;
    ali_video_decoder_rect src_rect;
    ali_video_decoder_rect dst_rect;
    int32_t pkt_hdr_sbm;
    int32_t pkt_data_sbm;
    int32_t dec_out_sbm;
    int32_t disp_in_sbm;
    ali_video_decoder_buffer dec_buf;
    int32_t video_id;
};
typedef struct ali_video_decoder_init_s  ali_video_decoder_init;

struct ali_decoder_private {
    int pkt_hdr_fd;
    int pkt_data_fd;
    int dec_out_fd;
    int disp_in_fd;
    int video_fd;
    int decode_mode;  /* 0: live play, 1: media player */
    unsigned int codec_id;
    int first_header_parsed;
    int first_pic_decoded;
    int first_pic_showed;
    unsigned int frames_decoded;
    unsigned int frames_displayed;
    int video_id;
};

struct ali_stc_private {
    int stc0_fd;
    int stc1_fd;
};

int ali_stc_get(ali_stc_id stc, uint8_t id, uint64_t *val_out);
int ali_stc_set(ali_stc_id stc, uint8_t id, uint64_t val);
int ali_stc_pause(ali_stc_id stc, uint8_t id, int32_t pause);
int ali_stc_change_speed(ali_stc_id stc, uint8_t id, int ppm);
int ali_stc_open(ali_stc_id *stc_out);
void ali_stc_close(ali_stc_id stc);

/** Open video decoder with given codec.
 *  \param codec_id  codec to be used
 *  \param type input from demux or user
 *  \param[out] decoder_out returned decoder instance
 *  \returns 0 on success, negative on error
 */
int ali_video_open(ali_video_decoder_init *decoder_init, ali_video_decoder_id *decoder_out);

/** Close given decoder instance.
 *  \param decoder  decoder instance created with \e ali_video_decoder_open()
 */
void ali_video_close(ali_video_decoder_id decoder);

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
int ali_video_get_status(ali_video_decoder_id decoder, ali_video_decoder_status *status);



/** Enable/disable STC sync.
 *  With sync enabled, decoder displays video frames according to pts/stc difference.
 *  With sync disabled, decoder displays video frames according to vsync.
 *  \param enable  enable/disable STC sync
 *  \returns 0 on success, negative on error
 */
int ali_video_set_sync(ali_video_decoder_id decoder, int32_t enable);

/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
int ali_video_pause(ali_video_decoder_id decoder, int32_t pause);

/** Choose what frames are to be decoded.
 *  \param types see \e ali_video_frame_types
 *  \returns 0 on success, negative on error
 */
int ali_video_set_frame_types(ali_video_decoder_id decoder, ali_video_frame_types types);

/// TODO: video viewports

/** Push next PES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
int ali_video_write_header(ali_video_decoder_id decoder, const struct av_packet *pkt_hdr);

/** Push next fragment of PES packet to video decoder.
 *  \param buf  buffer with fragment of one PES packet
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
int ali_video_write(ali_video_decoder_id decoder, const uint8_t* buf, uint32_t size);

/** Get memory used by decoder and media player.
 *  \param[out] decoder_mem  returned memory decoder using
 *  \returns 0 on success, negative on error
 */
int ali_video_get_memory(int32_t video_id, ali_video_decoder_mem *decoder_mem);

/** Get current buffer status.
 *  \param[out] buffer_status  returned current buffer status
 *  \returns 0 on success, negative on error
 */
int ali_video_get_buffer_status(ali_video_decoder_id decoder, ali_video_decoder_buffer_status *buffer_status);

int ali_video_ioctl(ali_video_decoder_id decoder, int cmd, uint32_t param);

/** Connect video decoder with demux channel.
 *  \param channel input source for video decoder's PES parser
 *  \returns 0 on success, negative on error
 */
//int ali_video_set_demux_channel(ali_video_decoder_id decoder, ali_demux_channel_id channel);

int ali_video_vpo_onoff(int32_t layer, int32_t onoff);

#endif

