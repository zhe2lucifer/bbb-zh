static alisl_handle snd_hdl;

TEST_GROUP(SndSetOutputFormat);

TEST_SETUP(SndSetOutputFormat)
{
    alislsnd_open(&snd_hdl);
}

TEST_TEAR_DOWN(SndSetOutputFormat)
{
    alislsnd_close(snd_hdl);
}

TEST(SndSetOutputFormat, SetOutputFormat)
{
	enum SndOutFormat output_format = SND_OUT_FORMAT_INVALID;
		
	printf("%s,%d\n",__FUNCTION__,__LINE__);
    CHECK(0 == alislsnd_set_output_format(snd_hdl,SND_OUT_FORMAT_PCM,SND_IO_ALL));
	CHECK(0 == alislsnd_get_output_format(snd_hdl,SND_IO_ALL,&output_format));
	CHECK(SND_OUT_FORMAT_PCM == output_format);
    TEST_ASSERT_BYTES_EQUAL('y',
                        cli_check_result("Is PCM output  ?",
                                         'y',
                                         'n'));
    CHECK(0 == alislsnd_set_output_format(snd_hdl,SND_OUT_FORMAT_BS,SND_IO_ALL));
	CHECK(0 == alislsnd_get_output_format(snd_hdl,SND_IO_ALL,&output_format));
	CHECK(SND_OUT_FORMAT_BS == output_format);
    TEST_ASSERT_BYTES_EQUAL('y',
                        cli_check_result("Is BS output  ?",
                                         'y',
                                         'n'));


}

TEST_GROUP_RUNNER(SndSetOutputFormat)
{
    RUN_TEST_CASE(SndSetOutputFormat, SetOutputFormat);
}

static void run_snd_set_output_format()
{
    RUN_TEST_GROUP(SndSetOutputFormat);
}

static int run_group_snd_output_format(int argc, char *argv[])

{
    UnityMain(argc,
              argv,
              run_snd_set_output_format);

    return 0;
}

