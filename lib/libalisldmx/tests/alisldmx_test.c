/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alisldmx_test.c
 *  @brief          
 *
 *  @version        1.0
 *  @date           04/30/2014 01:57:31 PM
 *  @revision       none
 *
 *  @author         Christian xie <christian.xie@alitech.com>
 */
#include <stdlib.h>

#include <alislcli.h>
#include <unity_fixture.h>
#include <alisldmx.h>
#include <alipltfretcode.h>
#include <sltest_common.h>

alisl_handle dmx0_dev;
alisl_handle dmx1_dev;
alisl_handle dmx2_dev;

alisl_handle nim_hdl;
alisl_handle tsi_hdl;
alisl_handle dmx_hdl;
alisl_handle vdec_hdl;
alisl_handle snd_hdl;
static sltest_config config;

TEST_GROUP(SlDmxOpen);
TEST_SETUP(SlDmxOpen) {}
TEST_TEAR_DOWN(SlDmxOpen) {} 
TEST(SlDmxOpen, case1)
{
    const int max_time = 5;
	int i,j;
	alisl_handle dmx_dev;
    
    for (i = (int)DMX_ID_DEMUX0; i < (int)DMX_NB_DEMUX; i++)
	{		
		for (j = 0; j < max_time; j++) 
		{
        	CHECK(alisldmx_open(&dmx_dev, i, 0) == 0); 
		} 
    	for (j = 0; j < max_time; j++) 
		{
        	CHECK(alisldmx_close(dmx_dev) == 0);
    	}
	}
}

TEST(SlDmxOpen, case2)
{
	alisl_handle dmx_dev;

	TEST_ASSERT_EQUAL_INT(ERROR_INVAL, alisldmx_open(&dmx_dev, DMX_NB_DEMUX, 0)); 
	TEST_ASSERT_NOT_NULL(dmx_dev);
}


TEST_GROUP(SlDmxClose);
TEST_SETUP(SlDmxClose) {}
TEST_TEAR_DOWN(SlDmxClose) {} 
TEST(SlDmxClose, case1)
{
	int i;
	alisl_handle dmx_dev;
    
    for (i = (int)DMX_ID_DEMUX0; i < (int)DMX_NB_DEMUX; i++)
	{		
		CHECK(alisldmx_open(&dmx_dev, i, 0) == 0); 
		CHECK(alisldmx_close(dmx_dev) == 0);
	}
}

TEST(SlDmxClose, case2)
{
	alisl_handle dmx_dev;
	CHECK(alisldmx_open(&dmx_dev, DMX_ID_DEMUX0, 0) == 0); 
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_close(NULL)); 
	TEST_ASSERT_NOT_NULL(dmx_dev);
	CHECK(alisldmx_close(dmx_dev) == 0);
}

TEST_GROUP(SlDmxStart);
TEST_SETUP(SlDmxStart)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx1_dev, DMX_ID_DEMUX1, 0)); 
	TEST_ASSERT_NOT_NULL(dmx1_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx2_dev, DMX_ID_DEMUX2, 0)); 
	TEST_ASSERT_NOT_NULL(dmx2_dev);

}
TEST_TEAR_DOWN(SlDmxStart)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx1_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx1_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx2_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx2_dev));
} 

TEST(SlDmxStart, case1)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx1_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx2_dev));
}

TEST(SlDmxStart, case2)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));
}

TEST(SlDmxStart, case3)
{
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_start(NULL));
}


TEST_GROUP(SlDmxStop);
TEST_SETUP(SlDmxStop)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));
}
TEST_TEAR_DOWN(SlDmxStop)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxStop, case1)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
}

TEST(SlDmxStop, case2)
{
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_stop(NULL));
}

TEST(SlDmxStop, case3)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));	
}


TEST_GROUP(SlDmxPause);
TEST_SETUP(SlDmxPause)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));
}
TEST_TEAR_DOWN(SlDmxPause)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxPause, case1)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_pause(dmx0_dev));
}

TEST(SlDmxPause, case2)
{
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_pause(NULL));
}

TEST(SlDmxPause, case3)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_pause(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_pause(dmx0_dev));
}


TEST_GROUP(SlDmxChannelOpen);
TEST_SETUP(SlDmxChannelOpen)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));
}
TEST_TEAR_DOWN(SlDmxChannelOpen)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxChannelOpen, case1)
{
	unsigned int channelid = 0;
	unsigned short sec_pid = 0;
	
	alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SECTION, &channelid);
	alisldmx_set_channel_pid(dmx0_dev, channelid, sec_pid);
}

