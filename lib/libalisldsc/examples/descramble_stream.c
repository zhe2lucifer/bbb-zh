#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/time.h>
#include <linux/dvb/frontend.h>
#include <ali_video_common.h>
#include <ali_dmx_common.h>
#include <ali_audio_common.h>
#include <linux/dvb/audio.h>
//#include <hld/deca/deca.h>
//#include <hld/snd/snd.h>
//#include <hld/scart/scart.h>

#include "keyladdersample.h"

#define NIM0_DEVICE "/dev/ali_m3501_nim0"
#define TSI_DEVICE "/dev/ali_m36_tsi_0"
#define DEMUX0_DEVICE "/dev/ali_m36_dmx_0"
#define DEMUX1_DEVICE "/dev/ali_m36_dmx_1"
#define DEMUX2_DEVICE "/dev/ali_dmx_pb_0_out"
#define DEMUX_PB_DEVICE "/dev/ali_dmx_pb_0_in"

#define SEE_DEVICE "/dev/ali_m36_dmx_see_0"
#define VIDEO_DEVICE "/dev/ali_video0" 
#define AUDIO_DEVICE "/dev/ali_m36_audio0" 
#define SCART_DEVICE "/dev/ali_scart" 
#define VPO_DEVICE "/dev/fb1"


#define VDEC_SET_DMA_CHANNEL                     0x24

int fe_fd, tsi_fd, demux2_vfd, demux2_afd, demux2_pfd, pb_fd, see_fd, video_fd, audio_fd, scart_fd, vpo_fd;	

/* CWPK --- odd key : the first 8 bytes; even key : the last 8 bytes*/
uint8_t enc_cwpk[16] = {0x77, 0xb1, 0x27, 0x3c, 0x86, 0xa1, 0x29, 0x83, 0x20, 0xa4, 0xb0, 0xb3, 0x87, 0x36, 0x31, 0x25};
/* CW --- odd key : the first 8 bytes; even key : the last 8 bytes*/
uint8_t enc_cw[3][16] = {
	{0x00,0x36,0x8a,0xf3,0x93,0x59,0xf5,0x5e,0xaf,0x02,0x2b,0x38,0x91,0xaf,0xff,0xcf},
	{0x01,0x75,0xa8,0x71,0xa8,0xce,0x2f,0xb2,0xaf,0x02,0x2b,0x38,0x91,0xaf,0xff,0xcf},
	{0x01,0x75,0xa8,0x71,0xa8,0xce,0x2f,0xb2,0xd8,0xdb,0x3c,0x60,0x87,0x1f,0x30,0x7a}
};

uint32_t video_free_pos = 0xff;
uint32_t audio_free_pos = 0xff;
uint32_t video_handler = 0xff;
uint32_t audio_handler = 0xff;
uint16_t stream_id = 0xff;


extern int g_ce_fd;
extern int g_csa_fd;
extern int g_dsc_fd;
extern int g_dmx_see_fd;
	
void usages(void)
{
	printf("	usage:hld_play [file] [vpid] [apid] [ppid] [vtype] [atype] [loop]\n");
}

void showfd(void)
{
    printf("opened %s %s %s %s %s\n",DEMUX_PB_DEVICE,DEMUX2_DEVICE,SEE_DEVICE,VIDEO_DEVICE,AUDIO_DEVICE);
    printf("pb_fd:%d,demux2_vfd:%d,demux2_afd:%d,demux2_pfd:%d,see_fd:%d,video_fd:%d,audio_fd:%d,vpo_fd:%d\n", \
           pb_fd,demux2_vfd,demux2_afd,demux2_pfd,see_fd,video_fd,audio_fd,vpo_fd);		
}


int openfd(void)
{
	pb_fd=demux2_vfd=demux2_afd=see_fd=video_fd=audio_fd=vpo_fd=scart_fd=0;
	g_ce_fd = g_csa_fd = g_dsc_fd = g_dmx_see_fd = 0;
	pb_fd = open(DEMUX_PB_DEVICE, O_RDWR);
	demux2_vfd = open(DEMUX2_DEVICE, O_RDONLY);
	demux2_afd = open(DEMUX2_DEVICE, O_RDONLY);
	see_fd = open(SEE_DEVICE, O_RDWR);	
	video_fd = open(VIDEO_DEVICE, O_RDWR);	
	audio_fd = open(AUDIO_DEVICE, O_RDWR);			
	scart_fd = open(SCART_DEVICE, O_RDWR);		
	vpo_fd = open(VPO_DEVICE, O_RDWR);
	
	if((pb_fd <0) || (demux2_vfd<0) || (demux2_afd<0) || (demux2_pfd<0)
		|| (see_fd<0) || (video_fd<0) || (audio_fd<0) || (vpo_fd<0) || (scart_fd<0)) 
	{	
		printf("open device fail.\n");
		showfd();
		return -1;
	}	

	showfd();	
	
	return 0;	
}


