/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldis.h
 *  @brief
 *
 *  @version            1.0
 *  @date               7/19/2013 17:01:56
 *  @revision           none
 *
 *  @author             Glen Dai  <glen.dai@alitech.com>
 */

#ifndef __ALISLDIS_H__
#define __ALISLDIS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <alipltfretcode.h>
#include <alisl_types.h> /**< tvsystem enums across all share lib module */

#define DIS_DAC_NUM (4)

enum {
    DIS_ERR_NONE           = ERROR_NONE,
    DIS_ERR_INVAL          = ERROR_INVAL,
    DIS_ERR_INVALHANDLE    = ERROR_INVAL,
    DIS_ERR_NOMEM          = ERROR_NOMEM,
    DIS_ERR_FILEOPENFAILED = ERROR_OPEN,
    DIS_ERR_IOCTLFAILED    = ERROR_IOCTL,
};

enum dis_dev_id {
    DIS_SD_DEV = 0,              /**< SD device id */
    DIS_HD_DEV = 1,              /**< HD device id */
};

struct dis_rect {
    int32_t x;                   /**< x of topleft point of rectangle */
    int32_t y;                   /**< y of topleft point of rectangle */
    int32_t w;                   /**< width of rectangle */
    int32_t h;                   /**< height of rectangle */
};

struct scale_param {
    int32_t h_dst;              /** dst height */
    int32_t h_src;              /** src height */
    int32_t v_dst;              /** dst width */
    int32_t v_src;              /** src width */
};

enum dis_layer {
    DIS_LAYER_MAIN,             /**< main picture layer */
    DIS_LAYER_AUXP,             /**< aux picture layer */
    DIS_LAYER_GMA1,             /**< osd layer */
    DIS_LAYER_GMA2,				/**< subtitle layer */
};

enum dis_aspect_ratio {
    DIS_AR_16_9,                 /**< 16:9 aspect ratio */
    DIS_AR_4_3,                  /**< 4:3 aspect ratio */
    DIS_AR_AUTO,                 /**< auto aspect ratio */
};

enum dis_display_mode {
    DIS_DM_PANSCAN,
    DIS_DM_PANSCAN_NOLINEAR,
    DIS_DM_LETTERBOX,
    DIS_DM_TWOSPEED,
    DIS_DM_PILLBOX,
    DIS_DM_VERTICALCUT,
    DIS_DM_NORMAL_SCALE,
    DIS_DM_LETTERBOX149,
    DIS_DM_AFDZOOM,
    DIS_DM_PANSCAN43ON169,
    DIS_DM_COMBINED_SCALE,
    DIS_DM_IGNORE,
    DIS_DM_VERTICALCUT_149,
};


typedef enum {
    DIS_ENHANCE_BRIGHTNESS = 0x1,       /**< enhance brightness  */
    DIS_ENHANCE_CONTRAST   = 0x2,
    DIS_ENHANCE_SATURATION = 0x4,
    DIS_ENHANCE_SHARPNESS  = 0x8,
    DIS_ENHANCE_HUE        = 0x10,
    DIS_ENHANCE_ALL        = 0x1f
} dis_enhance_type;

typedef enum {
	/** show the last picture of the last program on screen before the new program begins to play */
	DIS_CHG_STILL = 0x0,  
	/** show black screen when change program */
	DIS_CHG_BLACK = 0x1, 	
}dis_change_chane_mode;

struct dis_enhance {
    dis_enhance_type type;              /* enhance type set in following members */
    uint32_t brightness;                /* brightness value of enhance, value[0, 100], default 50  */
    uint32_t contrast;                  /* contrast value of enhance, value[0, 100], default 50  */
    uint32_t saturation;                /* saturation value of enhance, value[0, 100], default 50  */
    uint32_t sharpeness;                /* sharpeness value of enhance, value[0, 10], default 5 */
    uint32_t hue;                       /* hue value of enhance, value[0, 100], default 50  */
};

struct dis_color {
    uint8_t y;                          /**< y channel of ycbcr color */
    uint8_t cb;                         /**< cb channel of ycbcr color */
    uint8_t cr;                         /**< cr channel of ycbcr color */
};

