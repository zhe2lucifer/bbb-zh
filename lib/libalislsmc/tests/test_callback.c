#include "test_common.c"

TEST_GROUP(SmcRegisterCallback);
TEST_SETUP(SmcRegisterCallback)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_set_cfg(smc_dev, &smc_config));
}
TEST_TEAR_DOWN(SmcRegisterCallback)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcRegisterCallback, RegisterCallbackWithValidParam)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_register_callback(smc_dev, NULL, callback));
}

TEST(SmcRegisterCallback, RegisterCallbackWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_register_callback(NULL, NULL, callback));
}

TEST(SmcRegisterCallback, RegisterCallbackWithNullCallback)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_register_callback(smc_dev, NULL, NULL));
}

TEST(SmcRegisterCallback, GetMessageFromCallbackWhenInsertRemoveCard)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_register_callback(smc_dev, NULL, callback));

	printf("\nPlease remove the card if the card has been inserted!\n");
	cli_check_result("The card has been removed?", 'Y', 'n');

	cli_check_result("Do you get the correct message?", 'Y', 'n');

	printf("\nPlease insert the card if the card has been removed!\n");
	cli_check_result("The card has been inserted?", 'Y', 'n');

	cli_check_result("Do you get the correct message?", 'Y', 'n');
}

TEST_GROUP_RUNNER(SmcRegisterCallback)
{
	RUN_TEST_CASE(SmcRegisterCallback, RegisterCallbackWithValidParam);
	RUN_TEST_CASE(SmcRegisterCallback, RegisterCallbackWithNullHandle);
	RUN_TEST_CASE(SmcRegisterCallback, RegisterCallbackWithNullCallback);
	RUN_TEST_CASE(SmcRegisterCallback, GetMessageFromCallbackWhenInsertRemoveCard);
}

static void run_smc_callback()
{
	RUN_TEST_GROUP(SmcRegisterCallback);
}

static int run_group_smc_callback(int argc,
							   char *argv[])
{
	UnityMain(argc,
			  argv,
			  run_smc_callback);

	return 0;
}