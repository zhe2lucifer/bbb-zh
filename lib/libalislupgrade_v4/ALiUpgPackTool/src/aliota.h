#ifndef ALIOTA_H
#define ALIOTA_H


typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;


#define MAX_LEN        128

#define BLOCK_SIZE			4066		//4066 valid data byte at most per section
#define	MODULE_VERSION		0
#define	DOWNLOAD_ID			0x0B

#define MAX_GROUP_NUM		32			//32 at most
#define MAX_MODULE_IN_GROUP	256			//256 at most
#define MAX_SIZE_PER_MODULE (4066*256)	//256 section at most

#define sputc(data,buf,len) do{*buf++ = data; len++;}while(0)

enum OTAPKT_ERROR
{
	ERROR_NONE,
	ERROR_INVALID_PARAM,
	ERROR_FILE_ACCESS,
	ERROR_FILE_SIZE,
	ERROR_MEMORY_ALLOC,
};


struct module_info_t
{
	WORD  module_id;
	DWORD module_size;
	BYTE last_section_num;
};

struct group_info_t
{
	FILE *fin;
	DWORD oui;
	WORD hw_model;
	WORD hw_ver;
	WORD sw_ver;
	WORD sw_model;

	DWORD group_id;
	DWORD group_len;

    WORD module_num;
	module_info_t module_info[MAX_MODULE_IN_GROUP]; 
};

struct ota_user_param_t
{
    int oui;
    int hw_model;
    int hw_ver;
    int sw_model;
    int sw_ver;

	/* down_buffer_size is the buffer size(M) for stb temply stores the ota ts data in the RAM
	 * The size is desided by the user
	 */
    int  down_buffer_size;
    int  pid;
    int  insert_table;
    int  ts_id;
    int  pmt_pid;
    int  prog_num;
    char service_name[MAX_LEN];
    char service_provider[MAX_LEN];
};

int gen_otapkt_start(ota_user_param_t* p_ota_user_param, char* input_file, char* output_file);


#endif // ALIOTA_H
