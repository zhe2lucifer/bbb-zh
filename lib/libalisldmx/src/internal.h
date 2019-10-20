/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               internel.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               06/15/2013 09:20:42 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef __ALISLDMX_INTERNEL_H__
#define __ALISLDMX_INTERNEL_H__

/* system headers */
#include <pthread.h>

/* share library headers */
#include <alisldmx.h>

/* local headers */
#include "misc.h"

/* ali driver headers */
#include <ali_dmx_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define DMX_STATUS_NONE         (0)
#define DMX_STATUS_ALL          (0xFFFFFFFF)
#define DMX_STATUS_CONSTRUCT    (1 << 0)
#define DMX_STATUS_OPEN         (1 << 1)
#define DMX_STATUS_START        (1 << 2)
#define DMX_STATUS_PAUSE        (1 << 3)
#define DMX_STATUS_STOP         (1 << 4)
/**
 * Used for sync between playfeed pthread
 * and control operation from other async pthread.
 */
#define DMX_STATUS_IDLE_P1      (1 << 5)
#define DMX_STATUS_IDLE_P2      (1 << 6)
#define DMX_STATUS_IDLE_P3      (1 << 7)
#define DMX_STATUS_CLOSE        (1 << 8)
#define DMX_STATUS_CONFIGURE    (1 << 9)

#define CHANNEL_STATUS_NONE     (0)
#define CHANNEL_STATUS_ALL      (0xFFFFFFFF)
#define CHANNEL_STATUS_ENABLE   (1 << 0)
#define CHANNEL_STATUS_DISABLE  (1 << 1)

#define SECTION_FLAG_NONE             (0)
#define SECTION_FLAG_ALL              (0xFFFFFFFF)
/** bit 0-30  -- channel id 0-29, range: [0, min(29, MAX_SECTIONS - 1) ) */
#define SECTION_FLAG_ALLID            (0x3FFFFFFF)
/** bit 30    -- stop poll */
#define SECTION_FLAG_STOPPOLL         (1 << 30)
/** bit 31    -- stop request */
#define SECTION_FLAG_STOPREQUEST      (1 << 31)

#define FILTER_STATUS_NONE      (0)
#define FILTER_STATUS_ALL       (0xFFFFFFFF)
#define FILTER_STATUS_ENABLE    (1 << 0)
#define FILTER_STATUS_DISABLE   (1 << 1)

#define MAX_SECTIONS            DMX_MAX_SECTIONS
#define MAX_RECORDS             DMX_MAX_RECORDS
#define MAX_SERVICES            DMX_MAX_SERVICES
#define MAX_STREAMS             DMX_MAX_STREAMS
#define MAX_FILTERS             DMX_MAX_FILTERS

#define STACKSIZE               0x4000

#define DMX_TS_PKT_SIZE         188

#define PID_VALID(x)            (((x) & 0x1FFF) != DMX_ILLEGAL_PID)

#define REQ_DATA_LEN            (0x17800 >> 1)  /**< Usually 47KB for TS stream */
#define DECRYPT_KEY_LEN         128

/** 16 * 188, 16 of ts packets */
#define PLAYFEED_REQ_DATA_LEN   (16*DMX_TS_PKT_SIZE)

#define DMX_MAX_FILTER_SIZE     18
#define DMX_MAX_SECTION_SIZE    4096
#define DMX_MAX_SECFEED_SIZE    (DMX_MAX_SECTION_SIZE + DMX_TS_PKT_SIZE)

typedef enum sec_flt_flag{
    FLT_FLAG_INIT,
    FLT_FLAG_POLL,
    FLT_FLAG_DISPATCH,
    FLT_FLAG_CB,
}sec_flt_flag;

typedef struct dmx_section_filter {
	struct list_head        node;
	uint32_t                id;
	uint32_t                status;                    
	uint32_t                size;
	uint8_t                 filter_value [DMX_MAX_FILTER_SIZE];
	uint8_t                 filter_mask [DMX_MAX_FILTER_SIZE];
	uint8_t                 filter_mode [DMX_MAX_FILTER_SIZE];
	/**
	 * Continuously get filtered section packet or no.
	 */
	bool					continuous;
	/**
	 * Callback node for this filter.
	 */
	struct dmx_channel_callback callback;
    /*
    * whether filter node has been processed in section_dispatch
    */
    bool                    access;
    bool                    crc_check;
    /*
    * indicating filter`s flag in section_dispatch, used to fix 
    * sync issue between calling user cb and freeing filter.
    */
    volatile sec_flt_flag            flag;
    /*
    Because if one table data is so frequent(the section dispatch is called frequently, 
    filter is always in FLT_FLAG_CB) that the alisldmx_free_filter would wait to much time. So we should set this flag to
    notify the section dispatch that the filter is going to be closed.
    */
    volatile bool           wait_close;
    struct dmx_channel *p_ch;
    volatile int session_id;
} dmx_section_filter_t;

