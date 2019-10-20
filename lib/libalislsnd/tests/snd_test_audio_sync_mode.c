static alisl_handle snd_hdl;

TEST_GROUP(SndSetAudioSyncMode);

TEST_SETUP(SndSetAudioSyncMode)
{
    alislsnd_open(&snd_hdl);
}

TEST_TEAR_DOWN(SndSetAudioSyncMode)
{
    alislsnd_close(snd_hdl);
}

TEST(SndSetAudioSyncMode, SetAudioSyncMode)
{
	enum SndSyncMode sync_mode = SND_AVSYNC_FREERUN;
		
	printf("%s,%d\n",__FUNCTION__,__LINE__);
    CHECK(0 == alislsnd_set_av_sync_mode(snd_hdl,SND_AVSYNC_FREERUN));
	CHECK(0 == alislsnd_get_av_sync_mode(snd_hdl,&sync_mode));
	CHECK(SND_AVSYNC_FREERUN == sync_mode);
	printf("set audio av sync free run \n");
	sleep(10);
    CHECK(0 == alislsnd_set_av_sync_mode(snd_hdl,SND_AVSYNC_PTS));
	CHECK(0 == alislsnd_get_av_sync_mode(snd_hdl,&sync_mode));
	CHECK(SND_AVSYNC_PTS == sync_mode);
	printf("set audio av sync pts \n");
	sleep(10);

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is track mode right ?",
                                             'y',
                                             'n'));

}

TEST_GROUP_RUNNER(SndSetAudioSyncMode)
{
    RUN_TEST_CASE(SndSetAudioSyncMode, SetAudioSyncMode);
}

static void run_snd_set_audio_sync_mode()
{
    RUN_TEST_GROUP(SndSetAudioSyncMode);
}

static int run_group_snd_av_sync_mode(int argc, char *argv[])
{
    UnityMain(argc,
              argv,
              run_snd_set_audio_sync_mode);

    return 0;
}

