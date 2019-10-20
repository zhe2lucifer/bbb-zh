#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <alisldmx.h>

#include "lib_ota.h"
#include "lib_ota_dbc.h"

static unsigned long g_oui         =  (SYS_OUI & 0xffffff);
static unsigned short g_model      =  (SYS_HW_MODEL & 0xffff);
static unsigned short g_hw_version =  (SYS_HW_VERSION & 0xffff);
static unsigned short g_sw_model   =  (SYS_SW_MODEL & 0xffff);
static unsigned short g_sw_version =  (SYS_SW_VERSION & 0xffff);

static unsigned char* g_ota_ram1_addr = NULL;
static unsigned long g_ota_ram1_len   = 0;
static unsigned long g_ota_size       = 0;


static struct DCGroupInfo g_dc_group;
/* for linux 128M solution ota allcode upgrade, lib_ota_dbc.c also define */
#define DC_MODULE_MAX  128
static struct DCModuleInfo g_dc_module[DC_MODULE_MAX];
static unsigned char 	g_dc_module_num;
static unsigned short	g_dc_blocksize;

#define OTA_SECTION_BUF_LEN 5000
static unsigned char g_ota_section_buf[OTA_SECTION_BUF_LEN];

static struct dl_info g_dl_info;

static long received_section_num;
static long total_block_num;
static t_progress_disp download_progress_disp;

//#define DMX_CHID2INDEX(channel_id) ((channel_id) & 0xFFFF)
#define DMX_HLD_PORTING_SEC_REQ_DEFAULT_DELAY 6000
#define DMX_MAX_SECTION_SIZE    4096
static uint8_t tmp_buffer[DMX_MAX_SECTION_SIZE];

struct section_private
{
	alisl_handle handle;
	struct get_section_param *sec_para;
};

static long parse_dsmcc_sec(unsigned char *buffer, long buffer_length, void *param)
{
	unsigned char byte_hi, byte_lo;
	unsigned char table_id, section_num;
	unsigned short section_len, table_id_extension;
	unsigned short new_sw_version;
	int ret;

	struct section_param *sec_param = (struct section_param *)param;

	//int i =0;
	//printf("\n %d \n",buffer_length);
	//for(; i<buffer_length; i++)
	//{
	//	printf("%02x ",*(buffer+i));
	//}
	//printf("\n %d \n",buffer_length);

	table_id = *buffer++;
	byte_hi = *buffer++;
	byte_lo = *buffer++;
	section_len = ((byte_hi & 0x0f) << 8) + byte_lo;
	byte_hi = *buffer++;
	byte_lo = *buffer++;
	table_id_extension = (byte_hi << 8) + byte_lo;
	byte_lo = *buffer++;
	section_num = *buffer++;
//	last_section_num = *buffer++;

	//OTA_PRINTF("sec_param->section_type = %d section_len = %d\n", sec_param->section_type, section_len);

	switch (sec_param->section_type)
	{



		case DSI_NUM:

			/*------------------STEP_DSI---------------------------*/
			if (table_id_extension != 0x0000 && table_id_extension != 0x0001)
			{
				return OTA_FAILURE;
			}

			ret = alisl_dsi_message(buffer,
			                        section_len - 9,
			                        g_oui,
			                        g_model,
			                        g_hw_version,
			                        g_sw_model,
			                        g_sw_version,
			                        &new_sw_version,
			                        &g_dc_group);

			if (ret == DBC_SUCCESS)
			{
				g_dl_info.hw_version = (unsigned short)g_hw_version;
				g_dl_info.sw_version = new_sw_version;
				g_dl_info.sw_type = 0;
				g_dl_info.sw_size = g_dc_group.group_size;
				return OTA_SUCCESS;
			}

			OTA_PRINTF("table_id %d,section_len %d,table_id_extension%d\n",table_id,section_len,table_id_extension);
			return OTA_FAILURE;

		case DII_NUM:

			/*------------------STEP_DII---------------------------*/
			if (table_id_extension != (g_dc_group.group_id & 0xffff))
			{
				return OTA_FAILURE;
			}

			ret = alisl_dii_message(buffer,
			                        section_len - 9,
			                        g_dc_group.group_id,
			                        g_ota_ram1_addr,
			                        g_dc_module,
			                        &g_dc_module_num,
			                        &g_dc_blocksize);

			return (ret == DBC_SUCCESS) ? OTA_SUCCESS : OTA_FAILURE;

		case DDB_NUM:

			/*------------------STEP_DDB---------------------------*/
			if (table_id_extension != g_dc_module[sec_param->param8].module_id)
			{
				return OTA_FAILURE;
			}

			ret = alisl_ddb_data(buffer,
			                     section_len - 9,
			                     &g_dc_module[sec_param->param8],
			                     g_dc_blocksize,
			                     section_num);

			return (ret == DBC_SUCCESS) ? OTA_SUCCESS : OTA_FAILURE;

		default:
			return OTA_FAILURE;
	}
}