enum dis_dac_group_type {
    DIS_DAC_GROUP_YUV1,                /**< DAC group into RGB/YUV/SVIDEO/CVBS */
    DIS_DAC_GROUP_YUV2,
    DIS_DAC_GROUP_RGB1,
    DIS_DAC_GROUP_RGB2,
    DIS_DAC_GROUP_SVIDEO1,
    DIS_DAC_GROUP_CVBS1,
    DIS_DAC_GROUP_CVBS2,
    DIS_DAC_GROUP_CVBS3,
    DIS_DAC_GROUP_CVBS4,
    DIS_DAC_GROUP_CVBS5,
    DIS_DAC_GROUP_CVBS6,
};

enum dis_win_mode {
    DIS_WIN_MODE_MAINMAIN,
    DIS_WIN_MODE_PIPWIN,
    DIS_WIN_MODE_PREVIEW,
};

enum dis_dac_id {
    DIS_DAC1,                          /**< first DAC */
    DIS_DAC2,                          /**< second DAC */
    DIS_DAC3,                          /**< third DAC */
    DIS_DAC4,                          /**< fouth DAC */
    DIS_DAC5,                          /**< fifth DAC */
    DIS_DAC6,                          /**< sixth DAC */
};


struct dis_info {
    int progressive;                    /**< whether MainPic is progressive */
    int dst_height;
    int dst_width;
    int source_height;
    int source_width;
    int gma1_onoff;                     /**< 0 - off, other - on */
    enum dis_tvsys tvsys;
};

/**
 * The format of the picture output from HDMI.
 */
enum dis_hdmi_picture_format {
	DIS_HDMI_YCBCR_422,
	DIS_HDMI_YCBCR_444,
	DIS_HDMI_RGB_MODE1,
	DIS_HDMI_RGB_MODE2,
	DIS_HDMI_YCBCR_420,
};

typedef enum _dis_attr {
    DIS_ATTR_FULL_SCREEN_CVBS,
    DIS_ATTR_SET_PREVIEW_SAR_MODE,
    DIS_ATTR_SD_CC_ENABLE,
    DIS_ATTR_TVESDHD_SOURCE_SEL,
    DIS_ATTR_SET_LAYER_ORDER,
    DIS_ATTR_ALWAYS_OPEN_CGMS_INFO,
    DIS_ATTR_HDMI_OUT_PIC_FMT,
    DIS_ATTR_SET_PREVIEW_MODE,
    DIS_ATTR_704_OUTPUT,
    DIS_ATTR_SWAFD_ENABLE,
    DIS_ATTR_DIT_CHANGE,
    DIS_ATTR_PLAYMODE_CHANGE,
    DIS_ATTR_ENABLE_VBI,
    DIS_ATTR_DISAUTO_WIN_ONOFF,
    DIS_ATTR_MHEG_IFRAME_NOTIFY,
    DIS_ATTR_MHEG_SCENE_AR,
    DIS_ATTR_WRITE_WSS,
    DIS_ATTR_SET_VBI_OUT,
    DIS_ATTR_GET_DISPLAY_MODE,
    DIS_ATTR_GET_SRC_ASPECT,
    DIS_ATTR_GET_TV_ASPECT,
    DIS_ATTR_GET_REAL_DISPLAY_MODE,
    DIS_ATTR_GET_MP_SCREEN_RECT,
    DIS_ATTR_SET_OSD_SHOW_TIME,
    DIS_ATTR_SET_AUTO_WIN_ONOFF,
    DIS_ATTR_SET_CGMS_INFO,
    DIS_ATTR_SET_PARAMETER,
    DIS_ATTR_SET_AFD_CONFIG,
    DIS_ATTR_GET_OSD0_SHOW_TIME,
    DIS_ATTR_GET_OSD1_SHOW_TIME,
    DIS_ATTR_UNREG_DAC,
} dis_attr;

enum dis_output_type {

    DIS_TYPE_Y = 0,            /**<  Y signal component in YUV*/
    DIS_TYPE_U,                /**<  U signal component in YUV*/
    DIS_TYPE_V,                /**<  V signal component in YUV*/
    DIS_TYPE_CVBS,            /**<  CVBS type output */
    DIS_TYPE_SVIDEO_Y,        /**<  Y signal component in SVIDEO*/
    DIS_TYPE_SVIDEO_C,        /**<  C signal component in SVIDEO*/
    DIS_TYPE_R,                /**<  R signal component in RGB*/
    DIS_TYPE_G,                /**<  G signal component in RGB*/
    DIS_TYPE_B,                /**<  B signal component in RGB*/
    DIS_TYPE_NONE = 0xFF       /**<  this dac unreg */
} ;

