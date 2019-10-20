#include "test_common.c"

TEST_GROUP(SmcRawReadWrite);
TEST_SETUP(SmcRawReadWrite)
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

TEST_TEAR_DOWN(SmcRawReadWrite)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcRawReadWrite, RawReadWriteWithValidParam)
{
	unsigned char response[256];
	size_t size, actlen;
	int i;

	size = sizeof(raw_cmd);

	TEST_ASSERT_EQUAL_INT(0, alislsmc_raw_write(smc_dev, raw_cmd, sizeof(raw_cmd), &actlen));
	TEST_ASSERT_EQUAL_INT(size, actlen);
	TEST_ASSERT_EQUAL_INT(0, alislsmc_raw_read(smc_dev, response, sizeof(response), &actlen));

	print_response_data(response, actlen);
}

TEST(SmcRawReadWrite, RawWriteWithInValidParam)
{
	unsigned char response[256];
	size_t actlen;

	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_write(NULL, raw_cmd, sizeof(raw_cmd), &actlen));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_write(smc_dev, NULL, sizeof(raw_cmd), &actlen));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_write(smc_dev, raw_cmd, 0, &actlen));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_write(smc_dev, raw_cmd, sizeof(raw_cmd), NULL));
}

TEST(SmcRawReadWrite, RawReadWithInValidParam)
{
	unsigned char response[256];
	size_t actlen;

	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_read(NULL, response, sizeof(response), &actlen));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_read(smc_dev, NULL, sizeof(response), &actlen));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_read(smc_dev, response, 0, &actlen));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_raw_read(smc_dev, response, sizeof(response), NULL));
}

TEST_GROUP_RUNNER(SmcRawReadWrite)
{
	RUN_TEST_CASE(SmcRawReadWrite, RawReadWriteWithValidParam);
	RUN_TEST_CASE(SmcRawReadWrite, RawWriteWithInValidParam);
	RUN_TEST_CASE(SmcRawReadWrite, RawReadWithInValidParam);
}

static void run_smc_read_write()
{
	RUN_TEST_GROUP(SmcRawReadWrite);
}

static int run_group_smc_read_write(int argc,
									char *argv[])
{
	UnityMain(argc,
			  argv,
			  run_smc_read_write);

	return 0;
}

