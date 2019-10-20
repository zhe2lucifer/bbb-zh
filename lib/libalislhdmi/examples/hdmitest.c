/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: root_test.c
 *
 *  Description:
 *
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.02.22       Dylan.Yang     0.1.000      Add Video Demo
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>


#include <ali_hdmi_common.h>
#include <hdmi_io_common.h>






char vpo_hdmi_usage[] = "\nUsage:\n\tvpotest --hdmi ";
char hdmitest_usage[] = "\nUsage:\n\thdmitest ";
char cmd_param_usage[] = "cmd [param1] [param2]...\n\n , where cmd [param1] [param2]... is\n";


typedef int (*hdmitest_func)(int argc, char *argv[]);
typedef struct hdmi_parammap_s {
    const char*  cmd;
    const char*  ext_param;
    int func;
} hdmi_parammap_t;



int hdmitest_rd_reg(int argc, char *argv[]);
int hdmitest_wr_reg(int argc, char *argv[]);
int hdmitest_rd_sys_reg(int argc, char *argv[]);
int hdmitest_wr_sys_reg(int argc, char *argv[]);
int hdmitest_get_deep_color(int argc, char *argv[]);
int hdmitest_set_deep_color(int argc, char *argv[]);
int hdmitest_get_color_format(int argc, char *argv[]);
int hdmitest_set_color_format(int argc, char *argv[]);
int hdmitest_rd_scdc(int argc, char *argv[]);
int hdmitest_wr_scdc(int argc, char *argv[]);
int hdmitest_set_bist_mode(int argc, char *argv[]);
int hdmitest_set_avmute(int argc, char *argv[]);
int hdmitest_poll_new_edid(int argc, char *argv[]);
int hdmitest_dis_phy(int argc, char *argv[]);
int hdmitest_set_phy(int argc, char *argv[]);
//int hdmitest_set_de_color(int argc, char *argv[]);
int hdmitest_set_scramble_mode(int argc, char *argv[]);



int print_all_usage(int argc, char *argv[]);

hdmi_parammap_t hdmi_map_func[] = {
    {"rd_reg",               "offset",          (int)hdmitest_rd_reg},
    {"wr_reg",               "offset val",      (int)hdmitest_wr_reg},
    {"rd_sys_reg",           "offset",          (int)hdmitest_rd_sys_reg},
    {"wr_sys_reg",           "offset val",      (int)hdmitest_wr_sys_reg},
    {"rd_scdc",              "offset",          (int)hdmitest_rd_scdc},
    {"wr_scdc",              "offset val",      (int)hdmitest_wr_scdc},
    {"get_deep_color",       "",                (int)hdmitest_get_deep_color},
    {"set_deep_color",       "deep_color_mode(0:24bit,1:30bit:2:36bit,3:48bit)", (int)hdmitest_set_deep_color},
    {"get_color_format",      "",                (int)hdmitest_get_color_format},
    {"set_color_format",      "color_format(0:RGB,1:YCbCr422,2:YCbCr444)",      (int)hdmitest_set_color_format},
    {"set_bist_mode",        "on_off(0:off, 1:on)", (int)hdmitest_set_bist_mode},
    {"set_avmute",        "mute(0:off, 1:on)", (int)hdmitest_set_avmute},
    {"poll_new_edid",        "",                (int)hdmitest_poll_new_edid},
    {"dis_phy",              "item",            (int)hdmitest_dis_phy},
    {"set_phy",               "item val",        (int)hdmitest_set_phy},
//    {"set_de_color",         "color_format(0:YCBCR_411,1:YCBCR_420,2:YCBCR_422,3:YCBCR_444,4:RGB_MODE1,5:RGB_MODE2",   (int)hdmitest_set_de_color},
    {"set_scramble_mode",    "mode(0:normal, 1:force on, 2:force off)", (int)hdmitest_set_scramble_mode},
    {"?",                    "",                (int)print_all_usage},
    {NULL,                   NULL,              0},
};


int hdmi_fd;
HDMI_ioctl_rw_reg_t hdmi_rw_reg;
HDMI_ioctl_rw_scdc_t hdmi_rw_scdc;
HDMI_ioctl_deep_color_t hdmi_deep_color;
HDMI_ioctl_color_space_t hdmi_color_format;
HDMI_ioctl_bist_onoff_state_t hdmi_bist_mode;
HDMI_ioctl_av_mute_t hdmi_avmute;
HDMI_ioctl_dis_phy_t hdmi_dis_phy;
HDMI_ioctl_set_phy_t hdmi_set_phy;
//HDMI_ioctl_set_de_color_t hdmi_de_color;
HDMI_ioctl_scramble_mode_t hdmi_scramble_mode;



