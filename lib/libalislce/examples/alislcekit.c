/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alisldsckit.c
 *  @brief              Simple example making a single key derivation with KL.
 *                      This example only works with no OTP fused (=all set to 0)!
 *
 *  @version            1.0
 *  @date               10/03/2014
 *  @revision           none
 *
 *  @author             Vincent Pilloux <vincent.pilloux@alitech.com>
 */


/* system header */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* module header */
#include "alislce.h"


uint8_t stardard_encrypt_data[16]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,\
								 0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
uint8_t stardard_decrypt_data[16]={0x61,0x7b,0x3a,0x0c,0xe8,0xf0,0x71,0x00,\
							 	 0x92,0x31,0xf2,0x36,0xff,0x9a,0xa9,0x5c};
uint32_t standard_dec_data1[2] = {0xc3a7b61,0x71f0e8};
uint32_t standard_dec_data2[2] = {0x36f23192,0x5ca99aff};

uint8_t aes_key_1_level_zero_keys[16]={0x14, 0x0f, 0x0f, 0x10, 0x11, 0xb5, 0x22, 0x3d,
                                       0x79, 0x58, 0x77, 0x17, 0xff, 0xd9, 0xec, 0x3a};

uint8_t aes_key_2_3_level_zero_keys[2][16]={
	{0x28,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	0xe8,0x46,0xd4,0xb0,0xe3,0x32,0x06,0x71},
	{0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	0xf5,0x70,0x86,0x7e,0x9f,0x1d,0x05,0x4f}};


void dbg_ioctl(char *ioctl_name, void *p, int len)
{
	int i;
	
	printf("*** IOCTL DBG: %s\n", ioctl_name);
	for (i=0; i<len; i++)
	{
		printf("0x%02x ", *((uint8_t *)p+i));
		if (((i+1) & 0xf)==0)
			printf("\n");
	}
	printf("\n");
}

int main(int argc, char * argv[])
{
	alisl_handle alisl_hdl;
	OTP_PARAM otp_par;
	CE_DATA_INFO ce_data_info;
	CE_DEBUG_KEY_INFO clear_key;
	CE_FOUND_FREE_POS_PARAM key_pos[2];
	int ret, i, k, key_pos_in;
	int fd_ce;
	
	if (ret=alislce_open(&alisl_hdl))
	{
		printf("alislce_open failed...(err: %d)\n", ret);
		return -1;
	}		
	memset(&otp_par, 0, sizeof(OTP_PARAM));
	otp_par.otp_key_pos = OTP_KEY_0_1;  // KL position for OTP root key to be copied to
	otp_par.otp_addr = 0x51 /*OTP_ADDESS_1*/;
	//dbg_ioctl("get_otp_root_key", &otp_par, sizeof(otp_par));
	if (alislce_get_otp_root_key(alisl_hdl, &otp_par))
	{
       printf("\n_load OTP key failed!!\n");
		 return -1;
	}
	memset(&ce_data_info, 0, sizeof(CE_DATA_INFO));
	// key set to zeroes here...
	//memcpy(ce_data_info.data_info.crypt_data, stardard_encrypt_data, AES_CE_KEY_LEN);	
	ce_data_info.data_info.data_len 			= AES_CE_KEY_LEN;	
	ce_data_info.des_aes_info.aes_or_des 	= CE_SELECT_AES; 	
	ce_data_info.des_aes_info.crypt_mode 	= CE_IS_DECRYPT; 			
	ce_data_info.key_info.first_key_pos 	= OTP_KEY_0_1; // 1
	ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP; 
	ce_data_info.key_info.second_key_pos 	= OTP_KEY_0_1 + 4;  // hard coded
	for (i=1; i>=0; i--)  // create odd and even keys
	{
		ce_data_info.des_aes_info.des_low_or_high	= i;		
		//dbg_ioctl("alislce_generate_single_level_key", &ce_data_info, sizeof(ce_data_info));
		if (alislce_generate_single_level_key(alisl_hdl, &ce_data_info))
		{
       	printf("alislce_generate_single_level_key failed!!\n");
		 	return -1;
		}
	}
	
	memset(&clear_key, 0, sizeof(clear_key));
   clear_key.sel=CE_KEY_READ;
   clear_key.len=4;
	//dbg_ioctl("alislce_get_decrypt_key", &clear_key, sizeof(clear_key));
	if (alislce_get_decrypt_key(alisl_hdl, &clear_key))
	{
       printf("alislce_get_decrypt_key failed!!\n");
		 return -1;
	}
		
	if (memcmp(clear_key.buffer, aes_key_1_level_zero_keys, clear_key.len))
	{
		printf("CE key error!\nCalculated key (len %d): ", clear_key.len);
		for (i=0; i<clear_key.len;i++)
		 	printf("0x%08x   ", clear_key.buffer[i]);
		printf("\n");
	}
	else
	{
		printf("CE key level 1 OK!\n");
	}
	
	key_pos_in=OTP_KEY_0_1 + 4;
	
	/* *** level 2 & 3 ***/
	for (k=0; k<2; k++)
	{
		key_pos[k].ce_key_level=2+k;
		key_pos[k].pos=-1;
		key_pos[k].number=0;		// purpose?
		if (alislce_find_free_pos(alisl_hdl, &key_pos[k]))
		{
      	 printf("alislce_find_free_pos failed!!\n");
			 return -1;
		}
		ce_data_info.key_info.first_key_pos 	= key_pos_in; 
		ce_data_info.key_info.second_key_pos 	= key_pos[k].pos;  
		for (i=1; i>=0; i--)  // create odd and even keys
		{
			ce_data_info.des_aes_info.des_low_or_high	= i;		
			//dbg_ioctl("alislce_generate_single_level_key", &ce_data_info, sizeof(ce_data_info));
			if (alislce_generate_single_level_key(alisl_hdl, &ce_data_info))
			{
       		printf("alislce_generate_single_level_key failed!!\n");
				alislce_set_key_pos_idle(alisl_hdl, key_pos[i].pos);
		 		return -1;
			}
		}
		if (alislce_get_decrypt_key(alisl_hdl, &clear_key))
		{
      	 printf("alislce_get_decrypt_key failed!!\n");
			 alislce_set_key_pos_idle(alisl_hdl, key_pos[k].pos);
			 return -1;
		}
		if (memcmp(clear_key.buffer, aes_key_2_3_level_zero_keys[k], clear_key.len))
		{
			printf("CE key error!\nCalculated key (len %d): ", clear_key.len);
			for (i=0; i<clear_key.len;i++)
		 		printf("0x%08x   ", clear_key.buffer[i]);
			printf("\n");
		}
		else
		{
			printf("SL CE key level %d OK!\n", k+2);
		}
		key_pos_in = ce_data_info.key_info.second_key_pos;
	}
	
	for (k=0; k<2; k++)
		alislce_set_key_pos_idle(alisl_hdl, key_pos[k].pos);
	if (alislce_close(alisl_hdl))
	{
		printf("alislce_close failed...\n");
		return -1;
	}	
	return 0;
}
