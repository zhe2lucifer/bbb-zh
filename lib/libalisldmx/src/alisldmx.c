/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldmx.c
 *  @brief              ALi demux interfaces. All applications and other
 *                      function should only access hardware or driver by
 *                      this function interface.
 *
 *  @version            1.0
 *  @date               06/15/2013 09:29:04 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */
/****************************INCLUDE HEAD FILE************************************/
/* system headers */
#include <alidefinition/adf_basic.h>
#include <alidefinition/adf_pvr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/epoll.h>
#include <linux/version.h>
#include <poll.h>

/* ali driver headers */
#include <ali_basic_common.h>
#include <ali_dmx_common.h>
#include <ali_dsc_common.h>
#include <linux/Ali_DmxLibInternal.h>

/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alisldmx.h>
#include <bits_op.h>
#include <flag_op.h>

/* local headers */
#include "internal.h"

/****************************LOCAL MACRO******************************************/
#define MAX_POLL_CHANNELS 128
#define PLAYFEED_MIN_AVAILABLE_BUF (47 * 1024 + 2 * 188)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define DMX_DEV_CNT_MAX  6  /* dmx device count of max support currently */

/****************************LOCAL TYPE*******************************************/
typedef struct dev_demux {
	enum dmx_id  id;
	const char  *path;
	const char  *pathfeed; /**< when playback, we need to feed data to device specified by pathfeed */
} dev_demux_t;

/* If the driver support more DMX, need to add it to this global array. */
struct dev_demux dev_demux[DMX_DEV_CNT_MAX] = {
    {DMX_ID_DEMUX0, "/dev/ali_m36_dmx_0",    NULL},
    {DMX_ID_DEMUX1, "/dev/ali_m36_dmx_1",    NULL},
    {DMX_ID_DEMUX2, "/dev/ali_m36_dmx_2",    NULL},
    {DMX_ID_DEMUX3, "/dev/ali_m36_dmx_3",    NULL},
    {DMX_ID_SW_DEMUX0, "/dev/ali_dmx_pb_0_out", "/dev/ali_dmx_pb_0_in"},
    {DMX_ID_SW_DEMUX1, "/dev/ali_dmx_pb_1_out", "/dev/ali_dmx_pb_1_in"}
};

/****************************LOCAL VAR********************************************/
static struct dmx_device *m_dev[DMX_DEV_CNT_MAX] = {0};
static pthread_mutex_t m_mutex                   = PTHREAD_MUTEX_INITIALIZER;
//Usually,alisldmx_free_filter will be blocked until the user callback of this file returns.
static int BLOCK_CLOSE_FLT_WHILE_USR_CB_NOT_RETURN = 1;
/****************************LOCAL FUNC DECLARE***********************************/
static int  alisldmx_poll_start(struct dmx_device *dev);
static int  alisldmx_poll_stop(struct dmx_device *dev);
static int  alisldmx_poll_register(struct dmx_device *dev, struct dmx_channel *ch);
static int  alisldmx_poll_unregister(struct dmx_device *dev, struct dmx_channel *ch);

/****************************MODULE IMPLEMENT*************************************/
static alisl_retcode alisldmx_construct(alisl_handle *handle)
{
	struct dmx_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		SL_ERR("malloc memory failed!\n");
		return ERROR_NOMEM;
	}

	memset(dev, 0, sizeof(*dev));
	dev->sl_dmx_dsc_id = -1;
	dev->section_buffer = (uint8_t *)malloc(DMX_MAX_SECTION_SIZE);
	if (NULL == dev->section_buffer) {
		free(dev);
		dev = NULL;
		SL_ERR("malloc section memory failed!\n");
		return ERROR_NOMEM;
	}
	flag_init(&dev->status, DMX_STATUS_CONSTRUCT);

	*handle = dev;

	return 0;
}

static alisl_retcode alisldmx_destruct(alisl_handle *handle)
{
	struct dmx_device *dev = NULL;


	dev = (struct dmx_device *)(*handle);
	if (dev->section_buffer) {
		free(dev->section_buffer);
		dev->section_buffer = NULL;
	}
	free(dev);
	*handle = NULL;

	return 0;
}

static unsigned int calc_crc32(unsigned char *buf, int len)
{
    int i=0;
    unsigned int crc = 0xffffffff;
    
    for(i=0;i<len;i++)
    {
        crc=(ccitt_crc32_table[((crc >> 24)&0xff)^buf[i]]^(crc << 8))&0xffffffff;
    }
    SL_WARN("crc: %u\n", crc);
    return crc;
}

static alisl_retcode channel_stop(struct dmx_device *dev, struct dmx_channel *ch)
{
	pthread_mutex_t *mutex = NULL;

	SL_DBG("ch->type : %d, ch->status: %d\n", ch->type, ch->status);
	if (bit_test_any(ch->status, CHANNEL_STATUS_DISABLE)) {
		SL_DBG("WARNING: Channel already stopped!\n");
		return 0;
	}
	switch (ch->type) {
		case DMX_CHANNEL_SECTION:
			mutex = &dev->sect_mutex;
			break;
		case DMX_CHANNEL_RECORD:
			mutex = &dev->rec_mutex;
			break;
		case DMX_CHANNEL_SERVICE:
			mutex = &dev->serv_mutex;
			break;
		case DMX_CHANNEL_STREAM:
		default:
			break;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	/* Stop driver for this channel */
	ioctl(ch->fd, ALI_DMX_CHANNEL_STOP, 0);
	SL_DBG("out ioctl ALI_DMX_CHANNEL_STOP success\n");
	if (ch->ch_param.rec_whole_tp != 0) {
		ioctl(ch->fd, ALI_DMX_BYPASS_ALL, 0);
		SL_DBG("out ioctl ALI_DMX_BYPASS_ALL success\n");
	}

    switch (ch->type) {
        case DMX_CHANNEL_SECTION:
        case DMX_CHANNEL_SERVICE:
            alisldmx_poll_unregister(dev, ch);
            break;
        case DMX_CHANNEL_RECORD:
            if ((ch->callback.request_buffer) || (ch->callback.update_buffer))
                alisldmx_poll_unregister(dev, ch);
            break;
        default:
            break;
	}

	SL_DBG("in close ch->fd: %d success\n", ch->fd);
	close(ch->fd);
	bit_clear(ch->status, CHANNEL_STATUS_ENABLE);
	bit_set(ch->status, CHANNEL_STATUS_DISABLE);

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);
	SL_DBG("success and out!\n");
	return 0;
}

