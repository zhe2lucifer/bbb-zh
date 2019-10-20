
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
//#include <linux/videodev2.h>
#include <signal.h>
//#include <linux/fb.h>
#include <pthread.h>
#include <assert.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/types.h>
//#include <linux/netlink.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdbool.h>

#include <unistd.h>
#include <ali_hdmi_common.h>
#include <hdmi_io_common.h>

#include <stdio.h>


static const unsigned int bSelectIO = 0;

//////////////////////////////////////////////////////////////


static int hdcp_set_mem_sel(int fd, unsigned int mem_sel);
static int hdcp_set_key_info(int fd, unsigned char scramble, unsigned char *hdcp_ksv, unsigned char * encrypted_hdcp_keys);
static int hdcp_set_on_off(int fd, unsigned int on_off);
static int hdcp_set_hdcp22_key_info(int fd, unsigned char header, unsigned char *key_n, unsigned char * key_e, unsigned char *key_lc);
static int hdcp_set_hdcp22_ce_key_info(int fd, unsigned char *ce_key);
static int hdcp_get_link_status(int fd);
static int hdcp_get_version(int fd);
static bool hdcp_validate_ksv(unsigned char *ksv);

#define DEFAULT_HDMI_DEV_NODE "/dev/ali_hdmi_device"


static int g_hdmi_fd = -1;


/////////////////////////////////////////////////////////////////////////////

static int read_conetent_from_file(char *file_path, unsigned char *content, unsigned int len)
{
    FILE *fp = NULL;
    int ret  = 0;
    unsigned int file_size = 0;

    if (file_path == NULL) {
        printf("[hdcptest](%d)\n", __LINE__);
        ret = -1;
        goto exit;
    }

    fp = fopen(file_path, "rb");

    if (fp == NULL) {
        printf("[hdcptest](%d) Could not read file %s !!\n", __LINE__, file_path);
        ret = -1;
        goto exit;
    }

    fseek (fp , 0 , SEEK_END);
    file_size = (unsigned int)ftell (fp);
    rewind (fp);

    if (file_size < len) {
        printf("[hdcptest](%d)\n", __LINE__);
        ret = -1;
        goto exit;
    }

    ret = fread (content,1,len,fp);
    if (ret != len) {
        printf("[hdcptest](%d)\n", __LINE__);
        ret = -1;
        goto exit;
    }

exit:

    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    return ret;

}