static void ddb_section(struct get_section_param *sr_request)
{
	struct section_param sec_param;
	unsigned char* section_buf;
	unsigned short table_id_extension;
	unsigned int  index;
	struct modules_range* p_mdl_range;

	p_mdl_range = sr_request->p_mdl_range;
	section_buf = sr_request->buff;
	table_id_extension = (section_buf[3] << 8) + section_buf[4];

	if(p_mdl_range->end >= g_dc_module_num)
	{
		OTA_PRINTF("[%s(%d)] Invalid range !!!\n", __FUNCTION__, __LINE__);
		return;
	}

	for(index = p_mdl_range->start; index <= p_mdl_range->end; index++)
	{
		if (g_dc_module[index].module_id == table_id_extension)
		{
			break;
		}
	}

	if (index > p_mdl_range->end)
	{
		return;
	}
	
	if ((unsigned char)(g_dc_module[index].block_num - 1) != section_buf[7])
	{
		OTA_PRINTF("last_section_num wrong\n");
		return;
	}

	sec_param.section_type = DDB_NUM;
	sec_param.param8 = index;

	/* if parse_dsmcc_sec return 1, parse this section fail */
	if (parse_dsmcc_sec(sr_request->buff, sr_request->buff_len, (void*)(&sec_param)))
	{
		return;
	}

	for(index = p_mdl_range->start; index <= p_mdl_range->end; index++)
	{
		if (g_dc_module[index].module_download_finish != 1)
		{
			break;
		}
	}

	if (index == (p_mdl_range->end + 1))
	{
		sr_request->continue_get_sec = 0;
		pthread_mutex_unlock(&sr_request->continue_mutex);
	}
 	
	received_section_num ++;
}

static void sec_request_buf(void *private, uint32_t channel_id,
                            uint32_t filter_id, uint32_t length,
                            uint8_t  **buffer, uint32_t *actlen)
{
	if (DMX_MAX_SECTION_SIZE < length)
	{
		*buffer = NULL;
		*actlen = 0;
		return ;
	}

	*buffer = tmp_buffer;
	*actlen = length;
}

static void sec_update_buf(void *private, uint32_t channel_id,
                           uint32_t filter_id, uint32_t valid_len, uint16_t offset)
{
	(void)offset;
//	alisl_handle dmx_handle;
	struct dmx_channel_attr attr;
	struct section_private *priv;
	struct get_section_param *sec_para;

	priv = (struct section_private *)private;
	sec_para = priv->sec_para;
//	dmx_handle = priv->handle;
	sec_para->dmx_handle = priv->handle;
	sec_para->channel_id = channel_id;	
	memset(&attr, 0, sizeof(attr));

	if (DMX_MAX_SECTION_SIZE >= valid_len)
	{
		memcpy(sec_para->buff, tmp_buffer, valid_len);
		sec_para->sec_tbl_len = valid_len;
	}

	if (sec_para->get_sec_cb)
	{
		sec_para->get_sec_cb(sec_para);
	}
}

static alisl_handle dmx_handle = NULL;
#if 1



static int check_continue_thread(struct get_section_param *sec_para)
{
	pthread_mutex_lock(&sec_para->continue_mutex);
	alisldmx_control_channel(sec_para->dmx_handle, sec_para->channel_id, DMX_CTRL_DISABLE); 
	alisldmx_section_request_poll_release(sec_para->dmx_handle);
	OTA_PRINTF("Download finish, stop channel! \n");	
	pthread_mutex_unlock(&sec_para->continue_mutex);
        return 0;
}