static alisl_retcode channel_start(struct dmx_device *dev, struct dmx_channel *ch)
{
	struct dmx_ts_kern_recv_info *ts_kern_recv_info;
	pthread_mutex_t *mutex = NULL;
	alisl_retcode rc = 0;
	struct dmx_see_subtitle_param subtitle;

	memset(&subtitle, 0, sizeof(struct dmx_see_subtitle_param));
	if (bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
		SL_DBG("Channel already started!\n");
        if ((ch->callback.request_buffer) || (ch->callback.update_buffer)) {
            rc = alisldmx_poll_register(dev, ch);
        }
		return 0;
	}

	if (ch->nb_pid == 0 && ch->ch_param.rec_whole_tp == 0) {
		SL_ERR("Channel pid not set before start!\n");
		return ERROR_NOPID;
	}

    ch->need_rerequest_size = 0;
	ch->block_size = REQ_DATA_LEN;
	if((DMX_CHANNEL_RECORD == ch->type)&&(DMX_CHANNEL_OUTPUT_FORMAT_TS == ch->ch_param.output_format)){
		ch->rec_mode = DMX_REC_OUT_TS;
		if(ioctl(dev->fd, ALI_DMX_RECORD_MODE_GET, &ch->rec_mode)){
			SL_ERR("ALI_DMX_RECORD_MODE_GET fail\n");
			ch->rec_mode = DMX_REC_OUT_TS;
		}
		if(DMX_REC_OUT_BLOCK == ch->rec_mode) {
			if(ioctl(dev->fd, ALI_DMX_RECORD_BLOCKSIZE_GET, &ch->block_size)){
				SL_ERR("ALI_DMX_RECORD_BLOCKSIZE_GET fail\n");
				ch->block_size = REQ_DATA_LEN;
			}
		}
	}
	ch->fd = open(ch->path, O_RDONLY | O_NONBLOCK);
	if (ch->fd < 0) {
		SL_ERR("open %s fail\n", ch->path);
		return ERROR_OPEN;
	}

	if (ch->type == DMX_CHANNEL_STREAM) {
		if (ch->stream == DMX_STREAM_TELETEXT) {
			rc = ioctl(ch->seefd,
					   ALI_DMX_SEE_TELETXT_START,
					   &ch->ch_param.ts_param.pid_list[0]);
			if (rc < 0) {
				close(ch->fd);
				SL_ERR("ioctl ALI_DMX_SEE_TELETXT_START fail\n");
				return ERROR_CHSTART;
			}
		} else if (ch->stream == DMX_STREAM_SUBTITLE) {
			subtitle.id = 0; // subtitle device id 0:is dvb subtitle 1: isdbtcc
			subtitle.pid = ch->ch_param.ts_param.pid_list[0];
			rc = ioctl(ch->seefd,
					   ALI_DMX_SEE_SUBTITLE_START,
					   &subtitle);
			if (rc < 0) {
				close(ch->fd);
				SL_ERR("ioctl ALI_DMX_SEE_SUBTITLE_START fail\n");
				return ERROR_CHSTART;
			}
		} else if (ch->stream == DMX_STREAM_CC) {
			subtitle.id = 1; // subtitle device id 0:is dvb subtitle 1: isdbtcc
			subtitle.pid = ch->ch_param.ts_param.pid_list[0];
			rc = ioctl(ch->seefd,
					   ALI_DMX_SEE_SUBTITLE_START,
					   &subtitle);
			if (rc < 0) {
				close(ch->fd);
				SL_ERR("ioctl ALI_DMX_SEE_SUBTITLE_START fail\n");
				return ERROR_CHSTART;
			}
		}

		ts_kern_recv_info = &ch->ch_param.ts_param.kern_recv_info;
		rc = ioctl(ch->seefd, ALI_DMX_SEE_GET_TS_INPUT_ROUTINE,
				   ts_kern_recv_info);
		if (rc < 0) {
			close(ch->fd);
			SL_ERR("ioctl ALI_DMX_SEE_GET_TS_INPUT_ROUTINE fail\n");
			return ERROR_CHSTART;
		}
	}
	if((DMX_CHANNEL_RECORD ==ch->type) /*&&(0 == ch->ch_param.enc_para)*/&&(dev->sl_dmx_dsc_id!=-1)){
		SL_DBG("It should be non-ca process of multi-process re-en record success\n");
		ch->ch_param.m_dmx_dsc_param_mode = DMX_DSC_PARAM_INTERNAL_ID_MODE;
		ch->ch_param.enc_para = (void *)dev->sl_dmx_dsc_id;
	}
	rc = ioctl(ch->fd, ALI_DMX_CHANNEL_START, &ch->ch_param);
	if (rc < 0) {
		close(ch->fd);
		SL_ERR("ioctl ALI_DMX_CHANNEL_START fail\n");
		return ERROR_CHSTART;
	}

	if (ch->ch_param.rec_whole_tp != 0) {
		ioctl(ch->fd, ALI_DMX_BYPASS_ALL, 1);
	}

	if (ch->type == DMX_CHANNEL_STREAM) {
		ioctl(ch->fd, ALI_DMX_RESET_BITRATE_DETECT, 0);
	}

	switch (ch->type) {
		case DMX_CHANNEL_SECTION:
			mutex = &dev->sect_mutex;
			break;
		case DMX_CHANNEL_RECORD:
			mutex = &dev->rec_mutex;
			break;
		case DMX_CHANNEL_SERVICE:
			mutex = &dev->serv_mutex;
			break;
		case DMX_CHANNEL_STREAM:
		default:
			break;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	bit_clear(ch->status, CHANNEL_STATUS_DISABLE);
	bit_set(ch->status, CHANNEL_STATUS_ENABLE);

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

    switch (ch->type) {
        case DMX_CHANNEL_SECTION:
        case DMX_CHANNEL_SERVICE:
            rc = alisldmx_poll_register(dev, ch);
            break;
        case DMX_CHANNEL_RECORD:
            if ((ch->callback.request_buffer) || (ch->callback.update_buffer)) {
                rc = alisldmx_poll_register(dev, ch);
            } else {
                SL_DBG("no callback registered for %d %d\n", ch->fd, ch->type);
            }
            break;
        default:
            break;
    }
	if((0==rc)&&(DMX_CHANNEL_RECORD ==ch->type) /*&&(0 == ch->ch_param.enc_para)*/&&(dev->sl_dmx_dsc_id!=-1)){
		SL_DBG("multi-process re-en record success, reset sl_dmx_dsc_id to -1.\n");
		dev->sl_dmx_dsc_id = -1;
	}
	return rc;
}

static bool section_hit(struct dmx_section_filter *filter, uint8_t *buf)
{
	int i;
	uint8_t filt, mask, mode, mode_r, value;
	bool res1, res2, first;

	if (!bit_test_any(filter->status, FILTER_STATUS_ENABLE))
		return false;

	/*
	 * res1 is the check result of mode bits equal 1
	 * res2 is the check result of mode bits equal 0
	 * first is the check result of "is there any mode bits equal 0?"
	 */
	res1 = true;
	res2 = true;
	first = true;

	for (i = 0; i < filter->size; i++) {
		filt  = filter->filter_value[i];
		mask  = filter->filter_mask[i];
		mode  = filter->filter_mode[i];
		mode_r = ~mode;
		value = buf[i];

		mode  &= mask;
		mode_r &= mask;

		/*
		 * for mode bits equal 1, all bits need to equal,
		 * else section hit fail
		 */
		if ((mode & value) != (mode & filt))
			res1 = false;

		/*
		 * This condition tells us there is at least one mode bit equel 0
		 * else no mode bit equal 0, res2 must be "true"
		 */
		if (mode_r && first) {
			res2 = false;
			first = false;
		}

		/*
		 * for mode bits equal 0, there should be at least one bit un-equal
		 * else section hit fail unless no mode bit equal 0
		 */
		if ((mode_r & value) != (mode_r & filt))
			res2 = true;
	}

	return (res1 && res2);
}
/*
static void sl_dmx_dump_data(char *buf, int datalen, int dumplen)
{
	int len = 0;
	int i = 0;
	len = dumplen > datalen ? datalen : dumplen;
	printf("section[%p] len[%d] data:", buf, len);
	for(i = 0; i < 16; i++) {
		printf("%02x ", (unsigned char)buf[i]);
	}
	printf("\n");
}
*/
/*
There are too much coincidence in section stress test.
Because we unlock the section mutex b4 calling user callback function(req & update),
so once we get section mutex again, we should check whether the channel and filter is still the
same as the channel & filter before we unlock. Because the channel and filter may be changed
totally after unlocking the section mutex.
*/
static unsigned int sd_cnt = 0;
#define SESSION_ID (0x66668888)
static void section_dispatch(struct dmx_device *dev, struct dmx_channel *ch)
{
	uint8_t *tmp_buffer = NULL;
	volatile struct dmx_section_filter *filter = NULL;
	struct dmx_channel_callback *callback = NULL;
	struct dmx_buffer *buffer = NULL; 
	int nbytes = -1;
	struct statfs stbuf;
    struct list_head *head = NULL;
    uint32_t ch_id = 0;
    uint32_t flt_id = 0;
    unsigned char crc_wrong = 0;
    unsigned char crc_calc = 0;
    void *priv = NULL;

	pthread_mutex_lock(&dev->sect_mutex);
	sd_cnt++;
	if (!bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
		goto quit;
	}

	if (fstatfs(ch->fd, &stbuf) != 0) {
		goto quit;
	}

    buffer = &ch->buffer;
    tmp_buffer = dev->section_buffer;
    if (!tmp_buffer) {
    	SL_WARN("should never happen\n");
    	goto quit;
    }
	memset(tmp_buffer, 0, DMX_MAX_SECTION_SIZE);
	nbytes = read(ch->fd, tmp_buffer, DMX_MAX_SECTION_SIZE);
	if (nbytes <= 0) {
		goto quit;
	}
	
	//init access flag for next section_dispatch
    filter = NULL;
    list_for_each_entry(filter, &ch->filter, node)
    {
        filter->access = false;
    }
	
	head = &ch->filter;
	filter = list_entry(head->next, typeof(*filter), node);
    while(&filter->node != head)
    {
        /*
        * some filter nodes would have been processed successfully during the first iteration.
        * We may iterate from head again for exception(the accessing filter node is free)
        * and don`t want to process them again.
        */
        /*
        * filter is going to be closed, so we should not call its callback.
        */
        if ((!bit_test_any(filter->status, FILTER_STATUS_ENABLE))||(filter->access)
                  ||(filter->wait_close)) {
            SL_WARN("**C %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
            filter = list_entry(filter->node.next, typeof(*filter), node);
            SL_WARN("**N %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
            continue;
        }

        if(!section_hit((struct dmx_section_filter *)filter, tmp_buffer)) {
            SL_WARN("filter@%u not hit\n", filter->id);
            filter->access=true;
            filter = list_entry(filter->node.next, typeof(*filter), node);
            SL_WARN("**h %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
            continue;
        }
        //crc32 check
        if(filter->crc_check) {
            if(0 == crc_calc) {
                if(calc_crc32(tmp_buffer, nbytes)) {
                    SL_WARN("section data maybe wrong\n");
                    crc_wrong = 1;
                }
                crc_calc = 1;
            }
            if(crc_wrong) {
                SL_WARN("filter@%u not hit for crc check wrong\n", filter->id);
                filter->access=true;
                filter = list_entry(filter->node.next, typeof(*filter), node);
                SL_WARN("**crc %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
                continue;
            }
        }
        ch_id  = ch->id;
        flt_id = filter->id;
        callback = &((struct dmx_section_filter *)filter)->callback;
        memset(buffer, 0, sizeof(*buffer));
        if (callback->request_buffer) {
            alisldmx_channel_requestbuf_callback tmp_reqcb = callback->request_buffer;
            priv = callback->priv;
            filter->flag = FLT_FLAG_CB;
            filter->session_id = SESSION_ID;
            ch->session_id = SESSION_ID;
            //SL_WARN("call request cb\n");
            pthread_mutex_unlock(&dev->sect_mutex);
            tmp_reqcb(priv, ch_id, flt_id, DMX_MAX_SECTION_SIZE, &buffer->buf, &buffer->size);
            filter->flag = FLT_FLAG_DISPATCH;
            pthread_mutex_lock(&dev->sect_mutex);
            //SL_WARN("return request cb\n");
            // check whether user free channel and filter after calling callback
            if (!bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
                //if channel is dead and return
                SL_WARN("so weird, channel@%u is dead\n", ch_id);
                goto quit;
            } else {
                if (ch->session_id != SESSION_ID) {
                    SL_WARN("so weird, channel@%u is changed\n", ch_id);
                    goto quit;
                }
                ch->session_id = ~SESSION_ID;
            }
            if (filter->session_id == SESSION_ID) {
                if (!(bit_test_any(filter->status, FILTER_STATUS_ENABLE))) {
                    if (FILTER_STATUS_NONE == filter->status){
                        //iterate from channel head again
                        SL_WARN("so weird, filter@%u is dead\n", flt_id);
                        filter = list_entry(head->next, typeof(*filter), node);
                        SL_WARN("**fd %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                        filter->status,
                        filter->access,filter->wait_close,&filter->node, head);
                        filter->session_id = ~SESSION_ID;
                        continue;
                    } else {
                        SL_WARN("so weird, filter@%u is stopped\n", flt_id);
                        filter = list_entry(filter->node.next, typeof(*filter), node);
                        SL_WARN("**fs %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                        filter->status,
                        filter->access,filter->wait_close,&filter->node, head);
                        filter->session_id = ~SESSION_ID;
                        continue;
                    }
                }
            } else {
                SL_WARN("so weird, flt@%u is changed\n", ch_id);
                filter = list_entry(head->next, typeof(*filter), node);
                SL_WARN("**csf %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
                filter->session_id = ~SESSION_ID;
                continue;
            }
            //check user buf and size
			if ((DMX_MAX_SECTION_SIZE != buffer->size) ||(NULL == buffer->buf)) {
				SL_WARN("why come here? it never allowed.");
                filter = list_entry(filter->node.next, typeof(*filter), node);
                SL_WARN("**no %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
                continue;
            }
            memset(buffer->buf, 0, DMX_MAX_SECTION_SIZE);
            buffer->size = nbytes;
            memcpy(buffer->buf, tmp_buffer, nbytes);
		}
		
		if (callback->update_buffer) {
            alisldmx_channel_updatebuf_callback tmp_uptcb = callback->update_buffer;
            priv = callback->priv;
            filter->flag = FLT_FLAG_CB;
            filter->session_id = SESSION_ID;
            ch->session_id = SESSION_ID;
            //SL_WARN("call updata cb\n");
			pthread_mutex_unlock(&dev->sect_mutex);
            tmp_uptcb(priv, ch_id, flt_id, nbytes, 0xffff);
            filter->flag = FLT_FLAG_DISPATCH;
            pthread_mutex_lock(&dev->sect_mutex);            
            //SL_WARN("return from updata cb\n");
            
            // check whether user free channel and filter after calling callback
            if(!bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
                //if channel is dead and return
                SL_WARN("so weird, channel@%u is dead\n", ch_id);
                goto quit;
            } else {
                if (ch->session_id != SESSION_ID) {
                    SL_WARN("so weird, channel@%u is changed\n", ch_id);
                    goto quit;
                }
                ch->session_id = ~SESSION_ID;
            }
            if (filter->session_id == SESSION_ID) {
                if (!(bit_test_any(filter->status, FILTER_STATUS_ENABLE))) {
                    if (FILTER_STATUS_NONE == filter->status){
                        //iterate from channel head again
                        SL_WARN("so weird, filter@%u is dead\n", flt_id);
                        filter = list_entry(head->next, typeof(*filter), node);
                        SL_WARN("**fD %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                        filter->status,
                        filter->access,filter->wait_close,&filter->node, head);
                        filter->session_id = ~SESSION_ID;
                        continue;
                    } else {
                        SL_WARN("so weird, filter@%u is stopped\n", flt_id);
                        filter = list_entry(filter->node.next, typeof(*filter), node);
                        SL_WARN("**fS %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                        filter->status,
                        filter->access,filter->wait_close,&filter->node, head);
                        filter->session_id = ~SESSION_ID;
                        continue;
                    }
                }
            } else {
                SL_WARN("so weird, flt@%u is changed\n", ch_id);
                filter = list_entry(head->next, typeof(*filter), node);
                SL_WARN("**CSF %d-0x%08x-%u-%d-%d-%d-%p-%p\n", sd_cnt, ch->id, filter->id,
                filter->status,
                filter->access,filter->wait_close,&filter->node, head);
                filter->session_id = ~SESSION_ID;
                continue;
            }
		}

		/* with filter, but one shot */
		/* when "callback->update_buffer == NULL", the filter must be free */
		if ((bit_test_any(filter->status, FILTER_STATUS_ENABLE)) && (filter->continuous == 0)) {
			flag_bit_set(&dev->flag, 1 << DMX_CHID2INDEX(ch->id));
			/* Stop driver for this channel */
			bit_clear(filter->status, FILTER_STATUS_ENABLE);
			bit_set(filter->status, FILTER_STATUS_DISABLE);
		}
        //set filter node access flag when it is processed successfully 
        filter->access = true;
        filter = list_entry(filter->node.next, typeof(*filter), node);
	}
    
quit:
	pthread_mutex_unlock(&dev->sect_mutex);
	return ;
}

/**
 *  Function Name:      alisldmx_section_poll
 *  @brief              poll which section channel is hit.
 *
 *  @param handle       pointer to module handle
 *  @param id_mask      Specify which channel to poll
 *  @param timeout      timeout
 *  @param[out] flag    result flag
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_poll(alisl_handle handle, uint32_t id_mask,
									uint32_t timeout, uint32_t *flag)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc;

	*flag = 0;
	rc = flag_wait_bit_any(&dev->flag, id_mask | SECTION_FLAG_STOPPOLL, timeout);

	if (rc) {
		return rc;
	} else {
		*flag = (dev->flag.flg & id_mask);
		return 0;
	}
}

/**
 *  Function Name:      alisldmx_section_poll_release
 *  @brief              poll release, means stop poll
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_poll_release(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	flag_bit_set(&dev->flag, SECTION_FLAG_STOPPOLL);

	return 0;
}

/**
 *  Function Name:      alisldmx_section_poll_reset
 *  @brief              poll reset, means ready for next poll
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_poll_reset(alisl_handle handle)
{
    struct dmx_device *dev = (struct dmx_device *)handle;

    flag_bit_clear(&dev->flag, SECTION_FLAG_STOPPOLL);

    return 0;
}

/**
 *  Function Name:      alisldmx_section_request_poll
 *  @brief              poll which section channel is hit. \n
 *                      It's very similar to alisldmx_section_poll
 *
 *  @param handle       pointer to module handle
 *  @param id_mask      Specify which channel to poll
 *  @param timeout      timeout
 *  @param[out] flag    result flag
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_request_poll(alisl_handle handle, uint32_t id_mask,
                                            uint32_t timeout, uint32_t *flag)
{
    struct dmx_device *dev = (struct dmx_device *)handle;
    alisl_retcode rc;

    *flag = 0;
    rc = flag_wait_bit_any(&dev->flag, id_mask | SECTION_FLAG_STOPREQUEST, timeout);

    if (rc) {
        return rc;
    } else {
        *flag = (dev->flag.flg & id_mask);
        flag_bit_clear(&dev->flag, id_mask | SECTION_FLAG_STOPREQUEST);
        return 0;
    }
}

/**
 *  Function Name:      alisldmx_section_request_poll_release
 *  @brief              poll release, means stop request poll. \n
 *                      It's very similar to alisldmx_section_poll_release
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/22/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_section_request_poll_release(alisl_handle handle)
{
    struct dmx_device *dev = (struct dmx_device *)handle;

    flag_bit_set(&dev->flag, SECTION_FLAG_STOPREQUEST);

    return 0;
}

#define READ_THROW_SIZE (1504) //8*188, we can read 32 times to reach 47k.
#define READ_TIMES (32)
#define THROW_THRESHOLD (2566952) //about 3850240*2/3
static char throw_buf[READ_THROW_SIZE] = {0}; //348*188
static uint32_t record_dispatch(struct dmx_device *dev, struct dmx_channel *ch)
{
    struct dmx_buffer *buffer = &ch->buffer;
    struct dmx_channel_callback *callback = &ch->callback;
    uint32_t remain_size, request_size, remain_throw_size, throw_read;
    ssize_t nbytes = 0;
	unsigned char header_pkt[188] = {0};
    PVR_RECORD_HEADER_PKT *tmp_pkt = (PVR_RECORD_HEADER_PKT *)header_pkt;
    struct statfs stbuf;

    pthread_mutex_lock(&dev->rec_mutex);

    if (!bit_test_any(ch->status, CHANNEL_STATUS_ENABLE))
        goto quit;  /* skip not running channel */

    if (fstatfs(ch->fd, &stbuf) != 0)
        goto quit;  /* skip invalid fd */

    remain_size = buffer->size - buffer->write;
    request_size = ch->block_size;
    if (buffer->buf == NULL) {
        if (ch->ch_param.output_format == DMX_CHANNEL_OUTPUT_FORMAT_PES) {
            ioctl(ch->fd, ALI_DMX_CHANNEL_GET_CUR_PKT_LEN, &request_size);
        } else {
            /*when channel is uncache, there is not a TS packet on the begining of a block*/
            if (ch->ch_param.uncache_para == 0) {
                if (ch->need_rerequest_size) {
                    request_size = ch->need_rerequest_size;
                } else {
                    // Demux driver will insert a TS packet to the begining of a block to pass the iframe offset to user space.
                    // So we should find this packet first. Both FTA and re-encrypt record.
                    memset(tmp_pkt, 0, sizeof(PVR_RECORD_HEADER_PKT));
                    while (PVR_RECORD_BLOCK_HEADER_PKT_SYNC != tmp_pkt->sync) {
                        nbytes = read(ch->fd, (void *)tmp_pkt, 188);
                        if (188 != nbytes)
                        {
                            goto quit;
                        }
                    }
                    request_size = tmp_pkt->pkt_num * 188;                
                }
            }
        }

        if (callback->request_buffer) {
            callback->request_buffer(callback->priv, ch->id, -1, request_size, &buffer->buf, &buffer->size);
        }

        if (buffer->size == 0 || buffer->buf == NULL) {
            memset(buffer, 0, sizeof(*buffer));
            ch->need_rerequest_size = request_size;
            if (ch->ch_param.output_format == DMX_CHANNEL_OUTPUT_FORMAT_TS) {
                //system must be in abnormal status for low speed usb disk
                uint32_t dmx_remain_buf = 0;
                ioctl(ch->fd, ALI_DMX_CHANNEL_GET_CUR_PKT_LEN, &dmx_remain_buf);
                if (dmx_remain_buf >= THROW_THRESHOLD) {
                    //Throw by request_size.
                    remain_throw_size = request_size;
                    while (remain_throw_size > 0) {
                        nbytes = 0;
                        if (remain_throw_size > READ_THROW_SIZE) {
                            throw_read = READ_THROW_SIZE;
                        } else {
                            throw_read = remain_throw_size;
                        }
                        nbytes = read(ch->fd, throw_buf, throw_read);
                        if (nbytes != throw_read) {
                            SL_WARN("%s -> req fail and throw rec data %d\n", __func__, nbytes);
                            break;
                        }
                        remain_throw_size -= nbytes;
                    }
                    SL_ERR("** TLWS %u", request_size);
                    ch->need_rerequest_size = 0;
                }
            }
            goto quit;
        }
        buffer->write = 0;
        buffer->ifm_offset = tmp_pkt->ifm_offset;
        remain_size = buffer->size;
    }

    if (dev->fastcopy == true) {
        nbytes = read(ch->fd, buffer->buf, remain_size);
    } else {
        nbytes = read(ch->fd, buffer->buf + buffer->write, remain_size);
    }

    if (nbytes <= 0) {
        nbytes = 0;
        goto quit;
    }

    buffer->write += nbytes;
    remain_size = buffer->size - buffer->write;
    /*
    if buffer is full, we should call user callback immediately.
    For unache mode(AUI_DMX_DATA_RAW), we should call user callback immediately.
    */
    if (((buffer->buf != NULL) && (0==remain_size))
         || ch->ch_param.uncache_para) {
        /* Buffer is full */
        if (callback->update_buffer) {
            callback->update_buffer(callback->priv, ch->id, -1, buffer->write, buffer->ifm_offset);
        }
        memset(buffer, 0, sizeof(*buffer));
    }
    
quit:
    pthread_mutex_unlock(&dev->rec_mutex);
    return nbytes;
}

static void service_dispatch(struct dmx_device *dev, struct dmx_channel *ch)
{
    struct dmx_buffer *buffer = &ch->buffer;
    struct dmx_channel_callback *callback = &ch->callback;
    uint32_t remain_size, request_size;
    ssize_t nbytes;
    struct statfs stbuf;
    uint32_t ch_id = 0;
    void *priv = NULL;

    pthread_mutex_lock(&dev->serv_mutex);

    if (!bit_test_any(ch->status, CHANNEL_STATUS_ENABLE))
        goto quit;  /* skip not running channel */

    if (fstatfs(ch->fd, &stbuf) != 0)
        goto quit;  /* skip invalid fd */

    remain_size = buffer->size - buffer->write;
    if (buffer->buf == NULL) {
        if (ch->ch_param.output_format == DMX_CHANNEL_OUTPUT_FORMAT_PES) {
            ioctl(ch->fd, ALI_DMX_CHANNEL_GET_CUR_PKT_LEN, &request_size);
        } else {
            /*
             * WARNING: Usually never reach here.
             * Seams every service's output_format is PES packet.
             */
            request_size = REQ_DATA_LEN;
        }
        
        if (callback->request_buffer) {
            alisldmx_channel_requestbuf_callback tmp_reqcb = callback->request_buffer;
            ch_id = ch->id;
            priv = callback->priv;
            pthread_mutex_unlock(&dev->serv_mutex);
            tmp_reqcb(priv, ch_id, -1, request_size, &buffer->buf, &buffer->size);
            pthread_mutex_lock(&dev->serv_mutex);
            if(CHANNEL_STATUS_NONE == ch->status)
            {
                //if channel is dead and return
                SL_ERR("so weird, channel@%u is dead\n", ch_id);
                goto quit;
            }
        }

        if (buffer->buf == NULL || buffer->size == 0) {
            memset(buffer, 0, sizeof(*buffer));
            goto quit;
        }

        buffer->write = 0;
        remain_size = buffer->size;
    }

    nbytes = read(ch->fd, buffer->buf + buffer->write, remain_size);
    if (nbytes <= 0) {
        goto quit;
    }

    buffer->write += nbytes;
    /*
    if we got full pes package, we should call user callback immediately for subtitle avsync.   
    */
    remain_size = buffer->size - buffer->write; 
    if (buffer->buf != NULL && remain_size == 0) {
        /* Buffer is full */
        if (callback->update_buffer) {
            alisldmx_channel_updatebuf_callback tmp_uptcb = callback->update_buffer;
            priv = callback->priv;
            ch_id = ch->id;


            pthread_mutex_unlock(&dev->serv_mutex);
            tmp_uptcb(priv, ch_id,-1,buffer->write, 0xffff);
            pthread_mutex_lock(&dev->serv_mutex);
            if(CHANNEL_STATUS_NONE == ch->status)
            {
                //if channel is dead and return
                SL_ERR("so weird, channel@%u is dead\n", ch_id);
                goto quit;
            }
        }

        memset(buffer, 0, sizeof(*buffer));
    }
quit:
    pthread_mutex_unlock(&dev->serv_mutex);
    return ;
}

static void *alisldmx_poll_pthread(void *arg)
{
    struct dmx_device *dev = (struct dmx_device *)arg;
    struct dmx_channel *ch;
    int n, i;
    struct epoll_event evlist[MAX_POLL_CHANNELS];

    for (;;) {
        flag_wait_bit_any(&dev->status, DMX_STATUS_START, -1);

        n = epoll_wait(dev->poll_reactor.fd, evlist, MAX_POLL_CHANNELS, 2000);
        if (n <= 0) {
            continue;
        }

        for (i = 0; i < n; i++) {
            if (evlist[i].events & EPOLLIN) {

                ch = (struct dmx_channel *) evlist[i].data.ptr;
                if (!ch) {
                    continue;
                }

                if (ch->type == DMX_CHANNEL_SECTION) {
                    section_dispatch(dev, ch);
                }
                else if (ch->type == DMX_CHANNEL_RECORD) {
                    record_dispatch(dev, ch);
                }
                else if (ch->type == DMX_CHANNEL_SERVICE) {
                    service_dispatch(dev, ch);
                }
            }
        }
    }

	return NULL;
}

static int  alisldmx_poll_start(struct dmx_device *dev)
{
	pthread_attr_t attr;
	int ret = 0;

	dev->poll_reactor.fd = epoll_create1(EPOLL_CLOEXEC);
	if (dev->poll_reactor.fd == -1) {
		SL_ERR("epoll_create1 failed\n");
		return -1;
	}

	ret = pthread_mutex_init(&dev->poll_reactor.mutex, NULL);
	if (ret != 0) {
		SL_ERR("pthread_mutex_init failed\n");
		goto fail1;
	}

	pthread_attr_init(&attr);
	//pthread_attr_setstacksize(&attr, STACKSIZE); /* Need to delete it, because set stack size may cause Android JVM error. */
	//pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&dev->poll_reactor.tid, &attr, alisldmx_poll_pthread, (void *)dev);
	if (ret != 0) {
		SL_ERR("pthread_create poll thread failed: %s\n", strerror(errno));
		goto fail2;
	}

	goto exit;

fail2:
	pthread_mutex_destroy(&dev->poll_reactor.mutex);
fail1:
	close(dev->poll_reactor.fd);
exit:
        pthread_attr_destroy(&attr);
	return ret;
}

static int  alisldmx_poll_stop(struct dmx_device *dev)
{
	int ret = 0;

	pthread_cancel(dev->poll_reactor.tid);
	pthread_join(dev->poll_reactor.tid, NULL);

	close(dev->poll_reactor.fd);
	dev->poll_reactor.fd = -1;
	pthread_mutex_destroy(&dev->poll_reactor.mutex);

	return ret;
}

static int  alisldmx_poll_register(struct dmx_device *dev, struct dmx_channel *ch)
{
	struct epoll_event ev;
	int ret;

	pthread_mutex_lock(&dev->poll_reactor.mutex);

	ev.events = EPOLLIN;
	ev.data.ptr = ch;

	ret = epoll_ctl(dev->poll_reactor.fd, EPOLL_CTL_ADD, ch->fd, &ev);
	if (-1 == ret) {
		pthread_mutex_unlock(&dev->poll_reactor.mutex);
		return -1;
	}

	pthread_mutex_unlock(&dev->poll_reactor.mutex);

	return 0;
}

static int  alisldmx_poll_unregister(struct dmx_device *dev, struct dmx_channel *ch)
{
	int ret;

	pthread_mutex_lock(&dev->poll_reactor.mutex);

	ret = epoll_ctl(dev->poll_reactor.fd, EPOLL_CTL_DEL, ch->fd, NULL);
	if (-1 == ret) {
		pthread_mutex_unlock(&dev->poll_reactor.mutex);
		return -1;
	}

	pthread_mutex_unlock(&dev->poll_reactor.mutex);

	return 0;
}

static void *alisldmx_playfeed_pthread(void *arg)
{
	struct dmx_device *dev = (struct dmx_device *)arg;
	struct dmx_playback_param *pb = &dev->playback_param;
	struct dmx_fast_copy_param fcp;
	struct dmx_buffer buffer;
	ssize_t nbytes, count;
	bool idle = true;
	bool fastcopy = false;
	int32_t pkt_cnt = 0;

	memset(&buffer, 0, sizeof(buffer));

	for (;;) {
		if (flag_bit_test_any(&dev->status, DMX_STATUS_IDLE_P1)) {
			flag_bit_clear(&dev->status, DMX_STATUS_IDLE_P1);
			flag_bit_set(&dev->status, DMX_STATUS_IDLE_P2);
			flag_wait_bit_any(&dev->status, DMX_STATUS_IDLE_P3, -1);
			flag_bit_clear(&dev->status, DMX_STATUS_IDLE_P3);
		}

		flag_wait_bit_any(&dev->status, DMX_STATUS_START, -1);

		if (idle == true) {
			usleep(10000);
		}
		idle = true;

		pthread_mutex_lock(&dev->feed_mutex);

		ioctl(dev->feedfd, ALI_DMX_HW_GET_FREE_BUF_LEN, &pkt_cnt);
		if (pkt_cnt <= PLAYFEED_MIN_AVAILABLE_BUF) {
			pthread_mutex_unlock(&dev->feed_mutex);
			continue;
		}
		if (buffer.buf == NULL || buffer.size == 0) {
			/* buffer is empty, request data. */
			if (pb->cb_feed.request_data == NULL) {
				pthread_mutex_unlock(&dev->feed_mutex);
				continue;
			}

			pb->cb_feed.request_data(pb->priv, PLAYFEED_REQ_DATA_LEN,
									 &buffer.buf, &buffer.size, &fastcopy);

			if (buffer.size == 0 ||
				(buffer.buf == NULL && dev->fastcopy == false)) {
				pthread_mutex_unlock(&dev->feed_mutex);
				continue;
			}
		}

		count = buffer.size - buffer.write;
		if (dev->fastcopy == true && fastcopy == true) {
			fcp.data = (void*)(buffer.buf + buffer.write);
			fcp.len = count;
			nbytes = ioctl(dev->feedfd, ALI_DMX_PLAYBACK_FSTCPY, &fcp);
		} else {
			nbytes = write(dev->feedfd, buffer.buf + buffer.write, count);
		}

		if (nbytes <= 0) {
			/* Maybe driver is busy, nothing is written. */
			pthread_mutex_unlock(&dev->feed_mutex);
			continue;
		}

		buffer.write += nbytes;
		if (buffer.write >= buffer.size) {
			/* All data of the buffer is written. */
			memset(&buffer, 0, sizeof(buffer));
		}
		idle = false;

		pthread_mutex_unlock(&dev->feed_mutex);
	}

	return NULL;
}

/**
 *  Function Name:      alisldmx_allocate_filter
 *  @brief              Allocate a filter for a specific channel.
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param[out] filterid Save the filter id when allocate success.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note               Some type of channel will support filter, for example:
 *                      section channel. \n
 *                      But some type of channel only support pid or pid_list.
 */
alisl_retcode alisldmx_allocate_filter(alisl_handle handle,
									   uint32_t channelid, uint32_t *filterid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_channel *ch, *tmp;
	struct list_head *head = NULL;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	uint32_t i, max;
	struct dmx_section_filter *filter;
	alisl_retcode ret = 0;

	*filterid = DMX_ILLEGAL_FILTERID;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (type != DMX_CHANNEL_SECTION) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	pthread_mutex_lock(&dev->sect_mutex);

	list_for_each_entry_safe(ch, tmp, &dev->sections, node) {
		if (ch->id == channelid) {
			head = &ch->filter;
			break;
		}
	}

	if (head == NULL) {
		SL_DBG("Can't find the specific channel\n");
		pthread_mutex_unlock(&dev->sect_mutex);
		ret = ERROR_NOCH;
		goto OUT;
	}

	max = ARRAY_SIZE(dev->filters);
	filter = dev->filters;
	for (i = 0; i < max; i++) {
		if (filter->status == FILTER_STATUS_NONE) {
			break;
		}

		filter ++;
	}

	if (i == max) {
		SL_DBG("No available filter\n");
		pthread_mutex_unlock(&dev->sect_mutex);
		ret = ERROR_NOFILTER;
		goto OUT;
	}

	filter->id = i;
	filter->status = FILTER_STATUS_DISABLE;
	*filterid = i;
	filter->p_ch = ch;

	list_add_tail(&filter->node, head);
	ch->flt_cnt++;
	SL_WARN("**A 0x%08x-%u-%d-%d-%d-%p-%p-%p-%p-%u\n",ch->id, filter->id,
				filter->status,
				filter->access,filter->wait_close,filter->node.prev,&filter->node,filter->node.next,head,ch->flt_cnt);
	pthread_mutex_unlock(&dev->sect_mutex);

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_free_filter
 *  @brief              Free a filter.
 *
 *  @param handle       pointer to module handle
 *  @param filterid     free the specific filter
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_free_filter(alisl_handle handle, uint32_t filterid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode ret = 0;

	uint32_t i;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		goto OUT;
	}

	pthread_mutex_lock(&dev->sect_mutex);
	for (i = 0; i < ARRAY_SIZE(dev->filters); i++) {
		if (dev->filters[i].id == filterid
			&& dev->filters[i].status != FILTER_STATUS_NONE) {
			break;
		}
	}

	if (i == ARRAY_SIZE(dev->filters)) {
		SL_DBG("No specific filter found!\n");
		pthread_mutex_unlock(&dev->sect_mutex);
		ret = ERROR_NOFILTER;
		goto OUT;
	}
    /*
    We do not close filter actually until flag of this filter is not FLT_FLAG_CB.
    */
    int cnt = 0;
    pthread_t tid = pthread_self();
    while((tid != dev->poll_reactor.tid)&&(FLT_FLAG_CB == dev->filters[i].flag))
    {   
        cnt++;
        dev->filters[i].wait_close = true;
        //SL_DBG("filter is in user cb@%d\n", cnt);
        pthread_mutex_unlock(&dev->sect_mutex); 
        usleep(1000);
        pthread_mutex_lock(&dev->sect_mutex);
    }
    if(cnt != 0)
    {
        SL_DBG("filter in  user cb @%d\n", cnt);
    }
	if (dev->filters[i].status != FILTER_STATUS_DISABLE) {
		//SL_WARN("filter not disabled before free!\n");
		//SL_DBG("Disable the filter now!\n");
		bit_clear(dev->filters[i].status, FILTER_STATUS_ENABLE);
		bit_set(dev->filters[i].status, FILTER_STATUS_DISABLE);
	}
	dev->filters[i].p_ch->flt_cnt--;
	SL_DBG("**ffs 0x%08x-%u-%d-%d-%d-%p-%p-%p-%p-%u\n",dev->filters[i].p_ch->id,dev->filters[i].id,
				dev->filters[i].status,
				dev->filters[i].access,dev->filters[i].wait_close,
				dev->filters[i].node.prev, &dev->filters[i].node, dev->filters[i].node.next,
				&(dev->filters[i].p_ch->filter),dev->filters[i].p_ch->flt_cnt);
	list_del(&dev->filters[i].node);

	/*
	 * Reset filter variables, so it's ready
	 * for next usage
	 */
	memset(&dev->filters[i], 0, sizeof(struct dmx_section_filter));

	pthread_mutex_unlock(&dev->sect_mutex);

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_free_filter_all
 *  @brief              Free all filters of a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    filters of the specific channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_free_filter_all(alisl_handle handle, uint32_t channelid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_channel *ch, *tmp_ch;
	struct list_head *head = NULL;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_section_filter *filter, *tmp_flt;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (type != DMX_CHANNEL_SECTION) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	pthread_mutex_lock(&dev->sect_mutex);

	list_for_each_entry_safe(ch, tmp_ch, &dev->sections, node) {
		if (ch->id == channelid) {
			head = &ch->filter;
			break;
		}
	}

	if (head == NULL) {
		SL_DBG("Can't find the specific channel\n");
		pthread_mutex_unlock(&dev->sect_mutex);
		ret = ERROR_NOCH;
		goto OUT;
	}

	list_for_each_entry_safe(filter, tmp_flt, head, node) {
		ch->flt_cnt--;
		SL_DBG("**ffa 0x%08x-%u-%d-%d-%d-%p-%p-%p-%p-%u\n",ch->id, filter->id,
				filter->status,
				filter->access,filter->wait_close,filter->node.prev,&filter->node,filter->node.next,head,ch->flt_cnt);
		list_del(&filter->node);

		/*
		 * Reset filter variables, so it's ready
		 * for next usage
		 */
		memset(filter, 0, sizeof(*filter));
	}

	pthread_mutex_unlock(&dev->sect_mutex);
OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_set_filter
 *  @brief
 *
 *  @param handle       pointer to module handle
 *  @param filterid     Specific filter
 *  @param size         size of filter/mask/mode
 *  @param filter       The value to check for section filtering
 *                      (length bytes are skipped).
 *  @param mask         The bit to check in filter value.
 *  @param mode         The filtering mode to apply on the mask
 *                      (positive or negative).
 *	@param continuous	get the section data continuously.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 *                      Example1: Positive filtering mode on table id
 *                                and Extension Id. \n
 *                      Filter: 0x42 0x00 0x02 0x00 0x00 0x00 0x00 0x00 \n
 *                      Mask:   0XFF 0XFF 0XFF 0X00 0X00 0X00 0x00 0x00 \n
 *                      Mode:   0XFF 0XFF 0XFF 0X00 0X00 0X00 0x00 0x00 \n
 *                      Result: \n
 *                      All sections with TableId 0x42 and extensionId
 *                      0x0002 will be filtered. \n
 *                      \n
 *                      Example2: Negative filtering mode on version number. \n
 *                      Filter: 0x42 0x00 0x02 0x02 0x00 0x00 0x00 \n
 *                      Mask:   0XFF 0XFF 0XFF 0X3E 0X00 0X00 0X00 \n
 *                      Mode:   0XFF 0XFF 0XFF 0XC1 0X00 0X00 0X00 \n
 *                      Result: \n
 *                      All sections with TableId 0x42, extensionId  0x0002, \n
 *                      and version number different than 0x01 will be filtered.
 */
alisl_retcode alisldmx_set_filter(alisl_handle handle,
				  uint32_t filterid, uint32_t size,
				  uint8_t *filter, uint8_t *mask, uint8_t *mode, uint8_t continuous)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	int i;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (size > DMX_MAX_FILTER_SIZE) {
		SL_DBG("Invalidate parameter value!\n");
		ret = ERROR_INVAL;
		goto OUT;
	}
	pthread_mutex_lock(&dev->sect_mutex);

	for (i = 0; i < ARRAY_SIZE(dev->filters) && i < MAX_FILTERS; i++) {
		if (dev->filters[i].id == filterid
			&& dev->filters[i].status != FILTER_STATUS_NONE) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev->filters) || i == MAX_FILTERS) {
		SL_DBG("No specific filter found!\n");
		ret = ERROR_NOFILTER;
		pthread_mutex_unlock(&dev->sect_mutex);
		goto OUT;
	}


	if (size > 0 && filter)
		memcpy(dev->filters[i].filter_value, filter, size);
	if (size > 0 && mask)
		memcpy(dev->filters[i].filter_mask, mask, size);
	if (size > 0 && mode)
		memcpy(dev->filters[i].filter_mode, mode, size);

	dev->filters[i].size = size;
	dev->filters[i].continuous = continuous;
	pthread_mutex_unlock(&dev->sect_mutex);

OUT:
	return ret;
}

alisl_retcode alisldmx_set_filter_crc_check(alisl_handle handle, uint32_t filterid, uint8_t crc_check)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	int i;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}
	pthread_mutex_lock(&dev->sect_mutex);
	for (i = 0; i < ARRAY_SIZE(dev->filters) && i < MAX_FILTERS; i++) {
		if (dev->filters[i].id == filterid
			&& dev->filters[i].status != FILTER_STATUS_NONE) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev->filters) || i == MAX_FILTERS) {
		SL_DBG("No specific filter found!\n");
		ret = ERROR_NOFILTER;
		pthread_mutex_unlock(&dev->sect_mutex);
		goto OUT;
	}


    //SL_ERR("%s -> crc_check:%d\n",__FUNCTION__, crc_check);
    dev->filters[i].crc_check = crc_check;
	pthread_mutex_unlock(&dev->sect_mutex);

OUT:
	return ret;
}

/**
 *  @brief			Register callback functions for the specified section filter
 *
 *  @param[in]		handle		demux dev handle
 *  @param[in]		filterid	the id of the filter will be configured.
 *  @param[in]		callback	the new callback functions for this filter.
 *
 *  @return			0 		- success
 *  @return			others	- failed
 *
 *  @author			Peter Pan <peter.pan@alitech.com>
 *  @date			2/5/2015  11:16:52
 *
 *  @note
 */
alisl_retcode alisldmx_register_filter_callback(alisl_handle handle,
						uint32_t filterid,
						struct dmx_channel_callback *callback)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	int i;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	pthread_mutex_lock(&dev->sect_mutex);

	for (i = 0; i < ARRAY_SIZE(dev->filters) && i < MAX_FILTERS; i++) {
		if (dev->filters[i].id == filterid
			&& dev->filters[i].status != FILTER_STATUS_NONE) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev->filters) || i == MAX_FILTERS) {
		SL_DBG("No specific filter found!\n");
		pthread_mutex_unlock(&dev->sect_mutex);
		ret = ERROR_NOFILTER;
		goto OUT;
	}

	memcpy(&dev->filters[i].callback, callback, sizeof(struct dmx_channel_callback));

	pthread_mutex_unlock(&dev->sect_mutex);

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_control_filter
 *  @brief              Reset/Enable/Disable a filter
 *
 *  @param handle       pointer to module handle
 *  @param filterid     Specific filter
 *  @param ctrl         Refer to enum dmx_control
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_control_filter(alisl_handle handle,
									  uint32_t filterid, enum dmx_control ctrl)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	int i;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}
	pthread_mutex_lock(&dev->sect_mutex);
	for (i = 0; i < ARRAY_SIZE(dev->filters) && i < MAX_FILTERS; i++) {
		if (dev->filters[i].id == filterid
			&& dev->filters[i].status != FILTER_STATUS_NONE) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(dev->filters) || i == MAX_FILTERS) {
		SL_DBG("No specific filter found!\n");
		ret = ERROR_NOFILTER;
		pthread_mutex_unlock(&dev->sect_mutex);
		goto OUT;
	}

	if (ctrl == DMX_CTRL_ENABLE) {
		bit_clear(dev->filters[i].status, FILTER_STATUS_DISABLE);
		bit_set(dev->filters[i].status, FILTER_STATUS_ENABLE);
	} else if (ctrl == DMX_CTRL_DISABLE) {
		bit_clear(dev->filters[i].status, FILTER_STATUS_ENABLE);
		bit_set(dev->filters[i].status, FILTER_STATUS_DISABLE);
	} else if (ctrl == DMX_CTRL_RESET) {
		/* TODO: What's the behavior of reset a filter?? */
	}

	pthread_mutex_unlock(&dev->sect_mutex);

OUT:
	return ret;
}

