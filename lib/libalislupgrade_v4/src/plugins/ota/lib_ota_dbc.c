#include "lib_ota_dbc.h"

#include <string.h>

/* for linux 128M solution ota allcode upgrade */
#define DC_MODULE_MAX  128

#define get_2byte_value(data, value) \
	do { \
		unsigned char byte1 = *data++; \
		unsigned char byte2 = *data++; \
		value =  (byte1 << 8) + byte2; \
	} while (0)

#define get_3byte_value(data, value) \
	do { \
		unsigned char byte1 = *data++; \
		unsigned char byte2 = *data++; \
		unsigned char byte3 = *data++; \
		value = (byte1 << 16) + (byte2 << 8) + byte3; \
	} while (0)

#define get_4byte_value(data, value) \
	do { \
		unsigned char byte1 = *data++; \
		unsigned char byte2 = *data++; \
		unsigned char byte3 = *data++; \
		unsigned char byte4 = *data++; \
		value = (byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4; \
	} while (0)

#define skip_4byte(data) \
	do { \
		data += 4; \
	} while(0)

int alisl_dsi_message(unsigned char* data,
                      unsigned short len,
                      unsigned long OUI,
                      unsigned short hw_model,
                      unsigned short hw_version,
                      unsigned short sw_model,
                      unsigned short sw_version,
                      unsigned short* new_sw_version,
                      struct DCGroupInfo* group)
{
	unsigned long i, j;
	unsigned long transaction_id;
	unsigned char adaptation_len;
	unsigned short message_len;
	unsigned short private_data_len;
	unsigned short num_of_groups;
	unsigned long group_id;
	unsigned long group_size;
	unsigned short compatibility_descriptor_len;
	unsigned short descriptor_count;
	unsigned char descriptor_type;
	unsigned char descriptor_len;
	unsigned char specifier_type;
	unsigned long specifier_data;
	unsigned short model;
	unsigned short version;
	unsigned short group_info_len;
	unsigned char descriptor_tag;

//	int hardware_fit = 0;
//	int software_fit = 0;

	DBC_PRINTF("DSI message!\n");

	//i =0;
	//printf("\n %d \n",len);
	//for(; i<len; i++)
	//{
	//	printf("%02x ",*(data+i));
	//}
	//printf("\n %d \n",len);



	/* dsmccMessageHeader */
	skip_4byte(data);

	get_4byte_value(data, transaction_id);
    (void)transaction_id;

	data++;
	adaptation_len = *data++;
	get_2byte_value(data, message_len);
    (void)message_len;

	DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d\n",
	           len, adaptation_len, message_len);

	data += adaptation_len;
	/*~ dsmccMessageHeader */

	/* serverid */
	data += 20;
	/* compatibilityDescriptor()  : 0x0000 */
	get_2byte_value(data, compatibility_descriptor_len);

	data += compatibility_descriptor_len;

	get_2byte_value(data, private_data_len);
	get_2byte_value(data, num_of_groups);

	for (i = 0; i < num_of_groups; i++)
	{
//		hardware_fit = 0;
//		software_fit = 0;

		get_4byte_value(data, group_id);
		get_4byte_value(data, group_size);

		DBC_PRINTF("group_id = 0x%x,group_size = %d\n",
		           group_id, group_size);
		/* GroupCompatibility */
		get_2byte_value(data, compatibility_descriptor_len);

		DBC_PRINTF("compatibility_descriptor_len = %d\n",
		           compatibility_descriptor_len);

		if (compatibility_descriptor_len < 2)
		{
			data += compatibility_descriptor_len;
		}
		else
		{
			compatibility_descriptor_len -= 2;
			get_2byte_value(data, descriptor_count);

			for (j = 0; j < descriptor_count; j++)
			{
				descriptor_type = *data++;
				descriptor_len = *data++;
			    DBC_PRINTF("descriptor_count%d,descriptor_type%d,descriptor_len%d\n",
		           descriptor_count, descriptor_type ,descriptor_len);

				if (compatibility_descriptor_len < descriptor_len + 2)
				{
					data--;
					data--;

					data += compatibility_descriptor_len;
					compatibility_descriptor_len = 0;
					break;
				}
				else
				{
					specifier_type = *data++;
					get_3byte_value(data, specifier_data);
					get_2byte_value(data, model);
					get_2byte_value(data, version);

					data += descriptor_len - 8;

					if (descriptor_type == 0x01
					    && specifier_type == 0x01
					    && specifier_data == OUI
					    && model == hw_model
					    && version == hw_version)
					{
						//hardware_fit = 1;
					}
				    DBC_PRINTF("model = 0x%x version = %d \n",model,version);

					if (descriptor_type == 0x02
					    && specifier_type == 0x01
					    && specifier_data == OUI
					    && model == sw_model
					    && version >= sw_version)
					{
						*new_sw_version = version;
						//software_fit = 1;
					}

					compatibility_descriptor_len -= (descriptor_len + 2);
				}
			}

			data += compatibility_descriptor_len;
		}

		group->group_id = group_id;
		group->group_size = group_size;
		/* ~GroupCompatibility */

		get_2byte_value(data, group_info_len);

		DBC_PRINTF("group_info_len = %d\n",
		           group_info_len);
		
		while (group_info_len > 0)
		{
			descriptor_tag = *data++;
			descriptor_len = *data++;

			if (group_info_len < descriptor_len + 2)
			{
				return DBC_FAIL;
			}

			switch (descriptor_tag)
			{
				case 0x01:/*type*/
				{
					DBC_PRINTF("--------type Desc. \n");
					break;
				}

				case 0x02:/*name*/
				{
					DBC_PRINTF("--------name Desc. \n");
					break;
				}

				case 0x03:/*info*/
				{
					DBC_PRINTF("--------info Desc. \n");
					break;
				}

				case 0x06:/*location*/
				{
					DBC_PRINTF("--------location Desc. \n");
					break;
				}

				case 0x07:/*est_download_time*/
				{
					DBC_PRINTF("--------est_download_time Desc. \n");
					break;
				}

				case 0x08:/*group_link*/
				{
					DBC_PRINTF("--------group_link Desc. \n");
					break;
				}

				case 0x0a:
				{
					DBC_PRINTF("--------0x0a Desc. \n");
					break;
				}

				case 0x04:
				case 0x05:
				case 0x09:
				default:
				{
					DBC_PRINTF("--------other Desc. \n");
					break;
				}
			}

			data += descriptor_len;
			group_info_len -= (descriptor_len + 2);
		}

		/* BEGIN GROUPINFOINDICATION_102_006, inside */
		get_2byte_value(data, private_data_len);
		data += private_data_len;
		/* END GROUPINFOINDICATION_102_006, inside */

//		DBC_PRINTF("hardware_fit = %d,software_fit = %d\n", hardware_fit,software_fit);

		/* We don't check the version right now */
		return DBC_SUCCESS;

		//if (hardware_fit == 1 && software_fit == 1)
		//{
		//	DBC_PRINTF("Group 0x%x Fit!\n", group->group_id);
		//	return DBC_SUCCESS;
		//}

	}

	return DBC_FAIL;

	/* If any usage of following	*/
#if 0
	/* GROUPINFOINDICATION_301_192, outside */
	byte1 = *data++;
	byte2 = *data++;
	private_data_len = (byte1 << 8) + byte2;

	data += private_data_len;
#endif
}