void closefd(void)
{
    close(audio_fd);	
    close(video_fd);	
    close(demux2_vfd);		
    close(demux2_afd);		
    close(demux2_pfd);	
    close(see_fd);		
    close(pb_fd);		
    close(vpo_fd);
	close(scart_fd);
}

int Stop(int signo) 
{
    struct ali_video_rpc_pars rpc_pars;	
    int bclosevp =0;
    int bfillblack=0;
    	
    printf("oops! stop!!!\n");
    if(ioctl(audio_fd, AUDIO_STOP, 0))
    {
        printf("AUDIO_STOP failed\n");
        //return -1;							
    } 		
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_STOP;
    rpc_pars.arg_num = 2;
    rpc_pars.arg[0].arg = (void *)&bclosevp;
    rpc_pars.arg[0].arg_size = sizeof(bclosevp);			
    rpc_pars.arg[1].arg = (void *)&bfillblack;
    rpc_pars.arg[1].arg_size = sizeof(bfillblack);			

    if(ioctl(video_fd, ALIVIDEOIO_RPC_OPERATION, &rpc_pars))
    {
        printf("ALIVIDEOIO_RPC_OPERATION failed\n");
        //return -1;						
    }    
    if(ioctl(see_fd, ALI_DMX_SEE_AV_STOP, 0))
    {
        printf("ALI_DMX_SEE_AV_STOP failed\n");
        //return -1;
    } 
    if(demux2_vfd)
        ioctl(demux2_vfd, ALI_DMX_CHANNEL_STOP, 0);  				    

    if(demux2_afd)
        ioctl(demux2_afd, ALI_DMX_CHANNEL_STOP, 0);  
     
    if(demux2_pfd)     
        ioctl(demux2_pfd, ALI_DMX_CHANNEL_STOP, 0);       
    closefd();
	
	setup_dscrambler_param(0);
	delete_stream_to_ce(video_handler, stream_id, video_free_pos);
	delete_stream_to_ce(audio_handler, stream_id, audio_free_pos);
	close_as_device();  
 
	_exit(0);
}

int dmx_stop(unsigned int vpid, unsigned int apid,unsigned int ppid)
{
    int ret, data_length;
    struct dmx_channel_param      ch_para;
    struct dmx_ts_kern_recv_info *ts_kern_recv_info;    
    struct dmx_see_av_pid_info     see_av_info;
    struct ali_video_rpc_pars rpc_pars;	
    struct ali_fb_rpc_pars  fbrpc_pars;	
    enum dmx_see_av_sync_mode av_sync_mode;
    struct ali_audio_ioctl_command io_param;
    int bclosevp =0;
    int bfillblack=0;
	//step 0 

	//stop 1  stop vpid
    if(ioctl(demux2_vfd, ALI_DMX_CHANNEL_STOP, 0))
    {
       printf("ALI_DMX_CHANNEL_STOP vpid failed\n");
       return -1;
    }

    //step 2  stop apid
    if(apid != 65535)
    {
	    if(ioctl(demux2_afd, ALI_DMX_CHANNEL_STOP, 0))
	    {
	       printf("ALI_DMX_CHANNEL_STOP apid failed\n");
	       return -1;
	    }
    }
    //step 3  set ppid
    if(((ppid & 0x1fff) != (vpid & 0x1fff)) && ((ppid & 0x1fff) != (apid & 0x1fff)))
    {
	    if(ioctl(demux2_pfd, ALI_DMX_CHANNEL_STOP, 0))
	    {
	       printf("ALI_DMX_CHANNEL_STOP ppid failed\n");
	       return -1;
	    }
  	}   
    //step 4  stop see
    if(ioctl(see_fd, ALI_DMX_SEE_AV_STOP, 0))
    {
       printf("ALI_DMX_SEE_AV_STOP failed\n");
       return -1;
    } 
		
    //step 5 stop  vdec
    memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
    rpc_pars.API_ID = RPC_VIDEO_STOP;
    rpc_pars.arg_num = 2;
    rpc_pars.arg[0].arg = (void *)&bclosevp;
    rpc_pars.arg[0].arg_size = sizeof(bclosevp);			
    rpc_pars.arg[1].arg = (void *)&bfillblack;
    rpc_pars.arg[1].arg_size = sizeof(bfillblack);			

    if(ioctl(video_fd, ALIVIDEOIO_RPC_OPERATION, &rpc_pars))
    {
        printf("ALIVIDEOIO_RPC_OPERATION failed\n");
        return -1;						
    }

    //step  stop deca
    unsigned char enable =0;
    unsigned char param1 =100;  
	if(apid != 65535)
	{ 				
		if(ioctl(audio_fd, AUDIO_STOP, 0))
		{
	       printf("AUDIO_STOP failed\n");
	       return -1;			
		}						 	
	} 
}