static alisl_retcode get_channel_array_info(struct dmx_device *dev,
											enum dmx_channel_type type,
											struct dmx_channel **ch_head,
											struct list_head **list_head,
											pthread_mutex_t **mutex,
											int *array_size)
{
	switch (type) {
		case DMX_CHANNEL_SECTION:
			*array_size = ARRAY_SIZE(dev->ch_sections);
			*ch_head = &dev->ch_sections[0];
			*list_head = &dev->sections;
			*mutex = &dev->sect_mutex;
			break;
		case DMX_CHANNEL_RECORD:
			*array_size = ARRAY_SIZE(dev->ch_records);
			*ch_head = dev->ch_records;
			*list_head = &dev->records;
			*mutex = &dev->rec_mutex;
			break;
		case DMX_CHANNEL_SERVICE:
			*array_size = ARRAY_SIZE(dev->ch_services);
			*ch_head = dev->ch_services;
			*list_head = &dev->services;
			*mutex = &dev->serv_mutex;
			break;
		case DMX_CHANNEL_STREAM:
			*array_size = ARRAY_SIZE(dev->ch_streams);
			*ch_head = dev->ch_streams;
			*list_head = &dev->streams;
			*mutex = NULL;
			break;
		default:
			return ERROR_CHTYPE;
	}

	return 0;
}

