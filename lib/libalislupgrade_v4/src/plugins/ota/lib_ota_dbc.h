
#ifndef __LIB_OTA_DBC_H__
#define __LIB_OTA_DBC_H__

#include <upgrade_object.h>
#if 0
#define DBC_PRINTF	printf
#else
#define DBC_PRINTF(...)	do{}while(0);
#endif

/* 32*32 blocks in a module = 4Mbytes in a module */
#define BIT_MAP_NUM 32

#define DBC_SUCCESS  0
#define DBC_FAIL     (!DBC_SUCCESS)

struct DCModuleInfo
{
	unsigned short module_id;
	unsigned char module_linked;
	unsigned char module_first;
	unsigned char module_last;
	unsigned char* module_buf_pointer;
	unsigned long module_buf_size;
	unsigned short block_num;
	unsigned short next_module_id;
	unsigned short next_block_num;
	unsigned long block_received[BIT_MAP_NUM];
	unsigned char module_download_finish;
};

struct DCGroupInfo
{
	unsigned long group_id;
	unsigned long group_size;
#ifdef STAR_OTA_GROUP_INFO_BYTE
	unsigned long OUI;				/* Organization Unique Identifier */
	unsigned long stbid_start;		/* Start of ID of STB need OTA upgrade */
	unsigned long stbid_end;		/* End of ID of STB need OTA upgrade */
	unsigned long global_sw_ver;	/* Global software version */
	unsigned long global_hw_ver;	/* Global hardware version */
#endif
/*	
	unsigned long hw_oui;
	unsigned short hw_model;
	unsigned short hw_version;
	unsigned long sw_oui;
	unsigned short sw_model;
	unsigned short sw_version;
*/	
};

int alisl_dsi_message(unsigned char* data,unsigned short len,unsigned long OUI,unsigned short hw_model,unsigned short hw_version,unsigned short sw_model,unsigned short sw_version,unsigned short* new_sw_version,struct DCGroupInfo* group);
int alisl_dii_message(unsigned char* data,unsigned short len,unsigned long group_id,unsigned char* data_addr,struct DCModuleInfo* module,unsigned char* module_num,unsigned short* blocksize);
int alisl_ddb_data(unsigned char* data,unsigned short len,struct DCModuleInfo* module,unsigned short blocksize,unsigned char blk_id);

#endif /*__LIB_OTA_DBC_H__*/