typedef struct dmx_buffer {
	uint8_t                 *buf;
	uint32_t                size;
	uint32_t                write;
	uint16_t                ifm_offset;
} dmx_buffer_t;

typedef struct dmx_channel {
	struct list_head        node;

	/**
	 * id = (type << 16) | realid
	 */
	uint32_t                id;
	enum dmx_channel_type   type;
	enum dmx_stream_type    stream;
	uint32_t                status;         /**< channel status */

	const char              *path;          /**< point to dmx_device->path */
	int                     seefd;          /**< a copy of dmx_device->seefd */
	int                     fd;             /**< each channel's demux device fd */

	/**
	 * We always use this struct to save channel parameters
	 * which will be set into demux driver.
	 */
	struct dmx_channel_param ch_param;

	/**
	 * We may record several streams with different pid
	 * in one channel. And demux driver support this feature.
	 * We can just set a pid list and tell driver the number
	 * of valid pids in pid list. The pid_list is used to save
	 * the current used pid list. And so is the variable nb_pid.
	 */
	uint16_t                pid_list[DMX_REC_PID_LIST_MAX_LEN];
	uint32_t                nb_pid;

	/**
	 * Some channel need an array of section filters.
	 */
	struct list_head        filter;

	/**
	 * Saved the buffer informations which requested from client.
	 */
	struct dmx_buffer       buffer;

	/**
	 * Channel callback functions.
	 * Section channel doesn't use this callback, the callback in dmx_section_filter will be used.
	 */
	struct dmx_channel_callback callback;
	enum Ali_DmxRecordMode rec_mode;
	unsigned int block_size;
	/**
	 * Rerequest the same size buffer once request failed.
	 * Do not read the new special packet in this case to avoid data loose.
	 */
    unsigned int need_rerequest_size;
	unsigned int flt_cnt;
	volatile int session_id;
} dmx_channel_t;

typedef struct dmx_pollreactor {
	pthread_mutex_t         mutex;
	pthread_t               tid;
	int 	                fd;
} dmx_pollreactor_t;