static struct dmx_channel * find_channel(struct list_head *head, uint32_t id)
{
	struct dmx_channel *ch, *tmp, *t;

	ch = NULL;
	list_for_each_entry_safe(tmp, t, head, node) {
		if (tmp->id == id) {
			ch = tmp;
			break;
		}
	}

	return ch;
}

static int channel_parameter_default(struct dmx_device *dev,
									 struct dmx_channel *channel)
{
	struct dmx_channel_param *param = &channel->ch_param;

	if (channel->type == DMX_CHANNEL_SECTION) {
		param->output_space = DMX_OUTPUT_SPACE_USER;
		param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_SEC;
		param->sec_param.mask_len = 1;
		param->sec_param.mask[0] = 0;
		param->sec_param.value[0] = 0;
		param->sec_param.timeout = 30;
		param->sec_param.option = 0;
	} else if (channel->type == DMX_CHANNEL_RECORD) {
		/*
		 * "param->output_format" could be set by
		 * function alisldmx_set_channel_attr. And
		 * it could be DMX_OUTPUT_FORMAT_PES or
		 * DMX_OUTPUT_FORMAT_TS. But generally speaking,
		 * we usually record ts packet other than pes packet.
		 * So we set its default value DMX_OUTPUT_FORMAT_TS.
		 */
		param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
		param->output_space = DMX_OUTPUT_SPACE_USER;
		if (dev->fastcopy == true) {
			param->fst_cpy_slot = channel->id;
		} else {
			param->fst_cpy_slot = -1;
		}
	} else if (channel->type == DMX_CHANNEL_SERVICE) {
		param->output_space = DMX_OUTPUT_SPACE_USER;
		param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_PES;
		param->fe = dev->front_end;
		param->nim_chip_id = dev->nim_chip_id;
	} else if (channel->type == DMX_CHANNEL_STREAM) {
		param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
		param->output_space = DMX_OUTPUT_SPACE_KERNEL;
		param->fe = dev->front_end;
		param->nim_chip_id = dev->nim_chip_id;
	}
        return 0;
}

/**
 *  Function Name:      alisldmx_allocate_channel
 *  @brief              Allocate a channel with specific channel type
 *
 *  @param handle       pointer to module handle
 *  @param type         Channel type, refer to enum dmx_channel_type
 *  @param[out] channelid Save the channel id when allocate success. \n
 *                        channelid = (type << 16) | realid.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_allocate_channel(alisl_handle handle,
					enum dmx_channel_type type,
					uint32_t *channelid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_channel *ch = NULL;
	struct list_head *head = NULL;
	pthread_mutex_t *mutex = NULL;
	int i = 0, max = 0;
	alisl_retcode ret = 0;

	*channelid = DMX_ILLEGAL_CHANNELID;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	for (i = 0; i < max; i++) {
		if ((ch->status == CHANNEL_STATUS_NONE) || (ch->id == 0)) {
			break;
		}
		ch++;
	}

	if (i == max) {
		SL_ERR("NO available channel found!\n");
		if (NULL != mutex)
			pthread_mutex_unlock(mutex);
		ret = ERROR_NOCH;
		goto OUT;
	}

	ch->status = CHANNEL_STATUS_DISABLE;
	ch->id = DMX_TYPEINDEX2CHID(type, i);
	ch->type = type;
	ch->seefd = dev->seefd;
	ch->path = dev->path;
	INIT_LIST_HEAD(&ch->filter);
	*channelid = ch->id;
	ch->flt_cnt = 0;
	if (type == DMX_CHANNEL_SECTION)
		flag_bit_clear(&dev->flag, 1 << DMX_CHID2INDEX(ch->id));

	channel_parameter_default(dev, ch);

	list_add_tail(&ch->node, head);

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_free_channel
 *  @brief              Free a channel.
 *
 *  @param handle       pointer to module handle
 *  @param channelid    free the specific channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_free_channel(alisl_handle handle, uint32_t channelid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct list_head *head;
	uint32_t max;
	pthread_mutex_t *mutex = NULL;
	struct dmx_channel *ch = NULL;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex,(int*)&max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	/*
	 * Make sure all of the filters are freed
	 * for next usage.
	 */
	SL_DBG("ch->type : %d, ch->status: %d\n", ch->type, ch->status);
	alisldmx_free_filter_all(handle, channelid);
	SL_DBG("alisldmx_free_filter_all success\n");
	if (mutex != NULL) {
		pthread_mutex_lock(mutex);
	}

	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		if (mutex != NULL) {
			pthread_mutex_unlock(mutex);
		}
		ret = ERROR_NOCH;
		goto OUT;
	}

	if (bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
		/* Stop driver for this channel */
		ioctl(ch->fd, ALI_DMX_CHANNEL_STOP, 0);
		SL_DBG("ioctl ALI_DMX_CHANNEL_STOP success\n");
		close(ch->fd);
		SL_DBG("close ch->fd: %d success\n", ch->fd);
		bit_clear(ch->status, CHANNEL_STATUS_ENABLE);
		bit_set(ch->status, CHANNEL_STATUS_DISABLE);
	}

	list_del(&ch->node);

	/*
	 * Reset channel variabilities, so it's ready
	 * for next usage
	 */
	memset(ch, 0, sizeof(struct dmx_channel));

	if (mutex != NULL) {
		pthread_mutex_unlock(mutex);
	}

	SL_DBG("success and out\n");

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_control_channel
 *  @brief              Reset/Enable/Disable a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param ctrl         Refer to enum dmx_control
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_control_channel(alisl_handle handle,
									   uint32_t channelid, enum dmx_control ctrl)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_channel *ch;
	struct list_head *head;
	pthread_mutex_t *mutex = NULL;
	int max;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_lock(mutex);
	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_unlock(mutex);
	if (ctrl == DMX_CTRL_ENABLE) {
		/* means start a channel */
		ret = channel_start(dev, ch);
	} else if (ctrl == DMX_CTRL_DISABLE) {
		/* means stop a channel */
		ret = channel_stop(dev, ch);
	}

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_register_channel_callback
 *  @brief              Register callback functions to a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param callback     Pointer to callback functions
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note               Usually we should register callback functions \n
 *                      when record data from demux or implement a specific \n
 *                      service or get the filtered section packet data. \n
 *                      But when we play a tream, usually the data of the \n
 *                      stream is sent to decoder. Client don't need to register \n
 *                      callback functions to get the data.
 */
alisl_retcode alisldmx_register_channel_callback(alisl_handle handle,
						 uint32_t channelid,
						 struct dmx_channel_callback *callback)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_channel *ch;
	struct list_head *head;
	pthread_mutex_t *mutex;
	int max;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		goto OUT;
	}

	memcpy(&ch->callback, callback, sizeof(struct dmx_channel_callback));

	if (mutex != NULL)
		pthread_mutex_unlock(mutex);
OUT:
	return ret;
}
//this function will define but not used if disable DEBUG in Mini build ,unless take out static
char * channel_type2str(enum dmx_channel_type type)
{
	static char un_str[32];

	memset(un_str, 0, 32);

	if (type == DMX_CHANNEL_RECORD) {
		snprintf(un_str, 32, "RECORD");
	} else if (type == DMX_CHANNEL_SECTION) {
		snprintf(un_str, 32, "SECTION");
	} else if (type == DMX_CHANNEL_SERVICE) {
		snprintf(un_str, 32, "SERVICE");
	} else if (type == DMX_CHANNEL_STREAM) {
		snprintf(un_str, 32, "STREAM");
	} else {
		snprintf(un_str, 32, "Unknown");
	}

	return un_str;
}

static alisl_retcode channel_pidlist_update(struct dmx_channel *ch,
											uint16_t *pid_list, uint32_t nb_pid)
{
	uint16_t pid;
	int i, j, nb;

	nb = 0;
	for (i = 0; i < nb_pid; i++) {
		pid = pid_list[i] & 0x1FFF;

		if (!PID_VALID(pid))
			continue;

		for (j = 0; j < nb; j++) {
			if (pid == ch->pid_list[j])
				break;
		}

		if (j >= nb) {
			ch->pid_list[nb] = pid;
			nb ++;
			if (nb >= DMX_REC_PID_LIST_MAX_LEN) {
				SL_DBG("WARNING: pid list overflow!\n");
				break;
			}
		}
	}

	ch->nb_pid = nb;

	if (nb > 0)
		return 0;
	else
		return ERROR_INVAL;
}

static alisl_retcode channel_pid_update(struct dmx_channel *ch,
										uint16_t *pid_list, uint32_t nb_pid)
{
	uint16_t pid;
	int i;

	if (nb_pid > 1) {
		SL_WARN("channel with %s type usually have "
			"no more than 1 pid!\n", channel_type2str(ch->type));
		SL_DBG("Use the first valid pid!\n");
	}

	for (i = 0; i < nb_pid; i++) {
		pid = pid_list[i] & 0x1FFF;

		if (PID_VALID(pid))
			break;
	}

	if (i < nb_pid) {
		/*
		 * The valid bits of pid is bit0 - bit 12.
		 * But demux driver will select different video
		 * decoder according to bit13 of pid.
		 * It's very strange!!
		 * So, just pass all the bits of pid to demux channel
		 * parameter.
		 */
		ch->pid_list[0] = pid_list[i];
		ch->nb_pid = 1;

		return 0;
	}

	return ERROR_INVAL;
}

