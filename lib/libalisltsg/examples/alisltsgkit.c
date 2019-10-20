/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisltsgkit.c
 *  @brief
 *
 *  @version            1.0
 *  @date               7/25/2013 11:03:51
 *  @revision           none
 *
 *  @author             Franky.Liang <franky.liang@alitech.com>
 */

/* system headers */
#include <argp.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alisltsg.h>
#include <alisldmx.h>
#include <alisltsi.h>
#include <alisldis.h>
#include <alislvdec.h>
#include <alislsnd.h>

#define QUANTUM_SIZE 		(47*1024)
#define PAD_BUF_SIZE  		16*188
#define C_TS_PKT_SIZE       188
#define SECTIOIN_BUFFER_SIZE 4096

#if 0
#define SL_TSG_DEBUG_PRINT printf
#else
#define SL_TSG_DEBUG_PRINT(...)
#endif

unsigned char section_buffer[SECTIOIN_BUFFER_SIZE]={0};

static void tsg_fill_null_pkt(char *ptr,uint32_t ui_pkt_num)
{
    uint32_t i, j;
    for(i = 0; i < ui_pkt_num; i++, ptr += C_TS_PKT_SIZE)
	{
		ptr[0] = 0x47;
		ptr[1] = 0x1f;
		ptr[2] = 0xff;
		ptr[3] = (i&0xf)|0x10;
		for(j = 4; j < C_TS_PKT_SIZE; j++)
			*(ptr+j) = 0;
	}
}


static void tsg_requestbuf_callback(void *private,
			unsigned long channelid,
			unsigned long filterid,
			unsigned long length,
			unsigned char  **buffer,
			unsigned long *actlen)
{
	memset(section_buffer,0,SECTIOIN_BUFFER_SIZE);
	*buffer = section_buffer;
	*actlen = SECTIOIN_BUFFER_SIZE;
	printf("buffer is %p ;size is %d\n",*buffer,*actlen);
}

static void tsg_updatebuf_callback(void *private,
			unsigned long channelid,
			unsigned long filterid,
			unsigned long valid_len)
{
	printf("got section data valid_len[%d] !\n",valid_len);
	int i=0;
	for(i=0; i<8;i++)
		printf("%02x ",section_buffer[i]);
	printf("-------\n");
}