TEST(SlDmxChannelOpen, case2)
{
	unsigned short ts_pid = 512;
	unsigned int channelid = 0;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_TS;
	attr.enc_para = 0;

	alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_RECORD, &channelid);
	alisldmx_set_channel_attr(dmx0_dev, channelid, &attr);
	alisldmx_set_channel_pid(dmx0_dev, channelid, ts_pid);
}

TEST(SlDmxChannelOpen, case3)
{
	unsigned short service_pid = 16;
	unsigned int channelid = 0;

	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_PES;
	attr.enc_para = 0;
	alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SERVICE, &channelid);
	alisldmx_set_channel_attr(dmx0_dev, channelid, &attr);
	alisldmx_set_channel_pid(dmx0_dev, channelid, service_pid);
}

unsigned int channe_close_id = 0;
TEST_GROUP(SlDmxChannelClose);
TEST_SETUP(SlDmxChannelClose)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));

	
	unsigned short sec_pid = 0;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_PES;

	alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SECTION, &channe_close_id);
	alisldmx_set_channel_pid(dmx0_dev, channe_close_id, sec_pid);
}
TEST_TEAR_DOWN(SlDmxChannelClose)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxChannelClose, case1)
{
	TEST_ASSERT_EQUAL_INT(0,
		alisldmx_free_channel(dmx0_dev, channe_close_id));
}

TEST(SlDmxChannelClose, case2)
{
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_free_channel(NULL, channe_close_id));
}

unsigned int channe_start_id = 0;
TEST_GROUP(SlDmxChannelStart);
TEST_SETUP(SlDmxChannelStart)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));

	unsigned short sec_pid = 660;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_PES;

	alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SERVICE, &channe_start_id);
	alisldmx_set_channel_pid(dmx0_dev, channe_start_id, sec_pid);
}

TEST_TEAR_DOWN(SlDmxChannelStart)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_free_channel(dmx0_dev, channe_start_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxChannelStart, case1)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_start_id, DMX_CTRL_ENABLE));
}

TEST(SlDmxChannelStart, case2)
{
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_control_channel(NULL, channe_start_id, DMX_CTRL_ENABLE));
}

TEST(SlDmxChannelStart, case3)
{
	TEST_ASSERT_EQUAL_INT(ERROR_CHTYPE, alisldmx_control_channel(dmx0_dev, 0, DMX_CTRL_ENABLE));
}

TEST(SlDmxChannelStart, case4)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_start_id, DMX_CTRL_ENABLE));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_start_id, DMX_CTRL_ENABLE));
}

unsigned int channe_stop_id = 0;
TEST_GROUP(SlDmxChannelStop);
TEST_SETUP(SlDmxChannelStop)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));

	unsigned short sec_pid = 660;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_PES;

	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SERVICE, &channe_stop_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_channel_pid(dmx0_dev, channe_stop_id, sec_pid));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_stop_id, DMX_CTRL_ENABLE));
}

TEST_TEAR_DOWN(SlDmxChannelStop)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_free_channel(dmx0_dev, channe_stop_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxChannelStop, case1)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_stop_id, DMX_CTRL_DISABLE));
}

TEST(SlDmxChannelStop, case2)
{
	TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_control_channel(NULL, channe_stop_id, DMX_CTRL_DISABLE));
}

TEST(SlDmxChannelStop, case3)
{
	TEST_ASSERT_EQUAL_INT(ERROR_CHTYPE, alisldmx_control_channel(dmx0_dev, 0, DMX_CTRL_DISABLE));
}

TEST(SlDmxChannelStop, case4)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_stop_id, DMX_CTRL_DISABLE));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channe_stop_id, DMX_CTRL_DISABLE));
}

unsigned int filter_openclose_id = DMX_ILLEGAL_FILTERID;
unsigned int channel_openclose_id = 0;
TEST_GROUP(SlDmxFilterOpenClose);
TEST_SETUP(SlDmxFilterOpenClose)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));

	unsigned short sec_pid = 660;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_SEC;	
	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SECTION, &channel_openclose_id));
	alisldmx_set_channel_attr(dmx0_dev, channel_openclose_id, &attr);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_channel_pid(dmx0_dev, channel_openclose_id, sec_pid));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_openclose_id, DMX_CTRL_ENABLE));
}