/**
 *  Function Name:      alisldmx_set_channel_pidlist
 *  @brief              set pid list to a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid_list     pids to be set to the channel
 *  @param nb_pid       number of pids that to be set to the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/01/2013, Created
 *
 *  @note               In some cases, only the first valid pid in
 *                      pid_list is used.\n
 *                      But for record channel, a pid list will be
 *                      used.
 */
alisl_retcode alisldmx_set_channel_pidlist(alisl_handle handle,
					   uint32_t channelid,
					   uint16_t *pid_list,
					   uint32_t nb_pid)
{
	SL_DBG("pid:%d\n", pid_list[0]);
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_channel *ch;
	struct list_head *head;
	pthread_mutex_t *mutex;
	bool restart = false;
	int max;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);

	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	if (bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
		SL_WARN("WARNING: Channel is enabled when set pid!\n" \
			"Maybe you want to change pid of this channel!\n" \
			"Stop the channel, then start it after pid is set!\n");
		channel_stop(dev, ch);
		restart = true;
	}

	/*
	 * Until now, I'v known that only record channel
	 * may have multi pid. Other type of channels
	 * have only one pid.
	 */
	if (ch->type == DMX_CHANNEL_RECORD) {
		channel_pidlist_update(ch, pid_list, nb_pid);
	} else {
		channel_pid_update(ch, pid_list, nb_pid);
	}

	/*
	 * Set pid to pes/sec/ts parameters, but driver
	 * will use the correct pid by checking variable output_format
	 */
	if (ch->nb_pid > 0) {
		ch->ch_param.pes_param.pid = ch->pid_list[0];
		ch->ch_param.sec_param.pid = ch->pid_list[0];
		memcpy(ch->ch_param.ts_param.pid_list, ch->pid_list,
			   ch->nb_pid * sizeof(ch->pid_list[0]));
		ch->ch_param.ts_param.pid_list_len = ch->nb_pid;
		if(ch->stream == DMX_STREAM_PCR) {
			ch->ch_param.pcr_param.pid = ch->pid_list[0];
		}
	}

	if (restart == true) {
		SL_DBG("Restart the channel now!\n");
		channel_start(dev, ch);
	}

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_channel_read
 *  @brief              Read ts or pes from a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param buf          buffer to store ts or pes 
 *  @param len          buffer length
 *  @param[out] p_read    storing size has been read
 *  @param timeout_ms      timeout for read in ms
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/15/2017, Created
 *
 */
alisl_retcode alisldmx_channel_read(alisl_handle handle, uint32_t channelid,
                unsigned char *buf, unsigned long len, unsigned long *p_read, int timeout_ms)
{
    struct dmx_device *dev = (struct dmx_device *)handle;
    enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
    struct dmx_channel *ch;
    struct list_head *head;
    pthread_mutex_t *mutex = NULL;
    int max;
    alisl_retcode ret = 0;
    struct statfs stbuf;
    struct pollfd fds[1];
    unsigned long len_read = 0;
    unsigned long num = 0;
    int i = 0;

    if (dev == NULL) {
        SL_ERR("Try to open before construct!\n");
        ret = ERROR_NULLDEV;
        goto OUT;
    }

    if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
        ret = ERROR_CHTYPE;
        goto OUT;
    }
    
    if (mutex != NULL)
        pthread_mutex_lock(mutex);
    
    if ((ch = find_channel(head, channelid)) == NULL) {
        ret = ERROR_NOCH;
        if (mutex != NULL)
            pthread_mutex_unlock(mutex);
        goto OUT;
    }
    
    if (mutex != NULL)
        pthread_mutex_unlock(mutex);

    pthread_mutex_lock(&dev->rec_mutex);
    if (!bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
        ret = ERROR_STATUS;
        SL_WARN("channel is enable!\n");
        pthread_mutex_unlock(&dev->rec_mutex);
        goto OUT;  /* skip not running channel */
    }
    if (fstatfs(ch->fd, &stbuf) != 0) {
        ret = ERROR_INVAL;
        SL_ERR("fd is invalid!\n");
        pthread_mutex_unlock(&dev->rec_mutex);
        goto OUT;  /* skip invalid fd */
    }
        
    if ((ch->callback.request_buffer) || (ch->callback.update_buffer)) {
        ret = ERROR_FAILED;
        SL_ERR("cannot read when callback are enabled!\n");
        pthread_mutex_unlock(&dev->rec_mutex);
        goto OUT; /*cannot read when callback are enabled*/
    }

    fds[0].fd = ch->fd;
    fds[0].events = POLLIN | POLLRDNORM;
    pthread_mutex_unlock(&dev->rec_mutex);
    
    /* in case no event before timeout */
    if (poll(fds, 1, timeout_ms)) {
        pthread_mutex_lock(&dev->rec_mutex);
        if (fds[0].revents & POLLIN) {
            len_read = read(ch->fd, buf, len);
            for (num = 0; num < len_read; num += 188) {
                if (((buf+ num)[0] == 0x32) && ((buf+ num)[1] == 0x32) 
                        && ((buf+ num)[2] == 0x32) && ((buf+ num)[3] == 0x32)) {
                    /*
                                The data read from the channel contains a ts packet beginning with 0x32 0x32 0x32 0x32. 
                                When the read data is played locally with the TSG, the TSG does recognize this packet, 
                                resulting in a mosaic of the picture. So need to delete this ts package.
                                */
                    memmove(buf + num, buf + num + 188, len_read - num - 188*(i + 1));
                    num -= 188;
                    i++;
                }
            }
            *p_read = len_read - 188*i;
        }
        pthread_mutex_unlock(&dev->rec_mutex);
    }

OUT:
    return ret;
}

/**
 *  Function Name:      alisldmx_set_channel_pid
 *  @brief              set pid to a channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid          pid to be set to the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/08/2013, Created
 *
 *  @note
 */

alisl_retcode alisldmx_set_channel_pid(alisl_handle handle,
									   uint32_t channelid,
									   uint16_t pid)
{
	return alisldmx_set_channel_pidlist(handle, channelid, &pid, 1);
}

static int add_pidlist(struct dmx_channel *ch,
					   uint16_t *pid_list, uint32_t nb_pid)
{
	uint16_t pid;
	int i, j, nb;

	nb = ch->nb_pid;
	for (i = 0; i < nb_pid; i++) {
		pid = pid_list[i] & 0x1FFF;

		if (!PID_VALID(pid))
			continue;

		for (j = 0; j < nb; j++) {
			if (pid == ch->pid_list[j])
				break;
		}

		if (j >= nb) {
			ch->pid_list[nb] = pid;
			nb ++;
			if (nb >= DMX_REC_PID_LIST_MAX_LEN) {
				SL_DBG("WARNING: pid list overflow!\n");
				break;
			}
		}
	}

	ch->nb_pid = nb;
    memset(ch->ch_param.ts_param.pid_list, 0, 
        ch->ch_param.ts_param.pid_list_len*sizeof(ch->ch_param.ts_param.pid_list[0]));
    memcpy(ch->ch_param.ts_param.pid_list, ch->pid_list, ch->nb_pid*sizeof(ch->pid_list[0]));
    ch->ch_param.ts_param.pid_list_len = ch->nb_pid;
    return 0;
}

static int delete_pidlist(struct dmx_channel *ch,
						  uint16_t *pid_list, uint32_t nb_pid)
{
	uint16_t pid;
	int i, j, k, nb;

	nb = ch->nb_pid;
	for (i = 0; i < nb_pid; i++) {
		pid = pid_list[i] & 0x1FFF;

		if (!PID_VALID(pid))
			continue;

		for (j = 0; j < nb; j++) {
			if (pid == ch->pid_list[j]) {
				break;
			}
		}

		if (j < nb) {
			for (k = j; k < nb - 1; k++) {
				ch->pid_list[k] = ch->pid_list[k + 1];
			}
			nb--;
		}

	}

	ch->nb_pid = nb;
    memset(ch->ch_param.ts_param.pid_list, 0, 
        ch->ch_param.ts_param.pid_list_len*sizeof(ch->ch_param.ts_param.pid_list[0]));
    memcpy(ch->ch_param.ts_param.pid_list, ch->pid_list, ch->nb_pid*sizeof(ch->pid_list[0]));
    ch->ch_param.ts_param.pid_list_len = ch->nb_pid;
    return 0;
}

/**
 *  Function Name:      alisldmx_add_channel_pidlist
 *  @brief              add pidlist to a record channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid_list     pids to be set to the channel
 *  @param nb_pid       number of pids that to be set to the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               09/02/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_add_channel_pidlist(alisl_handle handle,
										   uint32_t channelid,
										   uint16_t *pid_list,
										   uint32_t nb_pid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_channel_param dmx_param;
	struct dmx_channel *ch;
	struct list_head *head;
	pthread_mutex_t *mutex;
	int max;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);
	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_unlock(mutex);


	/*
	 * Until now, I'v known that only record channel
	 * may have multi pid. Other type of channels
	 * only have one pid.
	 */
	if (ch->type != DMX_CHANNEL_RECORD) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		goto OUT;
	}

	memset(&dmx_param, 0, sizeof(dmx_param));
	memcpy(&dmx_param.ts_param.pid_list, pid_list, nb_pid * sizeof(uint16_t));
	dmx_param.ts_param.pid_list_len = nb_pid;
	dmx_param.output_space = DMX_OUTPUT_SPACE_USER;
	dmx_param.rec_whole_tp = ch->ch_param.rec_whole_tp;
	dmx_param.fst_cpy_slot =  -1;

	if (ioctl(ch->fd, ALI_DMX_CHANNEL_ADD_PID, &dmx_param) < 0) {
		SL_DBG("Add pid to record channel failed!\n");
		ret = ERROR_ADDPID;
		goto OUT;
	}

	add_pidlist(ch, pid_list, nb_pid);

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_del_channel_pidlist
 *  @brief              delete pidlist from a record channel
 *
 *  @param handle       pointer to module handle
 *  @param channelid    Specific channel
 *  @param pid_list     pids to be deleted from the channel
 *  @param nb_pid       number of pids that to be deleted from the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               09/02/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_del_channel_pidlist(alisl_handle handle,
										   uint32_t channelid,
										   uint16_t *pid_list,
										   uint32_t nb_pid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_channel_param dmx_param;
	struct dmx_channel *ch;
	struct list_head *head;
	pthread_mutex_t *mutex;
	int max;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);
	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_unlock(mutex);
	/*
	 * Until now, I'v known that only record channel
	 * may have multi pid. Other type of channels
	 * only have one pid.
	 */
	if (ch->type != DMX_CHANNEL_RECORD) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		goto OUT;
	}

	memset(&dmx_param, 0, sizeof(dmx_param));
	memcpy(&dmx_param.ts_param.pid_list, pid_list, nb_pid * sizeof(uint16_t));
	dmx_param.ts_param.pid_list_len = nb_pid;
	dmx_param.output_space = DMX_OUTPUT_SPACE_USER;
	dmx_param.rec_whole_tp = ch->ch_param.rec_whole_tp;
	dmx_param.fst_cpy_slot =  -1;

	if (ioctl(ch->fd, ALI_DMX_CHANNEL_DEL_PID, &dmx_param) < 0) {
		SL_DBG("Delete pid from record channel failed!\n");
		ret = ERROR_DELPID;
		goto OUT;
	}

	delete_pidlist(ch, pid_list, nb_pid);

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_set_channel_attr
 *  @brief              set channel attribute
 *
 *  @param handle       pointer to module handle
 *  @param channelid    id of the channel
 *  @param attr         attribute of the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/08/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_channel_attr(alisl_handle handle,
										uint32_t channelid,
										struct dmx_channel_attr *attr)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channelid);
	struct dmx_channel *ch;
	struct list_head *head;
	pthread_mutex_t *mutex;
	struct dmx_channel_param *param;
	int max;
	alisl_retcode ret = 0;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		goto OUT;
	}

	if (mutex != NULL)
		pthread_mutex_lock(mutex);
	if ((ch = find_channel(head, channelid)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		goto OUT;
	}
	if (mutex != NULL)
		pthread_mutex_unlock(mutex);

	param = &ch->ch_param;

	switch (attr->output_format) {
		case DMX_OUTPUT_FORMAT_TS:
			param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
            param->uncache_para = attr->uncache_para;
			break;
		case DMX_OUTPUT_FORMAT_PES:
			param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_PES;
            param->pes_param.needdiscramble = attr->is_encrypted; //fawn++
			break;
		case DMX_OUTPUT_FORMAT_SEC:
			param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_SEC;
            param->sec_param.needdiscramble = attr->is_encrypted; //fawn++
			break;
		case DMX_OUTPUT_FORMAT_PCR:
			/*update to DMX_CHANNEL_OUTPUT_FORMAT_TS to let dmx driver can send
			ts packet containing pcr to see. See can parse out pcr for avsync
			From PDK1.10a-20170119, pcr is parsed by hardware on main size, the same as pdk1.9.
			*/ 
			param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_PCR;
			break;
		case DMX_OUTPUT_FORMAT_NONE:
			if (attr->stream == DMX_STREAM_PCR)
				param->output_format = DMX_CHANNEL_OUTPUT_FORMAT_PCR;
		default:
			break;
	}

	ch->stream = attr->stream;

	param->rec_whole_tp = attr->rec_whole_tp;
	param->enc_para = attr->enc_para;

	if (attr->stream == DMX_STREAM_VIDEO)
		dev->video_id = attr->video_id;
	
	param->video_pid = attr->video_pid;
	param->video_type = attr->video_type;

OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_set_filter_attr
 *  @brief              set filter attribute
 *
 *  @param handle       pointer to module handle
 *  @param channel_id   id of the channel
 *  @param filter_id    id of the filter
 *  @param attr         attribute of the channel
 *
 *  @return             alisl_retcode
 *
 *  @author             copy from alan.song
 *  @date               08/31/2015, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_filter_attr(alisl_handle handle,
    uint32_t channel_id,
    uint32_t filter_id,
    struct dmx_channel_attr *attr)
{
    alisl_retcode ret = 0;
    struct dmx_section_filter *filter, *tmp_flt;
    struct list_head *head;
    struct dmx_channel *ch;
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_channel_type type = DMX_CHID2TYPE(channel_id);
	pthread_mutex_t *mutex;
	int max;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		ret = ERROR_NULLDEV;
		return ret;
	}

	if (get_channel_array_info(dev, type, &ch, &head, &mutex, &max)) {
		ret = ERROR_CHTYPE;
		return ret;
	}
    
	if (mutex != NULL) 
		pthread_mutex_lock(mutex);
	if ((ch = find_channel(head, channel_id)) == NULL) {
		SL_DBG("No specific channel!\n");
		ret = ERROR_NOCH;
		if (mutex != NULL)
			pthread_mutex_unlock(mutex);
		return ret;
	}
	
    list_for_each_entry_safe(filter, tmp_flt, &ch->filter, node) {
        if(filter->id == filter_id)
            break;
    }
    if(attr != NULL)
        filter->continuous = attr->continuous;
    if (mutex != NULL)
		pthread_mutex_unlock(mutex);
    return ret;
}

