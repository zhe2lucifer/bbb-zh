
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <linux/fb.h>
#include <pthread.h>
#include <assert.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <turbojpeg.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <ali_hdmi_common.h>
#include <hdmi_io_common.h>

extern int hdmitest(int argc, char *argv[]);
extern int disptest(int argc, char *argv[]);
extern int videotest(int argc, char *argv[], unsigned int b_wait_finish);
extern int videotest_stop(void);

#define DEFAULT_HDMI_DEV_NODE "/dev/ali_hdmi_device"
static const unsigned int bSelectIO = 0;

static int bTerminate = 0;



static void sig_kill(int signo)
{
    printf("[hdmi_kumsg]Receive SIGNAL: %d\n", signo);
    if ((signo == SIGTERM) || (signo == SIGINT))
        bTerminate = 1;
    return;
}

#define MAX_KUMSG_SIZE 1024

static int hdmi_get_edid_ready(int fd)
{
    int ret = -1;

    int ready = 0;

    if (fd != -1) {
        ret = ioctl(fd,HDMI_IOCG_EDIDRDY, &ready);
        if (ret == -1) {
            printf("[hdmikumsg](%d)%s\n", __LINE__, strerror(errno));
        } else {
            printf("[hdmikumsg](%d)xx Get edid ready = %d\n", __LINE__, ready);
        }
    }

    return ready;
}



static int hdmi_get_native_res(int fd, enum HDMI_API_RES *native_res_ptr)
{

    int ret = -1;

    enum HDMI_API_RES native;

    if (fd != -1) {
        ret = ioctl(fd,HDMI_IOCG_NATIVERES,  (void*)&native);
        if (ret == -1) {
            printf("[hdmikumsg](%d)%s\n", __LINE__, strerror(errno));
        } else {
            printf("[hdmikumsg](%d)Get native  res = 0x%08x (ret = %d)\n", __LINE__,native, ret);
            *native_res_ptr = native;
        }
    }
    return ret;

}

static int paly_video(int fd);
static char g_play_val_0[128]= {0x0};
static char g_play_val_1[128]= {0x0};
static char g_play_val_2[128]= {0x0};
static char g_play_val_3[128]= {0x0};
static char g_play_val_4[128]= {0x0};
static char g_play_val_5[128]= {0x0};
static char g_play_val_6[128]= {0x0};
static char g_play_val_7[128]= {0x0};

