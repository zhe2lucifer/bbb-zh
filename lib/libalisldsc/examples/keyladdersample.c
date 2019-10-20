/*
  * To decrypt a grogram, you need to follow these steps
  * (1) load the root key to key-ladder
  * (2) decrypt the encrypted CWPK, and save the clear one in key-ladder
  * (3) get a free pos in key-ladder to save the clear control word
  * (4) create a ts stream for a program, so the descrambler can get the ts data, and tell descrambler where
  *       the clear control word saved, so it can use them to descrytpt the program
  * (5) when you get new control word, update them to the key-ladder, so descrambler can get the new one
  * (6) when you change program, release the program's resouce you allocated to it.
  * (7) that's all. 
  *                                                                     Terry Wu
  */

#include "keyladdersample.h"

#define CE_DEVICE "/dev/ali_ce_0"
#define CSA_DEVICE "/dev/ali_csa_0"
#define DSC_DEVICE "/dev/ali_dsc_0"
#define DMX_SEE_DEVICE "/dev/ali_m36_dmx_see_0"

int g_ce_fd = 0;
int g_csa_fd = 0;
int g_dsc_fd = 0;
int g_dmx_see_fd = 0;

uint32_t ce_see_dev;
uint32_t dsc_see_dev;
uint32_t csa_see_dev;
uint32_t dmx_see_dev;

extern int see_fd;

alisl_handle ce_dev;
alisl_handle csa_dev;
alisl_handle dsc_dev;

int open_as_device(void)
{
    struct dsc_see_dev_hld see_dev_hld; 
     
    g_ce_fd = open(CE_DEVICE, O_RDWR);
    if(g_ce_fd < 0)
    {
        printf("open key ladder fail, fd = %d.\n", g_ce_fd);
        return -1;
    }
    else
    {
        ioctl(g_ce_fd, IO_CE_GET_DEV_HLD, &ce_see_dev);
    }
	
    g_csa_fd = open(CSA_DEVICE, O_RDWR);
    if(g_csa_fd < 0)
    {
        printf("open CSA fail, fd = %d.\n", g_csa_fd);
        return -1;
    }
    else
    {
        see_dev_hld.dsc_dev_id = 0;
        ioctl(g_csa_fd, IO_GET_DEV_HLD, &see_dev_hld);
        csa_see_dev = see_dev_hld.dsc_dev_hld;
    }
    
    g_dsc_fd = open(DSC_DEVICE, O_RDWR);
    if(g_dsc_fd < 0)
    {
        printf("open DSC fail, fd = %d.\n", g_dsc_fd);
        return -1;
    }
    else
    {
        see_dev_hld.dsc_dev_id = 0;
        ioctl(g_dsc_fd, IO_GET_DEV_HLD, &see_dev_hld);
        dsc_see_dev = see_dev_hld.dsc_dev_hld;
    }
	/*
    g_dmx_see_fd = open(DMX_SEE_DEVICE, O_RDWR);
    if(g_dmx_see_fd < 0)
    {
        printf("open DMX SEE fail, fd = %d.\n", g_dmx_see_fd);
        return -1;
    }
    */
	
	printf("open AS device success, g_ce_fd=%d, g_csa_fd=%d,g_dsc_fd=%d.\n", g_ce_fd, g_csa_fd, g_dsc_fd);
    return 0;
}

int close_as_device(void)
{
    close(g_ce_fd);
    close(g_csa_fd);
    close(g_dsc_fd);
	//close(g_dmx_see_fd);
}

