#include <QString>
#include <QSettings>
#include <QFileInfo>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iosfwd>
#include <fstream>

#include "aliota.h"

using namespace std;

#define GROUPINFOINDICATION_102_006		//groupInfoIndication (inside loop)

#define TEMP_MAX_PATH 512

static ota_user_param_t g_ota_user_param;
static group_info_t g_group_info[MAX_GROUP_NUM];

static DWORD g_group_num = 1;
static DWORD g_current_group = 0;

static WORD g_pid,g_ts_id,g_prog_num,g_pmt_pid;

static BYTE g_dsi_section_buf[4096];
static BYTE g_dii_section_buf[MAX_GROUP_NUM][4096];
static WORD g_dsi_section_len;
static WORD g_dii_section_len[MAX_GROUP_NUM];

static char g_SrvName[MAX_LEN];
static char g_SrvPrvdr[MAX_LEN];

BYTE g_table_byte[1024];

static unsigned int MG_CRC_Table[256];

//MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_CCITT		0x04c11db7
static unsigned int MG_Compute_CRC(register unsigned int crc,
                        register unsigned char *bufptr,
                        register int len)
{
    register int i;


//MG_CRC_32_ARITHMETIC_CCITT
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)(*bufptr) << 24;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80000000)	/*Highest bit procedure*/
                crc = (crc << 1) ^ MG_CRC_32_CCITT;
            else
                crc <<= 1;
        }
    }
    return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/

}