TEST_TEAR_DOWN(SlDmxFilterOpenClose)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_openclose_id, DMX_CTRL_DISABLE));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_free_channel(dmx0_dev, channel_openclose_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxFilterOpenClose, case1)
{
	unsigned long 			ul_mask_val_len = 2;
	unsigned char           *puc_mask;
	unsigned char           *puc_val;
	unsigned char           *puc_reverse;
	
	puc_val = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_val)
	{
		alisldmx_free_filter(dmx0_dev, channel_openclose_id);
	}
	puc_mask = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_mask)
	{
		alisldmx_free_filter(dmx0_dev, channel_openclose_id);
	}
	puc_reverse = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_reverse)
	{
		alisldmx_free_filter(dmx0_dev, channel_openclose_id);
	}

	memset(puc_val, 0, ul_mask_val_len);
	memset(puc_mask, 0, ul_mask_val_len);
	memset(puc_reverse, 0, ul_mask_val_len);

	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_filter(dmx0_dev, channel_openclose_id, &filter_openclose_id));
	TEST_ASSERT_EQUAL_INT(0,alisldmx_set_filter(dmx0_dev, filter_openclose_id, ul_mask_val_len,
		puc_val, puc_mask, puc_reverse, 1));
	free(puc_val);
	free(puc_mask);
	free(puc_reverse);
}

TEST(SlDmxFilterOpenClose, case2)
{
	unsigned long 			ul_mask_val_len = 32;
	unsigned char           *puc_mask;
	unsigned char           *puc_val;
	unsigned char           *puc_reverse;
	
	puc_val = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_val)
	{
		alisldmx_free_filter(dmx0_dev, channel_openclose_id);
	}
	puc_mask = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_mask)
	{
		alisldmx_free_filter(dmx0_dev, channel_openclose_id);
	}
	puc_reverse = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_reverse)
	{
		alisldmx_free_filter(dmx0_dev, channel_openclose_id);
	}

	memset(puc_val, 0, ul_mask_val_len);
	memset(puc_mask, 0, ul_mask_val_len);
	memset(puc_reverse, 0, ul_mask_val_len);

	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_filter(dmx0_dev, channel_openclose_id, &filter_openclose_id));
	TEST_ASSERT_EQUAL_INT(ERROR_INVAL, alisldmx_set_filter(dmx0_dev, filter_openclose_id,
		ul_mask_val_len, puc_val, puc_mask, puc_reverse, 1));
	free(puc_val);
	free(puc_mask);
	free(puc_reverse);
}


unsigned int filter_startstop_id = DMX_ILLEGAL_FILTERID;
unsigned int channel_startstop_id = 0;
unsigned long 			ul_mask_val_len = 2;
unsigned char           *puc_mask;
unsigned char           *puc_val;
unsigned char           *puc_reverse;

TEST_GROUP(SlDmxFilterStartStop);
TEST_SETUP(SlDmxFilterStartStop)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));

	unsigned short sec_pid = 660;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_SEC;	
	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SECTION, &channel_startstop_id));
	alisldmx_set_channel_attr(dmx0_dev, channel_startstop_id, &attr);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_channel_pid(dmx0_dev, channel_startstop_id, sec_pid));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_startstop_id, DMX_CTRL_ENABLE));
	
	puc_val = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_val)
	{
		alisldmx_free_filter(dmx0_dev, channel_startstop_id);
	}
	puc_mask = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_mask)
	{
		alisldmx_free_filter(dmx0_dev, channel_startstop_id);
	}
	puc_reverse = (unsigned char *)malloc(ul_mask_val_len);
	if (NULL == puc_reverse)
	{
		alisldmx_free_filter(dmx0_dev, channel_startstop_id);
	}

	memset(puc_val, 0, ul_mask_val_len);
	memset(puc_mask, 0, ul_mask_val_len);
	memset(puc_reverse, 0, ul_mask_val_len);

	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_filter(dmx0_dev, channel_startstop_id, &filter_startstop_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_filter(dmx0_dev, filter_startstop_id,
		ul_mask_val_len, puc_val, puc_mask, puc_reverse, 1));
	
}
TEST_TEAR_DOWN(SlDmxFilterStartStop)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_startstop_id, DMX_CTRL_DISABLE));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_free_channel(dmx0_dev, channel_startstop_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
	free(puc_val);
	free(puc_mask);
	free(puc_reverse);
} 

TEST(SlDmxFilterStartStop, case1)
{
	if (DMX_ILLEGAL_FILTERID != filter_startstop_id)
	{
		TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_startstop_id, DMX_CTRL_ENABLE));
	}
	if (DMX_ILLEGAL_FILTERID != filter_startstop_id)
	{
		TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_startstop_id, DMX_CTRL_DISABLE));
	}
}

