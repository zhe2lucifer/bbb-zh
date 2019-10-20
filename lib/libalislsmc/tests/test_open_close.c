static alisl_handle smc_dev_tmp = NULL;

/*
 * In sharelibrary, all of the APIs will return 0 when success,
 * otherwise a failure was returned.
 */
TEST_GROUP(SmcOpen);
TEST_SETUP(SmcOpen) {}
TEST_TEAR_DOWN(SmcOpen) {}
TEST(SmcOpen, OpenWithValidParam)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev_tmp, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev_tmp));
}

TEST(SmcOpen, OpenWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_open(NULL, get_smc_dev_id()));
}

/*
 * Currently, only DevicdID equal 0 is the valid ID
 * Because there is only one device which is "/dev/ali_smc_0"
 */
TEST(SmcOpen, OpenWithInvalidDeviceID)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_open(&smc_dev_tmp, 2));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_open(&smc_dev_tmp, 3));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_open(&smc_dev_tmp, 4));
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_open(&smc_dev_tmp, 5));
}

TEST_GROUP(SmcClose);
TEST_SETUP(SmcClose) {}
TEST_TEAR_DOWN(SmcClose) {}
TEST(SmcClose, CloseWithValidParam)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev_tmp, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev_tmp));
}

TEST(SmcClose, CloseWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_close(NULL));
}

TEST_GROUP_RUNNER(SmcOpen)
{
	RUN_TEST_CASE(SmcOpen, OpenWithValidParam);
	RUN_TEST_CASE(SmcOpen, OpenWithNullHandle);
	RUN_TEST_CASE(SmcOpen, OpenWithInvalidDeviceID);
}

TEST_GROUP_RUNNER(SmcClose)
{
	RUN_TEST_CASE(SmcClose, CloseWithValidParam);
	RUN_TEST_CASE(SmcClose, CloseWithNullHandle);
}

static void run_smc_open_close()
{
	RUN_TEST_GROUP(SmcOpen);
	RUN_TEST_GROUP(SmcClose);
}

static int run_group_smc_open_close(int argc,
									char *argv[])
{
	UnityMain(argc,
			  argv,
			  run_smc_open_close);

	return 0;
}