static unsigned int get_param_val(char *param)
{
    unsigned int param_val;

    //printf("%s, param %s\n",__FUNCTION__, param);

    if (strstr(param,"0x"))
        sscanf(param,"0x%x",&param_val);
    else if (strstr(param,"0X"))
        sscanf(param,"0X%x",&param_val);
    else
        sscanf(param,"%d",&param_val);

    return param_val;
}


void print_cmd_usage(char *cmd)
{
    hdmi_parammap_t* funcmmap;

    for (funcmmap = hdmi_map_func; funcmmap->cmd != NULL; funcmmap++) {
        if (strcmp(cmd, funcmmap->cmd) == 0) {
#ifdef HDMI_ONLY_UNITEST
            printf("%s\n", hdmitest_usage);
#else
            printf("%s\n", vpo_hdmi_usage);
#endif
            printf("%s %s\n", funcmmap->cmd, funcmmap->ext_param);
        }
    }
}

int print_all_usage(int argc, char *argv[])
{
    hdmi_parammap_t* funcmmap;

#ifdef HDMI_ONLY_UNITEST
    printf("%s\n",hdmitest_usage);
#else
    printf("%s\n",vpo_hdmi_usage);
#endif
    printf("%s\n",cmd_param_usage);

    for (funcmmap = hdmi_map_func; funcmmap->cmd != NULL; funcmmap++) {
        printf("\t%s\r\t\t\t\t\t%s\n", funcmmap->cmd, funcmmap->ext_param);
    }
    return 0;
}


