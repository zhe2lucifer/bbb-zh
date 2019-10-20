#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <ali_ce_common.h>
#include <ali_dsc_common.h>
#include <ali_dmx_common.h>

int open_as_device(void);
int close_as_device(void);
int load_csk_key( void );
int load_cwpk_to_ce (uint8_t * enc_cwpk );
int get_free_key_pos(uint32_t * free_pos);
int feed_cw_to_ce(uint8_t * enc_cw, uint16_t pid, uint8_t cw_type,  enum CE_CRYPT_TARGET sec_key_pos);
int create_stream_to_ce(uint16_t pid, uint32_t key_pos, uint32_t * handler, uint16_t stream_id);
int delete_stream_to_ce(uint32_t handler, uint16_t stream_id, enum CE_CRYPT_TARGET key_pos);
int setup_dscrambler_param(int start_dsc);
uint16_t get_free_stream_id(enum DMA_MODE mode);
