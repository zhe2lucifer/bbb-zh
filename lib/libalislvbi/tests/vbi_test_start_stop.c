#include <alisldis.h>
#include <alisldmx.h>
#include <alislvbi.h>



static alisl_handle vbi_hdl;

TEST_GROUP(VbiStartStop);

TEST_SETUP(VbiStartStop)
{
	vbi_output_callback output_callback;
	alisl_handle dis_dev = NULL;
	alisl_handle dmx_dev = NULL;
	struct dmx_channel_attr attr;
	enum dmx_id dmx_id;
	unsigned int channelid = -1;
	unsigned int ttx_pid = -1;

    CHECK( 0 == alislvbi_open(&vbi_hdl) );
	CHECK( 0 == alislvbi_get_output_callback(vbi_hdl, &output_callback));
	printf("%s,%d\n",__FUNCTION__,__LINE__);

	CHECK( 0 == alisldis_open(DIS_HD_DEV, &dis_dev) );
	CHECK( 0 == alisldis_set_attr(dis_dev, DIS_ATTR_SET_VBI_OUT, (unsigned int)output_callback) );
	// CHECK(ERROR_NONE == alisldis_set_tvsys(dis_dev, DIS_TVSYS_LINE_1080_25, false));
	CHECK( 0 == alisldis_close(dis_dev) );
	printf("%s,%d\n",__FUNCTION__,__LINE__);

	CHECK( 0 == alislvbi_set_output_device(vbi_hdl,VBI_OUTPUT_DEVICE_SD) );
	printf("%s,%d\n",__FUNCTION__,__LINE__);

	memset(&attr, 0, sizeof(attr));
	attr.stream = DMX_STREAM_TELETEXT;
	CHECK( 0 == alisldmx_open(&dmx_dev, DMX_ID_DEMUX0, 0) );
	CHECK( 0 == alisldmx_start(dmx_dev));
	CHECK( 0 == alisldmx_allocate_channel(dmx_dev, DMX_CHANNEL_STREAM, &channelid) );
	CHECK( 0 == alisldmx_set_channel_attr(dmx_dev, channelid, &attr) );
	CHECK( 0 == alisldmx_set_channel_pid(dmx_dev,  channelid, 772) );
	CHECK( 0 == alisldmx_control_channel(dmx_dev, channelid, DMX_CTRL_ENABLE));
}

TEST_TEAR_DOWN(VbiStartStop)
{
    alislvbi_close(vbi_hdl);
}

TEST(VbiStartStop, StartStop)
{
		
	printf("%s,%d\n",__FUNCTION__,__LINE__);
    CHECK(0 == alislvbi_start(vbi_hdl));

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Can you see  vbi data output  ?",
                                             'y',
                                             'n'));
	
	CHECK(0 == alislvbi_stop(vbi_hdl));
    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Does vbi data stop output  ?",
                                             'y',
                                             'n'));

}

TEST_GROUP_RUNNER(VbiStartStop)
{
    RUN_TEST_CASE(VbiStartStop, StartStop);
}

static void run_vbi_start_stop()
{
    RUN_TEST_GROUP(VbiStartStop);
}

static int run_group_vbi_start_stop(int argc, char *argv[])

{
    UnityMain(argc,
              argv,
              run_vbi_start_stop);

    return 0;
}