/**
  * read HDMI local register.
  *
  * The register value will be stored in hw_rw_reg.val
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:reg_offset.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_rd_reg(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    hdmi_rw_reg.offset = get_param_val(argv[1]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_RD_REG, &hdmi_rw_reg);

    printf("[%s][%d] reg[0x%08x] = 0x%08x\n",__FUNCTION__, __LINE__,(unsigned int)hdmi_rw_reg.offset, (unsigned int)hdmi_rw_reg.val);
    return ret;
}

/**
  * write HDMI local register.
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:reg_offset, argv[2]: reg_val.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_wr_reg(int argc, char *argv[])
{
    int ret =0;

    if (argc <3) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    hdmi_rw_reg.offset = get_param_val(argv[1]);
    hdmi_rw_reg.val = get_param_val(argv[2]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_WR_REG, &hdmi_rw_reg);

    printf("[%s][%d] reg[0x%08x] = 0x%08x\n",__FUNCTION__, __LINE__,(unsigned int)hdmi_rw_reg.offset, (unsigned int)hdmi_rw_reg.val);

    return ret;
}

/**
  * read HDMI system register.
  *
  * The register value will be stored in hw_rw_reg.val
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:reg_offset.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_rd_sys_reg(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    hdmi_rw_reg.offset = get_param_val(argv[1]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_RD_SYS_REG, &hdmi_rw_reg);

    printf("[%s][%d] reg[0x%08x] = 0x%08x\n",__FUNCTION__, __LINE__,(unsigned int)hdmi_rw_reg.offset, (unsigned int)hdmi_rw_reg.val);
    return ret;

}


/**
  * write HDMI system register.
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:reg_offset, argv[2]: reg_val.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_wr_sys_reg(int argc, char *argv[])
{
    int ret =0;

    if (argc <3) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    hdmi_rw_reg.offset = get_param_val(argv[1]);
    hdmi_rw_reg.val = get_param_val(argv[2]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_WR_SYS_REG, &hdmi_rw_reg);

    printf("[%s][%d] reg[0x%08x] = 0x%08x\n",__FUNCTION__, __LINE__,(unsigned int)hdmi_rw_reg.offset, (unsigned int)hdmi_rw_reg.val);

    return ret;
}

/**
  * read HDMI PHY related register.
  *
  * The item will be stored in hdmi_dis_phy.dis_name
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:item.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_dis_phy(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    strcpy((char *)hdmi_dis_phy.dis_name,argv[1]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_DIS_PHY, &hdmi_dis_phy);

    //printf("%s, item %s\n",__FUNCTION__,hdmi_dis_phy.dis_name);
    return ret;
}

/**
  * write HDMI PHY related register.
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:item, argv[2]: val.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_set_phy(int argc, char *argv[])
{
    int ret =0;

    if (argc <3) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    strcpy((char *)hdmi_set_phy.set_name,argv[1]);
    hdmi_set_phy.set_val = get_param_val(argv[2]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_PHY, &hdmi_set_phy);

    //printf("%s, item %s= 0x%x\n",__FUNCTION__,hdmi_set_phy.set_name,hdmi_set_phy.set_val);

    return ret;
}

#if 0
/**
  * set DE output color format.
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:color_format.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_set_de_color(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    hdmi_de_color.de_format = get_param_val(argv[1]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_DE_COLOR, &hdmi_de_color);

    printf("%s, color format %d\n",__FUNCTION__,hdmi_de_color.de_format);

    return ret;
}
#endif

int hdmitest_get_deep_color(int argc, char *argv[])
{
    int ret =0;

    ret = ioctl(hdmi_fd, HDMI_IOCT_GET_DEEP_COLOR, &hdmi_deep_color);

    switch (hdmi_deep_color.dp_mode) {
        case HDMI_DEEPCOLOR_24:
            printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_24");
            break;
        case HDMI_DEEPCOLOR_30:
            printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_30");
            break;
        case HDMI_DEEPCOLOR_36:
            printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_36");
            break;
        case HDMI_DEEPCOLOR_48:
            printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_48");
            break;
        default:
            printf("%s, dp_mode: %s\n",__FUNCTION__,"unknown DEEPCOLOR");
            break;
    }

    return ret;
}
int hdmitest_set_deep_color(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }


    hdmi_deep_color.dp_mode = get_param_val(argv[1]);
    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_DEEP_COLOR, &hdmi_deep_color);

    switch (hdmi_deep_color.dp_mode) {
        case HDMI_DEEPCOLOR_24:
            //printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_24");
            break;
        case HDMI_DEEPCOLOR_30:
            //printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_30");
            break;
        case HDMI_DEEPCOLOR_36:
            //printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_36");
            break;
        case HDMI_DEEPCOLOR_48:
            //printf("%s, dp_mode: %s\n",__FUNCTION__,"HDMI_DEEPCOLOR_48");
            break;
        default:
            //printf("%s, dp_mode: %s\n",__FUNCTION__,"unknown DEEPCOLOR");
            break;
    }

    return ret;
}

int hdmitest_get_color_format(int argc, char *argv[])
{
    int ret =0;

    ret = ioctl(hdmi_fd, HDMI_IOCT_GET_COLOR_SPACE, &hdmi_color_format);

    switch (hdmi_color_format.color_space) {
        case HDMI_RGB:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_RGB");
            break;
        case HDMI_YCBCR_422:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_YCBCR_422");
            break;
        case HDMI_YCBCR_444:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_YCBCR_444");
            break;
        case HDMI_YCBCR_420:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_YCBCR_420");
            break;
        default:
            printf("%s, color_format: %s\n",__FUNCTION__,"unknown color space");
            break;
    }

    return ret;
}

int hdmitest_set_color_format(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }

    hdmi_color_format.color_space = get_param_val(argv[1]);
    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_COLOR_SPACE, &hdmi_color_format);

    switch (hdmi_color_format.color_space) {
        case HDMI_RGB:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_RGB");
            break;
        case HDMI_YCBCR_422:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_YCBCR_422");
            break;
        case HDMI_YCBCR_444:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_YCBCR_444");
            break;
        case HDMI_YCBCR_420:
            printf("%s, color_format: %s\n",__FUNCTION__,"HDMI_YCBCR_420");
            break;
        default:
            printf("%s, color_format: %s\n",__FUNCTION__,"unknown color space");
            break;
    }

    return ret;

}



int hdmitest_set_bist_mode(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }


    hdmi_bist_mode.bist_on_off = get_param_val(argv[1]);
    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_BIST, &hdmi_bist_mode);
    printf("%s, bist mode (%d)\n",__FUNCTION__,hdmi_bist_mode.bist_on_off);


    return ret;
}



int hdmitest_set_scramble_mode(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }


    hdmi_scramble_mode.scramble_mode = get_param_val(argv[1]);
    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_SCRAMBLE, &hdmi_scramble_mode);
    printf("%s, scramble mode (%d)\n",__FUNCTION__,hdmi_scramble_mode.scramble_mode);


    return ret;
}

int hdmitest_set_avmute(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return -1;
    }


    hdmi_avmute.av_mute = get_param_val(argv[1]);
    ret = ioctl(hdmi_fd, HDMI_IOCT_SET_AV_MUTE, &hdmi_avmute);
    printf("%s, bist mode (%d)\n",__FUNCTION__,hdmi_avmute.av_mute);


    return ret;
}

/**
  * read HDMI scdc.
  *
  * The read back value will be stored in hdmi_rw_scdc.val
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:scdc_offset.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_rd_scdc(int argc, char *argv[])
{
    int ret =0;

    if (argc <2) {
        print_cmd_usage(argv[0]);
        return 0;
    }

    hdmi_rw_scdc.offset = get_param_val(argv[1]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_RD_SCDC, &hdmi_rw_scdc);

    printf("%s, offset 0x%x= 0x%x\n",__FUNCTION__,hdmi_rw_scdc.offset, hdmi_rw_scdc.val);
    return ret;

}

/**
  * write HDMI scdc.
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd, argv[1]:scdc_offset, argv[2]: val.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_wr_scdc(int argc, char *argv[])
{
    int ret =0;

    if (argc <3) {
        print_cmd_usage(argv[0]);
        return -1;
    }


    hdmi_rw_scdc.offset = get_param_val(argv[1]);
    hdmi_rw_scdc.val = get_param_val(argv[2]);

    ret = ioctl(hdmi_fd, HDMI_IOCT_WR_SCDC, &hdmi_rw_scdc);

    printf("%s, offset 0x%x= 0x%x\n",__FUNCTION__,hdmi_rw_scdc.offset, hdmi_rw_scdc.val);

    return ret;
}


/**
  * HDMI poll new EDID.
  *
  * Polling the new EDID message from driver.
  *
  * @param argc      argument number since cmd.
  * @param version   argv[0]:cmd.
  *
  * @return
  *     - 0: Success.
  *     - other: Failure.
  *
  */