/* laod CSK from OTP to CE */
int load_csk_key( void ) 
{
	int ret ;
	pOTP_PARAM otp_info;
    
    struct ali_ce_hld_param ce_param;
	
	otp_info = (pOTP_PARAM)malloc(sizeof(pOTP_PARAM));
	if(otp_info == NULL)
	{
		printf("malloc for otp fail.\n");
		return -1;
	}
	memset(otp_info ,0 , sizeof(pOTP_PARAM));
	otp_info->otp_addr = OTP_ADDESS_1;         /* OTP_ADDESS_1 : the place store the CSK key in OTP */
	otp_info->otp_key_pos = KEY_0_0;	       /* KEY_0_0 : the CSK position in key-ladder */

    ce_param.p[0] = (uint32_t)ce_see_dev;
    ce_param.p[1] = (uint32_t)otp_info;
    ret = ioctl(g_ce_fd, IO_OTP_ROOT_KEY_GET, &ce_param);
    if(ret != 0)
	{
		free(otp_info);
		printf("load csk failture!\n");
		return -1;	
	}
	
	free(otp_info);	
	printf("load csk key success!\n");
    
	return 0;
}


/* load encrypted cwpk to CE to be decrypted  */
int load_cwpk_to_ce(uint8_t * enc_cwpk) 
{
    int ret = 0;
	struct DES_64BITS_KEY_INFO en_c;
	CE_DATA_INFO Ce_data_info ;
    struct ali_ce_hld_param ce_param;

	memset(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	memcpy(&en_c , enc_cwpk, 16);
	Ce_data_info.data_info.data_len 			= 8;				/* aes is 16 bytes des/tdes is 8 bytes*/
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_DES ; 	/* select AES or DES module */
	Ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT; 	        /* select Decrypt or Encrypt */
	Ce_data_info.des_aes_info.des_low_or_high 	= 1;					/* high address for ODD key,low address for EVEN key*/
	Ce_data_info.key_info.first_key_pos 		= KEY_0_0 ;          /* the CSK save in it */ 
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	Ce_data_info.key_info.second_key_pos 		= KEY_1_0;                    /* will save the clear CWPK */
	memcpy(Ce_data_info.data_info.crypt_data,en_c.OddKey,8);

    ce_param.p[0] = (uint32_t)ce_see_dev;
    ce_param.p[1] = (uint32_t)&Ce_data_info;
    ret = ioctl(g_ce_fd, IO_CE_GENERATE_SINGLE_LEVEL_KEY, &ce_param);
    if (ret != 0)
	{
		printf("generate cwpk odd fail!\n");
		return -1;
	}
	
	Ce_data_info.des_aes_info.des_low_or_high 	= 0;	
	memcpy(Ce_data_info.data_info.crypt_data, en_c.EvenKey,8);
    ce_param.p[1] = (uint32_t)&Ce_data_info;
    ret = ioctl(g_ce_fd, IO_CE_GENERATE_SINGLE_LEVEL_KEY, &ce_param);
    if (ret != 0)
	{
		printf("generate cwpk even fail!\n");
		return -1;

	}
	
	printf("load cwpk to ce success.\n");
	return 0;
}

/* get a free key_pos for storing the clear control word */
int get_free_key_pos(uint32_t * free_pos)
{
	int ret = 0;
	CE_FOUND_FREE_POS_PARAM key_param;
    struct ali_ce_hld_param ce_param;
	
	memset(&key_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    key_param.ce_key_level = TWO_LEVEL;                     /* the clear CW should be stored in KEY_2_X */

    ce_param.p[0] = (uint32_t)ce_see_dev;
    ce_param.p[1] = (uint32_t)&key_param;
    ret = ioctl(g_ce_fd, IO_CRYPT_FOUND_FREE_POS, &ce_param);
	if (ret < 0)
	{
		*free_pos = 0xff;
		printf("get free key pos fail.\n");
	}
	else
	{
		*free_pos = key_param.pos;
        printf("get free key pos %d.\n", key_param.pos);
	}

	return ret;
}

/*
   * load encrypted control word to CE to be decrypt 
   * call when you get new control word
   */
int feed_cw_to_ce(uint8_t * enc_cw, uint16_t pid, uint8_t cw_type, enum CE_CRYPT_TARGET sec_key_pos)
{
	int ret = 0;
	CE_DATA_INFO Ce_data_info;
	struct ali_ce_hld_param ce_param;

	if(pid == 0x1FFF)
		return -1;

	memset(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	Ce_data_info.data_info.data_len 			= 8;				/* aes is 16 bytes des/tdes is 8 bytes*/
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_DES ; 	/* select AES or DES module*/
	Ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT;
	if(3 == cw_type)        /* odd key */
		Ce_data_info.des_aes_info.des_low_or_high   = 1;	                        
	else                  /* even key */
		Ce_data_info.des_aes_info.des_low_or_high 	= 0;
	Ce_data_info.key_info.first_key_pos 			= KEY_1_0 ;           /* the clear CWPK save in it */
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	Ce_data_info.key_info.second_key_pos 		= sec_key_pos;              /* the clear control word will save in it */
	memcpy(Ce_data_info.data_info.crypt_data, enc_cw, 8);
    
    ce_param.p[0] = (uint32_t)ce_see_dev;
    ce_param.p[1] = (uint32_t)&Ce_data_info;
    ret = ioctl(g_ce_fd, IO_CE_GENERATE_SINGLE_LEVEL_KEY, &ce_param);
    if (ret != 0)
	{
		printf("generate control word fail!\n");
		return -1;
	}

	printf("generate control word success.\n");
	return 0;
}

uint16_t get_free_stream_id(enum DMA_MODE mode)
{
	int ret = 0;
    uint16_t ca_ts_stream_id = 0xff;
    struct ali_dsc_hld_param dsc_param;
    
    /* get free stream id for a program, you should use the same stream id for all the pid of the program */    
    dsc_param.p[0] = mode;
    dsc_param.p[1] = (uint32_t)(&ca_ts_stream_id);
    ret = ioctl(g_dsc_fd, IO_GET_FREE_STREAM_ID, &dsc_param);  
    if(ret != 0)
    {
        printf("get free stream id fail\n");
        return 0xff;
    }

    printf("get free stream id success, stream id =%d.\n", ca_ts_stream_id);
    return ca_ts_stream_id;
}

/*  
  * create a stream for a program to be decrypt, not allow create exist stream and invalid pid stream handler
  */
int create_stream_to_ce(uint16_t pid, uint32_t key_pos, uint32_t * handler, uint16_t  stream_id) 
{
	int ret = 0;
	CSA_INIT_PARAM csa_param;
	KEY_PARAM key_param;
    struct ali_dsc_hld_param dsc_param;
		
    /* tell dsc use which ts_stream_id */
    dsc_param.p[0] = (uint32_t)dsc_see_dev;
    dsc_param.p[1] = stream_id;
    ret = ioctl(g_dsc_fd, IO_PARSE_DMX_ID_SET_CMD, &dsc_param);
    if(ret != 0)
    {
        printf("set stream id fail.\n");
    }
    
	memset(&csa_param, 0, sizeof(CSA_INIT_PARAM));
	csa_param.Dcw[0] = 0 ;                         /* reserved, not for Conax */
	csa_param.Dcw[1] = 1 ;
	csa_param.Dcw[2] = 2 ;
	csa_param.Dcw[3] = 3 ;
	csa_param.dma_mode = TS_MODE ;                 /* ts stream should use TS_MODE */
	csa_param.key_from = KEY_FROM_CRYPTO;          /* key from key-ladder */
	csa_param.parity_mode = AUTO_PARITY_MODE0 ;    /* for ts */
	csa_param.pes_en = 1;                          /*not used now*/
	csa_param.scramble_control = 0 ;               /*dont used default CW*/
	csa_param.stream_id = stream_id;
	csa_param.version = CSA1 ;

    dsc_param.p[0] = (uint32_t)csa_see_dev;
    dsc_param.p[1] = (uint32_t)&csa_param;
    ret = ioctl(g_csa_fd, IO_INIT_CMD, &dsc_param); 
	if(ret != 0)
	{
	    printf("CSA IO_INIT_CMD fail at str_id %d\n", stream_id);
		return -1;
	}

	memset(&key_param, 0, sizeof(KEY_PARAM));
    key_param.handle = 0xFF;                          /* output, will save the CSA handler*/
	key_param.ctr_counter = NULL ; 
	key_param.init_vector = NULL ;                	  /* for PURE_DATA_MODE */
	key_param.key_length = 128;  
	key_param.pid_len = 1; 
	key_param.pid_list = &pid;
	key_param.p_aes_iv_info = NULL ;                  /* reserve, not for CSA */
	key_param.stream_id = stream_id;
	key_param.force_mode = 1;
	key_param.pos = key_pos;

    dsc_param.p[0] = (uint32_t)csa_see_dev;
    dsc_param.p[1] = (uint32_t)&key_param;
    ret = ioctl(g_csa_fd, IO_CREAT_CRYPT_STREAM_CMD, &dsc_param); 
	if(ret != 0)
	{
	    printf("CSA IO_CREAT_CRYPT_STREAM_CMD fail at str_id %d\n", stream_id);
		return -1;
	}

	*handler = key_param.handle;
	
	printf("create stream for pid %d success, stream_id=%d.\n", pid, stream_id);
	return 0;
}


/* when change program, you should release the resource */
int delete_stream_to_ce(uint32_t handler, uint16_t stream_id, enum CE_CRYPT_TARGET key_pos) 
{
	int ret = 0;
	struct ali_dsc_hld_param dsc_param;
	struct ali_ce_hld_param ce_param;
	
	/* release the stream id */   
    dsc_param.p[0] = stream_id;
    ret = ioctl(g_dsc_fd, IO_SET_STREAM_ID_IDLE, &dsc_param);
    if(ret != 0)
    {
        printf("set stream id idle fail.\n");
    }
    
	/* release the CSA handler */
    dsc_param.p[0] = (uint32_t)csa_see_dev;
    dsc_param.p[1] = (uint32_t)handler;
    ret = ioctl(g_csa_fd, IO_DELETE_CRYPT_STREAM_CMD, &dsc_param); 
	if(ret != 0)
	{
	    printf("CSA IO_DELETE_CRYPT_STREAM_CMD fail at handler 0x%x\n", handler);
		return -1;
	}
    
	/* release the key_pos */
    ce_param.p[0] = (uint32_t)ce_see_dev;
    ce_param.p[1] = key_pos;
    ret = ioctl(g_ce_fd, IO_CRYPT_POS_SET_IDLE, &ce_param);
    if(ret != 0)
    {
        printf("set key pos idle fail.\n");
        return -1;
    }
    
	return 0;
}

int setup_dscrambler_param(int start_dsc)
{
	int ret;
	struct dec_parse_param dec_param;

	dec_param.dec_dev = (void *)csa_see_dev;
	dec_param.type = (uint32_t)CSA;
    //ret = ioctl(g_dmx_see_fd, ALI_DMX_SEE_CRYPTO_TYPE_SET, &dec_param);
    ret = ioctl(see_fd, ALI_DMX_SEE_CRYPTO_TYPE_SET, &dec_param);
	if(ret != 0)
    {
        printf("set crypto type fail.\n");
        return -1;
    }
	else
	{
		printf("set crypto type success.\n");
	}	

    if(start_dsc)
	{
        //ret = ioctl(g_dmx_see_fd, ALI_DMX_SEE_CRYPTO_START, 0);
        ret = ioctl(see_fd, ALI_DMX_SEE_CRYPTO_START, 0);
    }
	else
	{
        //ret = ioctl(g_dmx_see_fd, ALI_DMX_SEE_CRYPTO_STOP, 0);
        ret = ioctl(see_fd, ALI_DMX_SEE_CRYPTO_STOP, 0);
	}    
	if(ret != 0)
    {
        printf("operate descrambler fail.\n");
        return -1;
    }
	else
	{
		printf("operate dscrambler success.\n");
	}

    return 0;
}