TEST(SlDmxFilterStartStop, case2)
{
	if (DMX_ILLEGAL_FILTERID != filter_startstop_id)
	{
		TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_control_filter(NULL, filter_startstop_id, DMX_CTRL_ENABLE));
	}
	if (DMX_ILLEGAL_FILTERID != filter_startstop_id)
	{
		TEST_ASSERT_EQUAL_INT(ERROR_NULLDEV, alisldmx_control_filter(NULL, filter_startstop_id, DMX_CTRL_DISABLE));
	}

}

unsigned int filter_getdata_id = DMX_ILLEGAL_FILTERID;
unsigned int channel_getdata_id = 0;
static unsigned char section_buffer[4*1024];
/**********************Local function****************/ 
typedef long (*aui_p_fun_sectionCB)(unsigned char *p_section_data_buf_addr,unsigned long ul_section_data_len,void *usr_data,void *reserved);
typedef long (*aui_p_fun_data_req_wtCB)(void *p_user_hdl, unsigned long ul_req_size, void ** pp_req_buf, unsigned long *req_buf_size, void *pst_ctrl_blk);
typedef long (*aui_p_fun_data_up_wtCB)(void *p_user_hdl, unsigned long ul_size);

/** The attributes of DMX channel devices */
typedef struct sl_st_attr_dmx_filter
{
	/** The user callback that will be called when driver receive the target section data */
	aui_p_fun_sectionCB p_fun_sectionCB ;
	/** The user callback that will be called at first when driver receive the target PES/ES/TS raw data. \n 
	This fuction is used to get the valid application buffer address and length for storing the target data*/
	aui_p_fun_data_req_wtCB p_fun_data_req_wtCB ;
	/** The user callback that will be called after DMX send target data to the buffer request by callback function p_fun_data_req_wtCB. 
	It be used to notice the applicaton that DMX have copyed the target data to application buffer. \n */
	aui_p_fun_data_up_wtCB p_fun_data_up_wtCB ;

    /** The user priv data */
	void *usr_data;
}sl_attr_dmx_filter,*sl_p_attr_dmx_filter;


long fun_sectionCB
(
	unsigned char *p_section_data_buf_addr,
	unsigned long ul_section_data_len,
	void *usr_data,
	void *reserved
)
{
	printf("Got data\n");
	printf("User data is %08x\n", *(int *)usr_data);
}

static void sldmx_requestbuf_callback(void *private,
			unsigned long channelid,
			unsigned long filterid,
			unsigned long length,
			unsigned char  **buffer,
			unsigned long *actlen)
{
	struct sl_st_attr_dmx_filter *filter = private;

	*buffer = section_buffer;
	*actlen = (length <= 4*1024) ?
	           length : 4*1024;
	return ;
}

static void sldmx_updatebuf_callback(void *private,
			unsigned long channelid,
			unsigned long filterid,
			unsigned long valid_len,
			unsigned short offset)
{
	(void)offset;
	struct sl_st_attr_dmx_filter *filter = private;

	if (NULL == filter->p_fun_sectionCB)
	{
		return ;
	}

	filter->p_fun_sectionCB(
		section_buffer,
		valid_len,
		filter->usr_data,
		NULL);

	return ;
}
/**********end of local function********************/

TEST_GROUP(SlDmxFilterGetData);
TEST_SETUP(SlDmxFilterGetData)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_open(&dmx0_dev, DMX_ID_DEMUX0, 0)); 
	TEST_ASSERT_NOT_NULL(dmx0_dev);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_start(dmx0_dev));

	unsigned short sec_pid = 660;
	struct dmx_channel_attr attr;
	memset(&attr, 0, sizeof(attr));
	attr.output_format = DMX_OUTPUT_FORMAT_SEC;	
	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_channel(dmx0_dev, DMX_CHANNEL_SECTION, &channel_getdata_id));
	alisldmx_set_channel_attr(dmx0_dev, channel_getdata_id, &attr);
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_channel_pid(dmx0_dev, channel_getdata_id, sec_pid));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_getdata_id, DMX_CTRL_ENABLE));
	 
	TEST_ASSERT_EQUAL_INT(0, alisldmx_allocate_filter(dmx0_dev, channel_getdata_id, &filter_getdata_id));
	
}