static int ali_app_test_section(int dmx_id,unsigned int us_pid,unsigned char *value, unsigned char *mask, unsigned char *mode,int mask_len)
{
	alisl_handle dmx_handle = NULL;
	struct dmx_channel_attr attr;
	uint32_t channelid;
	uint32_t filterid;
	if (0!= alisldmx_open(&dmx_handle, dmx_id, 0))
	{
		SL_TSG_DEBUG_PRINT("dmx open fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (0!= alisldmx_start(dmx_handle))
	{
		SL_TSG_DEBUG_PRINT("dmx start fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_SECTION, &channelid);
	alisldmx_set_channel_pid(dmx_handle, channelid, us_pid);
	alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);

	if(alisldmx_allocate_filter(dmx_handle, channelid, &filterid) !=0 )
	{
		SL_TSG_DEBUG_PRINT("alisldmx_allocate_filter  fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
	printf("%s line:%d \n",__FUNCTION__,__LINE__);
	if(alisldmx_set_filter(dmx_handle,
					filterid,
					mask_len,
					value,
					mask,
					mode,
					1) !=0 )
	{
		SL_TSG_DEBUG_PRINT("alisldmx_set_filter  fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	attr.continuous = 1;
	alisldmx_set_channel_attr(dmx_handle,
				channelid,
				&attr);

	struct dmx_channel_callback cb;
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)tsg_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)tsg_updatebuf_callback;
	cb.priv = (void *)filterid;
	alisldmx_register_channel_callback(dmx_handle, channelid, &cb) == 0?printf("set callback ok!\n"):printf("set callback fail!\n");
	alisldmx_control_filter(dmx_handle, filterid, DMX_CTRL_ENABLE);
}

static int ali_app_test_record(int dmx_id,unsigned int us_pid,unsigned char *value, unsigned char *mask, unsigned char *mode,int mask_len)
{
	alisl_handle dmx_handle = NULL;
	struct dmx_channel_attr attr;
	uint32_t channelid;
	if (0!= alisldmx_open(&dmx_handle, dmx_id, 0))
	{
		SL_TSG_DEBUG_PRINT("dmx open fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (0!= alisldmx_start(dmx_handle))
	{
		SL_TSG_DEBUG_PRINT("dmx start fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_RECORD, &channelid);
	alisldmx_set_channel_pid(dmx_handle, channelid, us_pid);
	alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);

	struct dmx_channel_callback cb;
	cb.request_buffer = (alisldmx_channel_requestbuf_callback)tsg_requestbuf_callback;
	cb.update_buffer = (alisldmx_channel_updatebuf_callback)tsg_updatebuf_callback;
	cb.priv = (void *)channelid;
	alisldmx_register_channel_callback(dmx_handle, channelid, &cb) == 0?printf("set callback ok!\n"):printf("set callback fail!\n");
	return 0;
}

static int create_av_stream
(
	int dmx_id,
	unsigned int front_end,
	unsigned int nim_chipid,
	unsigned short video_pid,
	unsigned short audio_pid,
	unsigned short audio_desc_pid,
	unsigned short pcr_pid,alisl_handle *p_hdl
)
{
	alisl_handle dmx_handle = NULL;
	struct dmx_channel_attr attr;
	uint32_t channelid;
	int i = 0, ret = 0;

	if (0!= alisldmx_open(&dmx_handle, dmx_id, 0))
	{
		SL_TSG_DEBUG_PRINT("dmx open fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (0!= alisldmx_start(dmx_handle))
	{
		SL_TSG_DEBUG_PRINT("dmx start fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	memset(&attr, 0, sizeof(attr));

	alisldmx_set_front_end(dmx_handle, front_end);
	alisldmx_set_nim_chipid(dmx_handle, nim_chipid);

	if (video_pid != 0x1FFF)
	{
		attr.stream = DMX_STREAM_VIDEO;
		alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(dmx_handle, channelid, &attr);
		alisldmx_set_channel_pid(dmx_handle, channelid, video_pid);
		alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);
	}

	if (audio_pid != 0x1FFF)
	{
		attr.stream = DMX_STREAM_AUDIO;
		alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(dmx_handle, channelid, &attr);
		alisldmx_set_channel_pid(dmx_handle, channelid, audio_pid);
		alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);
	}

	if (audio_desc_pid != 0x1FFF)
	{
		attr.stream = DMX_STREAM_AUDIO_DESCRIPTION;
		alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(dmx_handle, channelid, &attr);
		alisldmx_set_channel_pid(dmx_handle, channelid, audio_desc_pid);
		alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);
	}

	if (video_pid != 0x1FFF &&
			audio_pid != 0x1FFF &&
			(pcr_pid & 0x1FFF) != (video_pid & 0x1FFF) &&
			(pcr_pid & 0x1FFF) != (audio_pid & 0x1FFF))
	{
		attr.stream = DMX_STREAM_PCR;
		alisldmx_allocate_channel(dmx_handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(dmx_handle, channelid, &attr);
		alisldmx_set_channel_pid(dmx_handle, channelid, pcr_pid);
		alisldmx_control_channel(dmx_handle, channelid, DMX_CTRL_ENABLE);
	}

	alisldmx_set_avsync_mode(dmx_handle, DMX_AVSYNC_LIVE);

	alisldmx_avstart(dmx_handle);

	*p_hdl=dmx_handle;

	return 0;
}


static int init_tsi_dmx(alisl_handle *dmx_handler, alisl_handle *tsi_handler,unsigned long video_pid,unsigned long audio_pid,unsigned long pcr_id)
{
	int handle = 0;
	int ret = 0;

	ret=alisltsi_open(tsi_handler,TSI_ID_M3602_0,0);
	if((0 != ret))
	{
		SL_TSG_DEBUG_PRINT("\n alisltsi_construct error ret[%d] tsi_handler[%d] \n",ret,tsi_handler);
		return -1;
	}

	alisltsi_setinput(*tsi_handler,ALISL_TSI_SPI_TSG,0x83);
	alisltsi_setchannel(*tsi_handler,ALISL_TSI_SPI_TSG,ALISL_TSI_TS_A);
	alisltsi_setoutput(*tsi_handler,ALISL_TSI_DMX_0,ALISL_TSI_TS_A);

	create_av_stream(DMX_ID_DEMUX0,0,0,video_pid,audio_pid,0x1fff,pcr_id,dmx_handler);

	unsigned char value[]={0x70};
	unsigned char mask[]={0xFF};
	unsigned char mode[]={0xFF};
	ali_app_test_section(DMX_ID_DEMUX0,20,value,mask,mode,1);
	//ali_app_test_record(DMX_ID_DEMUX0,6000,value,mask,mode,1);
	return 0;
}

static int ali_app_dis_init(unsigned long ul_dis_type,alisl_handle *p_hdl)
{
	alisl_handle dis_hdl = 0;
	alisl_retcode ret = 0;
	// DIS DIS_HD_DEV
	ret = alisldis_open(ul_dis_type, &dis_hdl);
	if((0 != ret) || (0 == dis_hdl))
	{
		SL_TSG_DEBUG_PRINT("\n alisldis_open error \n");
		return -1;
	}

	ret=alisldis_win_onoff_by_layer(dis_hdl, true, DIS_LAYER_MAIN);
	if((0 != ret))
	{
		SL_TSG_DEBUG_PRINT("\n alisldis_win_onoff_by_layer error \n");
		return -1;
	}

	*p_hdl=dis_hdl;
	return 0;
}

static int ali_app_decv_init(enum vdec_decoder_type ul_video_type, bool ul_preview_enable,alisl_handle *p_hdl)
{
	alisl_handle vdec_hdl=0;
	alisl_retcode ret = 0;
	// VDEC
	ret=alislvdec_open(&vdec_hdl, 0);
	if((0 != ret))
	{
		SL_TSG_DEBUG_PRINT("\n alislvdec_open error \n");
		return -1;
	}

	//SL_ASSERT(alislvdec_set_decoder(vdec_hdl, VDEC_DECODER_AVC, false)); // false --> no in preview mode
	ret=alislvdec_set_decoder(vdec_hdl, ul_video_type, ul_preview_enable);
	if((0 != ret))
	{
		SL_TSG_DEBUG_PRINT("\n alislvdec_set_decoder error \n");
		return -1;
	}

	ret=alislvdec_start(vdec_hdl);
	if((0 != ret))
	{
		SL_TSG_DEBUG_PRINT("\n alislvdec_start error \n");
		return -1;
	}
	*p_hdl=vdec_hdl;
	return 0;

}

static int ali_app_snd_init(uint8_t ul_volume ,alisl_handle *p_hdl)
{
	alisl_handle vdec_hdl=0;
	alisl_handle snd_hdl=0;
	unsigned int nim_chip_id=0;

	// SND
	if (0 != alislsnd_open(&snd_hdl))
	{
		SL_TSG_DEBUG_PRINT("\n snd open fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (0!= alislsnd_start(snd_hdl))
	{
		SL_TSG_DEBUG_PRINT("\n snd start fail\n", __FUNCTION__, __LINE__);
		return -1;
	}

	alislsnd_set_mute(snd_hdl,0,0);
	alislsnd_set_volume(snd_hdl,ul_volume,0);
	*p_hdl=snd_hdl;
	return 0;

}

static int ali_app_snd_deinit(alisl_handle sl_hdl)
{
	int ret=-1;
	ret = alislsnd_close(sl_hdl);
	if((0 != ret) )
	{
		SL_TSG_DEBUG_PRINT("\n alislsnd_close error \n");
		return -1;
	}
	return 0;
}

static int ali_app_dis_deinit(alisl_handle sl_hdl)
{
	int ret=-1;

	ret = alisldis_close(sl_hdl);
	if((0 != ret) )
	{
		SL_TSG_DEBUG_PRINT("\n alisldis_close error \n");
		return -1;
	}
	return 0;
}

static int ali_app_decv_deinit(alisl_handle sl_hdl)
{
	int ret=-1;
	ret=alislvdec_stop(sl_hdl,false,false);
	if((0 != ret) )
	{
		SL_TSG_DEBUG_PRINT("\n alislvdec_stop error \n");
		return -1;
	}

	ret = alislvdec_close(sl_hdl);
	if((0 != ret) )
	{
		SL_TSG_DEBUG_PRINT("\n alislvdec_close error \n");
		return -1;
	}
	return 0;
}

int ali_tsg_send_one(alisl_handle tsg_handler,alisl_handle dmx_handler)
{
	char pad_buf[C_TS_PKT_SIZE]={0};
	int ret=0;
	alisl_retcode sl_ret=0;
	uint32_t bitrate=0;     //0 is OK. In Fact,its value will not involve anything.!
	unsigned long buffer_empty=0;

	//set TSG clock
	sl_ret=alisltsg_set_clkasync(tsg_handler,0x18);
	if(sl_ret!=0)
	{
		SL_TSG_DEBUG_PRINT("fail to set TSG clock!\n");
		return -1;
	}

	//Fill full ts packet.
	tsg_fill_null_pkt(pad_buf,1);

	//start transfer
	sl_ret=alisltsg_insertionstart(tsg_handler, bitrate);
	if(sl_ret!=0)
	{
		SL_TSG_DEBUG_PRINT("fail to start tsg insert data!\n");
		return -1;
	}
	alisldmx_ioctl(dmx_handler,DMX_IOCMD_GET_FREE_BUF_LEN,(unsigned long)&buffer_empty);
	while(buffer_empty <512)
	{
		alisldmx_ioctl(dmx_handler,DMX_IOCMD_GET_FREE_BUF_LEN,(unsigned long)&buffer_empty);
		SL_TSG_DEBUG_PRINT("DMX_IOCMD_GET_FREE_BUF_LEN buffer_empty[%d]\n",buffer_empty);
		sleep(1);
	}
	alisldmx_set_avsync_mode(dmx_handler,DMX_AVSYNC_TSG_TIMESHIT);
	sl_ret = alisltsg_copydata(tsg_handler, pad_buf, 1);
	if(sl_ret !=0)
	{
		SL_TSG_DEBUG_PRINT("ali_sharelibrary copydata is wrong!\n");
		ret =1;
	}
	//stop transfer
	sl_ret=alisltsg_insertionstop(tsg_handler);
	if(sl_ret !=0)
	{
		SL_TSG_DEBUG_PRINT("fail to stop tsg insert data\n");
		ret =1;
	}
	return ret;
}

int ali_tsg_send_file(char *filename,alisl_handle tsg_handler,alisl_handle dmx_handler)
{
	int ret=0;
	alisl_retcode sl_ret=0;
	uint32_t bitrate=0;
	void * addr;
	uint32_t pkt_cnt=0;
	uint32_t buffer_empty = 0;
	int len=0;
	char pad_buf[PAD_BUF_SIZE]={0};
	unsigned int pad_number=0;
	unsigned int buf_number=0;

	// 1.open ts file
	FILE *ts_fp=fopen(filename,"r");
	SL_TSG_DEBUG_PRINT("filename[%s]\n",filename);
	if(NULL==ts_fp)
	{
		SL_TSG_DEBUG_PRINT("file[%s] opened fail!\n",filename);
		return 1;
	}

	// 2.mallock buffer
	addr=(void *)malloc(QUANTUM_SIZE);
	memset(addr,0,QUANTUM_SIZE);
	if(addr ==NULL)
	{
		SL_TSG_DEBUG_PRINT("fail ali_app_create_ts to malloc for addr!\n");
		return -1;
	}

	// 3.set TSG clock
	sl_ret=alisltsg_set_clkasync(tsg_handler,0x18);
	if(sl_ret!=0)
	{
		SL_TSG_DEBUG_PRINT("fail ali_app_create_ts to set clock for tsg!\n");
		return -1;
	}

	// 4.start insert
	sl_ret=alisltsg_insertionstart(tsg_handler, bitrate);
	if(sl_ret!=0)
	{
		SL_TSG_DEBUG_PRINT("fail ali_app_create_ts to start tsg insert data!\n");
		goto RETURN;
	}

	// 5.fill the full packat
	tsg_fill_null_pkt(pad_buf,16);

	while(1)
	{
		if(ts_fp == NULL)
		{
			SL_TSG_DEBUG_PRINT("g_slpf[%d] is null!\n",ts_fp);
			break;
		}

		len=fread(addr,1024,47,ts_fp);
		if(len<1)
			break;
		len = len*1024;
		if(len!=QUANTUM_SIZE && len != QUANTUM_SIZE/2){
			SL_TSG_DEBUG_PRINT("len [%d] read file over!\n",len);
			break;
		}

		alisldmx_ioctl(dmx_handler,DMX_IOCMD_GET_FREE_BUF_LEN,(unsigned long)&buffer_empty);
		SL_TSG_DEBUG_PRINT("DMX_IOCMD_GET_FREE_BUF_LEN remain buffer_empty[%d]\n",buffer_empty);
		while(buffer_empty <512)
		{
			alisldmx_ioctl(dmx_handler,DMX_IOCMD_GET_FREE_BUF_LEN,(unsigned long)&buffer_empty);
			SL_TSG_DEBUG_PRINT("DMX_IOCMD_GET_FREE_BUF_LEN buffer_empty[%d]\n",buffer_empty);
			sleep(1);
		}
		alisltsg_check_remain_buf(tsg_handler,&pkt_cnt);
		SL_TSG_DEBUG_PRINT("alisltsg_check_remain_buf remain buffer[%d]\n",pkt_cnt);
		alisldmx_set_avsync_mode(dmx_handler,DMX_AVSYNC_TSG_TIMESHIT);
		sl_ret = alisltsg_copydata(tsg_handler, pad_buf, PAD_BUF_SIZE/188);
		if(sl_ret !=0)
		{
			SL_TSG_DEBUG_PRINT("ali_sharelibrary send pad packet fail!\n");
			break;
		}
		pad_number+=PAD_BUF_SIZE/188;

		//SL_TSG_DEBUG_PRINT("alisltsg_copydata tsg_handler[%d] addr[%d] pcakate number[%d] start\n",tsg_handler, addr, len/188);
		sl_ret = alisltsg_copydata(tsg_handler, addr, len/188);
		if(sl_ret !=0)
		{
			SL_TSG_DEBUG_PRINT("ali_sharelibrary copydata is wrong!\n");
			break;
		}
		buf_number+=len/188;
		SL_TSG_DEBUG_PRINT("pad_number[%d] buf_number[%d]\n",pad_number,buf_number);


	}


RETURN:
	sl_ret=alisltsg_insertionstop(tsg_handler);
	SL_TSG_DEBUG_PRINT("----------   TSG insert over!-------------\n");
	if(NULL==ts_fp)
	{
		return 1;
	}
	if(0!=fclose(ts_fp))
	{
		return 2;
	}
	if(addr) free(addr);

	return ret;
}


#if 0
void print_tsg_status(TSG_STATUS status)
{
	switch(status)
	{
	case TSG_IDLE:
		printf("TSG is free!\n");
		break;
	case TSG_INIT:
		printf("TSG is initialized!!\n");
		break;
	case TSG_TRAGNSFER_START:
		printf("TSG is transferring data!\n");
		break;
	case TSG_TRAGNSFER_STOP:
		printf("TSG have stopped transfer data!\n");
		break;
	default:
		printf("Unknow status\n");
		return;
	}
}

#endif

int main (int argc, char **argv)
{

	alisl_retcode sl_ret=0;
	alisl_handle tsg_handler=0;
	alisl_handle dmx_handler=0;
	alisl_handle tsi_handler=0;
	alisl_handle dis_handler=0;
	alisl_handle decv_handler=0;
	alisl_handle snd_handler=0;
	enum tsg_id e_tsgid=TSG_ID_M36_0;
	char filename[256]={0};

	unsigned long video_pid=160;
	unsigned long audio_pid=80;
	unsigned long pcr_pid=160;


	char key;

	printf("Init share library TSG module!\n");
	/*
	sl_ret=alisltsg_construct(&tsg_handler);
	if(sl_ret !=0 || tsg_handler==NULL)
	{
		printf("ali_sharelibrary construct is wrong!\n");
		return -1;
	}
	*/
	sl_ret=alisltsg_open(&tsg_handler, e_tsgid, NULL);
	if(sl_ret !=0)
	{
		printf("ali_sharelibrary alisltsg_open is wrong!\n");
		return -1;
	}
	printf("TSG init sucessfully!\n");
	printf("Press any key to continue...\n");
	scanf("%d", &key);

	printf("****************  set up DMX ********************\n");
	printf("Input video_pid audio_pid,pcr_pid:");
	scanf("%d,%d,%d",&video_pid,&audio_pid,&pcr_pid);
	printf("Video_pid[%d] audio_pid[%d] pcr_pid[%d]\n",video_pid,audio_pid,pcr_pid);

	if(0 != init_tsi_dmx(&dmx_handler,&tsi_handler,video_pid, audio_pid, pcr_pid))
	{
		printf("dmx or tsi init failed!\n");
		goto RETURN;
	}

	printf("\n***************  Sample 1:send one packet with TSG ****************************\n");
	printf("Sample 1:send one packet with TSG\n");
	if(ali_tsg_send_one(tsg_handler,dmx_handler)!=0)
	{
		printf("TSG send one packet failed!\n");
	}else{
		printf("TSG send one packet sucessfully!\n");
	}
	printf("Press any key to continue...\n");
	scanf("%d", &key);

	printf("\n****************  Sample 2:send a ts file with TSG  ****************************\n");
	printf("Initializing DIS module...\n");
	if(0 != ali_app_dis_init(DIS_SD_DEV,&dis_handler))
	{
		printf("display module init failed!\n");
		goto RETURN;
	}
	printf("DIS module init successfully!\n");

	printf("Initializing VDEV module...\n");
	if(0 != ali_app_decv_init(VDEC_DECODER_MPEG,0,&decv_handler))
	{
		printf("decv module init failed!\n");
		goto RETURN;
	}
	printf("decv module init successfully!\n");

	printf("Initializing SND module...\n");
	if(0 != ali_app_snd_init(50,&snd_handler))
	{
		printf("SND module init failed!\n");
		goto RETURN;
	}
	printf("SND module init successfully!\n");

	printf("Please enter the filename. For example:/mnt/nfs/000.ts\n");
	printf("Filename:");
	scanf("%s", filename);
	printf("\nfilename[%s]\n",filename);
	if(ali_tsg_send_file(filename,tsg_handler,dmx_handler)!=0)
	{
		printf("Send file failed!\n");
		goto RETURN;
	}
	printf("TSG send a ts file sucessfully!\n");
	printf("Press any key to continue...\n");
	scanf("%d", &key);

RETURN:
	printf("\n***************  exit *****************************\n");
	if(snd_handler!=0)
	{
		sl_ret = ali_app_snd_deinit(snd_handler);
		if((0 != sl_ret) )
		{
			printf("\n ali_app_snd_deinit error \n");
		}
	}

	if(decv_handler!=0)
	{
		sl_ret = ali_app_decv_deinit(decv_handler);
		if((0 != sl_ret) )
		{
			printf("\n ali_app_decv_deinit error \n");
		}
	}

	if(dis_handler!=0)
	{
		sl_ret = ali_app_dis_deinit(dis_handler);
		if((0 != sl_ret) )
		{
			printf("\n alisldis_close error \n");
		}
	}

	if(dmx_handler !=0){
		sl_ret = alisldmx_close(dmx_handler);
		if((0 != sl_ret) )
		{
			printf("\n alisldmx_close error \n");
		}
	}

	if(tsi_handler!=0)
	{
		sl_ret = alisltsi_close(tsi_handler);
		if((0 != sl_ret) )
		{
			printf("\n alisltsi_close error \n");
		}
	}




	printf("TSG module deinit!\n");
	sl_ret = alisltsg_close(tsg_handler);
	if((0 != sl_ret) )
	{
		printf("\n alisltsi_close error \n");
	}

	printf("TSG module deinit succesfully!\n");
	scanf("Press any key to escape...n");
	return 0;
#if 0
	int c;
	int index = 0;
	int command=0;

	alisl_handle tsg_handler=0;
	alisl_handle dmx_handler=0;
	TSG_STATUS status=TSG_IDLE;
	char filename[256]={0};



	while (1){
		switch(index){
		case 0:
			printf("Please input your command!\n");
			printf("1 init the tsg!\n");
			printf("2 send data once with TSG!\n");
			printf("3 play back a ts file with TSG");
			printf("4 deinit the tsg");
			printf("5 exit");
			scanf("Command:%d\n",&command);

			if(command <1 || command >5){
				printf("Wrong command!\n");
				break;
			}
			index=command;
			command=0;
			break;
		case 1:
			enum tsg_id e_tsgid=0;
			if(status != TSG_IDLE){
				print_tsg_status(status);
				printf("Wrong TSG status!\n"):
				break;
			}
			printf("Please input TSG ID!\n");
			printf("0 TSG_ID_M36_0!\n");
			printf("1 back");
			scanf("TSG ID:%d\n",&command);
			if(command == 0){
				e_tsgid=TSG_ID_M36_0;
				if(ali_app_tsg_init(&tsg_handler,e_tsgid)==0)
				{
					printf("TSG init successfully!\n");
				}else{
					printf("TSG init fail!\n");
				}
				index=0;
				command=0;
				break;
			}
			if(command == 1)
			{
				index=0;
				command=0;
				break;
			}
			printf("Wrong TSG ID!\n"):
			break;
		case 2:
			if(status != TSG_INIT && status != TSG_TRAGNSFER_STOP){
				printf("Wrong TSG status!\n"):
				print_tsg_status(status);
				break;
			}
			printf("Please input filename of the file to transfer!\n");
			scanf("file:%d\n",&filename);
			ali_app_create_ts(filename,tsg_handler,dmx_handler);
			status=TSG_TRAGNSFER_START;
			break;
		case 3:
			if(status != TSG_TRAGNSFER_START){
				printf("Wrong TSG status!\n"):
				print_tsg_status(status);
				break;
			}

			break;
		case 4:
			break;
		case 5:
			break;
		default:
			printf("unknow error!\n");
			return 0;
		}
	}
#endif
	return 0;
}
