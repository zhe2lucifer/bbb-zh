static alisl_handle snd_hdl;

TEST_GROUP(SndSetAudioMute);

TEST_SETUP(SndSetAudioMute)
{
    alislsnd_open(&snd_hdl);
}

TEST_TEAR_DOWN(SndSetAudioMute)
{
    alislsnd_close(snd_hdl);
}

TEST(SndSetAudioMute, SetAudioMute)
{
	bool mute = 0;
		
	printf("%s,%d\n",__FUNCTION__,__LINE__);
    CHECK(0 == alislsnd_set_mute(snd_hdl,1,SND_IO_ALL));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_ALL,&mute));
	CHECK(1 == mute);
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is all the audio mute ?",
                                             'y',
                                             'n'));
    CHECK(0 == alislsnd_set_mute(snd_hdl,0,SND_IO_ALL));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_ALL,&mute));
	CHECK(0 == mute);
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is all the audio unmute ?",
                                             'y',
                                             'n'));
	
    CHECK(0 == alislsnd_set_mute(snd_hdl,0,SND_IO_RCA));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_RCA,&mute));
	CHECK(0 == mute);
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is only RCA audio mute ?",
                                             'y',
                                             'n'));
	
    CHECK(0 == alislsnd_set_mute(snd_hdl,0,SND_IO_HDMI));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_HDMI,&mute));
	CHECK(0 == mute);
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is  RCA and HDMI audio mute ?",
                                             'y',
                                             'n'));
	
    CHECK(0 == alislsnd_set_mute(snd_hdl,0,SND_IO_SPDIF));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_SPDIF,&mute));
	CHECK(0 == mute);
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is  RCA and HDMI and SPDIF audio mute ?",
                                             'y',
                                             'n'));

	CHECK(0 == alislsnd_set_mute(snd_hdl,1,SND_IO_RCA));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_RCA,&mute));
	CHECK(1 == mute);
	TEST_ASSERT_BYTES_EQUAL('y',
							cli_check_result("Is  RCA  audio unmute ?",
											 'y',
											 'n'));
	
	CHECK(0 == alislsnd_set_mute(snd_hdl,1,SND_IO_HDMI));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_HDMI,&mute));
	CHECK(1 == mute);
	TEST_ASSERT_BYTES_EQUAL('y',
							cli_check_result("Is  SND_IO_HDMI  audio unmute ?",
											 'y',
											 'n'));

	CHECK(0 == alislsnd_set_mute(snd_hdl,1,SND_IO_SPDIF));
	CHECK(0 == alislsnd_get_mute_state(snd_hdl,SND_IO_SPDIF,&mute));
	CHECK(1 == mute);
	TEST_ASSERT_BYTES_EQUAL('y',
							cli_check_result("Is  SND_IO_SPDIF  audio unmute ?",
											 'y',
											 'n'));

}

TEST_GROUP_RUNNER(SndSetAudioMute)
{
    RUN_TEST_CASE(SndSetAudioMute, SetAudioMute);
}

static void run_snd_set_audio_mute()
{
    RUN_TEST_GROUP(SndSetAudioMute);
}

static int run_group_snd_audio_mute(int argc, char *argv[])

{
    UnityMain(argc,
              argv,
              run_snd_set_audio_mute);

    return 0;
}


