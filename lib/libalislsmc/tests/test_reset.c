#include "test_common.c"

TEST_GROUP(SmcCardReset);
TEST_SETUP(SmcCardReset)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_set_cfg(smc_dev, &smc_config));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
}
TEST_TEAR_DOWN(SmcCardReset)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcCardReset, CardResetWithValidParam)
{
	unsigned char atr[256];
	unsigned short size;
	int i;

	TEST_ASSERT_EQUAL_INT(0, alislsmc_reset(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_get_atr(smc_dev, atr, &size));

	printf("\nreceive atr:\n");
	for (i = 0; i < size; i++) {
		printf("%02x ", atr[i]);
	}
	printf("\nover!\n");

	cli_check_result("Do you get the ATR correctly?", 'Y', 'n');
}

TEST(SmcCardReset, CardResetWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_reset(NULL));
}

TEST(SmcCardReset, GetAtrWithInvalidParam)
{
	unsigned char atr[256];
	unsigned short size = 0;

	TEST_ASSERT_NOT_EQUAL(0, alislsmc_get_atr(NULL, atr, &size));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_get_atr(smc_dev, NULL, &size));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_get_atr(smc_dev, atr, NULL));
}

TEST_GROUP_RUNNER(SmcCardReset)
{
	RUN_TEST_CASE(SmcCardReset, CardResetWithValidParam);
	RUN_TEST_CASE(SmcCardReset, CardResetWithNullHandle);
	RUN_TEST_CASE(SmcCardReset, GetAtrWithInvalidParam);
}

static void run_smc_reset()
{
	RUN_TEST_GROUP(SmcCardReset);
}

static int run_group_smc_reset(int argc,
							   char *argv[])
{
	UnityMain(argc,
			  argv,
			  run_smc_reset);

	return 0;
}

