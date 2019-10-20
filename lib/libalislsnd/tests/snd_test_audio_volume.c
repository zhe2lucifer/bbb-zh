static alisl_handle snd_hdl;

TEST_GROUP(SndSetAudioVolume);

TEST_SETUP(SndSetAudioVolume)
{
    alislsnd_open(&snd_hdl);
}

TEST_TEAR_DOWN(SndSetAudioVolume)
{
    alislsnd_close(snd_hdl);
}

TEST(SndSetAudioVolume, SetAudioVolume)
{
	unsigned char volume = 0;
	unsigned char i = 0;
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	for(i = 0;i < 255;i++)
	{
	    CHECK(0 == alislsnd_set_volume(snd_hdl,i,SND_IO_ALL));
		CHECK(0 == alislsnd_get_volume(snd_hdl,SND_IO_ALL,&volume));
		CHECK(i == volume);
		printf("current volume is:%d\n",i);
		sleep(1);
	}
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is track mode right ?",
                                             'y',
                                             'n'));
}

TEST_GROUP_RUNNER(SndSetAudioVolume)
{
    RUN_TEST_CASE(SndSetAudioVolume, SetAudioVolume);
}

static void run_snd_set_audio_volume()
{
    RUN_TEST_GROUP(SndSetAudioVolume);
}

static int run_group_snd_volume(int argc, char *argv[])

{
    UnityMain(argc,
              argv,
              run_snd_set_audio_volume);

    return 0;
}