static alisl_retcode dmx_req_check_continue(struct get_section_param *sec_para)
{
	pthread_t check_continue_tid;
	pthread_create(&check_continue_tid, NULL, (void*)check_continue_thread, sec_para);
	return ALISLUPG_ERR_NONE;
}

static int dmx_req_section(int dmx_id, struct get_section_param *sec_para)
{
	struct dmx_channel_attr attr;
	struct dmx_channel_callback cb;
	struct section_private priv;
	uint32_t channel_id, filter_id;
    unsigned int mask;
	uint8_t mode[32];
	unsigned long wait_delay = 0;
	unsigned long flg_got = 0;
	int i = 0, ret = OTA_SUCCESS;
	int timeout = 0;

	if (0 != alisldmx_open(&dmx_handle, dmx_id, 0))
	{
		OTA_PRINTF("dmx open fail, %s, L:%d\n", __FUNCTION__, __LINE__);
		return OTA_FAILURE;
	}

	if (0 != alisldmx_start(dmx_handle))
	{
		OTA_PRINTF("dmx start fail, %s, L:%d\n", __FUNCTION__, __LINE__);
		return OTA_FAILURE;
	}

	memset(&attr, 0, sizeof(attr));
	attr.continuous = sec_para->continue_get_sec;

	if (sec_para->get_sec_cb == NULL)
	{
		attr.continuous = 0;
	}

	ret = alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_SECTION, &channel_id);
	OTA_PRINTF("dmx allocate channel, ret=%d, channel_id=%d\n", ret, channel_id);
	ret = alisldmx_set_channel_attr(dmx_handle, channel_id, &attr);
	OTA_PRINTF("dmx set channel attr, ret=%d\n", ret);
	ret = alisldmx_set_channel_pid(dmx_handle, channel_id, sec_para->pid);
	OTA_PRINTF("dmx set channel pid, ret=%d\n", ret);

	priv.handle = dmx_handle;
	priv.sec_para = sec_para;
	cb.priv = (void *)(&priv);
	cb.request_buffer = sec_request_buf;
	cb.update_buffer = sec_update_buf;

	if (0 == sec_para->mask_value->tb_flt_msk)
	{
		OTA_PRINTF("value_num=%d\n", sec_para->mask_value->value_num);

		for (i = 0; i < sec_para->mask_value->value_num; i++)
		{
			ret = alisldmx_allocate_filter(dmx_handle, channel_id, &filter_id);
			OTA_PRINTF("dmx allocate filter, ret=%d\n", ret);

			memset(mode, 0xff, sizeof(mode));
			ret = alisldmx_set_filter(dmx_handle,
			                          filter_id,
			                          sec_para->mask_value->mask_len,
			                          &(sec_para->mask_value->value[i][0]),
			                          sec_para->mask_value->mask,
			                          mode,
			                          attr.continuous);
			OTA_PRINTF("dmx set filter, ret=%d\n", ret);

			ret = alisldmx_register_filter_callback(dmx_handle, filter_id, &cb);
			OTA_PRINTF("dmx register filter callback function, ret=%d\n", ret);

			ret = alisldmx_control_filter(dmx_handle, filter_id, DMX_CTRL_ENABLE);
			OTA_PRINTF("dmx control filter, ret=%d\n", ret);
		}
	}
	else
	{
		/* not realize here */
		OTA_PRINTF("[%s(%d)]not support case\n", __FUNCTION__, __LINE__);
		return OTA_FAILURE;
	}

	ret = alisldmx_register_channel_callback(dmx_handle, channel_id, &cb);
	OTA_PRINTF("dmx register channel callback, ret=%d\n", ret);

	if (attr.continuous != 0)
	{
		pthread_mutex_lock(&sec_para->continue_mutex);
	}

	ret = alisldmx_control_channel(dmx_handle, channel_id, DMX_CTRL_ENABLE);
	OTA_PRINTF("dmx control channel, ret=%d\n", ret);

	if (sec_para->wai_flg_dly != 0)
	{
		wait_delay = sec_para->wai_flg_dly;
	}
	else
	{
		wait_delay = DMX_HLD_PORTING_SEC_REQ_DEFAULT_DELAY;
	}

	mask = 1 << DMX_CHID2INDEX(channel_id);