/**
 All dac information     
*/

typedef struct dis_dac_info {
    int dac_type[DIS_DAC_NUM];            
} dis_dac_info;

/**
 * DAC must be grouped to form a standard video output interface, such
 * as S-Video (needs 2 DACs to carrie Y/C signal), CVBS (need 1 DAC),
 * component (needs 3 DACs to carrie Y/U/V signal seperately).
 */
struct dis_dac_group {
    enum dis_dac_group_type dac_grp_type;      /**< DAC group type, CVBS for example */
    union {
        struct {
            enum dis_dac_id r;         /**< DAC id for r component of rgb */
            enum dis_dac_id g;         /**< DAC id for g component of rgb */
            enum dis_dac_id b;         /**< DAC id for b component of rgb */
        } rgb;

        struct {
            enum dis_dac_id y;         /**< DAC id for y component of yuv */
            enum dis_dac_id u;         /**< DAC id for u component of yuv */
            enum dis_dac_id v;         /**< DAC id for v component of yuv */
        } yuv;

        struct {
            enum dis_dac_id y;         /**< DAC id for y component of svideo */
            enum dis_dac_id c;         /**< DAC id for c component of svideo */
        } svideo;

        enum dis_dac_id cvbs;
    };
};

typedef enum dis_InputSource
{
	DIS_INPUT_2D = 0x0, 
	DIS_INPUT_3D, 
}dis_InputSource_e;

struct dis_3d_attr
{
    int display_3d_enable;
    int side_by_side;
    int top_and_bottom;
    int display_2d_to_3d;
    int depth_2d_to_3d;
    int mode_2d_to_3d;
    int red_blue;
    dis_InputSource_e eInputSource;
};

struct dis_afd_attr
{
	int bSwscaleAfd;        //!< Whether to open software AFD or not. 
	int afd_solution;             //!< AFD specification. 
	int protect_mode_enable;     //!<Whether to open protect mode or not.
};

struct dis_cgms_aps_info
{
    /* Copy Generation Management System - Analog (CGMS-A), value 0-3: 
	 * 0: Copy Freely
	 * 1: Copy No More
	 * 2: Copy Once
	 * 3: Copy Never
	 */
	uint8_t cgms_a;
	
	/* Analog Protect System, value 0-3: 
	 * 0: off
	 * 1: Automatic Gain Control
	 * 2: 2 line color burst
	 * 3: 4 line color burst
     */
	uint8_t aps;
};