static int  hdcptest_set_hdcp14_key(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    unsigned char scramble = 0;
    unsigned char *hdcp_ksv = NULL;
    unsigned char *encrypted_hdcp_keys = NULL;
    unsigned char key[286] = {0x0};
    unsigned int len = 286;
    int ret = -1;
    bool is_validate_ksv = false;

    ret = read_conetent_from_file(argv[1], key, len);

    if (ret == -1) {
        goto exit;
    }

    scramble = key[0];
    hdcp_ksv = &key[1];
    encrypted_hdcp_keys = &key[6];

    if (!hdcp_ksv || !encrypted_hdcp_keys) {
        printf("[hdcptest](%d)\n", __LINE__);
        ret = -1;
        goto exit;
    }

    is_validate_ksv = hdcp_validate_ksv(hdcp_ksv);

    if (is_validate_ksv == false) {
        printf("[hdcptest](%d)\n", __LINE__);
        ret = -1;
        goto exit;
    }

    if (fd != -1) {
        ret = hdcp_set_key_info(fd, scramble, hdcp_ksv, encrypted_hdcp_keys);
    }
exit:
    return ret;
}
static int  hdcptest_set_hdcp22_key(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    unsigned char header = 0;
    unsigned char *key_n = NULL;
    unsigned char * key_e = NULL;
    unsigned char *key_lc = NULL;
    unsigned char key[402] = {0x0};
    unsigned int len = 402;
    int ret = -1;

    ret = read_conetent_from_file(argv[1], key, len);

    if (ret == -1) {
        goto exit;
    }

    header = key[0];
    key_n = &key[1];
    key_e = &key[385];
    key_lc = &key[386];

    if (!key_n || !key_e || !key_lc) {
        printf("[hdcptest](%d)\n", __LINE__);
        ret = -1;
        goto exit;
    }

    if (fd != -1) {
        ret = hdcp_set_hdcp22_key_info(fd, header, key_n, key_e, key_lc);
    }
exit:
    return ret;
}
static int  hdcptest_set_hdcp22_ce_key(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    unsigned char key[416] = {0x0};
    unsigned int len = 416;
    int ret = -1;

    ret = read_conetent_from_file(argv[1], key, len);

    if (ret == -1) {
        goto exit;
    }

    if (fd != -1) {
        ret = hdcp_set_hdcp22_ce_key_info(fd, key);
    }
exit:
    return ret;
}
static int  hdcptest_set_mem_sel(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    int ret = -1;
    unsigned int mem_sel = 0;


    if (argv[1] == NULL) {
        goto exit;
    }

    mem_sel = (unsigned int)strtoul(argv[1], NULL, 0);

    if (fd != -1) {
        ret = hdcp_set_mem_sel(fd, mem_sel);
    }
exit:
    return ret;
}
static int  hdcptest_set_on_off(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    int ret = -1;
    unsigned int on_off = 0;


    if (argv[1] == NULL) {
        goto exit;
    }

    on_off = (unsigned int)strtoul(argv[1], NULL, 0);

    if (fd != -1) {
        ret = hdcp_set_on_off(fd, on_off);
    }
exit:
    return ret;
}
static int  hdcptest_get_link_status(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    int ret = -1;

    if (fd != -1) {
        ret = hdcp_get_link_status(fd);

    }
    return ret;
}
static int  hdcptest_get_version(int argc, char *argv[])
{
    int fd = g_hdmi_fd;
    int ret = -1;

    if (fd != -1) {
        ret = hdcp_get_version(fd);

    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

typedef int (*hdcptest_func)(int argc, char *argv[]);
typedef struct {
    const char*  cmd;
    const char*  ext_param;
    int func;
} hdcptest_parammap_t;

static int  hdcptest_print_all_usage(int argc, char *argv[]);
static char cmd_param_usage[] = "cmd [param1] [param2]...\n\n , where cmd [param1] [param2]... is\n";

hdcptest_parammap_t hdcptest_map_func[] = {
    {"set_hdcp14_key",		"[key_bin_file_path]",          (int)hdcptest_set_hdcp14_key},
    {"set_hdcp22_key",		"[key_bin_file_path]",			(int)hdcptest_set_hdcp22_key},
    {"set_hdcp22_ce_key",	"[key_bin_file_path]",			(int)hdcptest_set_hdcp22_ce_key},
    {"set_mem_sel",         "[0:SRAM, 1:CE]",				(int)hdcptest_set_mem_sel},
    {"set_on_off",			"[0:off, 1:on]",				(int)hdcptest_set_on_off},
    {"get_link_staus",		"",								(int)hdcptest_get_link_status},
    {"get_version",			"",								(int)hdcptest_get_version},
    {"?",                   "",								(int)hdcptest_print_all_usage},
    {NULL,                  NULL,							0},
};
/////////////////////////////////////////////////////////////////////////////

static int  hdcptest_print_all_usage(int argc, char *argv[])
{
    int ret = 0;
    hdcptest_parammap_t* funcmmap = NULL;

    printf("%s\n",cmd_param_usage);


    for (funcmmap = hdcptest_map_func; funcmmap->cmd != NULL; funcmmap++) {
        printf("\t%s\r\t\t\t\t\t%s\n", funcmmap->cmd, funcmmap->ext_param);
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////

int hdcptest(int argc, char* argv[])
{

    int ret  = 0;

    char *devname = strdup(DEFAULT_HDMI_DEV_NODE);

    //int  i = 0;

    char* arg_func = NULL;
    hdcptest_func function = NULL;
    hdcptest_parammap_t* funcmmap = NULL;

    devname = strdup(DEFAULT_HDMI_DEV_NODE);
    /*
    	for (i = 0; i < argc; i++)
    	{
    		printf("argv[%d]=%s\n", i, argv[i]);
    	}
    */

    if (!devname) {
        printf("[hdcptest](%d)devname is NULL !!\n", __LINE__);
        goto exit;
    }

    if (bSelectIO) {
        g_hdmi_fd = open(devname, O_RDWR | O_NONBLOCK);
    } else {
        g_hdmi_fd = open(devname, O_RDWR);
    }

    if (g_hdmi_fd == -1) {
        printf("[hdcptest](%d) g_hdmi_fd == -1\n", __LINE__);
        goto exit;
    }

    arg_func = argv[0];

    for (funcmmap = hdcptest_map_func; funcmmap->cmd != NULL; funcmmap++) {
        if (strcmp(arg_func, funcmmap->cmd) == 0) {
            function = (hdcptest_func)(funcmmap->func);
        }
    }

    if (function != NULL) {
        ret =(*function)(argc, argv);
    }


exit:
    if (devname != NULL) {
        free(devname);
        devname = NULL;
    }
    if (g_hdmi_fd != -1) {
        close(g_hdmi_fd);
        g_hdmi_fd = -1;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////


static int hdcp_set_mem_sel(int fd, unsigned int mem_sel)
{
    int ret = -1;

    HDMI_ioctl_mem_sel_t  sel;

    //printf("[hdcptest](%d)(%s)\n", __LINE__, __FUNCTION__);

    memset(&sel, 0x0, sizeof(HDMI_ioctl_mem_sel_t));

    sel.mem_sel = mem_sel;

    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_HDCP_MEM_SEL, &sel);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {
            printf("[hdcptest](%d)HDCP msel = %u !!\n", __LINE__, sel.mem_sel);
        }
    }

    return ret;
}


static int hdcp_set_key_info(int fd, unsigned char scramble, unsigned char *hdcp_ksv, unsigned char * encrypted_hdcp_keys)
{
    int ret = -1;

    HDMI_ioctl_hdcp_key_info_t  info;

    // printf("[hdcptest](%d)(%s)\n", __LINE__, __FUNCTION__);

    if (!hdcp_ksv || !encrypted_hdcp_keys) {
        printf("[hdcptest](%d) ksv and encrypted hdcp keys is NULL!!\n", __LINE__);
        goto exit;
    }
    memset(&info, 0x0, sizeof(HDMI_ioctl_hdcp_key_info_t));

    info.scramble = scramble;
    memcpy(&info.hdcp_ksv[0], hdcp_ksv, sizeof(info.hdcp_ksv));
    memcpy(&info.encrypted_hdcp_keys[0], encrypted_hdcp_keys, sizeof(info.encrypted_hdcp_keys));


    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_HDCP_SET_KEY_INFO, &info);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {
            //printf("[hdcptest](%d)set key info !!\n", __LINE__);
        }
    }
exit:
    return ret;
}


static int hdcp_set_on_off(int fd, unsigned int on_off)
{
    int ret = -1;
    HDMI_ioctl_hdcp_state_t st;

    //printf("[hdcptest](%d)(%s)\n", __LINE__, __FUNCTION__);
    memset(&st, 0x0, sizeof(HDMI_ioctl_hdcp_state_t));
    st.hdcp_status = on_off;
    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_HDCP_SET_ONOFF, &st);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {
            //printf("[hdcptest](%d)HDCP on/off = %u\n", __LINE__, st.hdcp_status);
        }
    }

    return ret;
}