static void MG_Setup_CRC_Table()
{
    register int count;
    unsigned char zero=0;

    for(count = 0; count <= 255; count++) /*Comput input data's CRC, from 0 to 255*/

//MG_CRC_32_ARITHMETIC_CCITT
        MG_CRC_Table[count] = MG_Compute_CRC(count << 24,&zero,1);

}
static unsigned int MG_Table_Driven_CRC(register unsigned int crc,
                register unsigned char *bufptr,
                register int len)
{
    register int i;

    for(i = 0; i < len; i++)

//MG_CRC_32_ARITHMETIC_CCITT
        crc=(MG_CRC_Table[((crc >> 24) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0xffffffff;

    return(crc);
}

static void MG_FCS_Coder(unsigned char *pucInData,int len)
{
    unsigned int iFCS;


// MG_CRC_32_ARITHMETIC_CCITT
    iFCS = MG_Table_Driven_CRC(0xffffffff,pucInData,len);
    pucInData[len + 3] = (unsigned char)iFCS & 0xff;
    pucInData[len + 2] = (unsigned char)(iFCS >>  8) & 0xff;
    pucInData[len + 1] = (unsigned char)(iFCS >> 16) & 0xff;
    pucInData[len]     = (unsigned char)(iFCS >> 24) & 0xff;

}

void insert_PAT(FILE* fout, BYTE continuity_count,WORD ts_id,WORD prog_num,WORD pmt_pid)
{
    int i;
    BYTE table_byte[16];
    continuity_count = continuity_count%0x0f;


    putc(0x47,fout);	//sync_byte
    putc(0x40,fout);	//pid 0,payload_unit_start_indicator = 1
    putc(0x00,fout);	//pid 0
    putc(0x10|continuity_count,fout);	// adaptation_field_control = 01

    putc(0x00,fout); //field pointer 0

    table_byte[0] = 0x00;//putc(0x00,fout); //table id 0
    table_byte[1] = 0xB0;//putc(0xB0,fout); //
    table_byte[2] = 0x0D;//putc(0x0D,fout); //section length 0x000D
    table_byte[3] = ((ts_id&0xff00)>>8);//putc(0x00,fout); //transport stream id
    table_byte[4] = (ts_id&0x00ff);//putc(0x00,fout); //transport stream id 0x0000
    table_byte[5] = 0xC9;//putc(0xC1,fout); //	current_next_indicator 1
    table_byte[6] = 0x00;//putc(0x00,fout); //section number 0
    table_byte[7] = 0x00;//putc(0x00,fout); //last_section_number 0

    table_byte[8] = ((prog_num&0xff00)>>8);//putc(0x00,fout);
    table_byte[9] = (prog_num&0x00ff);//putc(0x01,fout);  //program_number 1
    table_byte[10] = 0xE0|((pmt_pid&0x1f00)>>8);//putc(0xE0,fout);
    table_byte[11] = (pmt_pid&0x00ff);//putc(0x20,fout);  //PMT PID	0x20
//CRC
    MG_FCS_Coder(table_byte,12);

    for(i=0;i<16;i++)
        putc(table_byte[i],fout);

    for(i=0;i<180-13;i++)
        putc(0xff,fout);

}

int add_system_software_update_info(BYTE *buf)
{
    DWORD i;
    BYTE *loop = buf+1;

    for(i=0; i<g_group_num; i++)
    {
        loop[0] = (g_group_info[i].oui>>16)&0xFF;
        loop[1] = (g_group_info[i].oui>>8)&0xFF;
        loop[2] = g_group_info[i].oui&0xFF;
        loop[3] = 0xF1;
        loop[4] = 0xC0;
        loop[5] = 0;

        loop += 6;
    }

    buf[0] = 6*g_group_num;

    return buf[0]+1;
}

void insert_PMT(FILE* fout,BYTE continuity_count,WORD pcr_pid,WORD es_pid, BYTE stream_type,WORD pmt_pid,WORD prog_num)
{
    int i;
    int section_len, es_info_len,info_len;
    //BYTE table_byte[45];
    continuity_count = continuity_count%0x0f;


    putc(0x47,fout);	//sync_byte
    putc(0x40|((pmt_pid&0x1f00)>>8),fout);	//pid 0,payload_unit_start_indicator = 1
    putc((pmt_pid&0x00ff),fout);	//pid 2
    putc(0x10|continuity_count,fout);	// adaptation_field_control = 01

    putc(0x00,fout); //field pointer 0


    g_table_byte[0] = 0x02;//putc(0x02,fout); //table id 2
    //table_byte[1] = 0xB0;//putc(0xB0,fout); //
    //table_byte[2] = 0x1D;//putc(0x12,fout); //section length 0x0012
    g_table_byte[3] = ((prog_num&0xff00)>>8);//putc(0x00,fout); //program_number 1
    g_table_byte[4] = (prog_num&0x00ff);//putc(0x01,fout); //program_number 1
    g_table_byte[5] = 0xC1;//putc(0xC1,fout); //	current_next_indicator 1
    g_table_byte[6] = 0x00;//putc(0x00,fout); //section number 0
    g_table_byte[7] = 0x00;//putc(0x00,fout); //last_section_number 0

    g_table_byte[8] = 0xE0|(pcr_pid>>8);//putc(0xE0|(pcr_pid>>8),fout);
    g_table_byte[9] = pcr_pid&0x00ff;//putc(pcr_pid&0x00ff,fout);
    g_table_byte[10] = 0xF0;//putc(0xF0,fout);
    g_table_byte[11] = 0x00;//putc(0x00,fout); //program_info_len

    g_table_byte[12] = stream_type;//putc(stream_type,fout);
    g_table_byte[13] = 0xE0|(es_pid>>8);//putc(0xE0|(es_pid>>8),fout);
    g_table_byte[14] = es_pid&0x00ff;//putc(es_pid&0x00ff,fout);
    //table_byte[15] = 0xF0;//putc(0xF0,fout);
    //table_byte[16] = 0x0b;//putc(0x00,fout);  //ES_info_len = 0x0b

    //data broadcast_id desc
    g_table_byte[17] = 0x66;//putc(0x00,fout);
    //table_byte[18] = 0x09;//putc(0x00,fout);
    g_table_byte[19] = 0x00;//putc(0x00,fout);
    g_table_byte[20] = 0x0a;//putc(0x00,fout);

    info_len = add_system_software_update_info(&g_table_byte[21]);

    es_info_len = info_len+4;

    section_len = 14+es_info_len+4;

    g_table_byte[1] = 0xB0 | (section_len>>8);
    g_table_byte[2]  = section_len&0xFF;
    g_table_byte[15] = 0xF0 | (es_info_len>>8);
    g_table_byte[16] = es_info_len&0xFF;
    g_table_byte[18] = info_len+2;

    //table_byte[21] = 0x06;//putc(0x00,fout);

    //table_byte[22] = 0x00;//putc(0x00,fout);
    //table_byte[23] = 0x0d;//putc(0x00,fout);
    //table_byte[24] = 0x64;//putc(0x00,fout);

    //table_byte[25] = 0xf2;//putc(0x00,fout);
    //table_byte[26] = 0xc0;//putc(0x00,fout);
    //table_byte[27] = 0x00;//putc(0x00,fout);

#ifdef ST_ID_DESC
    //stream_identify desc
    g_table_byte[28] = 0x52;//putc(0x00,fout);
    g_table_byte[29] = 0x01;//putc(0x00,fout);
    g_table_byte[30] = 0x01;//putc(0x00,fout);
#endif

#ifdef ST_ID_DESC
    MG_FCS_Coder(g_table_byte,31);

    for(i=0;i<35;i++)
        putc(g_table_byte[i],fout);

    for(i=0;i<180-32;i++)
        putc(0xff,fout);
#else
    MG_FCS_Coder(g_table_byte,21+info_len);

    for(i=0;i<21+info_len+4;i++)
        putc(g_table_byte[i],fout);

    for(i=0;i<188-(21+info_len+4+5);i++)
        putc(0xff,fout);
#endif
}

void insert_SDT(FILE* fout,BYTE continuity_count,WORD ts_id,WORD prog_num)
{
    int i;
    int section_len, desc_loop_len;
    BYTE table_byte[256];
    continuity_count = continuity_count%0x0f;

    WORD ori_network_id = 0x0001;
    WORD sn_len = strlen(g_SrvName);//g_SrvName.GetLength();
    WORD sp_len = strlen(g_SrvPrvdr);//g_SrvPrvdr.GetLength();

    putc(0x47,fout);	//sync_byte
    putc(0x40,fout);	//pid 0,payload_unit_start_indicator = 1
    putc(0x11,fout);	//pid 17
    putc(0x10|continuity_count,fout);	// adaptation_field_control = 01

    putc(0x00,fout); //field pointer 0


    table_byte[0] = 0x42;//putc(0x02,fout); //table id 2
    //table_byte[1] = 0xF0;//putc(0xB0,fout); //
    //table_byte[2] = 0x31;//putc(0x12,fout); //section length 0x0012
    table_byte[3] = ((ts_id&0xff00)>>8);//putc(0x00,fout); //program_number 1
    table_byte[4] = (ts_id&0x00ff);//putc(0x01,fout); //program_number 1
    table_byte[5] = 0xC9;//putc(0xC1,fout); //	current_next_indicator 1
    table_byte[6] = 0x00;//putc(0x00,fout); //section number 0
    table_byte[7] = 0x00;//putc(0x00,fout); //last_section_number 0

    table_byte[8] = ((ori_network_id&0xff00)>>8);//putc(0xE0|(pcr_pid>>8),fout);
    table_byte[9] = (ori_network_id&0x00ff);//putc(pcr_pid&0x00ff,fout);
    table_byte[10] = 0xFF;//putc(0xF0,fout);

    table_byte[11] = ((prog_num&0xff00)>>8);//putc(0x00,fout); //program_info_len
    table_byte[12] = (prog_num&0x00ff);//putc(stream_type,fout);
    table_byte[13] = 0xFC;//putc(0xE0|(es_pid>>8),fout);
    //table_byte[14] = 0X80;//putc(es_pid&0x00ff,fout);
    //table_byte[15] = 0x20;//putc(0xF0,fout);

    //data Service_desc
    table_byte[16] = 0x48;//putc(0x00,fout);  //Service_desc
    table_byte[17] = sn_len + sp_len + 3;//putc(0x00,fout); //des_len

    table_byte[18] = 0x0C;//putc(0x00,fout);
    table_byte[19] = (BYTE)sp_len;//putc(0x00,fout);
    memcpy(&(table_byte[20]),g_SrvPrvdr/*g_SrvPrvdr.GetBuffer(sp_len)*/,sp_len);
    table_byte[sp_len+20] = (BYTE)sn_len;//putc(0x00,fout);
    memcpy(&(table_byte[sp_len+21]),g_SrvName/*g_SrvName.GetBuffer(sn_len)*/,sn_len);

    //get desc loop length
    desc_loop_len = sn_len+sp_len+5;
    table_byte[14] = 0x80 | (desc_loop_len>>8);
    table_byte[15] = (desc_loop_len&0xFF);

    //get section length
    section_len = sn_len+sp_len+21+4-3;
    table_byte[1] = 0xF0 | (section_len>>8);
    table_byte[2] = (section_len&0xFF);

//CRC
    MG_FCS_Coder(table_byte,sn_len+sp_len+21);

    for(i=0;i<sn_len+sp_len+21+4;i++)//4 bytes CRC
        putc(table_byte[i],fout);

    for(i=0;i<188-(sn_len+sp_len+21+4+5);i++)
        putc(0xff,fout);
}

void table_insert(FILE *fin,
              FILE *fout,
              WORD l_ts_id,
              WORD l_prog_num,
              WORD l_pmt_pid,
              WORD l_pid)
{
    int i;
    BYTE prebyte;

    WORD pcr_pid = 0x1fff;
    WORD es_pid = l_pid;
    BYTE stream_type = 0xB;



    MG_Setup_CRC_Table();


    //continuity_count = 0;

    insert_PAT(fout,0,l_ts_id,l_prog_num,l_pmt_pid);
    insert_PMT(fout,1,pcr_pid,es_pid,stream_type,l_pmt_pid,l_prog_num);
    insert_SDT(fout,2,l_ts_id,l_prog_num);

    while(!feof(fin))
    {
        prebyte = getc(fin);
        if(prebyte!=0x47)
            break;
        else
            putc(prebyte,fout);
        for(i=0;i<187;i++)
        {
            putc(getc(fin),fout);
        }

    }

}


void gen_ts_stream(BYTE* sec_buf,WORD sec_len,FILE* fout)
{
    static BYTE c_c = 3; //continuity_counter
    BYTE payload_unit_start_indicator = 1;

    BYTE i;
    BYTE adaptation_field_control,pointer_field;
    BYTE tmp;//,adap_f_leng;
    WORD packet_len;

    adaptation_field_control = 1;

    while(sec_len>0)
    {
        /*
        if(sec_len>184 || (sec_len==183 && payload_unit_start_indicator==1))
        {
            adaptation_field_control = 1;
        }
        else
        {
            adaptation_field_control = 3;
        }
        */
        tmp = tmp & 0x00; //clear
        putc(0x47,fout); //sync_byte
        tmp = tmp & 0x7f;    // transport_error_indicator = 0
        if(payload_unit_start_indicator==1)
        {
            tmp = tmp | 0x40;
        }
        tmp = tmp & 0xdf;	// tranport_priority = 0
        tmp = tmp | ((0x1f00 & g_pid) >> 8);
        //printf("PID high byte= 0x%02x\n",tmp);
        putc(tmp,fout);
        tmp = (0x00ff & g_pid);
        //printf("PID low byte= 0x%02x\n",tmp);
        putc(tmp,fout);
        tmp = tmp & 0x00; //clear
        tmp = tmp & 0x3f; // transport_scrambling_control = 00
        if(adaptation_field_control == 3)
            tmp = tmp | 0x30; // adaptation_field_control = 11
        else
            tmp = tmp | 0x10; // adaptation_field_control = 11
        if(c_c > 0xf)
        {
            tmp = tmp & 0xf0 ; // wrap around to 0
            c_c = 1;
        }
        else
        {
            tmp = tmp + c_c;
            c_c++;
        }
        putc(tmp,fout);


        if(payload_unit_start_indicator==1)
        {
            pointer_field = 0;
            putc(pointer_field,fout);

            if(sec_len>183)
                packet_len = 183;
            else
                packet_len = sec_len;

            for(i=0;i<packet_len;i++)
                putc(*sec_buf++,fout);
            for(i=0;i<183-packet_len;i++)
                putc(0xff,fout);
        }
        else
        {
            if(sec_len>184)
                packet_len = 184;
            else
                packet_len = sec_len;

            for(i=0;i<packet_len;i++)
                putc(*sec_buf++,fout);
            for(i=0;i<184-packet_len;i++)
                putc(0xff,fout);
        }

        if(payload_unit_start_indicator==1)
        {
            payload_unit_start_indicator = 0;
        }
        sec_len -= packet_len;

    }
}

void put_dsi_section(BYTE* section_buf,	WORD* section_len)
{
    int i;
    DWORD transactionId,/*tCDownloadWindow,*/group_id,group_size/*,module_size*/;
    WORD dsmcc_section_length,table_id_extension;
    BYTE adaptationLength;
    WORD messageId,privateDateLength;

    WORD compatibility_descriptor_len;
    WORD num_of_groups;
    WORD group_info_len;
    WORD descriptor_count;
    BYTE descriptor_type;
    BYTE descriptor_len;
    BYTE specifier_type;
    DWORD specifier_data;
    WORD model;
    WORD version;


    MG_Setup_CRC_Table(); //init here


    BYTE* sec_buf = section_buf;
    WORD sec_len = 0;

    sec_buf[0] = 0x3B; //sputc(0x3B,sec_buf,sec_len);	//table_id

    //dsmcc_section_length = 5+ 76 + 4; //varialbe according to the following size---------------------
    //sec_buf[1] = 0; //sputc(0xB0+((dsmcc_section_length&0x0f00)>>8),sec_buf,sec_len);
    //sec_buf[2] = 0; //sputc((dsmcc_section_length&0x00ff),sec_buf,sec_len);

    transactionId = 0x80000000;
    table_id_extension = transactionId&0x0000ffff;
    sec_buf[3] = (table_id_extension&0xff00)>>8; //sputc((table_id_extension&0xff00)>>8,sec_buf,sec_len);
    sec_buf[4] = table_id_extension&0x00ff; //sputc((table_id_extension&0x00ff),sec_buf,sec_len);

    sec_buf[5] = 0xc1; //sputc(0xc1,sec_buf,sec_len);//reserved(11) + version_number(00000)+ current_next_ind(1)

    sec_buf[6] = 0; //sputc(0,sec_buf,sec_len);//section_number

    sec_buf[7] = 0; //sputc(0,sec_buf,sec_len);//last_section_number

    /*UN_Message*/
        /* dsmccMessageHeader */
        sec_buf[8] = 0x11; //sputc(0x11,sec_buf,sec_len);//protocolDiscriminator

        sec_buf[9] = 0x03; //sputc(0x03,sec_buf,sec_len);//dsmccType

        messageId = 0x1006;
        sec_buf[10] = (messageId&0xff00)>>8; //sputc((messageId&0xff00)>>8,sec_buf,sec_len);//messageId
        sec_buf[11] = messageId&0x00ff; //sputc((messageId&0x00ff),sec_buf,sec_len);

        sec_buf[12] = (transactionId&0xff000000)>>24; //sputc((transactionId&0xff000000)>>24,sec_buf,sec_len);
        sec_buf[13] = (transactionId&0x00ff0000)>>16; //sputc((transactionId&0x00ff0000)>>16,sec_buf,sec_len);
        sec_buf[14] = (transactionId&0x0000ff00)>>8; //sputc((transactionId&0x0000ff00)>>8,sec_buf,sec_len);
        sec_buf[15] = transactionId&0x000000ff; //sputc((transactionId&0x000000ff),sec_buf,sec_len);

        sec_buf[16] = 0xff; //sputc(0xff,sec_buf,sec_len);//reserved

        adaptationLength = 0;
        sec_buf[17] = adaptationLength; //sputc(adaptationLength,sec_buf,sec_len);//adaptationLength

        //messageLength = 64;//varialbe according to the following size ----------------------
        //sec_buf[18] = 0xff; //sputc((messageLength&0xff00)>>8,sec_buf,sec_len);
        //sec_buf[19] = 0xff; //sputc((messageLength&0x00ff),sec_buf,sec_len);

        //for(i=0;i<adaptationLength;i++)
        //	sputc(0xff,sec_buf,sec_len);
        /* ~dsmccMessageHeader */

        for(i=0;i<20;i++)//serverId (all "FF")
            sec_buf[20+i] = 0xff; //sputc(0xff,sec_buf,sec_len);

        sec_buf[40] = 0; //sputc(0,sec_buf,sec_len);//compatibilityDescriptor() (0x0000)
        sec_buf[41] = 0; //sputc(0,sec_buf,sec_len);

        //privateDateLength = 40;//varialbe according to the following size ----------------------
        //sec_buf[42] = 0xff; //sputc((privateDateLength&0xff00)>>8,sec_buf,sec_len);
        //sec_buf[43] = 0xff; //sputc((privateDateLength&0x00ff),sec_buf,sec_len);

        /*dsi private data*/
            num_of_groups = g_group_num;
            sec_buf[44] = (num_of_groups&0xff00)>>8; //sputc((num_of_groups&0xff00)>>8,sec_buf,sec_len);
            sec_buf[45] = num_of_groups&0x00ff; //sputc((num_of_groups&0x00ff),sec_buf,sec_len);

            sec_len = 46;
            for(i=0;i<num_of_groups;i++)
            {
                group_id = g_group_info[i].group_id;
                sec_buf[sec_len+0] = (group_id&0xff000000)>>24; //sputc((group_id&0xff000000)>>24,sec_buf,sec_len);
                sec_buf[sec_len+1] = (group_id&0x00ff0000)>>16; //sputc((group_id&0x00ff0000)>>16,sec_buf,sec_len);
                sec_buf[sec_len+2] = (group_id&0x0000ff00)>>8; //sputc((group_id&0x0000ff00)>>8,sec_buf,sec_len);
                sec_buf[sec_len+3] = group_id&0x000000ff; //sputc((group_id&0x000000ff),sec_buf,sec_len);

                group_size = g_group_info[i].group_len;//test
                sec_buf[sec_len+4] = (group_size&0xff000000)>>24; //sputc((group_size&0xff000000)>>24,sec_buf,sec_len);
                sec_buf[sec_len+5] = (group_size&0x00ff0000)>>16; //sputc((group_size&0x00ff0000)>>16,sec_buf,sec_len);
                sec_buf[sec_len+6] = (group_size&0x0000ff00)>>8; //sputc((group_size&0x0000ff00)>>8,sec_buf,sec_len);
                sec_buf[sec_len+7] = group_size&0x000000ff; //sputc((group_size&0x000000ff),sec_buf,sec_len);

                /*GroupCompatibility*/
                compatibility_descriptor_len = 24;//varialbe according to the following size ----------------------
                sec_buf[sec_len+8] = (compatibility_descriptor_len&0xff00)>>8; //sputc((compatibility_descriptor_len&0xff00)>>8,sec_buf,sec_len);
                sec_buf[sec_len+9] = compatibility_descriptor_len&0x00ff; //sputc((compatibility_descriptor_len&0x00ff),sec_buf,sec_len);

                descriptor_count = 2;
                sec_buf[sec_len+10] = (descriptor_count&0xff00)>>8; //sputc((descriptor_count&0xff00)>>8,sec_buf,sec_len);
                sec_buf[sec_len+11] = descriptor_count&0x00ff; //sputc((descriptor_count&0x00ff),sec_buf,sec_len);

                {// descriptor 0
                    descriptor_type = 0x01; //hardware descriptor
                    sec_buf[sec_len+12] = descriptor_type; //sputc(descriptor_type,sec_buf,sec_len);//descriptor_type
                    descriptor_len = 9;
                    sec_buf[sec_len+13] = descriptor_len; //sputc(descriptor_len,sec_buf,sec_len);//descriptor_len

                    specifier_type = 0x01; // IEEE OUI.
                    sec_buf[sec_len+14] = specifier_type; //sputc(specifier_type,sec_buf,sec_len);//specifier_type

                    specifier_data = g_group_info[i].oui;
                    sec_buf[sec_len+15] = (specifier_data&0x00ff0000)>>16; //sputc((specifier_data&0x00ff0000)>>16,sec_buf,sec_len);
                    sec_buf[sec_len+16] = (specifier_data&0x0000ff00)>>8; //sputc((specifier_data&0x0000ff00)>>8,sec_buf,sec_len);
                    sec_buf[sec_len+17] = specifier_data&0x000000ff; //sputc((specifier_data&0x000000ff),sec_buf,sec_len);

                    model = g_group_info[i].hw_model;
                    sec_buf[sec_len+18] = (model&0xff00)>>8; //sputc((model&0xff00)>>8,sec_buf,sec_len);
                    sec_buf[sec_len+19] = model&0x00ff; //sputc((model&0x00ff),sec_buf,sec_len);

                    version = g_group_info[i].hw_ver;
                    sec_buf[sec_len+20] = (version&0xff00)>>8; //sputc((version&0xff00)>>8,sec_buf,sec_len);
                    sec_buf[sec_len+21] = version&0x00ff; //sputc((version&0x00ff),sec_buf,sec_len);

                    sec_buf[sec_len+22] = 0; //sputc(0,sec_buf,sec_len);//subDescriptorCount

                }
                {// descriptor 1
                    descriptor_type = 0x02; //software descriptor
                    sec_buf[sec_len+23] = descriptor_type; //sputc(descriptor_type,sec_buf,sec_len);//descriptor_type
                    descriptor_len = 9;
                    sec_buf[sec_len+24] = descriptor_len; //sputc(descriptor_len,sec_buf,sec_len);//descriptor_len

                    specifier_type = 0x01; // IEEE OUI.
                    sec_buf[sec_len+25] = specifier_type; //sputc(specifier_type,sec_buf,sec_len);//specifier_type

                    specifier_data = g_group_info[i].oui;
                    sec_buf[sec_len+26] = (specifier_data&0x00ff0000)>>16; //sputc((specifier_data&0x00ff0000)>>16,sec_buf,sec_len);
                    sec_buf[sec_len+27] = (specifier_data&0x0000ff00)>>8; //sputc((specifier_data&0x0000ff00)>>8,sec_buf,sec_len);
                    sec_buf[sec_len+28] = specifier_data&0x000000ff; //sputc((specifier_data&0x000000ff),sec_buf,sec_len);

                    model = g_group_info[i].sw_model;
                    sec_buf[sec_len+29] = (model&0xff00)>>8; //sputc((model&0xff00)>>8,sec_buf,sec_len);
                    sec_buf[sec_len+30] = model&0x00ff; //sputc((model&0x00ff),sec_buf,sec_len);

                    version = g_group_info[i].sw_ver;
                    sec_buf[sec_len+31] = (version&0xff00)>>8; //sputc((version&0xff00)>>8,sec_buf,sec_len);
                    sec_buf[sec_len+32] = version&0x00ff; //sputc((version&0x00ff),sec_buf,sec_len);

                    sec_buf[sec_len+33] = 0; //sputc(0,sec_buf,sec_len);//subDescriptorCount

                }
                /*~GroupCompatibility*/

                group_info_len = 0;
                sec_buf[sec_len+34] = (group_info_len&0xff00)>>8; //sputc((group_info_len&0xff00)>>8,sec_buf,sec_len);
                sec_buf[sec_len+35] = group_info_len&0x00ff; //sputc((group_info_len&0x00ff),sec_buf,sec_len);
                /*
                {
                    descriptor_tag = 0x01; //Type Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);

                    descriptor_tag = 0x02; //Name Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);

                    descriptor_tag = 0x03; //Info Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);

                    descriptor_tag = 0x06; //Location Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);
                }
                */
#ifdef GROUPINFOINDICATION_102_006  //inside
                privateDateLength = 0;
                sec_buf[sec_len+36] = (privateDateLength&0xff00)>>8; //sputc((privateDateLength&0xff00)>>8,sec_buf,sec_len);
                sec_buf[sec_len+37] = privateDateLength&0x00ff; //sputc((privateDateLength&0x00ff),sec_buf,sec_len);
                //for(i=0;i<privateDateLength;i++)
                //	sputc(0xff,sec_buf,sec_len);
                sec_len += 38;
#else
                sec_len += 36;
#endif
            }//for(i=0;i<num_of_groups;i++)
#ifdef GROUPINFOINDICATION_301_192	//outside
            privateDateLength = 0;
            sec_buf[sec_len+0] = (privateDateLength&0xff00)>>8; //sputc((privateDateLength&0xff00)>>8,sec_buf,sec_len);
            sec_buf[sec_len+1] = privateDateLength&0x00ff; //sputc((privateDateLength&0x00ff),sec_buf,sec_len);
            //for(i=0;i<privateDateLength;i++)
            //	sputc(0xff,sec_buf,sec_len);
            sec_len += 2;
#endif
        sec_buf[42] = ((sec_len-44)&0xff00)>>8;
        sec_buf[43] = (sec_len-44)&0x00ff;
        /*~dsi private data*/
    /*~UN_Message*/
    sec_buf[18] = ((sec_len-20)&0xff00)>>8;
    sec_buf[19] = (sec_len-20)&0x00ff;

    dsmcc_section_length = sec_len+4-3;
    sec_buf[1] = 0xB0+((dsmcc_section_length&0x0f00)>>8); //sputc(0xB0+((dsmcc_section_length&0x0f00)>>8),sec_buf,sec_len);
    sec_buf[2] = dsmcc_section_length&0x00ff; //sputc((dsmcc_section_length&0x00ff),sec_buf,sec_len);

    MG_FCS_Coder(sec_buf,sec_len);
    sec_len += 4;
    //sputc(0xff,sec_buf,sec_len); //checksum byte1
    //sputc(0xff,sec_buf,sec_len); //checksum byte2
    //sputc(0xff,sec_buf,sec_len); //checksum byte3
    //sputc(0xff,sec_buf,sec_len); //checksum byte4

    *section_len = sec_len;
}

void put_dii_section(struct group_info_t *group_info, BYTE* section_buf,	WORD* section_len)
{
    int i;
    DWORD transactionId,downloadId,tCDownloadWindow,tCDownloadScenario,module_size;
    WORD dsmcc_section_length,table_id_extension;
    BYTE adaptationLength;
    WORD messageId,privateDateLength,blockSize,messageLength;


    WORD num_of_modules;
    BYTE module_info_len;
    WORD module_id;
    BYTE module_version;
    BYTE descriptor_tag;
    BYTE position;
    WORD next_module_id;

    BYTE descriptor_len;


    BYTE* sec_buf = section_buf;
    WORD sec_len = 0;


    sputc(0x3B,sec_buf,sec_len);//table_id

    if(group_info->module_num >1 )
        dsmcc_section_length = 5+12+(18+2+group_info->module_num*13+2)+4;  //varialbe according to the following size---------------------
    else
        dsmcc_section_length = 5+12+(18+2+group_info->module_num*8+2)+4;  //varialbe according to the following size---------------------

    sputc(0xB0+((dsmcc_section_length&0x0f00)>>8),sec_buf,sec_len);
    sputc((dsmcc_section_length&0x00ff),sec_buf,sec_len);

    transactionId = group_info->group_id;
    table_id_extension = transactionId&0x0000ffff;
    sputc((table_id_extension&0xff00)>>8,sec_buf,sec_len);
    sputc((table_id_extension&0x00ff),sec_buf,sec_len);

    sputc(0xc1,sec_buf,sec_len);//reserved(11) + version_number(00000)+ current_next_ind(1)

    sputc(0,sec_buf,sec_len);//section_number

    sputc(0,sec_buf,sec_len);//last_section_number

    /*UN_Message*/
        /* dsmccMessageHeader */
        sputc(0x11,sec_buf,sec_len);//protocolDiscriminator

        sputc(0x03,sec_buf,sec_len);//dsmccType

        messageId = 0x1002;
        sputc((messageId&0xff00)>>8,sec_buf,sec_len);//messageId
        sputc((messageId&0x00ff),sec_buf,sec_len);

        sputc((transactionId&0xff000000)>>24,sec_buf,sec_len);
        sputc((transactionId&0x00ff0000)>>16,sec_buf,sec_len);
        sputc((transactionId&0x0000ff00)>>8,sec_buf,sec_len);
        sputc((transactionId&0x000000ff),sec_buf,sec_len);

        sputc(0xff,sec_buf,sec_len);//reserved

        adaptationLength = 0;
        sputc(adaptationLength,sec_buf,sec_len);//adaptationLength


        messageLength = 18+2+group_info->module_num*13+2;//varialbe according to the following size ----------------------
        sputc((messageLength&0xff00)>>8,sec_buf,sec_len);
        sputc((messageLength&0x00ff),sec_buf,sec_len);

        for(i=0;i<adaptationLength;i++)
            sputc(0xff,sec_buf,sec_len);
        /* ~dsmccMessageHeader */

        downloadId = DOWNLOAD_ID;
        sputc((downloadId&0xff000000)>>24,sec_buf,sec_len);
        sputc((downloadId&0x00ff0000)>>16,sec_buf,sec_len);
        sputc((downloadId&0x0000ff00)>>8,sec_buf,sec_len);
        sputc((downloadId&0x000000ff),sec_buf,sec_len);

        blockSize = BLOCK_SIZE;
        sputc((blockSize&0xff00)>>8,sec_buf,sec_len);
        sputc((blockSize&0x00ff),sec_buf,sec_len);//blockSize

        sputc(0,sec_buf,sec_len);//windowSize
        sputc(0,sec_buf,sec_len);//ackPeriod

        tCDownloadWindow = 0;
        sputc((tCDownloadWindow&0xff000000)>>24,sec_buf,sec_len);
        sputc((tCDownloadWindow&0x00ff0000)>>16,sec_buf,sec_len);
        sputc((tCDownloadWindow&0x0000ff00)>>8,sec_buf,sec_len);
        sputc((tCDownloadWindow&0x000000ff),sec_buf,sec_len);

        tCDownloadScenario = 0;
        sputc((tCDownloadScenario&0xff000000)>>24,sec_buf,sec_len);
        sputc((tCDownloadScenario&0x00ff0000)>>16,sec_buf,sec_len);
        sputc((tCDownloadScenario&0x0000ff00)>>8,sec_buf,sec_len);
        sputc((tCDownloadScenario&0x000000ff),sec_buf,sec_len);

        sputc(0,sec_buf,sec_len);//compatibilityDescriptor() (0x0000)
        sputc(0,sec_buf,sec_len);

        /*dii private data*/
            num_of_modules = group_info->module_num;
            sputc((num_of_modules&0xff00)>>8,sec_buf,sec_len);
            sputc((num_of_modules&0x00ff),sec_buf,sec_len);

            for(i=0;i<num_of_modules;i++)
            {// module 0
                module_id = group_info->module_info[i].module_id;
                sputc((module_id&0xff00)>>8,sec_buf,sec_len);
                sputc((module_id&0x00ff),sec_buf,sec_len);

                module_size = group_info->module_info[i].module_size;//test
                /* put modulesize  --4 bytes*/
                sputc((module_size&0xff000000)>>24,sec_buf,sec_len);
                sputc((module_size&0x00ff0000)>>16,sec_buf,sec_len);
                sputc((module_size&0x0000ff00)>>8,sec_buf,sec_len);
                sputc((module_size&0x000000ff),sec_buf,sec_len);

                module_version = MODULE_VERSION;
                sputc(module_version,sec_buf,sec_len);

                if(num_of_modules>1)
                    module_info_len = 5;//varialbe according to the following size ----------------------
                else
                    module_info_len = 0;
                sputc(module_info_len,sec_buf,sec_len);


                {
                    /*
                    descriptor_tag = 0x01; //Type Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);

                    descriptor_tag = 0x02; //Name Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);

                    descriptor_tag = 0x03; //Info Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);

                    descriptor_tag = 0x06; //Location Desc.
                    sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                    sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                    for(k=0;k=descriptor_len;k++)
                        sputc(0xff,sec_buf,sec_len);
                    */

                    if(num_of_modules>1)
                    {
                        descriptor_tag = 0x04; //module_link Desc.
                        sputc(descriptor_tag,sec_buf,sec_len);//descriptor_tag
                        descriptor_len = 3;
                        sputc(descriptor_len,sec_buf,sec_len);//descriptor_len
                        if(i==0)
                            position = 0;
                        else if(i==num_of_modules-1)
                            position = 2;
                        else
                            position = 1;
                        sputc(position,sec_buf,sec_len);
                        next_module_id = group_info->module_info[i+1].module_id;
                        sputc((next_module_id&0xff00)>>8,sec_buf,sec_len);
                        sputc((next_module_id&0x00ff),sec_buf,sec_len);
                    }
                }

            }

            privateDateLength = 0;
            sputc((privateDateLength&0xff00)>>8,sec_buf,sec_len);
            sputc((privateDateLength&0x00ff),sec_buf,sec_len);
            for(i=0;i<privateDateLength;i++)
                sputc(0xff,sec_buf,sec_len);
        /*~dii private data*/
    /*~UN_Message*/
    MG_FCS_Coder(sec_buf-sec_len,sec_len);
    sec_len += 4;
    //sputc(0xff,sec_buf,sec_len); //checksum byte1
    //sputc(0xff,sec_buf,sec_len); //checksum byte2
    //sputc(0xff,sec_buf,sec_len); //checksum byte3
    //sputc(0xff,sec_buf,sec_len); //checksum byte4

    *section_len = sec_len;
}

void put_ddb_section(FILE *fin,WORD module_id,BYTE section_num,BYTE last_section_num,DWORD block_len,BYTE* section_buf,WORD* section_len)
{
    DWORD i;
    DWORD downloadId;
    WORD dsmcc_section_length,table_id_extension;
    BYTE adaptationLength;
    WORD messageId,messageLength;

    BYTE module_version;

    BYTE block_data;

    BYTE* sec_buf = section_buf;
    WORD sec_len = 0;

    sputc(0x3C,sec_buf,sec_len);//table_id

    dsmcc_section_length = block_len + 18 + 9; //varialbe according to the following size---------------------
    sputc(0xB0+((dsmcc_section_length&0x0f00)>>8),sec_buf,sec_len);
    sputc((dsmcc_section_length&0x00ff),sec_buf,sec_len);

    table_id_extension = module_id;
    sputc((table_id_extension&0xff00)>>8,sec_buf,sec_len);
    sputc((table_id_extension&0x00ff),sec_buf,sec_len);

    module_version = MODULE_VERSION;

#ifdef GMI_SPECIAL
    sputc(0xc0+((module_version&0x1f)<<1),sec_buf,sec_len);//reserved(11) + version_number(00000)+ current_next_ind(1)
#else
    sputc(0xc1+((module_version&0x1f)<<1),sec_buf,sec_len);//reserved(11) + version_number(00000)+ current_next_ind(1)
#endif
    sputc(section_num,sec_buf,sec_len);//section_number

    sputc(last_section_num,sec_buf,sec_len);//last_section_number

    /*UN_Message*/
        /* dsmccDownloadDataHeader */
        sputc(0x11,sec_buf,sec_len);//protocolDiscriminator

        sputc(0x03,sec_buf,sec_len);//dsmccType

        messageId = 0x1003;
        sputc((messageId&0xff00)>>8,sec_buf,sec_len);
        sputc((messageId&0x00ff),sec_buf,sec_len);

        downloadId = DOWNLOAD_ID;
        sputc((downloadId&0xff000000)>>24,sec_buf,sec_len);
        sputc((downloadId&0x00ff0000)>>16,sec_buf,sec_len);
        sputc((downloadId&0x0000ff00)>>8,sec_buf,sec_len);
        sputc((downloadId&0x000000ff),sec_buf,sec_len);

        sputc(0xff,sec_buf,sec_len);//reserved

        adaptationLength = 0;
        sputc(adaptationLength,sec_buf,sec_len);//adaptationLength

        messageLength = block_len + 6;//varialbe according to the following size ----------------------
        sputc((messageLength&0xff00)>>8,sec_buf,sec_len);
        sputc((messageLength&0x00ff),sec_buf,sec_len);

        for(i=0;i<adaptationLength;i++)
            sputc(0xff,sec_buf,sec_len);
        /* ~dsmccMessageHeader */

        sputc((module_id&0xff00)>>8,sec_buf,sec_len);
        sputc((module_id&0x00ff),sec_buf,sec_len);

        sputc(module_version,sec_buf,sec_len);
        sputc(0xff,sec_buf,sec_len);//reserved

        sputc((section_num&0xff00)>>8,sec_buf,sec_len);
        sputc((section_num&0x00ff),sec_buf,sec_len);

        /*ddb private data*/
            for(i=0;i<block_len;i++)
            {
                block_data = getc(fin);
                sputc(block_data,sec_buf,sec_len);
            }
        /*~dii private data*/
    /*~UN_Message*/
    MG_FCS_Coder(sec_buf-sec_len,sec_len);
    sec_len += 4;
    //sputc(0xff,sec_buf,sec_len); //checksum byte1
    //sputc(0xff,sec_buf,sec_len); //checksum byte2
    //sputc(0xff,sec_buf,sec_len); //checksum byte3
    //sputc(0xff,sec_buf,sec_len); //checksum byte4

    *section_len = sec_len;
}

static int gen_group_info(char *input_file, group_info_t *p_group_info)
{
    BYTE last_section_num;
    DWORD idx=0,module_count,group_length,remain_length;
    FILE *fin;

    if(NULL == input_file)
    {
        return ERROR_INVALID_PARAM;
    }
    if(NULL == p_group_info)
    {
        return ERROR_INVALID_PARAM;
    }

    p_group_info->oui = g_ota_user_param.oui;
    p_group_info->hw_model = g_ota_user_param.hw_model;
    p_group_info->hw_ver = g_ota_user_param.hw_ver;
    p_group_info->sw_model = g_ota_user_param.sw_model;
    p_group_info->sw_ver = g_ota_user_param.sw_ver;
    p_group_info->group_id = 0x80000000+((0+1)<<1);

    if((fin=fopen(input_file,"rb"))==NULL)
    {
        return ERROR_FILE_ACCESS;
    }
    fseek(fin,0,SEEK_END);
    if((p_group_info->group_len = ftell(fin)) > MAX_MODULE_IN_GROUP * MAX_SIZE_PER_MODULE)
    {
        return ERROR_FILE_SIZE;
    }
    fseek(fin,0,SEEK_SET);

    p_group_info->fin = fin;
    p_group_info->module_num = MAX_MODULE_IN_GROUP;
    group_length = p_group_info->group_len;
    module_count = (group_length%MAX_SIZE_PER_MODULE)?((group_length/MAX_SIZE_PER_MODULE)+1):group_length/MAX_SIZE_PER_MODULE;

    /*support muti module*/
    for(idx=0;idx<module_count-1;idx++)
    {
        p_group_info->module_info[idx].module_id = (((g_current_group+1)&0xff)<<8)|(idx&0xff);
        p_group_info->module_info[idx].module_size = MAX_SIZE_PER_MODULE;
        p_group_info->module_info[idx].last_section_num = 255;
    }

    //last module
    remain_length = group_length - (idx*MAX_SIZE_PER_MODULE);
    p_group_info->module_info[idx].module_id = (((g_current_group+1)&0xff)<<8)|(idx&0xff);
    p_group_info->module_info[idx].module_size = remain_length;
    last_section_num = (remain_length%BLOCK_SIZE)?(remain_length/BLOCK_SIZE):(remain_length/BLOCK_SIZE-1);
    p_group_info->module_info[idx].last_section_num = last_section_num;

    p_group_info->module_num = module_count;

    return ERROR_NONE;
}

int gen_otapkt_start(ota_user_param_t* p_ota_user_param, char* input_file_name, char* output_file_name)
{
    int ret = ERROR_NONE;
    DWORD i,j,k;
    char temp_file[TEMP_MAX_PATH] = {0};
    FILE *fout,*ftemp;
    int section_cnt;
    BYTE section_buf[4096];
    WORD section_len;
    WORD section_num;
    WORD block_len;

    if(NULL == p_ota_user_param)
    {
        return ERROR_INVALID_PARAM;
    }
    if(NULL == input_file_name)
    {
        return ERROR_INVALID_PARAM;
    }
    if(NULL == output_file_name)
    {
        return ERROR_INVALID_PARAM;
    }

    memcpy(&g_ota_user_param, p_ota_user_param, sizeof(ota_user_param_t));

    g_pid = (WORD)g_ota_user_param.pid;
    g_ts_id = (WORD)g_ota_user_param.ts_id;
    g_prog_num = (WORD)g_ota_user_param.prog_num;
    g_pmt_pid = (WORD)g_ota_user_param.pmt_pid;
    strcpy(g_SrvName, g_ota_user_param.service_name);
    strcpy(g_SrvPrvdr, g_ota_user_param.service_provider);

    for(i=0;i<g_group_num;i++)
    {
        /*
         * only support 1 group currently
         * if need to support multi group, the input file should modify for each group
         */
        ret = gen_group_info(input_file_name, &g_group_info[i]);
        if(ERROR_NONE != ret)
        {
            return ret;
        }
        g_current_group++;
    }

    put_dsi_section(g_dsi_section_buf,&g_dsi_section_len);

    for(i=0;i<g_group_num;i++)
    {
        put_dii_section(&(g_group_info[i]),g_dii_section_buf[i],&(g_dii_section_len[i]));
    }

    if((fout=fopen(output_file_name,"wb"))==NULL)
    {
        return ERROR_FILE_ACCESS;
    }
    if(0 != g_ota_user_param.insert_table)
    {
        snprintf(temp_file, sizeof(temp_file), "gen_ts_temp.ts");
        if((ftemp=fopen(temp_file,"wb"))==NULL)
        {
            return ERROR_FILE_ACCESS;
        }
    }
    else
    {
        ftemp = fout;
    }
    section_cnt =0;

    gen_ts_stream(g_dsi_section_buf,g_dsi_section_len,ftemp);
    for(i=0;i<g_group_num;i++)
    {
        gen_ts_stream(g_dii_section_buf[i],g_dii_section_len[i],ftemp);
    }

    //将多个文件(group)生成TS流时,交叉在一起,有利于升级
    for(j=0;j<MAX_MODULE_IN_GROUP;j++)
    {
        for(section_num=0;section_num<256;section_num++)
        {
            for(i=0;i<g_group_num;i++)
            {
                if (g_group_info[i].module_num <= j)
                    continue;

                if(g_group_info[i].module_info[j].module_size < section_num*BLOCK_SIZE)
                    continue;
                else if(g_group_info[i].module_info[j].module_size-section_num*BLOCK_SIZE >BLOCK_SIZE)
                    block_len = BLOCK_SIZE;
                else
                    block_len = g_group_info[i].module_info[j].module_size-section_num*BLOCK_SIZE;

                put_ddb_section(g_group_info[i].fin,
                    g_group_info[i].module_info[j].module_id,
                    section_num,
                    g_group_info[i].module_info[j].last_section_num,
                    block_len,
                    section_buf,
                    &section_len);
                gen_ts_stream(section_buf,section_len,ftemp);
                section_cnt ++;
                if((section_cnt%10) == 0) //DDB interval == 10 , insert DSI/DII
                {
                    gen_ts_stream(g_dsi_section_buf,g_dsi_section_len,ftemp);
//				    gen_ts_stream(g_dii_section_buf[(section_cnt%10)/g_group_num],g_dii_section_len[(section_cnt%10)/g_group_num],ftemp);
                    for(k=0;k<g_group_num;k++)
                    {
                        gen_ts_stream(g_dii_section_buf[k],g_dii_section_len[k],ftemp);
                    }
                }
            }//i
        }//section_num
    }//j

    if(0 != g_ota_user_param.insert_table)
    {
        fclose(ftemp);
        if((ftemp=fopen(temp_file,"rb"))==NULL)
        {
            return ERROR_FILE_ACCESS;
        }
        table_insert(ftemp,fout,g_ts_id,g_prog_num,g_pmt_pid,g_pid);
        fclose(ftemp);
        remove(temp_file);
    }

    for(i=0;i<g_group_num;i++)
        fclose(g_group_info[i].fin);
    fclose(fout);

    return ERROR_NONE;

}


