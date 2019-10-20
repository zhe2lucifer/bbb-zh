
static alisl_handle snd_hdl;

TEST_GROUP(SndAudioStartstop);

TEST_SETUP(SndAudioStartstop)
{
    alislsnd_open(&snd_hdl);
}

TEST_TEAR_DOWN(SndAudioStartstop)
{
    alislsnd_close(snd_hdl);
}

TEST(SndAudioStartstop, AudioStartstop)
{
	enum SndTrackMode track_mode = SND_TRACK_NONE;
		
	printf("%s,%d\n",__FUNCTION__,__LINE__);
    CHECK(0 == alislsnd_pause(snd_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                        cli_check_result("Is audio pause ?",
                                         'y',
                                         'n'));
	
    CHECK(0 == alislsnd_resume(snd_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                        cli_check_result("Is audio resume ?",
                                         'y',
                                         'n'));

	CHECK(0 == alislsnd_stop(snd_hdl));
	TEST_ASSERT_BYTES_EQUAL('y',
						cli_check_result("Is audio stop ?",
										 'y',
										 'n'));
	
	CHECK(0 == alislsnd_start(snd_hdl));
	TEST_ASSERT_BYTES_EQUAL('y',
						cli_check_result("Is audio start ?",
										 'y',
										 'n'));

}

TEST_GROUP_RUNNER(SndAudioStartstop)
{
    RUN_TEST_CASE(SndAudioStartstop, AudioStartstop);
}

static void run_snd_audio_start()
{
    RUN_TEST_GROUP(SndAudioStartstop);
}

static int run_group_snd_start_stop(int argc, char *argv[])
{
    UnityMain(argc,
              argv,
              run_snd_audio_start);

    return 0;
}