static int hdcp_set_hdcp22_key_info(int fd, unsigned char header, unsigned char *key_n, unsigned char * key_e, unsigned char *key_lc)
{

    int ret = -1;

    HDMI_ioctl_hdcp22_key_info_t  info;


    if (!key_n || !key_e  || !key_lc) {
        printf("[hdcptest](%d) hdcp22 key_n or key_e or key_lc is NULL!!\n", __LINE__);
        goto exit;
    }

    memset(&info, 0x0, sizeof(HDMI_ioctl_hdcp22_key_info_t));

    info.header = header;
    memcpy(&info.key_n[0], key_n, sizeof(info.key_n));
    memcpy(&info.key_e[0], key_e, sizeof(info.key_e));
    memcpy(&info.lc[0], key_lc, sizeof(info.lc));

    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_HDCP22_SET_KEY_INFO, &info);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {
            //printf("[hdcptest](%d)set key info !!\n", __LINE__);
        }
    }
exit:
    return ret;

}


static int hdcp_set_hdcp22_ce_key_info(int fd, unsigned char *ce_key)
{

    int ret = -1;

    HDMI_ioctl_hdcp22_ce_key_info_t  info;

    if (!ce_key) {
        printf("[hdcptest](%d)HDCP22 ce_key is NULL!!\n", __LINE__);
        goto exit;
    }

    memset(&info, 0x0, sizeof(HDMI_ioctl_hdcp22_ce_key_info_t));


    memcpy(&info.ce_key[0], ce_key, sizeof(info.ce_key));


    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_HDCP22_SET_CE_KEY_INFO, &info);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {
            //printf("[hdcptest](%d)set ce key info !!\n", __LINE__);
        }
    }
