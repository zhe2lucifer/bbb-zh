/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alismckit.c
 *  @brief
 *
 *  @version            1.0
 *  @date               06/08/2013 02:33:27 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */
#include "test_common.c"

TEST_GROUP(SmcT0Transfer);
TEST_SETUP(SmcT0Transfer)
{
	unsigned char atr[256];
	unsigned short size = 0;

	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_set_cfg(smc_dev, &smc_config));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_register_callback(smc_dev, NULL, callback));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_reset(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_get_atr(smc_dev, atr, &size));
	TEST_ASSERT_EQUAL_INT(0, detect_card_provider(atr));
}

TEST_TEAR_DOWN(SmcT0Transfer)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcT0Transfer, T0TransferWithValidParam)
{
	unsigned char response[256];
	size_t actlen;

	switch(card_provider) {
	case IRDETO:
		break;
	case CONAX:
		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				conax_cmd[1], 5, /* command and write length */
				response, 2,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				conax_cmd[0], 8, /* command and write length */
				response, 2,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		break;
	case VIACCESS:
		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				viaccess_cmd[0], 5, /* command and write length */
				response, 2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				viaccess_cmd[1], 5, /* command and write length */
				response, viaccess_cmd[1][4]+2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				viaccess_cmd[2], 5, /* command and write length */
				response, 2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				viaccess_cmd[3], 5, /* command and write length */
				response, viaccess_cmd[3][4]+2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				viaccess_cmd[4], 5, /* command and write length */
				response, 2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				viaccess_cmd[5], 5, /* command and write length */
				response, viaccess_cmd[5][4]+2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		break;
	case SECA:
		break;
	case NAGRA:
		break;
	case TONGDA:
		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongda_cmd[0], 5, /* command and write length */
				response, 256,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongda_cmd[1], 7, /* command and write length */
				response, 2,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongda_cmd[2], 5, /* command and write length */
				response, tongda_cmd[2][4]+2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongda_cmd[3], 7, /* command and write length */
				response, 2,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongda_cmd[4], 7, /* command and write length */
				response, tongda_cmd[4][4]+2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		break;

	case TONGFANG:
		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongfang_cmd[0], 10, /* command and write length */
				response, 2,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongfang_cmd[1], 9, /* command and write length */
				response, 2,     /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		memset(response, 0, sizeof(response));
		TEST_ASSERT_EQUAL_INT(0,
			alislsmc_iso_transfer(smc_dev,
				tongfang_cmd[2], 5, /* command and write length */
				response, tongfang_cmd[2][4]+2, /* response and read length */
				&actlen));

		print_response_data(response, actlen);

		break;
	case CRYPTWORKS:
		break;
	case JETCAS:
		break;
	case CTI:
		break;
	}

	TEST_ASSERT_EQUAL_INT(0, alislsmc_deactive(smc_dev));
}

TEST_GROUP_RUNNER(SmcT0Transfer)
{
	RUN_TEST_CASE(SmcT0Transfer, T0TransferWithValidParam);
}

static void run_smc_t0_transfer()
{
    RUN_TEST_GROUP(SmcT0Transfer);
}

static int run_group_smc_t0_transfer(int argc,
                                         char *argv[])
{
    UnityMain(argc,
              argv,
              run_smc_t0_transfer);

    return 0;
}
