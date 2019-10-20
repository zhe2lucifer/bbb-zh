/*
 *  (c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *  All rights reserved
 *
 *  @file           test_cap_frame.c
 *  @brief          unit test for alislvdec_captrue_frame().
 *
 *  @Version:       1.0
 *  @date:          04/23/2014 02:00:05 PM
 *  @revision:      none
 *
 *  @Author:        Peter Pan <peter.pan@alitech.com>
 */
#include <fcntl.h>

static alisl_handle m_dev_hdl;

TEST_GROUP(VDECCapture);

TEST_SETUP(VDECCapture)
{
    sltest_play_stream(1, "dvbs1");

    TEST_ASSERT_BYTES_EQUAL('y',
                            cli_check_result("Start play ?",
                                             'y',
                                             'n'));

    CHECK(ERROR_NONE == alislvdec_open(&m_dev_hdl, 0));
}

TEST_TEAR_DOWN(VDECCapture)
{
    sltest_stop_stream();

    CHECK(ERROR_NONE == alislvdec_close(m_dev_hdl));
}

TEST(VDECCapture, case1)
{
    struct vdec_info info;
    struct vdec_frame_cap cap;
    uint8_t *cap_buf = NULL;
    unsigned int buf_sz;
	int i;

    CHECK(ERROR_NONE == alislvdec_stop(m_dev_hdl, false, false));

    CHECK(ERROR_NONE == alislvdec_get_info(m_dev_hdl, &info));

    do {
        /* 1) alloc buffer */
        buf_sz = info.pic_width * info.pic_height * 3 / 2;
        printf("buf_sz = %d (%d K)\n", buf_sz, buf_sz / 1024);
        CHECK((cap_buf = (uint8_t *)malloc(buf_sz)) != 0);

        /* 2) construct cap status */
        cap.buf_addr = cap_buf;
        cap.buf_sz   = buf_sz;

        CHECK(ERROR_NONE == alislvdec_captrue_frame(m_dev_hdl, &cap));
    } while(0);

    /* 3) wait util done */
    int file = open("/tmp/vdec_capture.yuv", O_CREAT | O_RDWR | O_TRUNC);
    CHECK(1 == write(file, cap_buf, buf_sz));
    close(file);

    if (cap_buf) {
        free(cap_buf);
    }

}

TEST_GROUP_RUNNER(VDECCapture)
{
    RUN_TEST_CASE(VDECCapture, case1);
}

static void run_vdec_capture_frame()
{
    RUN_TEST_GROUP(VDECCapture);
}

static int run_group_vdec_capture_frame(int argc, char *argv[])
{
    UnityMain(argc, argv, run_vdec_capture_frame);

    return 0;
}