/**
 * Function Name: alisldis_open
 * @brief         Open DIS module, and return a handle to control the device.
 *
 * @param[in] dev_id Device ID, should be DIS_HD_DEV or DIS_SD_DEV
 * @param[out] hdl Device handle return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_open(enum dis_dev_id  dev_id,
              alisl_handle    *hdl);

/**
 * Function Name: alisldis_close
 * @brief         Close the VDEC device by handle
 *
 * @param[in] hdl Device handle to be closed
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_close(alisl_handle hdl);

/**
 * Function Name: alisldis_win_onoff_by_layer
 * @brief         Close the display by layer
 *
 * @param[in] hdl Device handle
 * @param[in] on Onoff value
 * @param[in] layer Layer to be controlled, currently only DIS_LAYER_MAIN is supported
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_win_onoff_by_layer(alisl_handle  hdl,
                            bool           on,
                            enum dis_layer layer);

/**
 * Function Name: alisldis_zoom_by_layer
 * @brief         Zoom display by layer
 *
 * @param[in] hdl Device handle
 * @param[in] srect Source rectangle
 * @param[in] drect Destination rectangle
 * @param[in] layer Layer to operated, currently only DIS_LAYER_MAIN is supported
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_zoom_by_layer(alisl_handle           hdl,
                       struct dis_rect       *srect,
                       struct dis_rect       *drect,
                       enum   dis_layer       layer);

/**
 * Function Name: alisldis_set_tvsys
 * @brief         Set display TV system.
 *
 * @param[in] hdl Device handle
 * @param[in] tvsys TV system to be set
 * @param[in] progressive Boolean value indicates progressive or not.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_tvsys(alisl_handle   hdl,
                   enum dis_tvsys tvsys,
                   bool           progressive);

/**
 * Function Name: alisldis_get_tvsys
 * @brief         Get TV system DIS module is currently using
 *
 * @param[in] hdl Device handle
 * @param[in] tvsys TV system currently using
 * @param[in] progressive Currently using progressive or not
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_tvsys(alisl_handle    hdl,
                   enum dis_tvsys *tvsys,
                   bool           *progressive);

/**
 * Function Name: alisldis_set_bgcolor
 * @brief         Set the background color of the DIS moduel
 *
 * @param[in] hdl Device handle
 * @param[in] color Color to be set
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_bgcolor(alisl_handle      hdl,
                     struct dis_color *color);

/**
 * Function Name: alisldis_reg_dac
 * @brief         Register DAC to DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] dac_grp DAC parameter to be set
 * @param[in] progressive Boolean value indicating whether DAC is output progressively
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_reg_dac(alisl_handle          hdl,
                 struct dis_dac_group *dac_grp,
                 bool                 progressive);

/**
 * Function Name: alisldis_unreg_dac
 * @brief         Unregister DAC to DIS
 *
 * @param[in] hdl Device handle
 * @param[in] dac_grp DAC group to be unregistered
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_unreg_dac(alisl_handle            hdl,
                   enum dis_dac_group_type dac_grp);

/**
 * Function Name: alisldis_set_enhance
 * @brief         Set enhancement of display
 *
 * @param[in] hdl Device handle
 * @param[in] enhance Enhance paramter
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_enhance(alisl_handle        hdl,
                     struct dis_enhance *enhance);

/**
 * Function Name: alisldis_set_aspect_mode
 * @brief         Set aspect mode of display
 *
 * @param[in] hdl Device handle
 * @param[in] display_mode Display mode paramter
 * @param[in] aspect_mode Aspect mode
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_aspect_mode(alisl_handle        hdl,
                         enum dis_display_mode display_mode,
                         enum dis_aspect_ratio aspect_mode);

#if 0
/**
 * Function Name: alisldis_set_parameter
 * @brief         Set paramter of DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] param Parameter to be set
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_parameter(alisl_handle hdl,
                       struct vpo_io_set_parameter *param);

/**
 * Function Name: alisldis_set_cgms_info
 * @brief         Set CGMS parmeter of DIS device
 *
 * @param[in] hdl Device handle
 * @param[in] info Parameter
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_cgms_info(alisl_handle hdl,
                       struct vpo_io_cgms_info *info);

/**
 * Function Name: alisldis_set_afd_config
 * @brief         Set AFD paramter of DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] config Config paramter
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_afd_config(alisl_handle hdl,
                        struct vp_io_afd_para *config);

/**
 * Function Name: alisldis_set_osd_show_time
 * @brief         Set OSD show/hide time
 *
 * @param[in] hdl Device handle
 * @param[in] show_time Show time parameter
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_osd_show_time(alisl_handle hdl,
                           vpo_osd_show_time_t *show_time);
#endif

/**
 * Function Name: alisldis_get_mp_screen_rect
 * @brief         Get show effective rectangle of display
 *
 * @param[in] hdl Device handle
 * @param[in] ret_rect Rectangle returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_mp_screen_rect(alisl_handle  hdl,
                            struct dis_rect *ret_rect);

/**
 * Function Name: alisldis_get_mp_info
 * @brief         Get MP info
 *
 * @param[in] hdl Device handle
 * @param[in] info Information return to caller.
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_mp_info(alisl_handle     hdl,
                     struct dis_info *info);

/**
 * Function Name: alisldis_get_real_display_mode
 * @brief         Get real display mode
 *
 * @param[in] hdl Device handle
 * @param[out] disp_mode Display mode returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_real_display_mode(alisl_handle  hdl,
                               enum dis_display_mode *disp_mode);

/**
 * Function Name: alisldis_real_display_mode
 * @brief         Get real display mode
 *
 * @param[in] hdl Device handle
 * @param[out] disp_mode Display mode returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_display_mode(alisl_handle  hdl,
                          enum dis_display_mode *disp_mode);

/**
 * Function Name: alisldis_get_aspect
 * @brief         Get aspect ration
 *
 * @param[in] hdl Device handle
 * @param[out] aspect Aspect return to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_aspect(alisl_handle  hdl,
                    enum dis_aspect_ratio *aspect);

/**
 * Function Name: alisldis_get_src_aspect
 * @brief         Get source apsect
 *
 * @param[in] hdl Device handle
 * @param[in] aspect Aspect returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_src_aspect(alisl_handle  hdl,
                        enum dis_aspect_ratio *aspect);

/**
 * Function Name: alisldis_backup_picture
 * @brief         Backup picture before channel change
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note          Must call alisldis_free_backup_picture() to release the backuped pitcture.
 */