exit:
    return ret;

}



static int hdcp_get_version(int fd)
{
    int ret = -1;
    HDMI_ioctl_hdcp_state_t st;

    memset(&st, 0x0, sizeof(HDMI_ioctl_hdcp_state_t));

    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_HDCP_GET_VERSION, &st);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {

            if (st.hdcp_status == 0) {
                printf("[hdcptest](%d)HDCP 1.4 !!\n", __LINE__);
            } else if (st.hdcp_status == 1) {
                printf("[hdcptest](%d)HDCP 2.2 !!\n", __LINE__);
            } else {
                printf("[hdcptest](%d)HDCP unkown version !!\n", __LINE__);
            }

        }
    }

    return ret;
}



static int hdcp_get_link_status(int fd)
{
    int ret = -1;
    HDMI_ioctl_link_status_t status;

    memset(&status, 0x0, sizeof(HDMI_ioctl_link_status_t));

    if (fd != -1) {
        ret = ioctl(fd, HDMI_IOCT_GET_LINK_ST, &status);
        if (ret == -1) {
            printf("[hdcptest](%d)%s\n", __LINE__, strerror(errno));
        } else {
            printf("[hdcptest](%d) link status = 0x%02x\n", __LINE__, status.link_status);
        }

    }

    if(status.link_status & HDMI_STATUS_LINK_HDCP_SUCCESSED) {
        printf("[hdcptest](%d) hdcp successful !!\n", __LINE__);
    }

    if(status.link_status & HDMI_STATUS_LINK_HDCP_FAILED) {
        printf("[hdcptest](%d) hdcp failed !!\n", __LINE__);
    }

    if(status.link_status & HDMI_STATUS_LINK_HDCP_IGNORED) {
        printf("[hdcptest](%d) hdcp ignore!! (hdcp is off)\n", __LINE__);
    }

    if (status.link_status & HDMI_STATUS_UNLINK) {
        printf("[hdcptest](%d) cable unlink !!\n", __LINE__);

    }

    if (status.link_status & HDMI_STATUS_LINK) {
        printf("[hdcptest](%d) cable link !!\n", __LINE__);
    }

    return ret;
}

static bool hdcp_validate_ksv(unsigned char *ksv)
{
    unsigned int i, j,  number_of_one = 0, number_of_zero = 0;

    // Check KSV has 20 ONE and 20 ZERO or not.
    for(i=0; i<5; i++) {
        for (j=0; j<8; j++) {
            if ((ksv[i] >> j) & 0x01)	number_of_one++;
            else            		    number_of_zero++;
        }
    }
    return (number_of_one == 20 && number_of_zero == 20) ? true : false;
}