#if 0
	ret = alisldmx_section_poll_reset(dmx_handle);
	OTA_PRINTF("dmx section pool reset, ret=%d\n", ret);
#endif

	if (attr.continuous != 0)
	{
		sec_para->dmx_handle = dmx_handle;
		sec_para->channel_id = channel_id;
		dmx_req_check_continue(sec_para);
	}

	OTA_PRINTF("wait_delay = %ld\n", wait_delay);
	timeout = alisldmx_section_request_poll(dmx_handle, mask, wait_delay, (uint32_t *)&flg_got);
	OTA_PRINTF("dmx section request pool, ret=%d\n", ret);

	ret = alisldmx_stop(dmx_handle);
	OTA_PRINTF("dmx stop, ret=%d\n", ret);
	ret = alisldmx_close(dmx_handle);
	OTA_PRINTF("dmx close, ret=%d\n", ret);
	//ret = alisldmx_destruct(&dmx_handle);
	//OTA_PRINTF("dmx destruct, ret=%d\n", ret);

	/**
	* dmx lib may have bugs when we construct/open/close/destruct frequency
	* so here wait for a while
	*/
	usleep(5000);

	OTA_PRINTF("[%s(%d)] req section success flg_got:%lx mask:%x\n", __FUNCTION__, __LINE__, flg_got, mask);
	

	//if (flg_got & mask)
	if (timeout == 0)
	{
		OTA_PRINTF("[%s(%d)] req section success\n", __FUNCTION__, __LINE__);
		return OTA_SUCCESS;
	}
	else
	{
		OTA_PRINTF("[%s(%d)] req section fail\n", __FUNCTION__, __LINE__);
		//return OTA_SUCCESS;
		return OTA_FAILURE;
	}

}
#else
/**
 * dmx lib may have bugs when we construct/open/close/destruct frequency
 * so right now we will just open dmx once and never close it
 */
static int dmx_req_section(int dmxid, struct get_section_param *sec_para)
{
	struct dmx_channel_attr attr;
	struct dmx_channel_callback cb;
	uint32_t channelid, filterid;
	int i = 0;
	uint8_t mode[32];
	unsigned long wait_delay = 0;
	unsigned long flg_got = 0;
	uint32_t mask;
	int ret = OTA_SUCCESS;
	struct section_private priv;

	if (NULL == dmx_handle)
	{
		if (0 != alisldmx_construct(&dmx_handle))
		{
			OTA_PRINTF("dmx construct fail\n", __FUNCTION__, __LINE__);
			return OTA_FAILURE;
		}

		if (0 != alisldmx_open(dmx_handle, dmxid, 0))
		{
			OTA_PRINTF("dmx open fail\n", __FUNCTION__, __LINE__);
			return OTA_FAILURE;
		}

		if (0 != alisldmx_start(dmx_handle))
		{
			OTA_PRINTF("dmx start fail\n", __FUNCTION__, __LINE__);
			return OTA_FAILURE;
		}
	}

	memset(&attr, 0, sizeof(attr));
	attr.continuous = sec_para->continue_get_sec;

	if (sec_para->get_sec_cb == NULL)
	{
		attr.continuous = 0;
	}

	ret = alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_SECTION, &channelid);
	OTA_PRINTF("dmx allocate channel, ret=%d, channelid=%d\n", ret, channelid);
	ret = alisldmx_set_channel_attr(dmx_handle, channelid, &attr);
	OTA_PRINTF("dmx set channel attr, ret=%d\n", ret);
	ret = alisldmx_set_channel_pid(dmx_handle, channelid, sec_para->pid);
	OTA_PRINTF("dmx set channel pid, ret=%d\n", ret);

	if (0 == sec_para->mask_value->tb_flt_msk)
	{
		OTA_PRINTF("value_num=%d\n", sec_para->mask_value->value_num);

		for (i = 0; i < sec_para->mask_value->value_num; i++)
		{
			ret = alisldmx_allocate_filter(dmx_handle, channelid, &filterid);
			OTA_PRINTF("dmx allocate filter, ret=%d\n", ret);

			memset(mode, 0xff, sizeof(mode));
			ret = alisldmx_set_filter(dmx_handle,
			                          filterid,
			                          sec_para->mask_value->mask_len,
			                          &(sec_para->mask_value->value[i][0]),
			                          sec_para->mask_value->mask,
			                          mode);
			OTA_PRINTF("dmx set filter, ret=%d\n", ret);

			ret = alisldmx_control_filter(dmx_handle, filterid, DMX_CTRL_ENABLE);
			OTA_PRINTF("dmx control filter, ret=%d\n", ret);
		}
	}
	else
	{
		/* not realize here */
		OTA_PRINTF("[%s(%d)]not support case\n", __FUNCTION__, __LINE__);
		return OTA_FAILURE;
	}


	priv.handle = dmx_handle;
	priv.sec_para = sec_para;
	cb.priv = (void *)(&priv);
	cb.request_buffer = sec_requestbuf;
	cb.update_buffer = sec_updatebuf;
	ret = alisldmx_register_channel_callback(dmx_handle, channelid, &cb);
	OTA_PRINTF("dmx register channel callback, ret=%d\n", ret);

	ret = alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);
	OTA_PRINTF("dmx control channel, ret=%d\n", ret);

	if (sec_para->wai_flg_dly != 0)
	{
		wait_delay = sec_para->wai_flg_dly;
	}
	else
	{
		wait_delay = DMX_HLD_PORTING_SEC_REQ_DEFAULT_DELAY;
	}

	mask = 1 << DMX_CHID2INDEX(channelid);