alisl_retcode
alisldis_backup_picture(alisl_handle hd, dis_change_chane_mode mode);

/**
 * Function Name: alisldis_free_backup_picture
 * @brief         Free the backuped picture, used with alisldis_backup_picture()
 *
 * @param[in] hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_free_backup_picture(alisl_handle  hdl);

#if 1
/**
 * Function Name: alisldis_win_mode
 * @brief         Set display win mode
 *
 * @param[in] hdl Device handle
 * @param[in] win_mode Win mode type to set to DIS
 * @param[in] mpcb Opaque MP callback from VCEC
 * @param[in] pipcb Opaque PIP callback from VDEC
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_win_mode(alisl_handle       hdl,
                  enum dis_win_mode  win_mode,
                  uint32_t          *mpcb,
                  uint32_t          *pipcb);
#endif

/**
 * Function Name: alisldis_set_attr
 * @brief         Set attribute of DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] attr_type Attribute type
 * @param[in] attr_val Attribute value
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_attr(alisl_handle hdl,
                  dis_attr attr_type,
                  uint32_t attr_val);

/**
 * Function Name: alisldis_set_attr
 * @brief         Set attribute of DIS module
 *
 * @param[in] hdl Device handle
 * @param[in] attr_type Attribute type
 * @param[in] attr_val Attribute value
 *
 * @return        alisl_retcode
 *
 * @author        Glen Dai <glen.dai@alitech.com>
 * @date          31/12/2013, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_attr(alisl_handle hdl,
                  dis_attr attr_type,
                  uint32_t *attr_addr);

/**
 *  @brief			scale the gma layer
 *
 *  @param[in]      hdl             alisldis module handle
 *  @param[in]      scale_param     scale parameters
 *  @param[in]      layer           layer type
 *
 *  @return         alisl_retcode
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/14/2014  14:42:57
 *
 *  @note
 */
alisl_retcode
alisldis_gma_scale(alisl_handle                 hdl,
                   const struct scale_param    *scale_param,
                   enum   dis_layer            layer);

/**
 *  @brief          set a global alpha for the specified layer
 *
 *  @param[in]      hdl                 display module handle
 *  @param[in]      layer               layer id
 *  @param[in]      global_alhpa        global alpha value
 *
 *  @return         alisl_retcode
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/14/2014  14:49:15
 *
 *  @note			Only GMA1 layer is supported right now.
 */
alisl_retcode
alisldis_set_global_alpha_by_layer(alisl_handle hdl,
                                   enum dis_layer  layer,
                                   unsigned char   global_alpha);

/**
 *  @brief          anti-flicker switcher
 *
 *  @param[in]      hdl     display module handle
 *  @param[in]      b_on    set anti-flicker on/off
 *
 *  @return         alisl_retcode
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           3/20/2014  18:7:23
 *
 *  @note
 */

alisl_retcode
alisldis_anti_flicker_onoff(alisl_handle hdl,
                            unsigned char b_on);

/**
 * @brief set the blending order of fb0 fb1 and fb2
 * @param[in] hdl shared lib handle
 * @param[in] order the order define in enum VP_LAYER_BLEND_ORDER
 * @return alisl_retcode
 */ 
alisl_retcode
alisldis_set_fb_order(alisl_handle hdl, int order);


