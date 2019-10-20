#include <alisldis.h>
#include <stdio.h>
#include <string.h>

alisl_handle g_hd_dev;
alisl_handle g_sd_dev;
alisl_handle g_layer_dev;

static bool g_initialed = false;

static int disptest_dis_open(void);
static int disptest_dis_close(void);
static int disptest_dual_output_tvsys_mode(unsigned int cmd_id);


static unsigned int disptest_param_val(char *param);

static int disptest_aspect_ratio(int argc, char *argv[]);
static int disptest_aspect_ratio_imp(unsigned int cmd_id);

static int disptest_output_pic_format(int argc, char *argv[]);
static int disptest_output_pic_format_imp(unsigned int cmd_id);

static int disptest_tvsys_mode(int argc, char *argv[]);
static int disptest_print_all_usage(int argc, char *argv[]);

static char cmd_param_usage[] = "cmd [param1] [param2]...\n\n , where cmd [param1] [param2]... is\n";
static unsigned int g_tvmode = (unsigned int)-1;

typedef int (*disptest_func)(int argc, char *argv[]);
typedef struct {
    const char*  cmd;
    const char*  ext_param;
    int func;
} disp_parammap_t;
/*
0: PAL
1: PAL_N
2: PAL_NC
3: PAL_M
4: PAL_60
5: NTSC
6: NTSC_443
7: 576P
8: 480P
9: 720P50
10: 720P60
11: 1080I50
12: 1080I60
13: 1080P25
14: 1080P30
15: 1080P24
16: 1080P50
17: 1080P60
18: 4096X2160P24
19: 3840X2160P24
20: 3840X2160P25
21: 3840X2160P30
22: 3840X2160P50
23: 3840X2160P60
24: 4096X2160P25
25: 4096X2160P30
26: 4096X2160P50
27: 4096X2160P60

*/
disp_parammap_t disp_map_func[] = {
    {"tvsysmode",            "[0~27,255]",          (int)disptest_tvsys_mode},
    {"aspect_ratio",         "[0 (4:3, PANSCAN),1 (4:3, LETTERBOX), 2 (16:9, PILLBOX), 3(auto, normal scale)]", (int)disptest_aspect_ratio},
    {"out_pic_fmt",         "[1: YUV444]", (int)disptest_output_pic_format},
    {"?",                    "",           (int)disptest_print_all_usage},
    {NULL,                   NULL,              0},
};



int disptest(int argc, char *argv[])
{
    int ret = 0;

    char* arg_func;
    disptest_func function = NULL;

    if (argc < 1) {
        disptest_print_all_usage(argc,argv);
        return -1;
    }


    if ((argv[1] == NULL) ||  (strcmp(argv[0], "?") == 0)) {
        disptest_print_all_usage(argc,argv);
        return -1;

    }

    g_tvmode = disptest_param_val(argv[1]);
    printf("g_tvmode = %u\n",g_tvmode);
    if (g_initialed == false) {
        if (disptest_dis_open() == 0) {
            g_initialed = true;
        }
    }

    if (g_initialed) {
        arg_func = argv[0];
        disp_parammap_t* funcmmap;
        for (funcmmap = disp_map_func; funcmmap->cmd != NULL; funcmmap++) {
            if (strcmp(arg_func, funcmmap->cmd) == 0)
                function = (disptest_func)(funcmmap->func);
        }
        if (function == NULL) {
            printf("No such cmd (%s)\n",arg_func);
            disptest_print_all_usage(argc,argv);
            ret = -1;
            goto exit;
        }

        if (ret ==0) {
            ret =(*function)(argc, argv);
        }

        disptest_dis_close();
    }
exit:
    return ret;
}


///////////////////////////////////////////////////////////////////////////////////////

static int disptest_aspect_ratio(int argc, char *argv[])
{

    int ret = 0;
    unsigned int cmd_id = disptest_param_val(argv[1]);

    if (cmd_id == 255) {
        ret = disptest_dis_close();
    } else {

        ret = disptest_aspect_ratio_imp(cmd_id);
    }
    return ret;
}

static int disptest_tvsys_mode(int argc, char *argv[])
{
    int ret = 0;
    unsigned int cmd_id = disptest_param_val(argv[1]);

    if (cmd_id == 255) {
        ret = disptest_dis_close();
    } else {
        ret = disptest_dual_output_tvsys_mode(cmd_id);
    }
    return ret;

}