typedef struct dmx_device {
	enum dmx_id             id;             /**< demux id number */

	/**
	 * demux device status.
	 * AO   = Already Opened.  \n
	 * NO   = Not Opened.      \n
	 * NST  = Not Started.     \n
	 * NSTP = Not Stoped.      \n
	 * AST  = Already Started. \n
	 * ASTP = Already Stoped.  \n
	 * AC   = Already Closed.  \n
	 * AP   = Already Paused.  \n
	 *  \n
	 * |---\---|-----|------|------|-----|------  \n
	 * |    \to|     |      |      |     |        \n
	 * |from \ |OPEN |START |PAUSE |STOP | CLOSE  \n
	 * |------\|-----|------|------|-----|------  \n
	 * | OPEN  | AO  | Yes  | NST  | NST | Yes    \n
	 * |-------|-----|------|------|-----|------  \n
	 * | START | AO  | AST  | Yes  | Yes | NSTP   \n
	 * |-------|-----|------|------|-----|------  \n
	 * | PAUSE | AO  | Yes  | AP   | Yes | NSTP   \n
	 * |-------|-----|------|------|-----|------  \n
	 * | STOP  | AO  | Yes  | NST  | ASTP| Yes    \n
	 * |-------|-----|------|------|-----|------  \n
	 * | CLOSE | Yes | NO   | NST  | NST | AC     \n
	 * |-------|-----|------|------|-----|------- \n
	 */
	struct flag             status;

	int                     seefd;          /**< fd of see demux device,
	                                             for common control usage*/
	enum dmx_see_id         see_dmx_id;	                                             
	int                     fd;             /**< fd of demux device,
	                                             for common control usage */
	int                     feedfd;         /**< fd of demux device,
	                                             for feed data usage */
	const char              *path;
	const char              *pathfeed;      /**< when playback, we need to feed data
	                                             to device specified by pathfeed */

	pthread_mutex_t         sect_mutex;
	pthread_mutex_t         rec_mutex;
	pthread_mutex_t         serv_mutex;
	pthread_mutex_t         feed_mutex;

	struct flag             flag;

	struct list_head        sections;       /**< list head of section channels */
	struct list_head        records;        /**< list head of record channels */
	struct list_head        services;       /**< list head of service channels */
	struct list_head        streams;        /**< list head of steam channels */

	/**
	 * For global storage.
	 * When we alloc a demux channel, we will find
	 * a dmx_channel struct from these global struct
	 * arrays if available. And add it into list_head
	 * of channel list_head sections/records/services/streams.
	 */
	struct dmx_channel      ch_sections[MAX_SECTIONS];
	struct dmx_channel      ch_records[MAX_RECORDS];
	struct dmx_channel      ch_services[MAX_SERVICES];
	struct dmx_channel      ch_streams[MAX_STREAMS];

	/**
	 * For global storage.
	 * When we alloc a filter, we will find a dmx_section_filter
	 * struct from the global struct arrays if available. And
	 * add it into list_head of section list_head filter.
	 */
	struct dmx_section_filter filters[MAX_FILTERS];

	/**
	 * Demux share library will create a pthreads for playfeed.
	 * playfeed_tid: Save playfeed pthread id number. When we playback
	 *               media stream from local storage, this pthread will
	 *               get ts data from user, and then write the data into
	 *               demux driver.
	 */
	pthread_t               playfeed_tid;

	/**
	 * The poll_reactor will run a thread to poll file descriptors for channels.
	 * User registers callback functions for section, record or service channels
	 * section: Packet read from demux driver for each channel are passed to user
	 *          via the registered callback function\n\n
	 * record:  Packet read from demux driver for each channel are passed to user
	 *          via the registered callback function. For record, the DMX SL
	 *          will not malloc memory to store the packet. It will simply call
	 *          registered callback function to get buffer from user, and update
	 *          the buffer information when packet is read into the buffer. \n\n
	 * service: Packet read from demux driver for each channel are passed to user
	 *          via the registered callback function. User can implement how to
	 *          process the data. The difference of record pthread is that with
	 *          record several pids of stream may be recorded in one channel.
	 *          But service usually process one pid of stream for one channel. \n\n
	 */
	struct dmx_pollreactor poll_reactor;

	struct dmx_playback_param   playback_param; /**< playback parameters */

	/**
	 * For a STREAM CHANNEL or a SERVICE CHANNEL,
	 * this parameter should be set.
	 * As a reference, we could find out that demux driver
	 * will check front end type, and the front end type
	 * is defined like below:\n
	 * #define SW_TYPE_PROJECT 0x00100000
	 * #define PROJECT_FE_DVBS  (SW_TYPE_PROJECT + 0x101)
	 * #define PROJECT_FE_DVBS2 (SW_TYPE_PROJECT + 0x105)
	 */
	uint32_t               front_end;

	/**
	 * For a STREAM CHANNEL or a SERVICE CHANNEL,
	 * this parameter should be set. User could get its
	 * value by nim's function interface.
	 */
	uint32_t               nim_chip_id;

	enum dmx_avsync_mode   avsync;

	bool                   fastcopy;

	uint32_t               open_cnt;

	unsigned char          video_id;//send video data the video decoder

	bool                   av_start;
	//add for multi-process rec&playback support, invalid value should be -1
	unsigned int sl_dmx_dsc_id;
	unsigned int ali_pvr_de_hdl; 
	uint8_t *section_buffer; //store section data read from driver
} dmx_device_t;

#if 0
/* 0x04c11db7 = 1'0000'0100'1100'0001'0001'1101'1011'0111b
 <= x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 
#define MG_CRC_32_CCITT		0x04c11db7
*/
static unsigned int crc32_table_element(unsigned int crc, unsigned char *bufptr, int len)
{
    int i=0;

    while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)(*bufptr) << 24;
		bufptr++;
		for(i=0;i<8;i++)
		{
			if(crc&0x80000000)	/*Highest bit procedure*/
				crc = (crc << 1) ^ (0x04c11db7);
			else
				crc <<= 1;
		}
	}
	return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
}

static void crc32_table_init()
{
    int cnt = 0;
    unsigned char zero = 0;
    
    for(cnt=0;cnt<256;cnt++)
    {
        crc32_table[cnt]=crc32_table_element(cnt<<24, &zero, 1);
    }
    printf("%s -> crc32_table content: \n", __FUNCTION__);
    for(cnt=1;cnt<257;cnt++)
    {
        printf("0x%08x, ", crc32_table[cnt-1]);
        if(cnt%8 == 0)
            printf("\n");
    }
}
#endif
static unsigned int ccitt_crc32_table[256] =
{
0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 
0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 
0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75, 
0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd, 
0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 
0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 
0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95, 
0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 
0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072, 
0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 
0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 
0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba, 
0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 
0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 
0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 
0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a, 
0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 
0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53, 
0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b, 
0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 
0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b, 
0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 
0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 
0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3, 
0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 
0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 
0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec, 
0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 
0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 
0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 
0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 
0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};
#ifdef __cplusplus
}
#endif

#endif /* __ALISLDMX_INTERNEL_H__ */
