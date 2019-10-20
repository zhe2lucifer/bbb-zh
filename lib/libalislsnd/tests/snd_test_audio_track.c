static alisl_handle snd_hdl;

TEST_GROUP(SndSetAudioTrack);

TEST_SETUP(SndSetAudioTrack)
{
    alislsnd_open(&snd_hdl);
}

TEST_TEAR_DOWN(SndSetAudioTrack)
{
    alislsnd_close(snd_hdl);
}

TEST(SndSetAudioTrack, SetAudioTrack)
{
	enum SndTrackMode track_mode = SND_TRACK_NONE;
		
	printf("%s,%d\n",__FUNCTION__,__LINE__);
    CHECK(0 == alislsnd_set_trackmode(snd_hdl,SND_TRACK_L,SND_IO_ALL));
	CHECK(0 == alislsnd_get_trackmode(snd_hdl,SND_IO_ALL,&track_mode));
	CHECK(SND_TRACK_L == track_mode);
	printf("set audio left track mode \n");
	sleep(10);
    CHECK(0 == alislsnd_set_trackmode(snd_hdl,SND_TRACK_R,SND_IO_ALL));
	CHECK(0 == alislsnd_get_trackmode(snd_hdl,SND_IO_ALL,&track_mode));
	CHECK(SND_TRACK_R == track_mode);
	printf("set audio right track mode \n");
	sleep(10);
	CHECK(0 == alislsnd_set_trackmode(snd_hdl,SND_TRACK_MONO,SND_IO_ALL));
	CHECK(0 == alislsnd_get_trackmode(snd_hdl,SND_IO_ALL,&track_mode));
	CHECK(SND_TRACK_MONO == track_mode);
	printf("set audio mono track mode \n");
	sleep(10);
	CHECK(0 == alislsnd_set_trackmode(snd_hdl,SND_TRACK_NONE,SND_IO_ALL));
	CHECK(0 == alislsnd_get_trackmode(snd_hdl,SND_IO_ALL,&track_mode));
	CHECK(SND_TRACK_NONE == track_mode);
	printf("set audio stereo track mode \n");
	sleep(10);

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Is track mode right ?",
                                             'y',
                                             'n'));

}

TEST_GROUP_RUNNER(SndSetAudioTrack)
{
    RUN_TEST_CASE(SndSetAudioTrack, SetAudioTrack);
}

static void run_snd_set_audio_track()
{
    RUN_TEST_GROUP(SndSetAudioTrack);
}

static int run_group_snd_audio_track(int argc, char *argv[])

{
    UnityMain(argc,
              argv,
              run_snd_set_audio_track);

    return 0;
}