int hdmikumsg(int argc, char* argv[])
{
    int ret = 0;
    int hdmifd = -1;
    int kumsgfd = -1;
    int flags = -1;

    int i = 0;

    unsigned char data[MAX_KUMSG_SIZE] = {0};
    unsigned char  msg_type   = 0;
    unsigned short msg_length = 0;
    unsigned char *msg_data   = NULL;
    char msg_str[512] = {0x0};

    //"/mnt/usb/sda1/av_inject/Philips_Ultra-HD_Light_Waves_supershop-demo-2.mp4", "0", "h265", "3840", "2160", "24", "aac", "48000"
    memset(g_play_val_0, 0x0, sizeof(g_play_val_0));
    memset(g_play_val_1, 0x0, sizeof(g_play_val_1));
    memset(g_play_val_2, 0x0, sizeof(g_play_val_2));
    memset(g_play_val_3, 0x0, sizeof(g_play_val_3));
    memset(g_play_val_4, 0x0, sizeof(g_play_val_4));
    memset(g_play_val_5, 0x0, sizeof(g_play_val_5));
    memset(g_play_val_6, 0x0, sizeof(g_play_val_6));
    memset(g_play_val_7, 0x0, sizeof(g_play_val_7));

    strncpy(g_play_val_0, argv[0], strlen(argv[0]));
    strncpy(g_play_val_1, argv[1], strlen(argv[1]));
    strncpy(g_play_val_2, argv[2], strlen(argv[2]));
    strncpy(g_play_val_3, argv[3], strlen(argv[3]));
    strncpy(g_play_val_4, argv[4], strlen(argv[4]));
    strncpy(g_play_val_5, argv[5], strlen(argv[5]));
    strncpy(g_play_val_6, argv[6], strlen(argv[6]));
    strncpy(g_play_val_7, argv[7], strlen(argv[7]));

    for (i = 0; i < argc; i++) {
        printf("[%d][%s]\n", i, argv[i]);
    }

    if (bSelectIO)
        hdmifd = open(DEFAULT_HDMI_DEV_NODE, O_RDWR | O_NONBLOCK);
    else
        hdmifd = open(DEFAULT_HDMI_DEV_NODE, O_RDWR);

    if (hdmifd == -1) {
        printf("[hdmi_kumsg](%d) hdmifd == -1\n", __LINE__);
        goto exit;
    }

    flags = O_CLOEXEC;
    kumsgfd = ioctl(hdmifd, HDMI_IOCT_GET_KUMSGQ, &flags);

    if (kumsgfd <= 0) {
        printf("[hdmi_kumsg](%d) kumsgfd == -1\n", __LINE__);
        goto exit;
    }

    signal(SIGTERM, sig_kill);
    signal(SIGINT, sig_kill);

    if (hdmi_get_edid_ready(hdmifd)) {
        paly_video(hdmifd);
    }

    while(1) {

        fd_set  rfds;

continue_select:

        if (bTerminate) {
            videotest_stop();
            break;
        }

        FD_ZERO(&rfds);
        FD_SET(kumsgfd, &rfds);

        ret = select(kumsgfd + 1, &rfds, NULL, NULL, NULL);

        if (ret == -1) {
            if (errno == EINTR) {
                printf("[%d]Got EINTR!!\n", __LINE__);
                continue;
            }
        }

        if (ret == 0)  goto  exit;

        if (!FD_ISSET(kumsgfd, &rfds)) continue;

        if (read(kumsgfd, data, MAX_KUMSG_SIZE) < 0) {

            if (errno == EAGAIN) {
                printf("[%d]Got EAGAIN!!\n", __LINE__);
                goto continue_select;
            }
            if (errno != EIO) {
                printf("Unable to read (%d) (%s).\n", errno, strerror(errno));
                goto exit;
            }

        } else {
            msg_type   = data[0];
            msg_length = (data[1] << 8) | data[2];
            msg_data   = &(data[3]);
            memset(msg_str, 0x0, sizeof(msg_str));
            /*
            {
            	printf("%02x,%02x, %02x, %02x,%02x, %02x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
            	printf("%04x\n", (data[1] << 8) | data[2]);
            	printf("%u\n", msg_length);
            }
            */
            memcpy(msg_str, msg_data, msg_length);

            switch (msg_type) {
                case ALI_HDMI_MSG_EDID:
                    printf("[%d]ALI_HDMI_MSG_EDID!! (%s)\n", __LINE__, msg_str);
                    break;
                case ALI_HDMI_MSG_PLUGIN:
                    printf("[%d]ALI_HDMI_MSG_PLUGIN!! (%s)\n", __LINE__,  msg_str);
                    paly_video(hdmifd);
                    break;
                case ALI_HDMI_MSG_PLUGOUT:
                    printf("[%d]ALI_HDMI_MSG_PLUGOUT!! (%s)\n", __LINE__, msg_str);
                    videotest_stop();
                    break;
                case ALI_HDMI_MSG_CEC:
                    printf("[%d]ALI_HDMI_MSG_CEC!! (%s)\n", __LINE__, msg_str);
                    break;
                case ALI_HDMI_MSG_HDCP: //fawn++
                    printf("[%d]ALI_HDMI_MSG_HDCP!! (%s)\n", __LINE__,  msg_str);
                    break;
                case ALI_HDMI_MSG_HDCP_RECEIVER_ID:
                    printf("[%d]ALI_HDMI_MSG_HDCP_RECEIVER_ID !! HDCP ID: %02X.%02X.%02X.%02X.%02X (len = %u)\n",__LINE__, msg_data[4], msg_data[3], msg_data[2], msg_data[1], msg_data[0],  msg_length);
                    break;
                default:
                    break;
            }
        }


    }//end of while

exit:
    if (hdmifd != -1) {
        close(hdmifd);
    }
    if (kumsgfd > 0) {
        close(kumsgfd);
    }

    return ret;
}