int dmx_play(unsigned int vpid, unsigned int apid,unsigned int ppid,unsigned int vtype, unsigned int atype)
{
    int ret, data_length;
    struct dmx_channel_param      ch_para;
    struct dmx_ts_kern_recv_info *ts_kern_recv_info;    
    struct dmx_see_av_pid_info     see_av_info;
    struct ali_video_rpc_pars rpc_pars;	
    struct ali_fb_rpc_pars  fbrpc_pars;	
    enum dmx_see_av_sync_mode av_sync_mode;
    struct ali_audio_ioctl_command io_param;
    unsigned short pids[3];

    //step 0 
    pids[0] = vtype?(vpid|0x2000):vpid;
	if(atype == 1)		   
		pids[1] = apid|0x2000;
	else if(atype == 2)		   
		pids[1] = apid|0x8000;
	else if(atype == 3)
		pids[1] = apid|0x4000;
	else
		pids[1] = apid;   				

	pids[2] = ppid; 
	pids[3] = 0x1fff; 

    //step 1  set vpid
    ch_para.output_format = DMX_CHANNEL_OUTPUT_FORMAT_TS;
    ch_para.output_space = DMX_OUTPUT_SPACE_KERNEL;
    ch_para.ts_param.pid_list[0] = pids[0];
    ch_para.ts_param.pid_list_len = 1;
    ts_kern_recv_info = &ch_para.ts_param.kern_recv_info;

    if(ioctl(see_fd, ALI_DMX_SEE_GET_TS_INPUT_ROUTINE, ts_kern_recv_info))
    {		
        printf("ALI_DMX_SEE_GET_TS_INPUT_ROUTINE vid failed\n");			
        return -1;
    }
    ret = ioctl(demux2_vfd, ALI_DMX_CHANNEL_START, &ch_para);
    if(ret)
    {
       printf("dmx2 return ret = %d\n", ret); 
       printf("ALI_DMX_CHANNEL_START vpid failed\n");
       return -1;
    }
    if(ioctl(demux2_vfd, ALI_DMX_RESET_BITRATE_DETECT, 0))
    {
        printf("ALI_DMX_RESET_BITRATE_DETECT vpid failed\n");
        return -1;
    }
    
    //step 2  set apid
    if(apid != 65535)
    {
	    ch_para.ts_param.pid_list[0] = pids[1];
	    if(ioctl(demux2_afd, ALI_DMX_CHANNEL_START, &ch_para))
	    {
	       printf("ALI_DMX_CHANNEL_START apid failed\n");
	       return -1;
	    }
	    if(ioctl(demux2_afd, ALI_DMX_RESET_BITRATE_DETECT, 0))
	    {
	       printf("ALI_DMX_RESET_BITRATE_DETECT apid failed\n");
	       return -1;
	    }
    }
    
    //step 3  set ppid
    if(((ppid & 0x1fff) != (vpid & 0x1fff)) && ((ppid & 0x1fff) != (apid & 0x1fff)))
    {
	    ch_para.ts_param.pid_list[0] = pids[2];
	    if(ioctl(demux2_pfd, ALI_DMX_CHANNEL_START, &ch_para))
	    {
	       printf("ALI_DMX_CHANNEL_START ppid failed\n");
	       return -1;
	    }
  	}   

    //step 4  start see
    see_av_info.v_pid = pids[0];
    see_av_info.a_pid = pids[1];
    see_av_info.p_pid = pids[2]; 
    see_av_info.ad_pid = pids[3]; 
    
    av_sync_mode = DMX_SEE_AV_SYNC_MODE_PLAYBACK;   
    /*if(ioctl(see_fd, ALI_DMX_SEE_AV_STOP, 0))
    {
       printf("ALI_DMX_SEE_AV_STOP  failed\n");
       return -1;
    } */
   
    if(ioctl(see_fd, ALI_DMX_SEE_AV_SYNC_MODE_SET, &av_sync_mode))
    {
       printf("ALI_DMX_SEE_AV_SYNC_MODE_SET failed\n");
       return -1;
    } 
    
    if(ioctl(see_fd, ALI_DMX_SEE_AV_START, &see_av_info))
    {
       printf("ALI_DMX_SEE_AV_START failed\n");
       return -1;
    } 

    int bOn = 1;
	memset((void *)&fbrpc_pars, 0, sizeof(fbrpc_pars));		
	fbrpc_pars.hd_dev = 1;
	fbrpc_pars.API_ID = RPC_FB_WIN_ON_OFF;
	fbrpc_pars.arg_num = 1;
	fbrpc_pars.arg[0].arg = (void *)&bOn;
	fbrpc_pars.arg[0].arg_size = sizeof(bOn);	
	if(ioctl(vpo_fd, FBIO_RPC_OPERATION, &fbrpc_pars))
	{
       printf("RPC_FB_WIN_ON_OFF failed\n");
       return -1;			
	}	
   	//select H.264/MPEG codec
   	int select = 0;
   	int in_preview = 0;
   	if(vtype)
   	{
   		select = 1;
   		memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
   		rpc_pars.API_ID = RPC_VIDEO_SELECT_DEC;
  		rpc_pars.arg_num = 2;	
   		rpc_pars.arg[0].arg = (void *)&select;
   		rpc_pars.arg[0].arg_size = sizeof(select);			
   		rpc_pars.arg[1].arg = (void *)&in_preview;
   		rpc_pars.arg[1].arg_size = sizeof(in_preview);	
  		if(ioctl(video_fd, ALIVIDEOIO_RPC_OPERATION, &rpc_pars))
   		{
   			printf("RPC_VIDEO_SELECT_DEC failed\n");
   			return -1;			
   		}                                                                                                                                  
   	} else {                          
	   select = 0;
	   memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	   rpc_pars.API_ID = RPC_VIDEO_SELECT_DEC;
	   rpc_pars.arg_num = 2;
	   rpc_pars.arg[0].arg = (void *)&select;
	   rpc_pars.arg[0].arg_size = sizeof(select);
	   rpc_pars.arg[1].arg = (void *)&in_preview;
	   rpc_pars.arg[1].arg_size = sizeof(in_preview);
	   if(ioctl(video_fd, ALIVIDEOIO_RPC_OPERATION, &rpc_pars))
	   {
  			printf("RPC_VIDEO_SELECT_DEC failed\n");
			return -1;
	   }

   	}
    //step 5 start vdec
	memset((void *)&rpc_pars, 0, sizeof(rpc_pars));
	rpc_pars.API_ID = RPC_VIDEO_START;
	rpc_pars.arg_num = 0;			
	if(ioctl(video_fd, ALIVIDEOIO_RPC_OPERATION, &rpc_pars))
	{
       printf("RPC_VIDEO_START failed\n");
       return -1;					
	} 	    

    //step  start deca
    unsigned char enable =0;
    if(apid != 65535)
    {   
		printf(" >>>>>>>>>>>>start deca \n");
		if(ioctl(audio_fd, AUDIO_SET_AV_SYNC, ADEC_SYNC_PTS))
		{
	       printf("AUDIO_CHANNEL_SELECT failed\n");
	       return -1;					
		}
		if(atype == 1) 
		{	
			if(ioctl(audio_fd, AUDIO_SET_STREAMTYPE, AUDIO_AC3))
			{
	       		printf("AUDIO_SET_STREAMTYPE failed\n");
	       		return -1;			
			} 
		} 
		else if (atype == 2)
		{
			if(ioctl(audio_fd, AUDIO_SET_STREAMTYPE, AUDIO_MPEG_ADTS_AAC))
			{
	       		printf("AUDIO_SET_STREAMTYPE failed\n");
			}		
		}
		else if (atype == 3)
		{
			if(ioctl(audio_fd, AUDIO_SET_STREAMTYPE, AUDIO_MPEG_AAC))
			{
	       		printf("AUDIO_SET_STREAMTYPE failed\n");
			}
		} else {
			if(ioctl(audio_fd, AUDIO_SET_STREAMTYPE, AUDIO_MPEG2))
			{
	       		printf("AUDIO_SET_STREAMTYPE failed\n");
			}
		}
			
		if(ioctl(audio_fd, AUDIO_PLAY, 0))
		{
	       printf("AUDIO_PLAY failed\n");
	       return -1;			
		}
		
        if(ioctl(audio_fd, AUDIO_CHANNEL_SELECT, SND_DUP_NONE))
        {
            printf("AUDIO_CHANNEL_SELECT failed\n");
            return -1;					
		}			 	      				
		if(ioctl(audio_fd, AUDIO_SET_VOLUME, 40))
		{
	       printf("AUDIO_SET_VOLUME failed\n");
	       return -1;			
		} 
		io_param.ioctl_cmd=FORCE_SPDIF_TYPE;
		io_param.param=SND_OUT_SPDIF_PCM;
		
		/*if(ioctl(audio_fd, AUDIO_SND_IO_COMMAND, &io_param))
		{
	       printf("AUDIO_SND_IO_COMMAND failed\n");
	       return -1;			
		}*/
			
		if(ioctl(scart_fd, 0x10009/*SCART_ENTRY_STADNBY*/, 1))
		{
	       printf("SCART_ENTRY_STADNBY failed\n");
	       return -1;			
		} 
									 	
	} 
}