static int disptest_output_pic_format(int argc, char *argv[])
{

    int ret = 0;
    unsigned int cmd_id = disptest_param_val(argv[1]);

    if (cmd_id == 255) {
        ret = disptest_dis_close();
    } else {
        ret =  disptest_output_pic_format_imp(cmd_id);
    }
    return ret;
}

static int disptest_print_all_usage(int argc, char *argv[])
{
    int ret = 0;
    disp_parammap_t* funcmmap = NULL;

    printf("%s\n",cmd_param_usage);


    for (funcmmap = disp_map_func; funcmmap->cmd != NULL; funcmmap++) {
        printf("\t%s\r\t\t\t\t\t%s\n", funcmmap->cmd, funcmmap->ext_param);
    }

    printf("The parameter of 'tvsysmode' could be :\n");
    printf("0: PAL\n");
    printf("1: PAL_N\n");
    printf("2: PAL_NC\n");
    printf("3: PAL_M\n");
    printf("4: PAL_60\n");
    printf("5: NTSC\n");
    printf("6: NTSC_443\n");
    printf("7: 576P\n");
    printf("8: 480P\n");
    printf("9: 720P50\n");
    printf("10: 720P60\n");
    printf("11: 1080I50\n");
    printf("12: 1080I60\n");
    printf("13: 1080P25\n");
    printf("14: 1080P30\n");
    printf("15: 1080P24\n");
    printf("16: 1080P50\n");
    printf("17: 1080P60\n");
    printf("18: 4096X2160P24\n");
    printf("19: 3840X2160P24\n");
    printf("20: 3840X2160P25\n");
    printf("21: 3840X2160P30\n");
    printf("22: 3840X2160P50\n");
    printf("23: 3840X2160P60\n");
    printf("24: 4096X2160P25\n");
    printf("25: 4096X2160P30\n");
    printf("26: 4096X2160P50\n");
    printf("27: 4096X2160P60\n");
    return ret;

}

///////////////////////////////////////////////////////////////////////////////////////

static unsigned int disptest_param_val(char *param)
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

static enum dis_dac_id dis_dac_id(uint32_t dac_index)
{
    switch (dac_index) {
        case 0:
                    return DIS_DAC1;
            case 1:
                return DIS_DAC2;
            case 2:
                return DIS_DAC3;
            case 3:
                return DIS_DAC4;
            case 4:
                return DIS_DAC5;
            case 5:
                return DIS_DAC6;
            default:
                return (enum dis_dac_id) -1;
        }
    }

    static int disptest_dac_reg(alisl_handle handle, enum dis_dac_group_type  dac_type)
{
    int ret  = 0;
    struct dis_dac_group dac_grp;

    switch (dac_type) {
        case DIS_DAC_GROUP_YUV1:
            memset(&dac_grp, 0, sizeof(dac_grp));
            dac_grp.dac_grp_type = DIS_DAC_GROUP_YUV1;
            dac_grp.yuv.y = dis_dac_id(2);
            dac_grp.yuv.u = dis_dac_id(1);
            dac_grp.yuv.v = dis_dac_id(0);
            if (DIS_ERR_NONE != alisldis_reg_dac( handle, &dac_grp, true)) {
                ret = -1;
                printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
            }
            break;
        case DIS_DAC_GROUP_CVBS1:
            memset(&dac_grp, 0, sizeof(dac_grp));
            dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS1;
            dac_grp.cvbs = dis_dac_id(3);
            if (DIS_ERR_NONE != alisldis_reg_dac( handle, &dac_grp, false)) {
                ret = -1;
                printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
            }
            break;
        default:
            break;
    }

    return ret;
}

static int disptest_dac_unreg(alisl_handle handle, enum dis_dac_group_type  dac_type)
{
    int ret  = 0;
    if (DIS_ERR_NONE != alisldis_unreg_dac(handle, dac_type)) {
        ret = -1;
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
    }

    return ret;
}

static int disptest_tv_system_set(alisl_handle handle, enum dis_tvsys  tvsys, bool b_progressive)
{
    alisl_retcode   sl_ret;
    int ret = 0;
    sl_ret = alisldis_set_tvsys(handle, tvsys, b_progressive);
    if (DIS_ERR_NONE != sl_ret) {
        ret = -1;
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
    }
    return ret;
}