TEST_TEAR_DOWN(SlDmxFilterGetData)
{
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_getdata_id, DMX_CTRL_DISABLE));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_free_channel(dmx0_dev, channel_getdata_id));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_stop(dmx0_dev));
	TEST_ASSERT_EQUAL_INT(0, alisldmx_close(dmx0_dev));
} 

TEST(SlDmxFilterGetData, case1)
{
	struct dmx_channel_callback cb;
	sl_attr_dmx_filter filter;
	unsigned char mask[] = {0};
	unsigned char value[] = {0};
	unsigned char reverse[] = {0xFF};
	static int DmxFilterGetData_usr_data= 0xAA55AA55;
	filter.usr_data = &DmxFilterGetData_usr_data;
	filter.p_fun_sectionCB = fun_sectionCB;
	
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_filter(dmx0_dev, filter_getdata_id, 1, value, mask, reverse, 1));

	memset(&cb, 0, sizeof(cb));
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)sldmx_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)sldmx_updatebuf_callback;
	cb.priv = (void *)&filter;
	TEST_ASSERT_EQUAL_INT(0, alisldmx_register_channel_callback(dmx0_dev, channel_getdata_id, &cb));
	
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really got section data continuously?", 'Y', 'n'));
}

TEST(SlDmxFilterGetData, case2)
{
	struct dmx_channel_callback cb;
	sl_attr_dmx_filter filter;
	unsigned char mask[] = {0};
	unsigned char value[] = {0};
	unsigned char reverse[] = {0xFF};
	static int DmxFilterGetData_usr_data= 0xAA55AA55;
	filter.usr_data = &DmxFilterGetData_usr_data;
	filter.p_fun_sectionCB = fun_sectionCB;

	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_filter(dmx0_dev, filter_getdata_id, 1, value, mask, reverse, 1));

	memset(&cb, 0, sizeof(cb));
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)sldmx_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)sldmx_updatebuf_callback;
	cb.priv = (void *)&filter;
	TEST_ASSERT_EQUAL_INT(0, alisldmx_register_channel_callback(dmx0_dev, channel_getdata_id, &cb));
		
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really got section data continuously?", 'Y', 'n'));

	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_DISABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really stop getting section data?", 'Y', 'n'));
}

TEST(SlDmxFilterGetData, case3)
{
	struct dmx_channel_callback cb;
	sl_attr_dmx_filter filter;
	unsigned char mask[] = {0};
	unsigned char value[] = {0};
	unsigned char reverse[] = {0xFF};
	static int DmxFilterGetData_usr_data= 0xAA55AA55;
	filter.usr_data = &DmxFilterGetData_usr_data;
	filter.p_fun_sectionCB = fun_sectionCB;
	
	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_filter(dmx0_dev, filter_getdata_id, 1, value, mask, reverse, 1));

	memset(&cb, 0, sizeof(cb));
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)sldmx_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)sldmx_updatebuf_callback;
	cb.priv = (void *)&filter;
	TEST_ASSERT_EQUAL_INT(0, alisldmx_register_channel_callback(dmx0_dev, channel_getdata_id, &cb));
		
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really got section data continuously?", 'Y', 'n'));

	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_getdata_id, DMX_CTRL_DISABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really stop getting section data?", 'Y', 'n'));
}

TEST(SlDmxFilterGetData, case4)
{
	struct dmx_channel_callback cb;
	sl_attr_dmx_filter filter;
	unsigned char mask[] = {0};
	unsigned char value[] = {0};
	unsigned char reverse[] = {0xFF};
	static int DmxFilterGetData_usr_data= 0xAA55AA55;
	filter.usr_data = &DmxFilterGetData_usr_data;
	filter.p_fun_sectionCB = fun_sectionCB;

	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_filter(dmx0_dev, filter_getdata_id, 1, value, mask, reverse, 1));

	memset(&cb, 0, sizeof(cb));
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)sldmx_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)sldmx_updatebuf_callback;
	cb.priv = (void *)&filter;
	TEST_ASSERT_EQUAL_INT(0, alisldmx_register_channel_callback(dmx0_dev, channel_getdata_id, &cb));
		
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really got section data continuously?", 'Y', 'n'));

	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_DISABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really stop getting section data?", 'Y', 'n'));

	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really start getting section data again?", 'Y', 'n'));
}