int alisl_dii_message(unsigned char* data,
                      unsigned short len,
                      unsigned long group_id,
                      unsigned char *data_addr,
                      struct DCModuleInfo *module,
                      unsigned char *module_num,
                      unsigned short *blocksize)
{
	unsigned long i, index;
	unsigned long transaction_id;
	unsigned char adaptation_len;
	unsigned short message_len;
	unsigned long download_id;
	unsigned short compatibility_descriptor_len;
	unsigned short num_of_modules;
	unsigned short module_id;
	unsigned long module_size;
//	unsigned char module_version;
	unsigned char module_info_len;
	unsigned char descriptor_tag;
	unsigned char descriptor_len;
	unsigned char position;
	unsigned short next_module_id;
	unsigned short private_data_len;

	DBC_PRINTF("DII message!\n");
	/* dsmccMessageHeader */
	skip_4byte(data);

	get_4byte_value(data, transaction_id);

	if (transaction_id != group_id)
	{
		DBC_PRINTF("DII transaction_id = %8x\n", transaction_id);
		return DBC_FAIL;
	}

	data++;
	adaptation_len = *data++;
	get_2byte_value(data, message_len);
    (void)message_len;
	DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d\n",
	           len, adaptation_len, message_len);

	data += adaptation_len;
	/*~ dsmccMessageHeader */

	get_4byte_value(data, download_id);
    (void)download_id;
    
	get_2byte_value(data, *blocksize);

	data++;/*windowsize*/
	skip_4byte(data);/*ackperiod*/

	skip_4byte(data);/*tCDownloadWindow*/
	data++;/*tCDownloadScenario*/

	/* compatibilityDescriptor()  : 0x0000 */
	get_2byte_value(data, compatibility_descriptor_len);
	data += compatibility_descriptor_len;

	get_2byte_value(data, num_of_modules);
	*module_num  = num_of_modules;

	DBC_PRINTF("[%s(%d)]module_num=%d\n",
	           __FUNCTION__, __LINE__, *module_num);

	if (num_of_modules > DC_MODULE_MAX)
	{
		DBC_PRINTF("num_of_modules=%d > DC_MODULE_MAX=%d error!\n",
		           num_of_modules, DC_MODULE_MAX);
		return DBC_FAIL;
	}

	for (index = 0; index < num_of_modules; index++)
	{
		get_2byte_value(data, module_id);
		get_4byte_value(data, module_size);
		//module_version = *data++;

		if (index == 0)
		{
			module[index].module_buf_pointer = data_addr;
		}
		else
		{
			module[index].module_buf_pointer = module[index - 1].module_buf_pointer
			                                   + module[index - 1].module_buf_size;
		}

		module[index].module_id = module_id;
		module[index].module_buf_size = module_size;

		if (module_size % (*blocksize))
			module[index].block_num = module_size / (*blocksize) + 1;
		else
			module[index].block_num = module_size / (*blocksize);

		module[index].module_linked = 0;
		module[index].module_first = 0;
		module[index].module_last = 0;
		module[index].next_block_num = 0;

		for (i = 0; i < BIT_MAP_NUM; i++)
			module[index].block_received[i] = 0xffffffff;

		for (i = 0; i < module[index].block_num; i++)
			module[index].block_received[i / 32] &= ~(1 << (i % 32));

		module[index].module_download_finish = 0;

		module_info_len = *data++;

		while (module_info_len > 0)
		{
			descriptor_tag = *data++;
			descriptor_len = *data++;

			if (module_info_len < descriptor_len + 2)
			{
				return DBC_FAIL;
			}

			switch (descriptor_tag)
			{
				case 0x04:/*module_link*/
				{
					DBC_PRINTF("--------module_link Desc. \n");

					if (descriptor_len < 3)
						return DBC_FAIL;

					position = *data++;
					get_2byte_value(data, next_module_id);
					module[index].module_linked = 1;

					if (position == 0x00)
					{
						module[index].module_first = 1;
						module[index].next_module_id = next_module_id;
					}
					else if (position == 0x01)
					{
						module[index].next_module_id = next_module_id;
					}
					else if (position == 0x02)
					{
						module[index].module_last = 1;
					}

					data += descriptor_len - 3;
					break;
				}

				case 0x01:/*type*/
				{
					DBC_PRINTF("--------type Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x02:/*name*/
				{
					DBC_PRINTF("--------name Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x03:/*info*/
				{
					DBC_PRINTF("--------info Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x05:/*CRC32*/
				{
					DBC_PRINTF("--------CRC32 Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x06:/*location*/
				{
					DBC_PRINTF("--------location Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x07:/*est_download_time*/
				{
					DBC_PRINTF("--------est_download_time Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x09:/*compressed_module*/
				{
					DBC_PRINTF("--------compressed_module Desc. \n");
					data += descriptor_len;
					break;
				}

				case 0x08:
				case 0x0a:
				default:
				{
					DBC_PRINTF("--------Other Desc. \n");
					data += descriptor_len;
					break;
				}
			}

			module_info_len -= (descriptor_len + 2);
		}
	}

	get_2byte_value(data, private_data_len);
	data += private_data_len;

	return DBC_SUCCESS;
}

int alisl_ddb_data(unsigned char* data,
                   unsigned short len,
                   struct DCModuleInfo *module,
                   unsigned short blocksize,
                   unsigned char blk_id)
{
	unsigned long i;
	unsigned long download_id;
	unsigned char adaptation_len;
	unsigned short message_len;
	unsigned short module_id;
//	unsigned char module_version;
	unsigned long module_size;
	unsigned short block_num;
	unsigned short block_len;
	unsigned char* module_pointer;
	unsigned char* block_pointer;

	DBC_PRINTF("DDB data!\n");
	/* dsmccDownloadDataHeader */
	skip_4byte(data);

	get_4byte_value(data, download_id);
    (void)download_id;
	data++;
	adaptation_len = *data++;
	get_2byte_value(data, message_len);

	/* DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d\n",
	           len, adaptation_len, message_len); */

	data += adaptation_len;
	/* ~dsmccDownloadDataHeader */

	get_2byte_value(data, module_id);
//	module_version = *data++;
	data++;/*reserved*/

	if (module->module_id != module_id)
	{
		return DBC_FAIL;
	}

	module_size = module->module_buf_size;
	module_pointer = module->module_buf_pointer;

	get_2byte_value(data, block_num);
	/* DBC_PRINTF("block_num = %d \n", block_num); */

	if ((unsigned char)block_num != blk_id)
	{
		DBC_PRINTF("ERROR : (unsigned char)block_num != blk_id!\n");
		return DBC_FAIL;
	}

	if (block_num > module->block_num - 1)
	{
		DBC_PRINTF("ERROR : block_num > module->block_num-1!\n");
		return DBC_FAIL;
	}

	if ((unsigned long)(block_num * blocksize) >= module_size)
	{
		return DBC_FAIL;
	}
	else if (module->block_received[block_num / 32] & (1 << (block_num % 32)))
	{
		DBC_PRINTF("block exist\n");
		return DBC_FAIL;
	}
	else if ((module_size - block_num * blocksize) > blocksize)
	{
		block_len = blocksize;
	}
	else
	{
		block_len = module_size - block_num * blocksize;
	}

	block_pointer = module_pointer + block_num * blocksize;

	if (message_len - 6 < block_len)
	{
		return DBC_FAIL;
	}

	memcpy(block_pointer, data, block_len);

	module->block_received[block_num / 32] |= (1 << (block_num % 32));
	
	//printf("block_num = %d, module->block_received[block_num / 32] = 0x%x \n", block_num, module->block_received[block_num / 32]);
	/* check all module download info */
	module->module_download_finish = 1;

	for (i = 0; i < BIT_MAP_NUM; i++)
	{
		if (module->block_received[i] != 0xffffffff)
		{
			module->module_download_finish = 0;
		}
	}

	return DBC_SUCCESS;
}