static int disptest_dis_open(void)
{
    alisl_retcode   sl_ret;
    int ret  = 0;
    //enum dis_layer sl_layer = DIS_LAYER_MAIN;

    if (g_initialed) {
        return 0;
    }

    sl_ret = alisldis_open(DIS_HD_DEV, &(g_hd_dev));
    if (DIS_ERR_NONE != sl_ret) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        ret = -1;
        goto exit;
    }
    sl_ret = alisldis_open(DIS_SD_DEV, &(g_sd_dev));
    if (DIS_ERR_NONE != sl_ret) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        ret = -1;
        goto exit;
    }

    if (g_tvmode >=  18) {
        sl_ret =  alisldis_open(DIS_HD_DEV, &g_layer_dev);
        if (DIS_ERR_NONE != sl_ret) {
            printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
            ret = -1;
            goto exit;
        }


        /*
        	sl_ret = alisldis_win_onoff_by_layer(g_layer_dev, 1, sl_layer);
        	if (DIS_ERR_NONE != sl_ret) {
        		printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        		ret = -1;
        		goto exit;
        	}
        */
        if (DIS_ERR_NONE != alisldis_win_onoff_by_layer(g_layer_dev, 1, DIS_LAYER_GMA1)) {
            printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
            ret = -1;
            goto exit;
        }


        //alisldis_set_global_alpha_by_layer(g_layer_dev, DIS_LAYER_GMA1, 0xff);
    }
exit:

    return ret;
}

static int disptest_dis_close(void)
{
    //disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_YUV1);
    //disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_RGB1);
    //disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_SVIDEO1);
    //disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_CVBS1);
    //disptest_dac_unreg(g_sd_dev, DIS_DAC_GROUP_CVBS1);

    printf("[%s][%d]Call clsoe !!\n", __FUNCTION__ , __LINE__);

    if (DIS_ERR_NONE !=alisldis_close(g_sd_dev)) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
    }

    if (DIS_ERR_NONE !=alisldis_close(g_hd_dev)) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
    }


    if (g_tvmode >= 18) {
        /*
        		if (DIS_ERR_NONE != alisldis_set_global_alpha_by_layer(g_layer_dev, DIS_LAYER_GMA1, 0x00))
        		{
        			printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        		}
        */

        if (DIS_ERR_NONE != alisldis_win_onoff_by_layer(g_layer_dev, 0, DIS_LAYER_GMA1)) {
            printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        }
        /*
        		if (DIS_ERR_NONE != alisldis_win_onoff_by_layer(g_layer_dev, 0, DIS_LAYER_MAIN))
        		{
        			printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        		}
        */
        if (DIS_ERR_NONE != alisldis_close(g_layer_dev)) {
            printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        }
    }

    g_initialed = false;
    return 0;
}

/*
enum dis_hdmi_picture_format {
	DIS_HDMI_YCBCR_422,
	DIS_HDMI_YCBCR_444,
	DIS_HDMI_RGB_MODE1,
	DIS_HDMI_RGB_MODE2,
	DIS_HDMI_YCBCR_420,
};


*/
static int disptest_output_pic_format_imp(unsigned int cmd_id)
{
    alisl_retcode   sl_ret;
    int ret = 0;
    dis_attr attr_type = DIS_ATTR_HDMI_OUT_PIC_FMT;
    uint32_t attr_val = DIS_HDMI_YCBCR_420;

    switch (cmd_id) {
        case 0:
            attr_val = DIS_HDMI_YCBCR_420;
            break;
        case 1:
            attr_val = DIS_HDMI_YCBCR_444;
            break;
        default:
            goto exit;
    }

    printf(">>>>>>>> cmd_id = %u, attr_val=%u\n", cmd_id, attr_val);
    sl_ret = alisldis_set_attr(g_hd_dev, attr_type, attr_val);
    if (DIS_ERR_NONE != sl_ret) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        ret = -1;
        goto exit;
    }
exit:
    return ret;

}