TEST(SlDmxFilterGetData, case5)
{
	struct dmx_channel_callback cb;
	sl_attr_dmx_filter filter;
	unsigned char mask[] = {0};
	unsigned char value[] = {0};
	unsigned char reverse[] = {0xFF};
	static int DmxFilterGetData_usr_data= 0xAA55AA55;
	filter.usr_data = &DmxFilterGetData_usr_data;
	filter.p_fun_sectionCB = fun_sectionCB;

	TEST_ASSERT_EQUAL_INT(0, alisldmx_set_filter(dmx0_dev, filter_getdata_id, 1, value, mask, reverse, 1));

	memset(&cb, 0, sizeof(cb));
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)sldmx_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)sldmx_updatebuf_callback;
	cb.priv = (void *)&filter;
	TEST_ASSERT_EQUAL_INT(0, alisldmx_register_channel_callback(dmx0_dev, channel_getdata_id, &cb));
		
	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_filter(dmx0_dev, filter_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really got section data continuously?", 'Y', 'n'));

	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_getdata_id, DMX_CTRL_DISABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really stop getting section data?", 'Y', 'n'));

	TEST_ASSERT_EQUAL_INT(0, alisldmx_control_channel(dmx0_dev, channel_getdata_id, DMX_CTRL_ENABLE));

	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Really start getting section data again?", 'Y', 'n'));
}

/*
 *  @brief          prepare to play a live stream.
 *
 *  @param[in]      nim_cnt     which nim is selected.
 *  @param[in]      group       which stream selected to play, set in sltest.ini
 *
 *  @return         0 - success
 *
 *  @author         christian <christian.xie@alitech.com>
 *  @date           05/08/2014 03:39:26 PM
 *
 *  @note
 */
int sltest_prepare_stream(int nim_cnt, const char *group, sltest_config *config)
{
    sltest_config test_config;
    test_frequency_info freq_info;
    test_dvbs_nim_config dvbs_config;
    test_tsi_config tsi_config;
    dmx_id_t i_dmx;

    if (0 != sltest_get_config(&test_config, nim_cnt, group)) {
        printf("No config file!\n");
        return -1;
    }

	i_dmx = test_config.dmxID;
    memcpy(&freq_info, &(test_config.freq_config), sizeof(freq_info));
    memcpy(&dvbs_config, &(test_config.dvbs_config), sizeof(dvbs_config));
    memcpy(&tsi_config, &(test_config.tsi_config), sizeof(tsi_config));
    printf("freq: %d, sym: %d, vpid: %d, apid: %d, ppid: %d\n",
           freq_info.freq, freq_info.sym_rate, freq_info.v_pid,
           freq_info.a_pid, freq_info.p_pid);
    memcpy(config, &test_config, sizeof(test_config));
	
    if (0 != sltest_nim_start_dvbs(&nim_hdl, &dvbs_config,
                                   freq_info.freq, freq_info.sym_rate, 0)) {
        return -1;
    }

    if (0 != sltest_tsi_start(&tsi_hdl, &tsi_config)) {
        return -1;
    }

	if( 0 != sltest_display_start()) {
		return -1;
	}

	if (0 != sltest_vdec_start(&vdec_hdl, NULL, freq_info.v_type)) {
        return -1;
    }

    if (0 != sltest_snd_start(&snd_hdl, 30, freq_info.a_type)) {
        return -1;
    }

    return 0;
}

TEST_GROUP(SlDmxPlayStream);

TEST_SETUP(SlDmxPlayStream)
{
	sltest_prepare_stream(1, "dvbs1", &config);
}

TEST_TEAR_DOWN(SlDmxPlayStream)
{
	sltest_stop_stream();
}

TEST(SlDmxPlayStream, PlayStream)
{
	TEST_ASSERT_EQUAL_INT(0, sltest_dmx_start(&dmx_hdl, DMX_ID_DEMUX0, &(config.freq_config))); 
	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("video start play ?", 'Y', 'n'));

    CHECK(alisldmx_avstop(dmx_hdl) == 0);
	
	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Video stop play?", 'Y', 'n'));

	CHECK(alisldmx_avstart(dmx_hdl) == 0); 
	
	TEST_ASSERT_EQUAL_INT8('y',
		cli_check_result("Video start play?", 'Y', 'n'));

}

TEST_GROUP_RUNNER(SlDmxOpen)
{
	RUN_TEST_CASE(SlDmxOpen, case1);
	RUN_TEST_CASE(SlDmxOpen, case2);
}

TEST_GROUP_RUNNER(SlDmxClose)
{
	RUN_TEST_CASE(SlDmxClose, case1);
	RUN_TEST_CASE(SlDmxClose, case2);
}