#if 0
	ret = alisldmx_section_poll_reset(dmx_handle);
	OTA_PRINTF("dmx section pool reset, ret=%d\n", ret);
#endif
	ret = alisldmx_section_request_poll(dmx_handle, mask, wait_delay, &flg_got);
	OTA_PRINTF("dmx section request pool, ret=%d\n", ret);

#if 0 /* never close dmx */
	ret = alisldmx_stop(dmx_handle);
	OTA_PRINTF("dmx stop, ret=%d\n", ret);
	ret = alisldmx_close(dmx_handle);
	OTA_PRINTF("dmx close, ret=%d\n", ret);
	ret = alisldmx_destruct(&dmx_handle);
	OTA_PRINTF("dmx destruct, ret=%d\n", ret);
#endif

	if (flg_got & mask)
	{
		OTA_PRINTF("[%s(%d)] req section success\n", __FUNCTION__, __LINE__);
		return OTA_SUCCESS;
	}
	else
	{
		OTA_PRINTF("[%s(%d)] req section fail\n", __FUNCTION__, __LINE__);
		return OTA_FAILURE;
	}

}
#endif

static long si_private_sec_parsing_start(unsigned char section_type,
                unsigned short pid,
                section_parser_t section_parser,
                void *param)
{
	OTA_PRINTF("[%s(%d)]\n", __FUNCTION__, __LINE__);

	int i = 0, dmx_id = 0;
	struct get_section_param sr_request;
	struct restrict_t sr_restrict;
	struct section_param* p_sec_param;

	memset(&sr_request, 0, sizeof(struct get_section_param));
	memset(&sr_restrict, 0, sizeof(struct restrict_t));

	sr_request.buff = (unsigned char *)g_ota_section_buf;
	sr_request.buff_len = OTA_SECTION_BUF_LEN;
	sr_request.crc_flag = 1;
	sr_request.pid = pid;
	sr_request.mask_value = &sr_restrict;
	sr_request.wai_flg_dly = 300000;

	//sr_request.wai_flg_dly = 600000;
	

	if (section_type == DSI_NUM)
	{
		/* DSI  */
		sr_restrict.mask_len = 7;
		sr_restrict.value_num = 1;
		memset(sr_restrict.mask, 0, sr_restrict.mask_len);
		memset(sr_restrict.value[0], 0, sr_restrict.mask_len);
		sr_restrict.mask[0] = 0xff;
		sr_restrict.mask[1] = 0x80;
		sr_restrict.mask[3] = 0xff;
		sr_restrict.mask[4] = 0xfe;
		sr_restrict.mask[6] = 0xff;
		sr_restrict.value[0][0] = 0x3b;
		sr_restrict.value[0][1] = 0x80;
		sr_restrict.value[0][3] = 0x00;
		sr_restrict.value[0][4] = 0x00;
		sr_restrict.value[0][6] = 0x00;

		sr_request.get_sec_cb = NULL;
		sr_request.continue_get_sec = 0;

	    p_sec_param = (struct section_param* )param;
		sr_request.wai_flg_dly = p_sec_param->param32;
	}
	else if (section_type == DII_NUM)
	{
		/* DII  */
		sr_restrict.mask_len = 7;
		sr_restrict.value_num = 1;
		memset(sr_restrict.mask, 0, sr_restrict.mask_len);
		memset(sr_restrict.value[0], 0, sr_restrict.mask_len);
		sr_restrict.mask[0] = 0xff;
		sr_restrict.mask[1] = 0x80;
		sr_restrict.mask[3] = 0xff;
		sr_restrict.mask[4] = 0xff;
		sr_restrict.mask[6] = 0xff;
		sr_restrict.value[0][0] = 0x3b;
		sr_restrict.value[0][1] = 0x80;
		sr_restrict.value[0][3] = ((g_dc_group.group_id) >> 8) & 0xff;
		sr_restrict.value[0][4] = (g_dc_group.group_id) & 0xff;
		sr_restrict.value[0][6] = 0x00;

		sr_request.get_sec_cb = NULL;
		sr_request.continue_get_sec = 0;

	    p_sec_param = (struct section_param* )param;
		sr_request.wai_flg_dly = p_sec_param->param32;
	}
	else if (section_type == DDB_NUM)
	{
		/* DDB  */
		if (g_dc_module_num > 4)
		{
			sr_restrict.mask_len = 2;
			sr_restrict.value_num = 1;
			memset(sr_restrict.mask, 0, sr_restrict.mask_len);
			memset(sr_restrict.value[0], 0, sr_restrict.mask_len);
			sr_restrict.mask[0] = 0xff;
			sr_restrict.mask[1] = 0x80;
			sr_restrict.value[0][0] = 0x3c;
			sr_restrict.value[0][1] = 0x80;
		}
		else
		{
			sr_restrict.mask_len = 5;
			sr_restrict.value_num = g_dc_module_num;
			memset(sr_restrict.mask, 0, sr_restrict.mask_len);
			sr_restrict.mask[0] = 0xff;
			sr_restrict.mask[1] = 0x80;
			sr_restrict.mask[3] = 0xff;
			sr_restrict.mask[4] = 0xff;

			for (i = 0; i < g_dc_module_num; i++)
			{
				memset(sr_restrict.value[i], 0, sr_restrict.mask_len);
				sr_restrict.value[i][0] = 0x3c;
				sr_restrict.value[i][1] = 0x80;
				sr_restrict.value[i][3] = ((g_dc_module[i].module_id) >> 8) & 0xff;
				sr_restrict.value[i][4] = (g_dc_module[i].module_id) & 0xff;
			}
		}

		sr_request.get_sec_cb = ddb_section;
		sr_request.continue_get_sec = 1;
		if( (param != NULL) && (section_parser == NULL) )
		{
			sr_request.p_mdl_range = (struct modules_range* )param;
		}
	}

	dmx_id = 0;

	if (dmx_req_section(dmx_id, &sr_request) != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : get section failure !");
		return OTA_FAILURE;
	}

	if (section_parser != NULL)
	{
		return section_parser(sr_request.buff, sr_request.buff_len, param);
	}

	return OTA_SUCCESS;
}