static int disptest_aspect_ratio_imp(unsigned int cmd_id)
{
    alisl_retcode   sl_ret;
    int ret = 0;
    enum dis_display_mode display_mode;
    enum dis_aspect_ratio aspect_mode;

    switch (cmd_id) {
        case 0:
            display_mode = DIS_DM_PANSCAN;
            aspect_mode = DIS_AR_4_3;
            break;
        case 1:
            display_mode = DIS_DM_LETTERBOX;
            aspect_mode = DIS_AR_4_3;
            break;
        case 2:
            display_mode = DIS_DM_PILLBOX;
            aspect_mode = DIS_AR_16_9;
            break;
        case 3:
            display_mode = DIS_DM_NORMAL_SCALE;
            aspect_mode =  DIS_AR_AUTO;
            break;
        case 4:
            display_mode =  DIS_DM_COMBINED_SCALE;
            aspect_mode =  DIS_AR_4_3;
            break;
        case 5:
            display_mode =  DIS_DM_COMBINED_SCALE;
            aspect_mode =  DIS_AR_16_9;
            break;
        default:
            return 0;

    }


    sl_ret = alisldis_set_aspect_mode(g_hd_dev, display_mode, aspect_mode);
    if (DIS_ERR_NONE != sl_ret) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

static int disptest_dual_output_tvsys_mode(unsigned int cmd_id)
{
    int ret = 0;

    unsigned int output_width = 0;
    unsigned int output_height = 0;
    unsigned int input_width = 0;
    unsigned int input_height = 0;
    unsigned int freq = 0;
    alisl_retcode   sl_ret;
    unsigned int is_progressive = 0;
    disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_YUV1);
    disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_RGB1);
    disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_SVIDEO1);
    disptest_dac_unreg(g_hd_dev, DIS_DAC_GROUP_CVBS1);
    disptest_dac_unreg(g_sd_dev, DIS_DAC_GROUP_CVBS1);

    disptest_aspect_ratio_imp(3);

    switch(cmd_id) {
        case 0://AUI_DIS_SD_576I_PAL
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_PAL, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 720;
            output_height = 576;
            input_width = 720;
            input_height = 576;
            freq = 50;
            is_progressive = 0;
            break;
        case 1://AUI_DIS_SD_576I_PAL_N
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_PAL, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL_N, false);
            output_width = 720;
            output_height = 576;
            input_width = 720;
            input_height = 576;
            freq = 50;
            is_progressive = 0;
            break;
        case 2://AUI_DIS_SD_576I_PAL_NC
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_PAL, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL_NC, false);
            output_width = 720;
            output_height = 576;
            input_width = 720;
            input_height = 576;
            freq = 50;
            is_progressive = 0;
            break;
        case 3://AUI_DIS_SD_480I_PAL_M
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_NTSC, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL_M, false);
            output_width = 720;
            output_height = 480;
            input_width = 720;
            input_height = 480;
            freq = 60;
            is_progressive = 0;
            break;
        case 4://AUI_DIS_SD_480I_PAL_60
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_NTSC, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL_60, false);
            output_width = 720;
            output_height = 480;
            input_width = 720;
            input_height = 480;
            freq = 60;
            is_progressive = 0;
            break;
        case 5://AUI_DIS_SD_480I_NTSC
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_NTSC, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 720;
            output_height = 480;
            input_width = 720;
            input_height = 480;
            freq = 60;
            is_progressive = 0;
            break;
        case 6://AUI_DIS_SD_480I_NTSC_443
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_NTSC, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC_443, false);
            output_width = 720;
            output_height = 480;
            input_width = 720;
            input_height = 480;
            freq = 60;
            is_progressive = 0;
            break;

        case 7://AUI_DIS_HD_576P
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_PAL, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 720;
            output_height = 576;
            input_width = 720;
            input_height = 576;
            freq = 50;
            is_progressive = 1;
            break;
        case 8://AUI_DIS_HD_480P
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_NTSC, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 720;
            output_height = 480;
            input_width = 720;
            input_height = 480;
            freq = 60;
            is_progressive = 1;
            break;
        case 9://AUI_DIS_HD_720P50
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_720_25, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 1280;
            output_height = 720;
            input_width = 1280;
            input_height = 720;
            freq = 50;
            is_progressive = 1;
            break;
        case 10://AUI_DIS_HD_720P60
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_720_30, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 1280;
            output_height = 720;
            input_width = 1280;
            input_height = 720;
            freq = 60;
            is_progressive = 1;
            break;
        case 11://AUI_DIS_HD_1080I50
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_25, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 50;
            is_progressive = 0;
            break;
        case 12://AUI_DIS_HD_1080I60
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_30, false);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 60;
            is_progressive = 0;
            break;
        case 13://AUI_DIS_HD_1080P25
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_25, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 25;
            is_progressive = 1;
            break;
        case 14://AUI_DIS_HD_1080P30
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_30, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 30;
            is_progressive = 1;
            break;
        case 15://AUI_DIS_HD_1080P24
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_24, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 24;
            is_progressive = 1;
            break;
        case 16://AUI_DIS_HD_1080P50
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_50, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 50;
            is_progressive = 1;
            break;
        case 17://AUI_DIS_HD_1080P60
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_1080_60, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 1920;
            output_height = 1080;
            input_width = 1920;
            input_height = 1080;
            freq = 60;
            is_progressive = 1;
            break;
        case 18://AUI_DIS_HD_4096X2160P24
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_4096X2160_24, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 4096;
            output_height = 2160;
            input_width = 4096;
            input_height = 2160;
            freq = 24;
            is_progressive = 1;
            break;
        case 19://AUI_DIS_HD_3840X2160P24
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_3840X2160_24, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 3840;
            output_height = 2160;
            input_width = 3840;
            input_height = 2160;
            freq = 24;
            is_progressive = 1;
            break;
        case 20://AUI_DIS_HD_3840X2160P25
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_3840X2160_25, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 3840;
            output_height = 2160;
            input_width = 3840;
            input_height = 2160;
            freq = 25;
            is_progressive = 1;
            break;
        case 21://AUI_DIS_HD_3840X2160P30
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_3840X2160_30, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 3840;
            output_height = 2160;
            input_width = 3840;
            input_height = 2160;
            freq = 30;
            is_progressive = 1;
            break;
        case 22://AUI_DIS_HD_3840X2160P50
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_3840X2160_50, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 3840;
            output_height = 2160;
            input_width = 3840;
            input_height = 2160;
            freq = 50;
            is_progressive = 1;
            break;
        case 23://AUI_DIS_HD_3840X2160P60
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_3840X2160_60, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 3840;
            output_height = 2160;
            input_width = 3840;
            input_height = 2160;
            freq = 60;
            is_progressive = 1;
            break;
        case 24://AUI_DIS_HD_4096X2160P25
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_4096X2160_25, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 4096;
            output_height = 2160;
            input_width = 4096;
            input_height = 2160;
            freq = 25;
            is_progressive = 1;
            break;
        case 25://AUI_DIS_HD_4096X2160P30
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_4096X2160_30, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_NTSC, false);
            output_width = 4096;
            output_height = 2160;
            input_width = 4096;
            input_height = 2160;
            freq = 30;
            is_progressive = 1;
            break;
        case 26: //AUI_DIS_HD_4096X2160P50
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_4096X2160_50, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 4096;
            output_height = 2160;
            input_width = 4096;
            input_height = 2160;
            freq = 50;
            is_progressive = 1;
            break;
        case 27: //AUI_DIS_HD_4096X2160P60
            disptest_tv_system_set(g_hd_dev, DIS_TVSYS_LINE_4096X2160_60, true);
            disptest_tv_system_set(g_sd_dev, DIS_TVSYS_PAL, false);
            output_width = 4096;
            output_height = 2160;
            input_width = 4096;
            input_height = 2160;
            freq = 60;
            is_progressive = 1;
            break;
        default:
            break;

    }


    printf("display format: %u x %u @ %u HZ (is_progressive = %u)(in:%u x %u)\n", output_width, output_height, freq, is_progressive, input_width, input_height);

    if (g_tvmode >= 18) {
        struct scale_param slparam;
        enum dis_layer sl_layer = DIS_LAYER_GMA1;

        input_width = 1280;
        input_height = 720;
        slparam.h_dst = output_width;
        slparam.h_src = input_width;
        slparam.v_dst = output_height;
        slparam.v_src = input_height;
        if (DIS_ERR_NONE != alisldis_gma_scale(g_layer_dev, &slparam, sl_layer)) {
            printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
            ret = -1;
            goto exit;
        }

    }

    sl_ret = alisldis_set_attr(g_hd_dev, DIS_ATTR_TVESDHD_SOURCE_SEL, 0);
    if (DIS_ERR_NONE != sl_ret) {
        printf("[%s][%d]\n", __FUNCTION__ , __LINE__);
        ret = -1;
        goto exit;
    }

    disptest_dac_reg(g_hd_dev, DIS_DAC_GROUP_YUV1);
    disptest_dac_reg(g_sd_dev, DIS_DAC_GROUP_CVBS1);
exit:
    return ret;

}