TEST_GROUP_RUNNER(SlDmxStart)
{
	RUN_TEST_CASE(SlDmxStart, case1);
	RUN_TEST_CASE(SlDmxStart, case2);
	RUN_TEST_CASE(SlDmxStart, case3);
}

TEST_GROUP_RUNNER(SlDmxStop)
{
	RUN_TEST_CASE(SlDmxStop, case1);
	RUN_TEST_CASE(SlDmxStop, case2);
	RUN_TEST_CASE(SlDmxStop, case3);
}

TEST_GROUP_RUNNER(SlDmxPause)
{
	RUN_TEST_CASE(SlDmxPause, case1);
	RUN_TEST_CASE(SlDmxPause, case2);
	RUN_TEST_CASE(SlDmxPause, case3);
}


TEST_GROUP_RUNNER(SlDmxChannelOpen)
{
	RUN_TEST_CASE(SlDmxChannelOpen, case1);
	RUN_TEST_CASE(SlDmxChannelOpen, case2);
	RUN_TEST_CASE(SlDmxChannelOpen, case3);
}

TEST_GROUP_RUNNER(SlDmxChannelClose)
{
	RUN_TEST_CASE(SlDmxChannelClose, case1);
	RUN_TEST_CASE(SlDmxChannelClose, case2);
}

TEST_GROUP_RUNNER(SlDmxChannelStart)
{
	RUN_TEST_CASE(SlDmxChannelStart, case1);
	RUN_TEST_CASE(SlDmxChannelStart, case2);
	RUN_TEST_CASE(SlDmxChannelStart, case3);
	RUN_TEST_CASE(SlDmxChannelStart, case4);
}

TEST_GROUP_RUNNER(SlDmxChannelStop)
{
	RUN_TEST_CASE(SlDmxChannelStop, case1);
	RUN_TEST_CASE(SlDmxChannelStop, case2);
	RUN_TEST_CASE(SlDmxChannelStop, case3);
	RUN_TEST_CASE(SlDmxChannelStop, case4);
}

TEST_GROUP_RUNNER(SlDmxFilterOpenClose)
{
	RUN_TEST_CASE(SlDmxFilterOpenClose, case1);
	RUN_TEST_CASE(SlDmxFilterOpenClose, case2);
}

TEST_GROUP_RUNNER(SlDmxFilterStartStop)
{
	RUN_TEST_CASE(SlDmxFilterStartStop, case1);
	RUN_TEST_CASE(SlDmxFilterStartStop, case2);
}

TEST_GROUP_RUNNER(SlDmxFilterGetData)
{
	RUN_TEST_CASE(SlDmxFilterGetData, case1);
	RUN_TEST_CASE(SlDmxFilterGetData, case2);
	RUN_TEST_CASE(SlDmxFilterGetData, case3);
	RUN_TEST_CASE(SlDmxFilterGetData, case4);
	RUN_TEST_CASE(SlDmxFilterGetData, case5);
}

TEST_GROUP_RUNNER(SlDmxPlayStream)
{
	RUN_TEST_CASE(SlDmxPlayStream, PlayStream);
}

static void run_dmx_test_all()
{
	RUN_TEST_GROUP(SlDmxOpen);
	RUN_TEST_GROUP(SlDmxClose);
	RUN_TEST_GROUP(SlDmxStart);	
	RUN_TEST_GROUP(SlDmxStop);
	RUN_TEST_GROUP(SlDmxPause);
	RUN_TEST_GROUP(SlDmxChannelOpen);
	RUN_TEST_GROUP(SlDmxChannelClose);
	RUN_TEST_GROUP(SlDmxChannelStart);
	RUN_TEST_GROUP(SlDmxChannelStop);
	RUN_TEST_GROUP(SlDmxFilterOpenClose);
	RUN_TEST_GROUP(SlDmxFilterStartStop);
	RUN_TEST_GROUP(SlDmxFilterGetData);
	RUN_TEST_GROUP(SlDmxPlayStream);
}

static void run_dmx_test_open()
{
	RUN_TEST_GROUP(SlDmxOpen);
}

static void run_dmx_test_close()
{
	RUN_TEST_GROUP(SlDmxClose);
}

static void run_dmx_test_start()
{
	RUN_TEST_GROUP(SlDmxStart);
}

static void run_dmx_test_stop()
{
	RUN_TEST_GROUP(SlDmxStop);
}

static void run_dmx_test_pause()
{
	RUN_TEST_GROUP(SlDmxPause);
}