/*
 * @set 3D parameters
 * @param[in] hdl shared lib handle
 * @param[in] dis 3d parameters
 * @return alisl_retcode
 * @ Note:
 */ 
alisl_retcode alisldis_set_3d(alisl_handle hdl, struct dis_3d_attr *dis_3d_paras);


/*
 * @set afd parameters
 * @param[in] hdl shared lib handle
 * @param[in] dis afd parameters
 * @return alisl_retcode
 * @ Note:
 */ 
alisl_retcode alisldis_set_afd(alisl_handle hdl, struct dis_afd_attr *dis_afd_paras); 


/**
 * Function Name: alisldis_get_boot_media_status
 * @brief         check boot show media if finish
 *
 * @param[in] hdl Device handle
 * @param[out] show media status  0:show media not finish, 1:show media finish
 *
 * @return        alisl_retcode
 *
 * @author        Vedic.Fu <vedic.fu@alitech.com>
 * @date          2/6/2015, Created
 *
 * @note just support HD/DEN handler
 */
alisl_retcode alisldis_get_boot_media_status(alisl_handle hdl, int *status);


alisl_retcode alisldis_set_enhance_osd(alisl_handle hdl,
                     struct dis_enhance *enhance);

/**
 * Function Name: alisldis_exchange_video_layer
 * @brief         swap video display layer
 *
 * @param[in]  hdl Device handle
 *
 * @return        alisl_retcode
 *
 * @author        Wendy.He <wendy.he@alitech.com>
 * @date          2/6/2015, Created
 *
 * @note just support HD handle
 */
alisl_retcode alisldis_exchange_video_layer(alisl_handle hdl);

/**
 * Function Name: alisldis_deo_control_onoff
 * @brief         control deo when off the SD
 *
 * @param[in]     hdl Device handle
 * @param[in]     0: when off the SD, the cvbs output will not effect
 *                1: when off the SD, the cvbs output will be close(black screen)
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          18/05/2016, Created
 *
 * @note
 */
alisl_retcode
alisldis_deo_control_onoff(alisl_handle hdl, bool on);

/**
 * Function Name: alisldis_set_enhance_max_value
 * @brief         Set the max value of the enhancement of display, 
 *                the range of the enhancement is set to [0, max]
 *                only affects the following enhance setting
 *
 * @param[in] hdl device handle
 * @param[in] max value of enhancement
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          19/7/2016, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_enhance_max_value(alisl_handle        hdl,
                               unsigned int        max);
                               
/**
 * Function Name: alisldis_get_mp_source_rect
 * @brief         Get source rectangle of display
 *
 * @param[in] hdl Device handle
 * @param[in] ret_rect Rectangle returned to caller
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          9/10/2016, Created
 *
 * @note
 */
alisl_retcode
alisldis_get_mp_source_rect(alisl_handle     hdl,
                            struct dis_rect *ret_rect);                               

/**
 * Function Name: alisldis_set_afd_scale_mode
 * @brief         Set the scale mode, the scale mode will affect the display match mode of afd 
 *
 * @param[in] hdl Device handle
 * @param[in] enable:
 *            0: when afd is enable, use afd value stored in stream to implement 
 *               the display match mode, and the display position is defined in spec
 *            1: when afd is enable, use default afd value(0xff) to implement 
 *               the display match mode, and the display position is matched 
 *               the general description
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          20/1/2017, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_afd_scale_mode(alisl_handle  hdl,
                            unsigned char enable);

/**
 * Function Name: alisldis_set_cgms_aps
 * @brief         Copy Generation Management System - Analog (CGMS-A) and Analog Protection System
 *                Set CGMS-A and APS
 *
 * @param[in] hdl Device handle
 * @param[in] cgms_aps: cgms and aps info
 *
 * @return        alisl_retcode
 *
 * @author        Wendy He <wendy.he@alitech.com>
 * @date          23/1/2017, Created
 *
 * @note
 */
alisl_retcode
alisldis_set_cgms_aps(alisl_handle  hdl,
                      struct dis_cgms_aps_info *cgms_aps);                            


alisl_retcode
alisldis_get_dac_configure(alisl_handle hdl,
                  dis_dac_info *p_dac_info);

#ifdef __cplusplus
}
#endif

#endif    /* __ALISLDIS_H__ */