static int ota_cmd_get_download_info(unsigned short pid, struct dl_info *info)
{
//	unsigned long i;
	long ret ;
	struct section_param sec_param;

	OTA_PRINTF("Get download_info...\n");
	OTA_PRINTF("g_ota_ram1_addr = 0x%8x\n", (int)g_ota_ram1_addr);

	/* step 1 : DSI  */
	sec_param.section_type = 0x00;
	sec_param.param32 = OTA_DOWNLOAD_INFO_TIMEOUT;
	ret = si_private_sec_parsing_start(DSI_NUM,
	                                   pid,
	                                   parse_dsmcc_sec,
	                                   (void*)(&sec_param));

	if (ret != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : No  ota service exist!\n");
		return OTA_FAILURE;
	}

	g_ota_size =  g_dl_info.sw_size;
	memcpy(info, &g_dl_info, sizeof(struct dl_info));

	/* step 2 : DII  */
	sec_param.section_type = 0x01;
	sec_param.param32 = OTA_DOWNLOAD_INFO_TIMEOUT;
	ret = si_private_sec_parsing_start(DII_NUM,
	                                   pid,
	                                   parse_dsmcc_sec,
	                                   (void*)(&sec_param));

	if (ret != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : No  ota service exist!\n");
		return OTA_FAILURE;
	}

	OTA_PRINTF("Get download info done!\n");
	return OTA_SUCCESS;
}