static void run_dmx_test_channel_open()
{
	RUN_TEST_GROUP(SlDmxChannelOpen);
}

static void run_dmx_test_channel_close()
{
	RUN_TEST_GROUP(SlDmxChannelClose);
}
static void run_dmx_test_channel_start()
{
	RUN_TEST_GROUP(SlDmxChannelStart);
}

static void run_dmx_test_channel_stop()
{
	RUN_TEST_GROUP(SlDmxChannelStop);
}

static void run_dmx_test_filter_openclose()
{
	RUN_TEST_GROUP(SlDmxFilterOpenClose);
}

static void run_dmx_test_filter_startstop()
{
	RUN_TEST_GROUP(SlDmxFilterStartStop);
}

static void run_dmx_test_filter_getdata()
{
	RUN_TEST_GROUP(SlDmxFilterGetData);
}

static void run_dmx_play_stream()
{
	RUN_TEST_GROUP(SlDmxPlayStream);
}

/*******************UnityMain register********************/
static int run_dmx_test_all_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_all);

	return 0;
}

static int run_dmx_test_open_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_open);

	return 0;
}

static int run_dmx_test_close_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_close);

	return 0;
}

static int run_dmx_test_start_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_start);

	return 0;
}

static int run_dmx_test_stop_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_stop);

	return 0;
}

static int run_dmx_test_pause_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_pause);

	return 0;
}

static int run_dmx_test_channel_open_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_channel_open);

	return 0;
}

static int run_dmx_test_channel_close_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_channel_close);

	return 0;
}

static int run_dmx_test_channel_start_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_channel_start);

	return 0;
}

static int run_dmx_test_channel_stop_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_channel_stop);

	return 0;
}

static int run_dmx_test_filter_openclose_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_filter_openclose);

	return 0;
}

static int run_dmx_test_filter_startstop_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_filter_startstop);

	return 0;
}

static int run_dmx_test_filter_getdata_group(int argc, char** argv)
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_test_filter_getdata);

	return 0;
}


static int run_dmx_test_play_stream_group(int argc, char *argv[]) 
{
	cli_ignore_check_result(1);

	UnityMain(argc, argv, run_dmx_play_stream);
	
	return 0;
}

/*****************************************************************/

void alisldmx_test_register(struct cli_command *parent)
{
	struct cli_command *dmx;
	
	dmx = cli_register_command(parent,"All", run_dmx_test_all_group,
		   						CLI_CMD_MODE_SELF, "Run all dmx test case");
	dmx = cli_register_command(parent,"1", run_dmx_test_open_group,
		   						CLI_CMD_MODE_SELF, "DMX open test");
	dmx = cli_register_command(parent,"2", run_dmx_test_close_group,
		   						CLI_CMD_MODE_SELF, "DMX close test");
	dmx = cli_register_command(parent,"3", run_dmx_test_start_group,
		   						CLI_CMD_MODE_SELF, "DMX start test");
	dmx = cli_register_command(parent,"4", run_dmx_test_stop_group,
		   						CLI_CMD_MODE_SELF, "DMX stop test");
	dmx = cli_register_command(parent,"5", run_dmx_test_pause_group,
		   						CLI_CMD_MODE_SELF, "DMX pause test");
	dmx = cli_register_command(parent,"6", run_dmx_test_channel_open_group,
		   						CLI_CMD_MODE_SELF, "Channel open test");
	dmx = cli_register_command(parent,"7", run_dmx_test_channel_close_group,
		   						CLI_CMD_MODE_SELF, "Channel close test");
	dmx = cli_register_command(parent,"8", run_dmx_test_channel_start_group,
		   						CLI_CMD_MODE_SELF, "Channel start test");
	dmx = cli_register_command(parent,"9", run_dmx_test_channel_stop_group,
								CLI_CMD_MODE_SELF, "Channel stop test");
	dmx = cli_register_command(parent,"10", run_dmx_test_filter_openclose_group,
								CLI_CMD_MODE_SELF, "Filter openclose test");
	dmx = cli_register_command(parent,"11", run_dmx_test_filter_startstop_group,
		   						CLI_CMD_MODE_SELF, "Filter startstop test");
	dmx = cli_register_command(parent,"12", run_dmx_test_filter_getdata_group,
		   						CLI_CMD_MODE_SELF, "Filter get data test");
	dmx = cli_register_command(parent,"13", run_dmx_test_play_stream_group,
		   						CLI_CMD_MODE_SELF, "DMX play stream test");
	return;
}