/**
 *  Function Name:      alisldmx_set_avsync_mode
 *  @brief              Set av sync mode to demux
 *
 *  @param handle       pointer to module handle
 *  @param avsync       av sync mode, refer to enum dmx_avsync_mode
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_avsync_mode(alisl_handle handle, enum dmx_avsync_mode avsync)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	enum dmx_see_av_sync_mode see_avsync;

	dev->avsync = avsync;
	switch (avsync) {
		case DMX_AVSYNC_LIVE:
			see_avsync = DMX_SEE_AV_SYNC_MODE_LIVE;
			break;
		case DMX_AVSYNC_PLAYBACK:
			see_avsync = DMX_SEE_AV_SYNC_MODE_PLAYBACK;
			break;
		case DMX_AVSYNC_TSG_TIMESHIT:
			see_avsync = DMX_SEE_AV_SYNC_MODE_TSG_TIMESHIT;
			break;
		case DMX_AVSYNC_TSG_LIVE:
			see_avsync = DMX_SEE_AV_SYNC_MODE_TSG_LIVE;
			break;
		case DMX_AVSYNC_NONE:
		default:
			see_avsync = DMX_SEE_AV_SYNC_MODE_LIVE;
			break;
	}

	if (dev->pathfeed != NULL) {
		see_avsync = DMX_SEE_AV_SYNC_MODE_PLAYBACK;
	}

	ioctl(dev->seefd, ALI_DMX_SEE_AV_SYNC_MODE_SET, &see_avsync);

	return 0;
}

static struct dmx_channel * find_channel_by_stream_type(struct dmx_device *dev,
														enum dmx_stream_type stream)
{
	struct dmx_channel *ch, *tmp;

	list_for_each_entry_safe(ch, tmp, &dev->streams, node) {
		if (ch->stream == stream &&
			bit_test_any(ch->status, CHANNEL_STATUS_ENABLE)) {
			return ch;
		}
	}

	return NULL;
}

static uint16_t find_stream_pid(struct dmx_device *dev,
								enum dmx_stream_type stream)
{
	struct dmx_channel *ch, *tmp;
	uint16_t pid = DMX_ILLEGAL_PID;

	list_for_each_entry_safe(ch, tmp, &dev->streams, node) {
		if (ch->stream == stream && ch->nb_pid > 0) {
			pid = ch->pid_list[0];
			break;
		}
	}

	return pid;
}

static struct dmx_channel * find_channel_by_pid(struct dmx_device *dev,
												unsigned short pid)
{
    struct dmx_channel *ch = NULL;
    unsigned short vpid = DMX_ILLEGAL_PID;
    unsigned short apid = DMX_ILLEGAL_PID;

    /** notice: must be & 0x1FFF, because the pid have been changed in aui_dmx_av_stream_start */
    vpid = (find_stream_pid(dev, DMX_STREAM_VIDEO)) & DMX_MASK_PID;
    apid = (find_stream_pid(dev, DMX_STREAM_AUDIO)) & DMX_MASK_PID;
    
    if ((DMX_ILLEGAL_PID != pid) && (pid == vpid)) {
        ch = find_channel_by_stream_type(dev, DMX_STREAM_VIDEO);
    } else if ((DMX_ILLEGAL_PID != pid) && (pid == apid)) {
        ch = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO);
    } else {
        SL_WARN("pid[%d] is not support, only support video pid and audio pid\n", pid);
        ch = NULL;
    }

    return ch;
}

/**
 *  Function Name:      alisldmx_avstart
 *  @brief              Start av play.
 *                      After all stream channels are enabled(video/audio/
 *                      audio description/pcr), this function should
 *                      be called to start av play.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note               After all stream channels are enabled(video/audio/
 *                      audio description/pcr), this function should
 *                      be called to start av play.
 */
alisl_retcode alisldmx_avstart(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_see_av_pid_info see_av_info;
	int rc=0;
	alisl_retcode ret = 0;

	if (NULL == dev) {
		SL_DBG("dmx device handle is NULL!\n");
		ret = ERROR_NULLDEV;
		goto OUT;
	}

	memset(&see_av_info, 0, sizeof(see_av_info));

	see_av_info.v_pid = find_stream_pid(dev, DMX_STREAM_VIDEO);
	see_av_info.a_pid = find_stream_pid(dev, DMX_STREAM_AUDIO);
	see_av_info.ad_pid = find_stream_pid(dev, DMX_STREAM_AUDIO_DESCRIPTION);
	see_av_info.p_pid = find_stream_pid(dev, DMX_STREAM_PCR);
	see_av_info.v_id = dev->video_id;
	SL_DBG("dmx id: %d, video id : %d\n", dev->id, dev->video_id);

	rc = ioctl(dev->seefd, ALI_DMX_SEE_AV_START, &see_av_info);
	if (rc < 0) {
		SL_DBG("Demux see av start failed!\n");
		ret = ERROR_AVSTART;
		goto OUT;
	}
    dev->av_start = true;
OUT:
	return ret;
}

/**
 *  Function Name:      alisldmx_avstop
 *  @brief              Stop av play.
 *                      After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop av play.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note               After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop av play.
 */
alisl_retcode alisldmx_avstop(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (NULL == dev) {
		SL_DBG("dmx device handle is NULL!\n");
		return ERROR_NULLDEV;
	}

	ioctl(dev->seefd, ALI_DMX_SEE_AV_STOP, 0);
    dev->av_start = false;
	return 0;
}

/**
 *  Function Name:      alisldmx_avstop_with_param
 *  @brief              Stop/Pause av play.
 *                      After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop/pause av play.
 *
 *  @param handle       pointer to module handle
 *  @param param        pause -- DMA_INVALID_CHA, stop -- 0
 *
 *  @return             alisl_retcode
 *
 *  @author             Will Qian <will.qian@alitech.com>
 *  @date               08/06/2015, Created
 *
 *  @note               After all stream channels are disabled or freed(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to stop/pause av play.
 */
alisl_retcode alisldmx_avstop_with_param(alisl_handle handle, unsigned int param)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (NULL == dev) {
		SL_DBG("dmx device handle is NULL!\n");
		return ERROR_NULLDEV;
	}

	ioctl(dev->seefd, ALI_DMX_SEE_AV_STOP, param);
	dev->av_start = false;
    
	return 0;
}

/**
 *  Function Name:      alisldmx_avpause
 *  @brief              Pause av play.
 *                      After all stream channels are enabled(video/
 *                      audio/audio description/pcr), this function should
 *                      be called to pause av play.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Nick Li <nick.li@alitech.com>
 *  @date               02/09/2017, Created
 *
 *  @note               After use alisldmx_avpause to pause av play, 
 *                      should be called alisldmx_avstart to enable playback.
 */
alisl_retcode alisldmx_avpause(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (NULL == dev) {
		SL_DBG("dmx device handle is NULL!\n");
		return ERROR_NULLDEV;
	}

	ioctl(dev->seefd, ALI_DMX_SEE_PLAYBACK_PAUSE, 0);
    dev->av_start = false;

	return 0;
}

/**
 *  Function Name:      alisldmx_change_audio_pid
 *  @brief              change audio pid to see.
 *
 *  @param handle       pointer to module handle
 *  @param audio_pid    new audio pid
 *  @param audio_desc_pid new audio description pid
 *  @param audio_chid   new channel id of audio
 *  @param audio_desc_chid new channelid of audio description
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/10/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_change_audio_pid(alisl_handle handle,
										uint16_t audio_pid, uint16_t audio_desc_pid,
										uint32_t *audio_chid, uint32_t *audio_desc_chid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_see_av_pid_info see_av_info;
	struct dmx_channel *ch_a, *ch_ad;
	uint16_t a_pid, ad_pid;
	uint32_t a_chid, ad_chid;
	struct dmx_channel_attr attr;

	a_pid = find_stream_pid(dev, DMX_STREAM_AUDIO);
	ad_pid = find_stream_pid(dev, DMX_STREAM_AUDIO_DESCRIPTION);
	if (a_pid == audio_pid && ad_pid == audio_desc_pid) {
		return 0;
	}

	ch_a = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO);
	ch_ad = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO_DESCRIPTION);

	if (ch_a != NULL) {
		alisldmx_free_channel(handle, ch_a->id);
	}
	if (ch_ad != NULL) {
		alisldmx_free_channel(handle, ch_ad->id);
	}

	memset(&see_av_info, 0, sizeof(see_av_info));
	see_av_info.a_pid = audio_pid;
	see_av_info.ad_pid = audio_desc_pid;
	ioctl(dev->seefd, ALI_DMX_SEE_A_CHANGE_PID, &see_av_info);

	memset(&attr, 0, sizeof(attr));
	*audio_chid = DMX_ILLEGAL_CHANNELID;
	*audio_desc_chid = DMX_ILLEGAL_CHANNELID;
	if (PID_VALID(audio_pid)) {
		attr.stream = DMX_STREAM_AUDIO;
		alisldmx_allocate_channel(handle, DMX_CHANNEL_STREAM, &a_chid);
		alisldmx_set_channel_attr(handle, a_chid, &attr);
		alisldmx_set_channel_pid(handle, a_chid, audio_pid);
		alisldmx_control_channel(handle, a_chid, DMX_CTRL_ENABLE);
		*audio_chid = a_chid;
	}

	if (PID_VALID(audio_desc_pid)) {
		attr.stream = DMX_STREAM_AUDIO_DESCRIPTION;
		alisldmx_allocate_channel(handle, DMX_CHANNEL_STREAM, &ad_chid);
		alisldmx_set_channel_attr(handle, ad_chid, &attr);
		alisldmx_set_channel_pid(handle, ad_chid, audio_desc_pid);
		alisldmx_control_channel(handle, ad_chid, DMX_CTRL_ENABLE);
		*audio_desc_chid = ad_chid;
	}

	return 0;
}

/**
 *  Function Name:      alisldmx_set_playback_param
 *  @brief              Set playback parameters.
 *
 *  @param handle       pointer to module handle
 *  @param
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_playback_param(alisl_handle handle,
										  struct dmx_playback_param *param)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}

	pthread_mutex_lock(&dev->feed_mutex);
	memcpy(&dev->playback_param, param, sizeof(*param));
	pthread_mutex_unlock(&dev->feed_mutex);

	return 0;
}

/**
 *  Function Name:      alisldmx_direct_write_playback
 *  @brief              Write playback dmx.
 *
 *  @param handle       pointer to module handle
 *  @param
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_direct_write_playback(alisl_handle handle,
											 struct dmx_write_playback_param *param)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_playback_param *pb;
	struct dmx_fast_copy_param fcp;
	ssize_t nbytes;

	if (dev == NULL) {
		SL_DBG("Try to open before construct!\n");
		return ERROR_NULLDEV;
	}
	pb = &dev->playback_param;

	pthread_mutex_lock(&dev->feed_mutex);
	if (dev->fastcopy == true && param->fastcopy == true) {
		fcp.data = (void*)param->start_buf;
		fcp.len = param->length;
		nbytes = ioctl(dev->feedfd, ALI_DMX_PLAYBACK_FSTCPY, &fcp);
	} else {
		nbytes = write(dev->feedfd, param->start_buf, param->length);
	}
	pb->write_size = nbytes;
	pthread_mutex_unlock(&dev->feed_mutex);
	return 0;
}

/**
 *  Function Name:      alisldmx_get_write_pb_length
 *  @brief              Get length of write playback dmx.
 *
 *  @param handle       pointer to module handle
 *  @param
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/09/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_get_write_pb_length(alisl_handle handle,
										   unsigned long *param)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_playback_param *pb = &dev->playback_param;

	pthread_mutex_lock(&dev->feed_mutex);
	*param = pb->write_size;
	pthread_mutex_unlock(&dev->feed_mutex);
	return 0;
}

static alisl_retcode alisldmx_open_see_dmx(struct dmx_device *dev, enum dmx_see_id see_id)
{
	int seefd = -1;
	if (see_id == DMX_SEE_ID_DEMUX1) {
		seefd = open("/dev/ali_m36_dmx_see_1", O_RDWR);
		if (seefd < 0) {
			close(seefd);
			SL_DBG("ali_m36_dmx_see_1 open failed!\n");
			seefd = -1;
		} 
		dev->see_dmx_id = DMX_SEE_ID_DEMUX1;
	} else {
		seefd = open("/dev/ali_m36_dmx_see_0", O_RDWR);
		if (seefd < 0) {
			close(seefd);
			SL_DBG("ali_m36_dmx_see_0 open failed!\n");
			seefd = -1;
		}
		dev->see_dmx_id = DMX_SEE_ID_DEMUX0;
	}
	dev->seefd = seefd;	
	return 0;
}

/**
 *  Function Name:      alisldmx_open
 *  @brief              Open ali_m36_dmx_see_0 here, and open linux demux device
 *                      for common usage. But don't open linux demux
 *                      device for each channel. Because we will open linux demux
 *                      device When really allocate a channel. We just remember
 *                      which demux will be used later.
 *                      When open a demux device, we will create some daemon
 *                      pthread. The daemon pthreads are blocked and wait for
 *                      alisldmx_start. When call alisldmx_start, they will begin
 *                      to process data for all opened channels.
 *
 *  @param handle       pointer to module handle
 *  @param id           demux id
 *  @param see_id       see demux id
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/17/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_open(alisl_handle *handle, enum dmx_id id, enum dmx_see_id see_id)
{
    struct dmx_device *dev = NULL;
    pthread_attr_t attr;
    alisl_retcode retcode = ERROR_NOOPEN;
    int ret = 0;
    int i = 0;
    
    /* check dmx dev id, and find the corresponding path / pathfeed */
    for (i = 0; i < ARRAY_SIZE(dev_demux); i++) {
        if (id == dev_demux[i].id) {
            break;
        }
    }

    if (i >= ARRAY_SIZE(dev_demux)) {
        SL_DBG("Invalidate demux id!\n");
        return ERROR_INVAL;
    }

    pthread_mutex_lock(&m_mutex);

    /* construct dmx dev */
    if (m_dev[i] == NULL) {
        if (alisldmx_construct((alisl_handle *)&dev)) {
            retcode = ERROR_NOMEM;
            goto exit_fail;
        }
    } else {
        dev = m_dev[i];
    }

    if (flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
        SL_DBG("Already opened!\n");
        retcode = 0;
        goto exit_success;
    }

    /* copy the current dmx dev id path pathfeed to struct dmx_device *dev */
    dev->id = id;
    dev->path = dev_demux[i].path;
    dev->pathfeed = dev_demux[i].pathfeed;

    dev->fd = open(dev->path, O_RDONLY);
    if (dev->fd < 0) {
        SL_DBG("%s open failed!\n", dev->path);
        retcode = ERROR_OPEN;
        goto fail0;
    }

    if (dev->pathfeed != NULL) {
        dev->feedfd = open(dev->pathfeed, O_RDWR);
        if (dev->feedfd < 0) {
            close(dev->fd);
            SL_DBG("%s open failed\n", dev->pathfeed);
            retcode = ERROR_OPEN;
            goto fail0;
        }
    }

    SL_DBG("open dmx id = %d, path = %s, dev->fd = %d, pathfeed = %s, dev->feedfd = %d\n", 
        dev->id, dev->path, dev->fd, dev->pathfeed, dev->feedfd);
    
    alisldmx_open_see_dmx(dev, see_id);
    pthread_mutex_init(&dev->sect_mutex, NULL);
    pthread_mutex_init(&dev->rec_mutex, NULL);
    pthread_mutex_init(&dev->serv_mutex, NULL);
    pthread_mutex_init(&dev->feed_mutex, NULL);
    flag_init(&dev->flag, 0);
    INIT_LIST_HEAD(&dev->sections);
    INIT_LIST_HEAD(&dev->records);
    INIT_LIST_HEAD(&dev->services);
    INIT_LIST_HEAD(&dev->streams);

    pthread_attr_init(&attr);
    //pthread_attr_setstacksize(&attr, STACKSIZE); /* Need to delete it, because set stack size may cause Android JVM error. */
    //pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

    ret = alisldmx_poll_start(dev);
    if (ret != 0) {
        SL_DBG("alisldmx_poll_start failed\n");
        goto fail1;
    }

    if (dev->feedfd > 0) {
        ret = pthread_create(&dev->playfeed_tid, &attr,
                             alisldmx_playfeed_pthread, (void *)dev);
        if (ret != 0) {
            SL_DBG("Create monitor pthread failed: %s\n", strerror(errno));
            goto fail2;
        }
    }

    pthread_attr_destroy(&attr);

    flag_bit_clear(&dev->status, DMX_STATUS_CLOSE);
    flag_bit_set(&dev->status, DMX_STATUS_OPEN);