static int ota_cmd_start_download_by_piece(unsigned short pid, struct modules_range* p_mdl_range,
						t_progress_disp progress_disp)
{
	unsigned long i, j;
	long ret;

//	unsigned char* block_addr;
//	unsigned long check_size, check_crc, offset;
	int valid_size = 0;

	total_block_num = 0;
	received_section_num = 0;


	for(j = p_mdl_range->start; j <= p_mdl_range->end; j++)
	{
		g_dc_module[j].module_download_finish = 0;
		total_block_num += g_dc_module[j].block_num;

		for (i = 0; i < BIT_MAP_NUM; i++)
		{
			g_dc_module[j].block_received[i] = 0xffffffff;
		}

		for (i = 0; i < g_dc_module[j].block_num; i++)
		{
			g_dc_module[j].block_received[i / 32] &= ~(1 << (i % 32));
		}
	}

	OTA_PRINTF("OTA Debug: total_block_num = %d \n", (int)total_block_num);

	ret = si_private_sec_parsing_start(DDB_NUM, pid, NULL, p_mdl_range);

	if (ret != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : No ota service exist!\n");
		return OTA_FAILURE;
	}

	for(j = p_mdl_range->start; j <= p_mdl_range->end; j++)
	{
		valid_size += g_dc_module[j].module_buf_size;
		OTA_PRINTF("OTA Debug: valid_size = 0x%x, g_dc_module[j].module_buf_size = 0x%x \n", valid_size, (unsigned int)g_dc_module[j].module_buf_size);
	}

	p_mdl_range->valid_size = valid_size;

	OTA_PRINTF("Piece Download Finished !\n");
	return OTA_SUCCESS;
}

static int ota_cmd_start_download_one_shot(unsigned short pid, struct modules_range* p_mdl_range, t_progress_disp progress_disp)
{
	unsigned long i;
	long ret;

//	unsigned char* block_addr;
  //unsigned long check_size, check_crc, offset, code_size;

	if (g_ota_size > g_ota_ram1_len)
	{
		OTA_PRINTF("ERROR: download_buf exceed!\n");
		return OTA_FAILURE;
	}

	/* step 1: process_download */
	OTA_PRINTF("Start Download...\n");

	progress_disp(0);

	download_progress_disp = progress_disp;

	received_section_num = 0;
	total_block_num = 0;

	for (i = 0; i < g_dc_module_num; i++)
	{
		total_block_num += g_dc_module[i].block_num;
	}

	ret = si_private_sec_parsing_start(DDB_NUM, pid, NULL, p_mdl_range);

	if (ret != OTA_SUCCESS)
	{
		OTA_PRINTF("ERROR : No  ota service exist!\n");
		return OTA_FAILURE;
	}

	progress_disp(100);

	OTA_PRINTF("Download Finished !\n");
	return OTA_SUCCESS;
}

