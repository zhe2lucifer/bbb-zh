#ifndef __LIB_OTA_H__
#define __LIB_OTA_H__

#include <upgrade_object.h>
#if 0
#define OTA_PRINTF  printf
#else
#define OTA_PRINTF(...)  do{}while(0);
#endif

#ifndef	NULL
#define NULL 			((void *)0)
#endif

#define	OTA_SUCCESS		0
#define	OTA_FAILURE		(!OTA_SUCCESS)
#define	OTA_CONTINUE	OTA_SUCCESS
#define	OTA_BREAK		(!OTA_SUCCESS)

#define SYS_OUI               0x090E6
#define SYS_HW_MODEL          0x3516
#define SYS_HW_VERSION        0x0000
#define SYS_SW_MODEL          0x0000
#define SYS_SW_VERSION        0x0000

#define DSI_NUM 0
#define DII_NUM 1
#define DDB_NUM 2

enum DEMUX_STATE{
    DEMUX_FIND_START = 0,
    DEMUX_HEADER,
    DEMUX_DATA,
    DEMUX_SKIP 
};

#define MAX_SEC_MASK_LEN                  16
#define MAX_MV_NUM                              8
struct restrict{
    unsigned char  mask[MAX_SEC_MASK_LEN];
    unsigned char  value[MAX_MV_NUM][MAX_SEC_MASK_LEN];
	/** Mask length in unit of byte */
    unsigned char  mask_len;
	/** Target value number */                   
    unsigned char  value_num;                    
    unsigned char multi_mask[MAX_MV_NUM][MAX_SEC_MASK_LEN];
    unsigned short  tb_flt_msk; 
};

/** This struct use to manage a section buffer */
struct get_section_param {
    /** Start address of section buffer */
    unsigned char * buff;
	/** The end address of available data in section buffer */                                                           
    unsigned long  cur_pos;
	/** The size of allocated section buffer */               
    unsigned short  buff_len;
	
	/** The length of the whole section = section_length + 3 */                  
    unsigned short  sec_tbl_len;     
	
	/** 1: already get section length; 0: not yet */   
    unsigned char    get_sec_len;
	/** 0: needn't CRC verification;
	 *  1: need CRC verification;  
	 *  2 or 3: crc err 
	 */  
    unsigned char    crc_flag;                   
    unsigned char    conti_conter;
	/** currently useless */
    unsigned char    overlap_flag; 
	/** get section delay, defined by app; use default if wai_flg_dly equals to 0 */         
    unsigned long  wai_flg_dly;            
    unsigned long  crc_result;
	/** DEMUX_HEADER, DEMUX_DATA, DEMUX_SKIP */
    enum    DEMUX_STATE  dmx_state;                 
    struct    restrict * mask_value;
	/** 
	 *  for continuously get section mode 
	 *  dmx will call this callback function if continue_get_sec equals to 1
	 */
    void      (*get_sec_cb)(struct get_section_param *); 
    
	/** indicated pid to get */
    unsigned short  pid;
	/** report which value matched */                 
    unsigned short  sec_hit_num;
	/** indicate dmx will continuously get section */    
    unsigned char   continue_get_sec;
	/** RETRIEVE_SEC: retrieve section, RETRIEVE_TS: retrieve TS */
    unsigned char   retrieve_sec_fmt;
    /*until continue_get_sec change to 0*/
#if 0
    unsigned long priv_param; // private use
#endif
};

struct section_param 
{
	unsigned char  section_type;
	unsigned char  param8;
	unsigned short param16;
	unsigned long param32;
	void  *private;	
};

struct dl_info
{
	unsigned short hw_version;
	unsigned short sw_version;
	unsigned char  sw_type;
	unsigned long sw_size;
	unsigned long data;
	unsigned long time;		
};

typedef long (*section_parser_t)(unsigned char *buffer, long buffer_length, void *param);
typedef void (*t_progress_disp)(unsigned long);

int alislupg_ota_mem_config(unsigned char* addr,unsigned long len);
int alislupg_ota_get_download_info(unsigned short pid,struct dl_info *info);
int alislupg_ota_start_download(unsigned short pid,t_progress_disp progress_disp);
int alislupg_ota_download_abort();

#endif /* __LIB_OTA_H__ */