exit_success:
    dev->open_cnt++;
    m_dev[i] = dev;
    *handle = dev;
    pthread_mutex_unlock(&m_mutex);
    SL_DBG("dmx open success!\n");
    
    return 0;

fail2:
    alisldmx_poll_stop(dev);
fail1:
    pthread_attr_destroy(&attr);

    retcode = ERROR_PTCREATE;
fail0:
    if (dev->section_buffer)
        free(dev->section_buffer);
    free(dev);
exit_fail:
    *handle = NULL;
    pthread_mutex_unlock(&m_mutex);
    return retcode;
}

/**
 *  Function Name:      alisldmx_set_nim_chipid
 *  @brief              Set nim chip id. So the module will remembor it
 *                      and set to channel parameter when start the channel.
 *
 *  @param handle       pointer to module handle
 *  @param nim_chipid   nim chip id
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_nim_chipid(alisl_handle handle, uint32_t nim_chipid)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	dev->nim_chip_id = nim_chipid;

	return 0;
}

/**
 *  Function Name:      alisldmx_set_front_end
 *  @brief              Set front_end. So the module will remember it
 *                      and set to channel parameter when start the channel.
 *
 *  @param handle       pointer to module handle
 *  @param frong_end    front_end
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_set_front_end(alisl_handle handle, uint32_t front_end)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	dev->front_end = front_end;

	return 0;
}

/**
 *  Function Name:      alisldmx_fastcopy_enable
 *  @brief              Enable or disable fast copy feature.
 *
 *  @param handle       pointer to module handle
 *  @param enable       set 0 to disable fast copy feature,\n
 *                      Other to enable fast copy feature.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               09/25/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_fastcopy_enable(alisl_handle handle, bool enable)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	dev->fastcopy = !!enable;

	return 0;
}

/**
 *  Function Name:      alisldmx_start
 *  @brief              Start a demux device, then we can allocate a channel
 *                      to get section, record data...
 *                      The daemon pthreads will running when call this function.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/17/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_start(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Demux not opened!\n");
		return ERROR_NOOPEN;
	}

	if (flag_bit_test_any(&dev->status, DMX_STATUS_START)) {
		SL_DBG("Demux already started!\n");
		return 0;
	}

	if (dev->feedfd > 0) {
		if(!flag_bit_test_any(&dev->status, DMX_STATUS_PAUSE))
		{
		/*
		 * When pause -> start, don't need to flush.
		 * When other status -> start, flush buffer.
		 */
		ioctl(dev->feedfd, ALI_DMX_CHANNEL_FLUSH_BUF, 0);
		ioctl(dev->feedfd, ALI_DMX_IO_SET_PLAYBACK_SRC_TYPE,
			  dev->playback_param.is_radio);
		}
		else
		{
			ioctl(dev->seefd, ALI_DMX_SEE_PLAYBACK_PAUSE, 1); //resume
		}
	}

	flag_bit_clear(&dev->status, DMX_STATUS_STOP | DMX_STATUS_PAUSE);
	flag_bit_set(&dev->status, DMX_STATUS_START);

	return 0;
}

/**
 *  Function Name:      alisldmx_pause
 *  @brief              Pause a demux.
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/29/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_pause(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_START)) {
		SL_DBG("Demux not started!\n");
		return 0;
	}

	if (flag_bit_test_any(&dev->status, DMX_STATUS_PAUSE)) {
		SL_DBG("Demux already paused!\n");
		return 0;
	}

	if (dev->feedfd > 0) {
		/*
		 * The playfeed pthread must be running, so sync with
		 * playfeed pthread and wait it be idle.
		 */
		ioctl(dev->seefd, ALI_DMX_SEE_PLAYBACK_PRE_PAUSE, 0);
		flag_bit_set(&dev->status, DMX_STATUS_IDLE_P1);
		flag_wait_bit_any(&dev->status, DMX_STATUS_IDLE_P2, -1);
		flag_bit_clear(&dev->status, DMX_STATUS_IDLE_P2);
		flag_bit_clear(&dev->status, DMX_STATUS_START);
		flag_bit_set(&dev->status, DMX_STATUS_IDLE_P3);
		ioctl(dev->seefd, ALI_DMX_SEE_PLAYBACK_PAUSE, 0);
	}

	flag_bit_clear(&dev->status, DMX_STATUS_START);
	flag_bit_set(&dev->status, DMX_STATUS_PAUSE);

	return 0;
}

/**
 *  Function Name:      alisldmx_stop
 *  @brief              Stop a demux. It will clear device status bit
 *                      DMX_STATUS_START. And all the daemon pthread will block
 *                      on flag_wait_bit_any(&dev->status, DMX_STATUS_START)
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/29/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_stop(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status,
						   DMX_STATUS_START | DMX_STATUS_PAUSE)) {
		SL_DBG("Demux device not started or paused!\n");
		return 0;
	}

	if (flag_bit_test_any(&dev->status, DMX_STATUS_STOP)) {
		SL_DBG("Demux device already stopped!\n");
		return 0;
	}

	if (dev->feedfd > 0) {
		if (flag_bit_test_any(&dev->status, DMX_STATUS_START)) {
			/*
			 * If playfeed pthread is running, then sync with
			 * playfeed pthread and wait it be idle.
			 */
			flag_bit_set(&dev->status, DMX_STATUS_IDLE_P1);
			flag_wait_bit_any(&dev->status, DMX_STATUS_IDLE_P2, -1);
			flag_bit_clear(&dev->status, DMX_STATUS_IDLE_P2);
			flag_bit_clear(&dev->status, DMX_STATUS_START);
			flag_bit_set(&dev->status, DMX_STATUS_IDLE_P3);
		}
		ioctl(dev->feedfd, ALI_DMX_CHANNEL_FLUSH_BUF, 0);
	}

	flag_bit_clear(&dev->status, DMX_STATUS_START | DMX_STATUS_PAUSE);
	flag_bit_set(&dev->status, DMX_STATUS_STOP);

	return 0;
}

/**
 *  Function Name:      alisldmx_close
 *  @brief              close demux device
 *
 *  @param handle       pointer to module handle
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               06/21/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_close(alisl_handle handle)
{
    struct dmx_device *dev = (struct dmx_device *)handle;
    struct dmx_channel *ch = NULL;
    struct dmx_channel *tmp = NULL;
	int i = 0;
    
    if (NULL == dev) {
        SL_ERR("NULL dev data structure!\n");
        return ERROR_NULLDEV;
    }

    /* find the opened dmx dev id */
    for (i = 0; i < ARRAY_SIZE(dev_demux); i++) {
        if (dev->id == dev_demux[i].id) {
            break;
        }
    }

    if (i == ARRAY_SIZE(dev_demux)) {
        SL_DBG("Invalidate demux id!\n");
        return ERROR_INVAL;
    }

    SL_DBG("close dmx id = %d, path = %s, dev->fd = %d, pathfeed = %s, dev->feedfd = %d\n", 
        dev->id, dev->path, dev->fd, dev->pathfeed, dev->feedfd);

    pthread_mutex_lock(&m_mutex);
    SL_DBG("dev:%p open_cnt:%d\n", dev, dev->open_cnt);
    if (--dev->open_cnt) {
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
        SL_DBG("Demux device not opened!\n");
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    if (flag_bit_test_any(&dev->status, DMX_STATUS_CLOSE)) {
        SL_DBG("Demux device already closed!\n");
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }

    if (!flag_bit_test_any(&dev->status, DMX_STATUS_STOP)) {
        SL_DBG("Demux device not stopped!\n");
        alisldmx_stop(handle);
    }

    /*
     * We just need to stop all channels.
     * Because all global storages will be reset.
     */
    list_for_each_entry_safe(ch, tmp, &dev->sections, node) {
        channel_stop(dev, ch);
    }
    list_for_each_entry_safe(ch, tmp, &dev->records, node) {
        channel_stop(dev, ch);
    }
    list_for_each_entry_safe(ch, tmp, &dev->services, node) {
        channel_stop(dev, ch);
    }
    list_for_each_entry_safe(ch, tmp, &dev->streams, node) {
        channel_stop(dev, ch);
    }

    if(dev->av_start)
    {
        alisldmx_avstop(handle);
    }
    alisldmx_poll_stop(dev);

    if (dev->feedfd > 0) {
        pthread_cancel(dev->playfeed_tid);
        pthread_join(dev->playfeed_tid, NULL);
        close(dev->feedfd);
        dev->feedfd = -1;
    }

    close(dev->fd);
    dev->fd = -1;

    /* 
      close see dmx, driver will close the device when the last closing,
      so the see dmx can work normally if other dmx is using it.
    */
    close(dev->seefd);
    dev->seefd = -1;
    
    /* Clear global storages */
    memset(dev->ch_sections, 0, sizeof(dev->ch_sections));
    memset(dev->ch_records, 0, sizeof(dev->ch_records));
    memset(dev->ch_services, 0, sizeof(dev->ch_services));
    memset(dev->ch_streams, 0, sizeof(dev->ch_streams));
    memset(dev->filters, 0, sizeof(dev->filters));

    flag_bit_clear(&dev->status, DMX_STATUS_OPEN | DMX_STATUS_STOP);
    flag_bit_set(&dev->status, DMX_STATUS_CLOSE);

    m_dev[i] = NULL;
    alisldmx_destruct((alisl_handle *)&dev);
    pthread_mutex_unlock(&m_mutex);
    SL_DBG("dmx close success!");

    return 0;
}

/**
 *  Function Name:      alisldmx_get_scram_status
 *  @brief              Get a/v stream scramble status
 *
 *  @param handle       pointer to module handle
 *  @param[out] status  a/v stream scramble status
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_get_scram_status(alisl_handle *handle, uint32_t *status)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc;
	uint8_t st;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Demux device not opened!\n");
		return 0;
	}

	rc = ioctl(dev->seefd, ALI_DMX_SEE_AV_SCRAMBLE_STATUS, &st);
	if (rc < 0) {
		// ALI_DMX_SEE_AV_SCRAMBLE_STATUS is not availible in M3733
		// always return false for M3733
		st = 0;
		//return ERROR_INVAL;
	}

	*status = (uint32_t)st;

	return 0;
}

/**
 *  Function Name:      alisldmx_get_scram_status_ext
 *  @brief              Get a/v stream scramble status.
 *                      It is different from alisldmx_get_scram_status.
 *
 *  @param handle       pointer to module handle
 *  @param[out] status  a/v stream scramble status
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/15/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_get_scram_status_ext(alisl_handle *handle,
											uint16_t *pid_list, uint32_t nb_pid,
											uint32_t *status)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_see_av_scram_info  scram_param;
	alisl_retcode rc;
	//uint8_t st;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Demux device not opened!\n");
		return 0;
	}

	if (nb_pid != 2) {
		return ERROR_INVAL;
	}

	scram_param.pid[0] = pid_list[0];
	scram_param.pid[1] = pid_list[1];

	rc = ioctl(dev->seefd, ALI_DMX_SEE_AV_SCRAMBLE_STATUS_EXT,
			   &scram_param);
	if (rc < 0) {
		return ERROR_INVAL;
	}

	*status = (uint32_t)scram_param.scram_status;

	return 0;
}

/*
Ca-process get m_dmx_dsc_id by calling aui_dmx_data_path_set, then send it to non-ca process to encrypt-record.
*/
alisl_retcode alisldmx_en_rec_info_get(alisl_handle handle, struct dmx_record_deencrypt_info* info) {
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc = ERROR_INVAL;

	if ((dev == NULL)||(!info)) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	rc = ioctl(dev->fd, ALI_DMX_RECORD_DEENCRYPT_INFO, (unsigned long)info);
	if(rc) {
		SL_DBG("ALI_DMX_RECORD_DEENCRYPT_INFO fail, de_fd: %d, en_fd: %d, dmx_id: %d\n",
			info->m_dmx_dsc_fd.decrypt_fd, info->m_dmx_dsc_fd.encrypt_fd, dev->id);
	}
	else {
		SL_DBG("ALI_DMX_RECORD_DEENCRYPT_INFO success, de_fd: %d, en_fd: %d, dsc_id: 0x%x, dmx_id: %d\n",
			info->m_dmx_dsc_fd.decrypt_fd, info->m_dmx_dsc_fd.encrypt_fd, info->m_dmx_dsc_id, dev->id);
	}
	return rc;
}