static int tvmodeset(enum HDMI_API_RES res)
{
    int ret = 0;
    int  argc  = 2;
    char *argv[2] = {NULL, NULL};
    char argv0[32] = {0x0};
    char argv1[32] = {0x0};
    unsigned int width  = 0;
    unsigned int height = 0;
    memset(argv0, 0x0, sizeof(argv0));
    memset(argv1, 0x0, sizeof(argv1));

    strncpy(argv0, "tvsysmode", strlen("tvsysmode"));

    switch (res) {
        case HDMI_RES_INVALID:
            goto exit;

        case HDMI_RES_480P:
            width = 720;
            height  = 480;
            strncpy(argv1, "8", strlen("8"));
            break;
        case HDMI_RES_480P_120:
            //NO supported
            break;
        case HDMI_RES_480P_240:
            //NO supported
            break;
        case HDMI_RES_480I:
            width = 720;
            height  = 480;
            strncpy(argv1, "5", strlen("5"));
            break;
        case HDMI_RES_480I_60:
            //NO supported
            break;
        case HDMI_RES_480I_120:
            //NO supported
            break;
        case HDMI_RES_576I:
            width = 720;
            height  = 576;
            strncpy(argv1, "0", strlen("0"));
            break;
        case HDMI_RES_576I_50:
            //NO supported
            break;
        case HDMI_RES_576I_100:
            //NO supported
            break;
        case HDMI_RES_576P:
            width = 720;
            height  = 576;
            strncpy(argv1, "7", strlen("7"));
            break;
        case HDMI_RES_576P_100:
            //NO supported
            break;
        case HDMI_RES_576P_200:
            //NO supported
            break;
        case HDMI_RES_720P_50:
            width = 1280;
            height  = 720;
            strncpy(argv1, "9", strlen("9"));
            break;
        case HDMI_RES_720P_60:
            width = 1280;
            height  = 720;
            strncpy(argv1, "10", strlen("10"));
            break;
        case HDMI_RES_720P_24:
            //NO supported
            break;
        case HDMI_RES_720P_25:
            //NO supported
            break;
        case HDMI_RES_720P_30:
            //NO supported
            break;
        case HDMI_RES_720P_100:
            //NO supported
            break;
        case HDMI_RES_720P_120:
            //NO supported
            break;
        case HDMI_RES_1080I_25:
            //NO supported
            break;
        case HDMI_RES_1080I_30:
            //NO supported
            break;
        case HDMI_RES_1080P_24:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "15", strlen("15"));
            break;
        case HDMI_RES_1080P_25:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "13", strlen("13"));
            break;
        case HDMI_RES_1080P_30:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "14", strlen("14"));
            break;
        case HDMI_RES_1080P_50:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "16", strlen("16"));
            break;
        case HDMI_RES_1080P_60:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "17", strlen("17"));
            break;
        case HDMI_RES_1080I_50:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "11", strlen("11"));
            break;
        case HDMI_RES_1080I_60:
            width = 1920;
            height  = 1080;
            strncpy(argv1, "12", strlen("12"));
            break;
        case HDMI_RES_1080P_100:
            //NO supported
            break;
        case HDMI_RES_1080P_120:
            //NO supported
            break;
        case HDMI_RES_3840X2160_24:
            width = 3840;
            height  = 2160;
            strncpy(argv1, "19", strlen("19"));
            break;
        case HDMI_RES_3840X2160_25:
            width = 3840;
            height  = 2160;
            strncpy(argv1, "20", strlen("20"));
            break;
        case HDMI_RES_3840X2160_30:
            width = 3840;
            height  = 2160;
            strncpy(argv1, "21", strlen("21"));
            break;
        case HDMI_RES_3840X2160_50:
            width = 3840;
            height  = 2160;
            strncpy(argv1, "22", strlen("22"));
            break;
        case HDMI_RES_3840X2160_60:
            width = 3840;
            height  = 2160;
            strncpy(argv1, "23", strlen("23"));
            break;
        case HDMI_RES_4096X2160_24:
            width = 4096;
            height  = 2160;
            strncpy(argv1, "18", strlen("18"));
            break;
        case HDMI_RES_4096X2160_25:
            width = 4096;
            height  = 2160;
            strncpy(argv1, "24", strlen("24"));
            break;
        case HDMI_RES_4096X2160_30:
            width = 4096;
            height  = 2160;
            strncpy(argv1, "25", strlen("25"));
            break;
        case HDMI_RES_4096X2160_50:
            width = 4096;
            height  = 2160;
            strncpy(argv1, "26", strlen("26"));
            break;
        case HDMI_RES_4096X2160_60:
            width = 4096;
            height  = 2160;
            strncpy(argv1, "27", strlen("27"));
            break;
        default:
            goto exit;
    }
    printf("[hdmikumsg]Set video resolution: [%ux%u][%u]\n", width, height, res);

    argv[0] = &argv0[0];
    argv[1] = &argv1[0];

    disptest(argc, argv);
exit:
    return ret;

}




static int paly_video(int fd)
{
    int argc = 8;
    char *argv[8] = {NULL};
    int ret = -1;



    argv[0] = g_play_val_0;
    argv[1] = g_play_val_1;
    argv[2] = g_play_val_2;
    argv[3] = g_play_val_3;
    argv[4] = g_play_val_4;
    argv[5] = g_play_val_5;
    argv[6] = g_play_val_6;
    argv[7] = g_play_val_7;


    enum HDMI_API_RES res;
    if (fd == -1) {
        return ret;
    }


    hdmi_get_native_res(fd, &res);

    //NOTE: Call distest() to set tvmode.
    ret = tvmodeset(res);
    if (ret == 0) {
        ret = videotest(argc, argv, 0);
    }

    return ret;
}