int hdmitest_poll_new_edid(int argc, char *argv[])
{
    int ret =0;
    int hdmi_kumsg_fd = -1;
    int open_flag = O_NONBLOCK;
    int msg_len;
    char rd_buf[20];
    int try_cnt =5;
    //int i;

    hdmi_kumsg_fd = ioctl(hdmi_fd, HDMI_IOCT_GET_KUMSGQ, &open_flag);

    printf("%s, hdmi_kumsg_fd (%d) \n",__FUNCTION__,hdmi_kumsg_fd);

    if (hdmi_kumsg_fd >0) {
        while (1) {
            msg_len = -1;
            msg_len =read(hdmi_kumsg_fd,rd_buf,11);
            if (msg_len >0) {
                printf("\n\n*********%s, msg_len %d\n",__FUNCTION__,msg_len);
                switch (rd_buf[0]) {
                    case ALI_HDMI_MSG_EDID:
                        printf(" get ALI_HDMI_MSG_EDID\n**************\n\n");
                        break;
                    case ALI_HDMI_MSG_PLUGIN:
                        printf(" get ALI_HDMI_MSG_PLUGIN\n**************\n\n");
                        break;
                    case ALI_HDMI_MSG_PLUGOUT:
                        printf(" get ALI_HDMI_MSG_PLUGOUT\n***************\n\n");
                        break;
                    case ALI_HDMI_MSG_CEC:
                        printf(" get ALI_HDMI_MSG_CEC\n***************\n\n");
                        break;
                    default:
                        printf(" unknown kumsg\n***************\n\n");
                        break;
                }
                //for (i=0;i<11;i++)
                //    printf("0x%x,",rd_buf[i]);
                try_cnt--;
            }
            if (try_cnt ==0)
                break;
        }
        printf("\n");
    }

    return ret;
}

#ifdef HDMI_ONLY_UNITEST
// HDMI only
int main(int argc, char *argv[])
#else
// combined in vpotest
int hdmitest(int argc, char *argv[])
#endif
{
    int ret =0;


    char* arg_func;
    hdmitest_func function = NULL;
    printf("[%s][%d]argc = %d\n",__FUNCTION__, __LINE__, argc);
    //printf("%s: argc(%d) argv[0](%s)\n", __FUNCTION__, argc, argv[0]);
#ifdef HDMI_ONLY_UNITEST
    // advance to cmd, (to match the usage with those passed from vpotest)
    printf("[%s][%d]argc = %d\n",__FUNCTION__, __LINE__, argc);
    argc -=1;
    argv = &argv[1]
#endif

           printf("[%s][%d]argc = %d\n",__FUNCTION__, __LINE__, argc);
    if (argc < 1) {
        printf("[%s][%d]argc = %d\n",__FUNCTION__, __LINE__, argc);
        print_all_usage(argc,argv);
        return -1;
    }

    arg_func = argv[0];
    hdmi_fd = open("/dev/ali_hdmi_device", O_RDWR);
    //printf("device open, hdmi_fd(%d)\n",hdmi_fd);


    hdmi_parammap_t* funcmmap;
    for (funcmmap = hdmi_map_func; funcmmap->cmd != NULL; funcmmap++) {
        if (strcmp(arg_func, funcmmap->cmd) == 0)
            function = (hdmitest_func)(funcmmap->func);
    }

    if (function == NULL) {
        printf("[%s][%d]argc = %d\n",__FUNCTION__, __LINE__, argc);
        printf(" no such cmd (%s)\n",arg_func);
        print_all_usage(argc,argv);
        ret = -1;
    }

    if (ret ==0)
        ret =(*function)(argc, argv);

    close(hdmi_fd);
    return ret;
}