/*
Non-CA process(pvrlib->hld-sl) of Multi-process set internal dsc to record
*/
alisl_retcode alisldmx_dsc_id_set(alisl_handle handle, unsigned int dmx_dsc_id){
	if (handle == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
	struct dmx_device *dev = (struct dmx_device *)handle;
	if(DMX_ID_SW_DEMUX0==dev->id) {
		SL_ERR("the data is not available to DMX_ID_SW_DEMUX0\n");
		return -1;
	}
	dev->sl_dmx_dsc_id= dmx_dsc_id;
	SL_DBG("sl_dmx_dsc_id: 0x%x, dmx_id: %d\n", dev->sl_dmx_dsc_id, dev->id);
	return 0;
}
#if 0
/*
Non-CA process(pvrlib->hld-sl) of Multi-process get internal dsc to record
*/
alisl_retcode alisldmx_dsc_id_get(alisl_handle handle, unsigned int *p_dmx_dsc_id){
	if ((!handle)||(!p_dmx_dsc_id)) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
	struct dmx_device *dev = (struct dmx_device *)handle;
	if(DMX_ID_SW_DEMUX0==dev->id) {
		SL_ERR("the data is not available to DMX_ID_SW_DEMUX0\n");
		return -1;
	}
	*p_dmx_dsc_id= dev->sl_dmx_dsc_id;
	SL_DBG("sl_dmx_dsc_id: %u, dmx_id: %d\n", dev->sl_dmx_dsc_id, dev->id);
	return 0;
}
#endif
/*
Non-CA process(pvrlib->hld-sl) of Multi-process get ali pvr decrypt handle  for
PVR_IO_SET_DECRYPT_RES & PVR_IO_DECRYPT_EVO & PVR_IO_RELEASE_DECRYPT_RES
*/
alisl_retcode alisldmx_ali_pvr_de_hdl_get(alisl_handle handle, unsigned int *p_ali_pvr_de_hdl){
	if ((handle == NULL)||(!p_ali_pvr_de_hdl)) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
	struct dmx_device *dev = (struct dmx_device *)handle;
	if(dev->id!=DMX_ID_SW_DEMUX0) {
		SL_ERR("the data is only available to DMX_ID_SW_DEMUX0\n");
		return -1;
	}
	*p_ali_pvr_de_hdl = dev->sl_dmx_dsc_id;
	SL_DBG("sl_dmx_dsc_id: %u\n", dev->sl_dmx_dsc_id);
	return 0;
}

/*
Non-CA process of Multi-process set ali pvr decrypt handle got by PVR_IO_CAPTURE_DECRYPT_RES 
when calling aui_dmx_data_path_set.
*/
alisl_retcode alisldmx_ali_pvr_de_hdl_set(alisl_handle handle, unsigned int ali_pvr_de_hdl){
	if (handle == NULL) {
		SL_ERR("NULL dev data structure!");
		return ERROR_NULLDEV;
	}
	struct dmx_device *dev = (struct dmx_device *)handle;
	if(dev->id!=DMX_ID_SW_DEMUX0) {
		SL_ERR("the data is only available to DMX_ID_SW_DEMUX0");
		return -1;
	}
	dev->ali_pvr_de_hdl = ali_pvr_de_hdl;
	SL_DBG("sl_dmx_dsc_id: %u", dev->sl_dmx_dsc_id);
	return 0;
}

/**
 *  Function Name:      alisldmx_ioctl
 *  @brief              ioctl of demux module, implement some misc
 *                      function interfaces.
 *
 *  @param handle       pointer to module handle
 *  @param cmd          ioctl command
 *  @param param        parameter of the ioctl command
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               08/24/2013, Created
 *
 *  @note
 */
alisl_retcode alisldmx_ioctl(alisl_handle handle,
							 unsigned int cmd, unsigned long param)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc = ERROR_INVAL;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	switch (cmd) {
		case DMX_IOCMD_GET_AUDIO_BITRATE: 
        {
			uint32_t bitrate;
			struct dmx_channel *ch;
			ch = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO);
			if (ch == NULL) {
				rc = ERROR_NOCH;
				break;
			}

			ioctl(ch->fd, ALI_DMX_GET_PROG_BITRATE, &bitrate);

			*(uint32_t *)param = bitrate;
			rc = 0;

			break;
		}

		case DMX_IOCMD_GET_FREE_BUF_LEN: {
			uint32_t pkt_cnt = 0;
            uint32_t main2see_free_buf_len = 0;
            uint32_t m2s_buf_total_size = 0;
			if (dev->feedfd > 0) {
				ioctl(dev->feedfd, ALI_DMX_HW_GET_FREE_BUF_LEN, &pkt_cnt);
                if(dev->av_start &&(dev->seefd > 0))
                {
                    ioctl(dev->seefd, ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN, &main2see_free_buf_len);
                    ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_GET, &m2s_buf_total_size);
                    pkt_cnt += (m2s_buf_total_size-main2see_free_buf_len);
                }
			} else {
				ioctl(dev->fd, ALI_DMX_HW_GET_FREE_BUF_LEN, &pkt_cnt);
			}
			    
			*(uint32_t *)param = pkt_cnt;
			rc = 0;

			break;
		}

		case DMX_IOCMD_GET_TOTAL_BUF_LEN: {//only for SW dmx
			uint32_t pkt_cnt=0;
            uint32_t m2s_buf_total_size = 0;
			ioctl(dev->feedfd, ALI_DMX_GET_TOTAL_BUF_LEN, &pkt_cnt);
            if(dev->av_start &&(dev->seefd > 0))
            {   
                ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_GET, &m2s_buf_total_size);
            }
			*(uint32_t *)param = pkt_cnt+m2s_buf_total_size;//0xbc000;
			rc = 0;
			break;
		}
        
		case DMX_IOCMD_SET_PLAYBACK_SPEED: {
			uint32_t speed;

			speed = param;
			rc = ioctl(dev->seefd, ALI_DMX_SEE_SET_PLAYBACK_SPEED, &speed);

			break;
		}

		case DMX_IOCMD_SET_DEC_HANDLE: {

		    rc = ioctl(dev->seefd, ALI_DMX_SEE_CRYPTO_DECRYPT_FD_SET,*((int *)param));
		    if(rc)
		    {
				SL_ERR("set dsc fd fail!\n");
		    }
			break;
		}
		#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 0)
		case DMX_IOCMD_BIND_CRYPTO_SESSION: {

		    rc = ioctl(dev->seefd,  ALI_DMX_SEE_BIND_CRYPTO_SESSION,*((int *)param));
		    if(rc)
		    {
				SL_ERR("bind crypt session fail!\n");
		    }
			break;
		}
		#endif
		case DMX_IOCMD_CRYPTO_START: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_CRYPTO_START, 0);

			break;
		}

		case DMX_IOCMD_CRYPTO_STOP: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_CRYPTO_STOP, 0);

			break;
		}

		case DMX_IOCMD_GET_DISCONTINUE_COUNT: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_GET_DISCONTINUE_COUNT, param);

			break;
		}

		case DMX_IOCMD_CLEAR_DISCONTINUE_COUNT: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_CLEAR_DISCONTINUE_COUNT, param);

			break;
		}

		case DMX_IOCMD_GET_MAIN2SEEBUF_REAMIN_DATA_LEN: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_GET_MAIN2SEEBUF_REAMIN_DATA_LEN, param);
			break;
		}

		case DMX_IOCMD_MAIN2SEE_BUF_VALIDSIZE_SET: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET, param);
			break;
		}

		case DMX_IOCMD_MAIN2SEE_BUF_VALIDSIZE_GET: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_GET, param);
			break;
		}

		case DMX_IOCMD_MAIN2SEE_SRC_SET: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_SRC_SET, param);
			break;
		}

		case DMX_IOCMD_MAIN2SEE_SRC_GET: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_SRC_GET, param);
			break;
		}

		case DMX_IOCMD_MAIN2SEE_BUF_REQ_SIZE: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_BUF_REQ_SIZE, param);
			break;
		}

		case DMX_IOCMD_MAIN2SEE_BUF_RET_SIZE: {
			rc = ioctl(dev->seefd, ALI_DMX_SEE_MAIN2SEE_BUF_RET_SIZE, param);
			break;
		}
		
		case DMX_IOCMD_DO_DDP_CERTIFICATION: {
			uint32_t enabel = 0;
			enabel = param;

			rc = ioctl(dev->seefd, ALI_DMX_SEE_DO_DDP_CERTIFICATION, &enabel);

			break;
		}

		case DMX_IOCMD_SET_HW_INFO: {
			rc = ioctl(dev->fd, ALI_DMX_SET_HW_INFO, param);
            		break;
		}        
    
		case DMX_IOCMD_SET_SAT2IP: {
			uint32_t enabel = 0;
			enabel = param;
			rc = ioctl(dev->seefd, ALI_DMX_SEE_SET_SAT2IP, &enabel);
            		break;
		}
		case DMX_IO_SET_FTA_REC_MODE:
			if(param == DMX_FTA_TO_FTA){
				rc = ioctl(dev->fd, ALI_DMX_IO_SET_FTA_REC_MODE,FTA_TO_FTA);
			}else if(param == DMX_FTA_TO_ENCRYPT){
				rc = ioctl(dev->fd, ALI_DMX_IO_SET_FTA_REC_MODE,FTA_TO_ENCRYPT);
			}else{
				return rc;
			}
			SL_DBG("set ALI_DMX_IO_SET_FTA_REC_MODE:0x%08x,0x%08x rc=%d success!",
			       ALI_DMX_IO_SET_FTA_REC_MODE, ALI_DMX_IO_GET_FTA_REC_MODE, rc);
			break;
		case DMX_IO_GET_FTA_REC_MODE:
			if((void *)param == NULL){
				return rc;
			}
			rc = ioctl(dev->fd, ALI_DMX_IO_GET_FTA_REC_MODE,param);
			SL_DBG("param:0x%08x value:%d!", param, *(int *)param);
			if(*(int *)param == (int)FTA_TO_FTA){
				*(unsigned long *)param = (unsigned long)DMX_FTA_TO_FTA;
			}else if(*(int *)param == (int)FTA_TO_ENCRYPT)
			{
				*(unsigned long *)param = (unsigned long)DMX_FTA_TO_ENCRYPT;
			}else{
				rc = ERROR_INVAL;
				return rc;
			}
			SL_DBG("get ALI_DMX_IO_GET_FTA_REC_MODE rc=%d success!\n", rc);
			break;
        case DMX_IOCMD_SET_RECORD_MODE:
            rc = ioctl(dev->fd, ALI_DMX_RECORD_MODE_SET, param);
            break;
        case DMX_IOCMD_SET_RECORD_BLOCKSIZE:
            rc = ioctl(dev->fd, ALI_DMX_RECORD_BLOCKSIZE_SET, param);
            break;
        case DMX_IOCMD_GET_RECORD_MODE:
			rc = ioctl(dev->fd, ALI_DMX_RECORD_MODE_GET, param);
			break;
		case DMX_IOCMD_GET_RECORD_BLOCKSIZE:
			rc = ioctl(dev->fd, ALI_DMX_RECORD_BLOCKSIZE_GET, param);
			break;
        case DMX_IOCMD_RCV_PKG_CNT_GET: {
            if (NULL == (void *)param) {
                SL_ERR("param is NULL!!!\n");
                rc = ERROR_INVAL;
                break;
			}
            unsigned long ts_pkg_count = 0;
            struct dmx_channel *ch_v = NULL;
            struct dmx_channel *ch_a = NULL;
            struct Ali_DmxDrvTsFltStatInfo info;
            memset(&info, 0, sizeof(struct Ali_DmxDrvTsFltStatInfo));
            
            ch_v = find_channel_by_stream_type(dev, DMX_STREAM_VIDEO);
            if (NULL != ch_v) {
                memset(&info, 0, sizeof(struct Ali_DmxDrvTsFltStatInfo));
                rc = ioctl(ch_v->fd, ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET, &info);
                ts_pkg_count += (unsigned long)info.TsInCnt;
            }

            ch_a = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO);
            if (NULL != ch_a) {
                memset(&info, 0, sizeof(struct Ali_DmxDrvTsFltStatInfo));
                rc = ioctl(ch_a->fd, ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET, &info);
                ts_pkg_count += (unsigned long)info.TsInCnt;
            }

            *(unsigned long *)param = ts_pkg_count;
            break;
        }
        case DMX_IOCMD_RCV_TS_PKG_CNT_GET_BY_PID: {
            if (NULL == (void *)param) {
                SL_ERR("param is NULL!!!\n");
                rc = ERROR_INVAL;
                break;
			}
            struct sl_dmx_get_ts_pkg_cnt_by_pid *io_param = (struct sl_dmx_get_ts_pkg_cnt_by_pid *)param;
            struct Ali_DmxDrvTsFltStatInfo info;
            memset(&info, 0, sizeof(struct Ali_DmxDrvTsFltStatInfo));
            struct dmx_channel *ch = NULL;
            /** notice: must be & 0x1FFF, because the pid have been changed in aui_dmx_av_stream_start */
            unsigned short pid = (unsigned short)((io_param->ul_pid) & DMX_MASK_PID);
            unsigned short vpid = (find_stream_pid(dev, DMX_STREAM_VIDEO)) & DMX_MASK_PID;
            unsigned short apid = (find_stream_pid(dev, DMX_STREAM_AUDIO)) & DMX_MASK_PID;
            ch = find_channel_by_pid(dev, pid);

            if (NULL != ch) {
                SL_INFO("pid: %d, vpid: %d, apid: %d, ch: %p, ch->fd: %d, output_format: %d, output_space: %d\n",
                    pid, vpid, apid, ch, ch->fd, ch->ch_param.output_format, ch->ch_param.output_space);
                if (pid == vpid) {
                    rc = ioctl(ch->fd, ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET, &info);
                } else if (pid == apid) {
                    rc = ioctl(ch->fd, ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET, &info);
                } else {
                    SL_ERR("pid[%d] is not supported for the channel\n", pid);
                    rc = ERROR_INVAL;
                    break;
                }
            }
            if (ERROR_INVAL == rc) {
                SL_ERR("ioctl return FAIL!!\n");
                break;
            }
            io_param->ul_ts_pkg_cnt = (unsigned long)info.TsInCnt;
            break;
        }
		case DMX_IOCMD_RESET_MAIN2SEEBUF_REAMIN_BUF:
			rc = ioctl(dev->seefd,ALI_DMX_RESET_MAIN2SEE_BUFF,param);
			break;
		default:
			break;
	}

	return rc;
}

alisl_retcode alisldmx_cache_set(alisl_handle handle,
								 dmx_cache_param_t* param)
{
	alisl_retcode rc = ERROR_INVAL;

	struct dmx_device *dev = (struct dmx_device *)handle;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	rc = ioctl(dev->fd, ALI_DMX_CACHE_SET, param);

	return rc;
}

alisl_retcode alisldmx_hw_buffer_clean(uint32_t dmx_index)
{
    alisl_retcode rc = ERROR_INVAL;
    int fd = -1;

    fd = open(dev_demux[dmx_index].path, O_RDONLY);
    if (fd < 0) {
        SL_DBG("%s open failed!\n", dev_demux[dmx_index].path);
        return ERROR_OPEN;
    }

    rc = ioctl(fd, ALI_DMX_HW_BUF_CLEAN, 0);
    close(fd);

    return rc;
}

alisl_retcode alisldmx_cache_retrace_set(alisl_handle handle, dmx_cache_retrace_param_t* param)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc = ERROR_INVAL;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	rc = ioctl(dev->fd, ALI_DMX_CACHE_RETRACE_SET, param);

	return rc;
}

alisl_retcode alisldmx_cache_retrace_start(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc = ERROR_INVAL;

	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}

	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}

	rc = ioctl(dev->fd, ALI_DMX_CACHE_RETRACE_START, 0);

	return rc;
}

alisl_retcode alisldmx_get_see_dmx_id(alisl_handle handle, int * id)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	alisl_retcode rc = ERROR_INVAL;
	if (dev == NULL) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	*id = dev->see_dmx_id;
	return rc;
}

alisl_retcode alisldmx_get_discontinue_pkt_cnt(alisl_handle handle, unsigned int *cnt)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_channel *ch_a=NULL, *ch_v=NULL;
	struct Ali_DmxDrvTsFltStatInfo info;
	int drv_ret = 0;

	if ((NULL == dev)||(NULL == cnt)) {
		SL_DBG("NULL dev data structure!");
		return ERROR_NULLDEV;
	}
	*cnt = 0;
	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!");
		return ERROR_NOOPEN;
	}
	ch_a = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO);
	ch_v = find_channel_by_stream_type(dev, DMX_STREAM_VIDEO);
	if (ch_a) {
		memset(&info, 0, sizeof(info));
		drv_ret = ioctl(ch_a->fd, ALI_DMX_CHAN_AUDIO_FILTER_INFO_GET, &info);
		if (drv_ret < 0)
			SL_ERR("a ALI_DMX_CHAN_TS_FILTER_INFO_GET fail!");
		*cnt += info.TsLostCnt;
	}
	if (ch_v) {
		memset(&info, 0, sizeof(info));
		drv_ret = ioctl(ch_v->fd, ALI_DMX_CHAN_VIDEO_FILTER_INFO_GET, &info);
		if (drv_ret < 0)
			SL_ERR("v ALI_DMX_CHAN_TS_FILTER_INFO_GET fail!");
		*cnt += info.TsLostCnt;
	}
	return 0;
}

alisl_retcode alisldmx_clear_discontinue_av_pkt_cnt(alisl_handle handle)
{
	struct dmx_device *dev = (struct dmx_device *)handle;
	struct dmx_channel *ch_a=NULL, *ch_v=NULL;
	int drv_ret = 0;

	if (NULL == dev) {
		SL_DBG("NULL dev data structure!\n");
		return ERROR_NULLDEV;
	}
	if (!flag_bit_test_any(&dev->status, DMX_STATUS_OPEN)) {
		SL_DBG("Not opened!\n");
		return ERROR_NOOPEN;
	}
	ch_a = find_channel_by_stream_type(dev, DMX_STREAM_AUDIO);
	ch_v = find_channel_by_stream_type(dev, DMX_STREAM_VIDEO);
	if (ch_a) {
		drv_ret = ioctl(ch_a->fd, ALI_DMX_CHAN_STATUS_RESET, 0);
		if (drv_ret < 0) {
			SL_ERR("a ALI_DMX_CHAN_STATUS_RESET fail!\n");
			return -1;
		}
	}
	if (ch_v) {
		drv_ret = ioctl(ch_v->fd, ALI_DMX_CHAN_STATUS_RESET, 0);
		if (drv_ret < 0) {
			SL_ERR("v ALI_DMX_CHAN_STATUS_RESET fail!\n");
			return -1;
		}
	}
	return 0;
}

void alisldmx_control_free_flt_behavior(int block) {
    BLOCK_CLOSE_FLT_WHILE_USR_CB_NOT_RETURN = block;
}


