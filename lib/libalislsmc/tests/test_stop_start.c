#include "test_common.c"

TEST_GROUP(SmcStart);
TEST_SETUP(SmcStart)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev, get_smc_dev_id()));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_set_cfg(smc_dev, &smc_config));
}
TEST_TEAR_DOWN(SmcStart)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcStart, StartWithValidParam)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
}

TEST(SmcStart, StartWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_start(NULL));
}

TEST(SmcStart, MultiStartWithValidParam)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
}

TEST_GROUP(SmcStop);
TEST_SETUP(SmcStop)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_open(&smc_dev, get_smc_dev_id()));
}
TEST_TEAR_DOWN(SmcStop)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_close(smc_dev));
}

TEST(SmcStop, StopWithValidParamAfterStarted)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
}

TEST(SmcStop, StopWithValidParamWhenNotStarted)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
}

TEST(SmcStop, StopWithNullHandle)
{
	TEST_ASSERT_NOT_EQUAL(0, alislsmc_stop(NULL));
}

TEST(SmcStop, MultiStopWithValidParamAfterStarted)
{
	TEST_ASSERT_EQUAL_INT(0, alislsmc_start(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
	TEST_ASSERT_EQUAL_INT(0, alislsmc_stop(smc_dev));
}

TEST_GROUP_RUNNER(SmcStart)
{
	RUN_TEST_CASE(SmcStart, StartWithValidParam);
	RUN_TEST_CASE(SmcStart, StartWithNullHandle);
	RUN_TEST_CASE(SmcStart, MultiStartWithValidParam);
}

TEST_GROUP_RUNNER(SmcStop)
{
	RUN_TEST_CASE(SmcStop, StopWithValidParamAfterStarted);
	RUN_TEST_CASE(SmcStop, StopWithValidParamWhenNotStarted);
	RUN_TEST_CASE(SmcStop, StopWithNullHandle);
	RUN_TEST_CASE(SmcStop, MultiStopWithValidParamAfterStarted);
}

static void run_smc_stop_start()
{
	RUN_TEST_GROUP(SmcStart);
	RUN_TEST_GROUP(SmcStop);
}

static int run_group_smc_stop_start(int argc,
									char *argv[])
{
	UnityMain(argc,
			  argv,
			  run_smc_stop_start);

	return 0;
}