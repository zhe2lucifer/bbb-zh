/**@file
 *  (c) Copyright 2014-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldsckit.c
 *  @brief              Simple RAM to RAM AES decryption with key in RAM.
 *
 *  @version            1.2
 *  @date               15.4.2014
 *
 *  @author             Vincent Pilloux <vincent.pilloux@alitech.com>
 */

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>

#include <alisldsc.h>
#include <sys/ioctl.h>
#include <ali_dsc_common.h>

#define R2R_PID 0x1234


int main(int argc, char *argv[])
{
	alisl_retcode ret = 0;
	uint32_t dev_id, i;
	KEY_PARAM key;
	AES_IV_INFO iv;
	uint32_t stream_id;
	AES_KEY_PARAM aes_in_key;
	struct aes_init_param aes_param;
	unsigned char plain_key[16] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d };
	unsigned char *enc_16_cbc = "\xe9\x84\x7e\xfe\x26\x5e\x2e\x5c\xf9\xf9\x81\x41\x7c\x26\x75\x1a";
	unsigned char *dec_source = "\xc2\x86\x69\x6d\x88\x7c\x9a\xa0"\
				    "\x61\x1b\xbb\x3e\x20\x25\xa4\x5a";
	unsigned char result[16];
	unsigned short r2r_pid=R2R_PID;
	alisl_handle dsc_handle;
	void *aes_handle;


	memset(result, 0, 16);
	printf("alisldsc_dsc_open...\n");
	ret = alisldsc_dsc_open(&dsc_handle);
	if (ret != 0)
	{
		printf("open DSC fail.\n");
		return ret;
	}

	/* force freeing of busy dev IDs (Kernel issue to be fixed!):
	for (i=0;i<8; i++)
		alisldsc_dsc_set_subdev_idle(dsc_handle, AES, i);*/

	/* init AES device */
	ret = alisldsc_dsc_get_free_subdev(dsc_handle, AES, &dev_id);
	if (ret != 0)
	{
		printf("get AES free device id fail.\n");
		alisldsc_dsc_close(dsc_handle);
		return ret;
	}

	printf("alisldsc_dsc_get_free_stream_id (dev_id=%d)\n", dev_id);
	ret = alisldsc_dsc_get_free_stream_id(dsc_handle, PURE_DATA_MODE, &stream_id);
	if (ret != 0)
	{
		printf("alisldsc_dsc_get_free_stream_id failed\n");
		alisldsc_dsc_set_subdev_idle(dsc_handle, AES, dev_id);
		alisldsc_dsc_close(dsc_handle);
		return ret;
	}
	printf("stream ID: %d, dev_id: %d\n", stream_id, dev_id);

	printf("alisldsc_algo_open(AES)...\n");
	ret = alisldsc_algo_open(&aes_handle, AES, dev_id);
	if (ret != 0)
	{
		printf("alisldsc_algo_open AES fail. (%d)\n", ret);
		alisldsc_dsc_set_stream_id_idle(dsc_handle, stream_id);
		alisldsc_dsc_set_subdev_idle(dsc_handle, AES, dev_id);
		alisldsc_dsc_close(dsc_handle);
		return ret;
	}

	memset(&aes_param, 0, sizeof(struct aes_init_param));
	aes_param.dma_mode = PURE_DATA_MODE;
	aes_param.key_from = KEY_FROM_SRAM;
	aes_param.key_mode = AES_128BITS_MODE;
	aes_param.parity_mode = EVEN_PARITY_MODE;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE;
	aes_param.scramble_control = 0;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_CBC;
	aes_param.cbc_cts_enable = 0;

	memset(&key, 0, sizeof(KEY_PARAM));
	memset(&iv, 0, sizeof(iv));
	//key.handle = -1;
	key.ctr_counter = NULL;
	key.init_vector = (unsigned char *)&iv;
	key.key_length = 16 * 8;
	key.pid_len = 1;
	key.pid_list = &r2r_pid;
	key.p_aes_iv_info = &iv;
	key.stream_id = stream_id;
	key.p_aes_key_info = (AES_KEY_PARAM *)plain_key;
	key.force_mode = 1;

	ret = alisldsc_algo_create_crypt_stream(aes_handle, &aes_param, &key);
	if (ret)
	{
		printf("alisldsc_algo_create_crypt_stream failed!\n");
		goto end;
	}

	if (key.handle==0xFFFFFFFF)
	{
		printf("WARNING: key.handle==0xFFFFFFFF after alisldsc_algo_create_crypt_stream!\n");
		ret=alisldsc_algo_update_cw(aes_handle, &key);
		if (ret)
			printf("alisldsc_algo_update_cw failed!\n");
		goto end;
	}

	ret = alisldsc_algo_decrypt(aes_handle, stream_id,
				    enc_16_cbc, result, 16);
	if (ret)
	{
		printf("alisldsc_algo_decrypt AES fail. (%d)\n", ret);
	}
	if (0 != memcmp(result, dec_source, 16))
		printf("SL DSC: AES decrypt error\n");
	else
		printf("\n---> SL DSC test OK! <---\n\n");

end:
	alisldsc_algo_delete_crypt_stream(aes_handle, key.handle);
	alisldsc_dsc_set_stream_id_idle(dsc_handle, stream_id);
	alisldsc_dsc_set_subdev_idle(dsc_handle, AES, dev_id);
	alisldsc_dsc_close(dsc_handle);
	alisldsc_algo_close(aes_handle);

	return ret;
}