int main(int argc, char *argv[])
{
	int lock,loop,filefd,read_size;
	unsigned int apid,vpid,ppid,vtype, atype;
	unsigned char read_buf[1024];
	uint8_t temp_cwpk[16];
	int timeout=0;
	/*	
	uint32_t video_free_pos;
	uint32_t audio_free_pos;
	uint32_t video_handler, audio_handler;
	uint16_t stream_id = 0xff;
	*/

	if(argc != 8)
	{
		usages();
		return -1;
	}
    signal(SIGINT, Stop); 		
	openfd();
	vpid = strtoul(argv[2], 0, 0);
	apid = strtoul(argv[3], 0, 0);
	ppid = strtoul(argv[4], 0, 0);
	vtype = strtoul(argv[5], 0, 0);
	atype = strtoul(argv[6], 0, 0);
	loop = strtoul(argv[7], 0, 0);
	printf("Start playback\n");		
	dmx_play(vpid, apid, ppid, vtype, atype);

    open_as_device();
	setup_dscrambler_param(1);

    do{
    	filefd = open(argv[1], O_RDONLY);
    	if(filefd <= 0)
    	{
    		printf("open %s error!\n", argv[1]);
    		return -1;
    	}
		
		load_csk_key();
		memcpy(&temp_cwpk[0], &enc_cwpk[0], 16);
		load_cwpk_to_ce(temp_cwpk);
        
		stream_id = get_free_stream_id(TS_MODE);
		get_free_key_pos(&video_free_pos);
		create_stream_to_ce(vpid, video_free_pos, &video_handler, stream_id);
		get_free_key_pos(&audio_free_pos);
		create_stream_to_ce(apid, audio_free_pos, &audio_handler, stream_id);
	
	  	feed_cw_to_ce(&enc_cw[0][8], vpid, 2, video_free_pos);
		feed_cw_to_ce(&enc_cw[0][8], apid, 2, audio_free_pos);
        feed_cw_to_ce(&enc_cw[0][0], vpid, 3, video_free_pos);
		feed_cw_to_ce(&enc_cw[0][0], apid, 3, audio_free_pos);
	
    	while(1)
    	{
            read_size = read(filefd, read_buf, 1024);
            if(read_size == 1024)
    		{
                write(pb_fd, read_buf, 1024);
                usleep(100);
    		}
    		else
    			break;
    	}
    	close(filefd);
    }while(loop);
		
    printf("Finish playback\n");				
	setup_dscrambler_param(0);
	delete_stream_to_ce(video_handler, stream_id, video_free_pos);
	delete_stream_to_ce(audio_handler, stream_id, audio_free_pos);
	close_as_device();  
    dmx_stop(vpid,apid,ppid);
    closefd();	

	return 0;
}
