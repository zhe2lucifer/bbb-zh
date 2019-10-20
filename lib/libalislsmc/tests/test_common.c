#ifndef __SMC_TEST_COMMON__
#define __SMC_TEST_COMMON__

#include <sltest_common.h>

enum card_provider {
	NONE = 0,
	IRDETO,
	CONAX,
	VIACCESS,
	SECA,
	NAGRA,
	TONGDA,
	TONGFANG,
	CRYPTWORKS,
	JETCAS,
	CTI
};

static void *smc_dev = NULL;

struct smc_device_cfg smc_config;
unsigned int init_clk = 3600000;

static enum card_provider card_provider = NONE;

/*
 * Different transmission protocol, such as T=0, T=1, T=14 as well as raw data
 * transmission, has different command header.
 * Below command is a example of those command headers.
 * When we use alislsmc_iso_transfer() to transmit data through those different
 * protocols, such command headers are mandantory to be put at the head of
 * input parameter "void *command" of this function.
 */
static unsigned char raw_cmd[5] = {0x10, 0x11, 0x12, 0x13, 0x14};
static unsigned char t0_cmd[5]  = {0x30, 0x31, 0x32, 0x33, 0x34};
static unsigned char t14_cmd[5] = {0x20, 0x21, 0x22, 0x23, 0x24};
static unsigned char t1_cmd[8]  = {0xA0, 0xCA, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x06};

static unsigned char viaccess_cmd[6][5] = {
	{0xca, 0xac, 0xa4, 0x00, 0x00}, /* request unique id */
	{0xca, 0xb8, 0x00, 0x00, 0x07}, /* read unique id, read size is 0x07 */
	{0xca, 0xa4, 0x00, 0x00, 0x00}, /* select issuer 0 */
	{0xca, 0xc0, 0x00, 0x00, 0x1a}, /* show provider properties, read size is 0x1a */
	{0xca, 0xac, 0xa5, 0x00, 0x00}, /* request sa */
	{0xca, 0xb8, 0x00, 0x00, 0x07}, /* read sa, read size is 0x07 */
};

static unsigned char tongfang_cmd[3][10] = {
	{0x00, 0xA4, 0x04, 0x00, 0x05, 0xF9, 0x5A, 0x54, 0x00, 0x06},
	{0x80, 0x46, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x04},
	{0x00, 0xC0, 0x00, 0x00, 0x04}
};

static unsigned char tongda_cmd[5][7] = {
	{0x00, 0x84, 0x00, 0x00, 0x04},             /* get challenge, random */
	{0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00}, /* select MF */
	{0x00, 0xC0, 0x00, 0x00, 0x00},             /* get responce */
	{0x00, 0xA4, 0x00, 0x00, 0x02, 0x2F, 0x11}, /* select file 2F11 */
	{0x00, 0xC0, 0x00, 0x00, 0x00}              /* get responce */
};
static unsigned char conax_cmd[2][8] = {
	/* 0x26, 0x10, 0x01, 0x01 (00 00 2 9011; 00 40 2 9011) */
	{0xdd, 0x26, 0x00, 0x00, 0x03, 0x10, 0x01, 0x01},

	/* (00 00 2 9011; 00 40 2 9011) */
	{0xdd, 0xca, 0x00, 0x00, 0x00}
};

static int detect_card_provider(const unsigned char *atr)
{
	switch (atr[1]) {
		case 0x9F:
			card_provider = IRDETO;
			break;
		case 0x24:
		case 0x34:
			card_provider = CONAX;
			printf("card type: CONAX card\n");
			break;
		case 0x77:
		case 0x3F:
			card_provider = VIACCESS;
			break;
		case 0xF7:
			card_provider = SECA;
			break;
		case 0xFF:
			printf("card type: NAGRA card\n");
			card_provider = NAGRA;
			break;
		case 0x6D:
			card_provider = TONGDA;
			break;
		case 0x6C:
			card_provider = TONGFANG;
			break;
		case 0x78:
			card_provider = CRYPTWORKS;
			break;
		case 0x7F:
			card_provider = JETCAS;
			break;
		case 0xE9:
			card_provider = CTI;
			break;

		default:
			printf("\nUnknown CA system card!\n");
			card_provider = NONE;

			return -1;
	}

	return 0;
}

static void callback(void *p_user_data, uint32_t param)
{
	switch (param) {
		case 0:
			printf("\nMessage: No card has been detected, or card has been removed!\n");
			break;
		case 1:
			printf("\nMessage: Detected one card, or card has been inserted!\n");
			break;
		default:
			printf("\nMessage: Unknown message received!\n");
			break;
	}
}

static int print_response_data(const unsigned char *response, size_t actlen)
{
	int i;

	printf("Response Data:\n");
	for (i = 0; i < actlen; i++) {
		printf(" 0x%x", response[i]);
	}
	printf("\n");
}

static int config_has_got = 0;
static int smc_dev_id = 0;

static int get_smc_dev_id()
{
	sltest_config slconfig;

	if (!config_has_got) {
		sltest_get_config(&slconfig, 1, "dvbc1");
		smc_dev_id = slconfig.smc_dev_id;
	}

	memset(&smc_config, 0, sizeof(smc_config));
	smc_config.init_clk_trigger = 1;
	smc_config.init_clk_number = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;
	smc_config.def_etu_trigger = 1;
	smc_config.default_etu = 372;
	smc_config.warm_reset_trigger = 1;
	//smc_config.force_tx_rx_cmd = 0xdd;
	//smc_config.force_tx_rx_cmd_len = 5;
	smc_config.init_clk_array = &init_clk;
	smc_config.invert_detect = 1;

	return smc_dev_id;
}

#endif