int alislupg_ota_mem_config_by_piece(unsigned char* addr, unsigned long len,
				unsigned short* p_piece_num, struct modules_range* p_mdl_range)
{
	int i, j;
	unsigned long remain = len;
	unsigned char* cur_ptr = addr;

	/* At least we need one piece */
	int piece_num = 1;
//	int valid_size = 0;

	OTA_PRINTF("[%s(%d)] addr = 0x%x, len = %ld\n",	__FUNCTION__, __LINE__, (int)addr, len);

	g_ota_ram1_addr = addr;
	g_ota_ram1_len = len;

	OTA_PRINTF("[%s(%d)] g_dc_module_num = %d\n",	__FUNCTION__, __LINE__, (int)g_dc_module_num);

	if(g_dc_module[0].module_buf_size > len)
	{
		OTA_PRINTF("[%s(%d)] ERROR: OTA PIECE buffer is too small! \n", __FUNCTION__, __LINE__);
		return OTA_FAILURE;
	}

    p_mdl_range->len = len/g_dc_module[0].module_buf_size;
    p_mdl_range->total = g_dc_module_num;
	j = p_mdl_range->len;

	for (i = 0; i< g_dc_module_num; i++)
	{

		if( (remain < (g_dc_module[i].module_buf_size)) || (j == 0))
		{
			remain = len;
			cur_ptr = addr;
			piece_num++;
			j = p_mdl_range->len;
		}

		g_dc_module[i].module_buf_pointer = cur_ptr;
		remain = remain - g_dc_module[i].module_buf_size;
		cur_ptr = cur_ptr + g_dc_module[i].module_buf_size;
		j--;

		OTA_PRINTF("[%s(%d)] g_dc_module[i].module_buf_pointer = 0x%x g_dc_module[i].module_buf_size = 0x%lx, j=%d\n",
		           __FUNCTION__, __LINE__,  (int)g_dc_module[i].module_buf_pointer,g_dc_module[i].module_buf_size, j);
	}

	*p_piece_num = piece_num;
	OTA_PRINTF("[%s(%d)] len = 0x%lx, g_dc_module[0].module_buf_size = %ld len/g_dc_module[0].module_buf_size=%ld\n",
	__FUNCTION__, __LINE__, len, g_dc_module[0].module_buf_size, len/g_dc_module[0].module_buf_size);


	received_section_num = 0;

	return OTA_SUCCESS;

}


int alislupg_ota_mem_config_one_shot(unsigned char* addr, unsigned long len, struct modules_range* p_mdl_range)
{
	int i = 0;

	OTA_PRINTF("[%s(%d)] addr = 0x%x, len = %ld\n",	__FUNCTION__, __LINE__, (int)addr, len);
	g_ota_ram1_addr = addr;
	g_ota_ram1_len = len;

	OTA_PRINTF("[%s(%d)] g_dc_module_num = %d\n",	__FUNCTION__, __LINE__, (int)g_dc_module_num);

	for (i = 0; i < g_dc_module_num; i++)
	{
		if (i == 0)
		{
			g_dc_module[i].module_buf_pointer = addr;
			OTA_PRINTF("[%s(%d)] module[i].module_buf_pointer = 0x%lx, buf_addr = 0x%lx\n",
			           __FUNCTION__, __LINE__,  (unsigned long)g_dc_module[i].module_buf_pointer, (unsigned long)addr);
		}
		else
		{
			g_dc_module[i].module_buf_pointer = g_dc_module[i - 1].module_buf_pointer
			                                    + g_dc_module[i - 1].module_buf_size;
			OTA_PRINTF("[%s(%d)] g_dc_module[i].module_buf_pointer = 0x%lx\n",
			           __FUNCTION__, __LINE__,  (unsigned long)g_dc_module[i].module_buf_pointer);
		}
	}
	p_mdl_range->start = 0;
	p_mdl_range->end = g_dc_module_num - 1;
    p_mdl_range->len = len/g_dc_module[0].module_buf_size;

	return OTA_SUCCESS;
}

int alislupg_ota_get_download_info(unsigned short pid, struct dl_info *info)
{
	return ota_cmd_get_download_info(pid, info);
}

int alislupg_ota_start_download_one_shot(unsigned short pid, struct modules_range* p_mdl_range, t_progress_disp progress_disp)
{
	return ota_cmd_start_download_one_shot(pid, p_mdl_range, progress_disp);
}

int alislupg_ota_start_download_by_piece(unsigned short pid, struct modules_range* p_mdl_range, t_progress_disp progress_disp)
{
	return ota_cmd_start_download_by_piece(pid, p_mdl_range, progress_disp);
}

int alislupg_ota_download_abort()
{
	return alisldmx_section_request_poll_release(dmx_handle);
}

int alislupg_ota_set_dl_param(unsigned long oui, 
									unsigned short hw_model, unsigned short hw_version, 
									unsigned short sw_model, unsigned short sw_version)
{
	g_oui        = oui;
	g_model      = hw_model;
	g_hw_version = hw_version;
	g_sw_model   = sw_model;
	g_sw_version = sw_version;

	return OTA_SUCCESS;
}

