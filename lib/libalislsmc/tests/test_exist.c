#include "test_common.c"

TEST_GROUP(SmcCardExist);
TEST_SETUP(SmcCardExist)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_set_cfg(smc_dev, &smc_config));
}
TEST_TEAR_DOWN(SmcCardExist)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcCardExist, CardExistWithValidParamWhenCardRemoved)
{
	printf("\nPlease remmove the card first!\n");
	cli_check_result("The card has been removed?", 'Y', 'n');

	TEST_ASSERT_EQUAL_INT(false, alislsmc_card_exist(smc_dev));
}

TEST(SmcCardExist, CardExistWithValidParamWhenCardInserted)
{
	printf("\nPlease insert the card first!\n");
	cli_check_result("The card has been inserted?", 'Y', 'n');

	TEST_ASSERT_EQUAL_INT(true, alislsmc_card_exist(smc_dev));
}

TEST(SmcCardExist, CardExistWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_card_exist(NULL));
}

TEST_GROUP_RUNNER(SmcCardExist)
{
	RUN_TEST_CASE(SmcCardExist, CardExistWithValidParamWhenCardRemoved);
	RUN_TEST_CASE(SmcCardExist, CardExistWithValidParamWhenCardInserted);
	RUN_TEST_CASE(SmcCardExist, CardExistWithNullHandle);
}

static void run_smc_exist()
{
	RUN_TEST_GROUP(SmcCardExist);
}

static int run_group_smc_exist(int argc,
							   char *argv[])
{
	UnityMain(argc,
			  argv,
			  run_smc_exist);

	return 0;
}