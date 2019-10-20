/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.h
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
* 	2.  Dec.19.2003		 Justin Wu		 Ver 0.2    Add ESC CMD macros.
*	3.  Sep.23.2005		 Justin Wu		 Ver 0.3    Add pan information.
*****************************************************************************/

#ifndef __ALISLINPUT_H__
#define __ALISLINPUT_H__

#include <alipltfretcode.h>
#include <sys/time.h>
#include <linux/input.h>

#include <alislinput_dev.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*alislinput_callback)(void *user_data);

/*! @addtogroup pan
 *  @{
 */
#define PAN_KEY_INVALID				0xFFFFFFFF

/* Key value of Front keypad. */
#define PAN_KEY_01				0xFFFF0001
#define PAN_KEY_02				0xFFFF0002
#define PAN_KEY_03				0xFFFF0004
#define PAN_KEY_04				0xFFFF0008
#define PAN_KEY_05				0xFFFF0010
#define PAN_KEY_06				0xFFFF0020
#define PAN_KEY_07				0xFFFF0040
#define PAN_KEY_08				0xFFFF0080

//multi-panel key pressed
#define PAN_COMPOUND_KEY			0x1A2B3C4D

/* ESC command: 27 (ESC code), PAN_ESC_CMD_xx (CMD type), param1, param2 */
#define PAN_ESC_CMD_LBD			'L'		//!<LBD operate command
#define PAN_ESC_CMD_LBD_FUNCA	0		//!<Extend function LBD A
#define PAN_ESC_CMD_LBD_FUNCB	1		//!<Extend function LBD B
#define PAN_ESC_CMD_LBD_FUNCC	2		//!<Extend function LBD C
#define PAN_ESC_CMD_LBD_FUNCD	3		//!<Extend function LBD D
#define PAN_ESC_CMD_LBD_LEVEL	5			//!<Level status LBD, no used
#define PAN_ESC_CMD_LBD_POWER   PAN_ESC_CMD_LBD_FUNCA       //!< Power status LBD
#define PAN_ESC_CMD_LBD_LOCK    PAN_ESC_CMD_LBD_FUNCD       //!<Lock status LBD

#define PAN_ESC_CMD_LED		'E'		//!<LED operate command
#define PAN_ESC_CMD_LED_LEVEL	0		//!<Level status LED

#define	PAN_ESC_CMD_LBD_ON	1		//!<Set LBD to turn on status
#define	PAN_ESC_CMD_LBD_OFF	0		//!<Set LBD to turn off status

#if ((UPGRADE_FORMAT & BOOT_UPG) == BOOT_UPG)
#define PAN_ESC_CMD_STANDBY_LED			'P'		//!<LED operate command
#endif

#ifndef __ASSEMBLER__

//#include <hld/pan/pan_dev.h>

/*!@struct alislinput_gpio_info
   @brief 用于配置前面板按键GPIO的信息。
*/
struct alislinput_gpio_info			/* Total 2 byte */
{
	unsigned short	polar	: 1;		//!<Polarity of GPIO, 0 or 1 active(light)
	unsigned short	io		: 1;	//!<HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h
	unsigned short	position: 14;		//!<GPIO index, upto over 64 GPIO
} __attribute__((packed));

/*!@struct alislinput_hw_info
   @brief 用于配置前面板硬件信息。
*/
#if 0
struct alislinput_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panal device type */
	unsigned char					type_kb	: 2;	//!<Key board (array) type
	unsigned char					type_scan:1;	///!<0: slot scan, 1: shadow scan
	unsigned char					type_key: 1;	//!<Key exit or not
	unsigned char					type_irp: 3;	//!<0: not IRP, 1: NEC, 2: LAB
	unsigned char					type_mcu: 1;	//!<MCU exit or not
	/* Offset 1: GPIO number */
	unsigned char					num_com	: 4;	//!<Number of COM PIN, 0: no com; <= 8
	unsigned char					pos_colon:4;	//!<Position of colon flag, 0 to 7, 8 no colon
    	/* Offset 2: */
	unsigned char					num_scan: 2;	//!<Number of scan PIN, 0: no scan; <= 2
    	unsigned char					rsvd_bits:6;	//!<in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"
	/* Offset 3: Panel shift control */
	struct alislinput_gpio_info	flatch;			//!<Shifter latch PIN
	struct alislinput_gpio_info	fclock;			//!<Shifter clock PIN
	struct alislinput_gpio_info	fdata;			//!<Shifter data PIN
	/* Offset 6: Panel scan control */
	struct alislinput_gpio_info	scan[2];		//!<Panel scan PIN
	/* Offset 8: Panel com PIN */
	struct alislinput_gpio_info	com[8];			//!<COM PIN
	/* Offset 16: Panel LBD control */
	struct alislinput_gpio_info	lbd[4];			//!<LBD GPIO PIN
	/* Offset 20: Panel input attribute */
	unsigned long	intv_repeat_first;				//!<Repeat interval first in mS
    	/* 24 */
	unsigned long	intv_repeat;					//!<Repeat interval in mS
    	/* 28 */
	unsigned long	intv_release;					//!<Release interval in mS
    	/* 32 */
	unsigned long	(*hook_scan)(struct alislinput_device *dev, unsigned long key);
    	/* 36 */
	unsigned long	(*hook_show)(struct alislinput_device *dev, char *data, unsigned long len);
    	/* 40 */
} __attribute__((packed));
#else
struct alislinput_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panel device type */
	unsigned char					type_kb	: 2;	//!<Key board (array) type
	unsigned char					type_scan:1;	//!<0: slot scan, 1: shadow scan
	unsigned char					type_key: 1;	//!<Key exit or not
	unsigned char					type_irp: 3;	//!<0: not IRP, 1: NEC, 2: LAB
	unsigned char					type_mcu: 1;	//!<MCU exit or not
	/* Offset 1: GPIO number */
	unsigned char					num_com : 4;	//!<Number of COM PIN, 0: no com; <= 8
	unsigned char					pos_colon:4;	//!<Position of colon flag, 0 to 7, 8 no colon
	/* Offset 2: */
	unsigned char					num_scan: 2;	//!<Number of scan PIN, 0: no scan; <= 2
	unsigned char					rsvd_bits:6;	//!<in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"
	/* Offset 3: */
	unsigned char 					rsvd_byte;	//!<Reserved for alignment
	/* Offset 4: Panel shift latch */
	struct alislinput_gpio_info	flatch;			//!<Shifter latch PIN
	struct alislinput_gpio_info	fclock;			//!<Shifter clock PIN
	struct alislinput_gpio_info	fdata;			//!<Shifter data PIN
	/* Offset 10: Panel scan control */
	struct alislinput_gpio_info	scan[2];		//!<Panel scan PIN
	/* Offset 14: Panel com PIN */
	struct alislinput_gpio_info	com[8];			//!<COM PIN
	/* Offset 30: Panel LBD control */
	struct alislinput_gpio_info	lbd[4];			//!<LBD GPIO PIN
	struct alislinput_gpio_info	rsvd_hw;		//!<Reserved for alignment

	/* Offset 40: Panel input attribute */
	unsigned long	intv_repeat_first;			//!<Repeat interval first in mS
    	/* 44 */
	unsigned long	intv_repeat;				//!<Repeat interval in mS
    	/* 48 */
	unsigned long	intv_release;				//!<Release interval in mS
    	/* 52 */
	//unsigned long	(*hook_scan)(struct alislinput_device *dev, unsigned long key);
    	/* 56 */
	//unsigned long	(*hook_show)(struct alislinput_device *dev, char *data, unsigned long len);
    	/* 60 */
} __attribute__((packed));
#endif

/*!@struct led_bitmap
   @brief LED bitmap。
*/
#ifdef PAN_HT_16315_COMPATIBLE
struct led_bitmap
{
	unsigned char character;
	unsigned short bitmap;
};
#else
struct led_bitmap
{
	unsigned char character;
	unsigned char bitmap;
};
#endif

/*!@struct led_bitmap32
   @brief LED bitmap。
*/
struct led_bitmap32
{
	unsigned char character;
	unsigned long bitmap;
};

/*!@struct alislinput_configuration
   @brief Panel driver configuration。
*/
struct alislinput_configuration
{
	struct alislinput_hw_info *hw_info;			//!<Panel GPIO information
	int    bitmap_len;				//!<Bitmap list length
	struct led_bitmap  *bitmap_list;		//!<Bitmap list
};

/*!@struct alislinput_configuration32
   @brief Panel driver configuration。
*/
struct alislinput_configuration32
{
	struct alislinput_hw_info *hw_info;			//!<Panel GPIO information
	int    bitmap_len;				//!<Bitmap list length
	struct led_bitmap32  *bitmap_list;		//!<Bitmap list
};


/*!
@brief 挂接面板设备。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_attach(void);

/*!
@brief 卸载面板设备。
@param[in] dev 需卸载的面板设备节点。
*/
void alislinput_detach(struct alislinput_device *dev);

/*!
@brief 打开前面板设备。
@param[in] dev 需打开的前面板设备节点。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_open(struct alislinput_device **dev);

/*!
@brief 关闭前面板设备。
@param[in] dev 需关闭的前面板设备节点。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_close(struct alislinput_device *dev);

/*!
@brief 显示面板设备。
@param[in] dev 需显示的面板设备节点。
@param[in] data 需显示信息的地址。
@当data[0] == 27时，是ESC command。
@data[1] PAN_ESC_CMD_LBD。
@data[2] PAN_ESC_CMD_LBD_POWER，控制power LED状态; PAN_ESC_CMD_LBD_LOCK，控制LOCK LED状态。
@data[3] = PAN_ESC_CMD_LBD_ON，点亮LED；PAN_ESC_CMD_LBD_OFF，关闭LED。
@param[in] len 需显示信息的长度。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_display(struct alislinput_device *dev, char *data, unsigned long len);

/*!
@brief 获得面板设备当前显示的信息。
@param[in] dev 需获得信息的面板设备节点。
@param[out] data 接收显示信息的地址。
@param[out] len 接收显示信息的长度。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_get_display(struct alislinput_device *dev, char *data, unsigned long * len);


/*!
@brief 面板 模块的io contorl 操作。
@param[in] dev 指向面板模块 的指针。
@param[in] cmd 操作 的命令类型。参考pan_device_ioctrl_command定义。
@param[in,out] param 操作的参数，根据不同的命令会有不同的参数。
@return RET_CODE。
@retval  RET_SUCCESS       申请 成功。
@retval  !RET_SUCCESS     申请 失败，参数错误或状态错误。
@note  IO命令介绍:
<table class="doxtable"  width="800" border="1" style="border-collapse:collapse;table-layout:fixed;word-break:break-all;" >
  <tr>
    <th width="200">命令</th>
    <th width="200">参数</th>
    <th width="80">传输特性</th>
    <th width="320">备注</th>
  </tr>

  <tr align="center">
    <td>PAN_DRIVER_ATTACH</td>
    <td>struct alislinput_hw_info *</td>
    <td>输入</td>
    <td>Front panel driver attach</td>
  </tr>
*/
long alislinput_io_control(struct alislinput_device *dev, long cmd, unsigned long param);

/*!
@brief 获得前面板设备的按键信息。
@param[in] dev 需获得按键信息的前面板设备节点。
@param[in] timeout 接收按键信息的超时次数。
@return  struct alislinput_key *。
@retval  struct alislinput_key *      成功。
@retval  NULL  失败。
*/
struct alislinput_key * alislinput_get_key(struct alislinput_device *dev, unsigned long timeout);

/*!
@brief 获得前面板设备的按键信息。
@param[in] dev 需获得按键信息的前面板设备节点。
@param[in] timeout 接收按键信息的超时次数。
@return  struct alislinput_key_info *。
@retval  struct alislinput_key_info *      成功。
@retval  NULL  失败。
*/
struct alislinput_key_info *  alislinput_get_key_info(struct alislinput_device *dev, unsigned long timeout);

/*!
@brief 往面板设备发送数据。
@param[in] dev 需发送数据的前面板设备节点。
@param[in] data 发送数据的地址。
@param[in] len 发送数据的长度。
@param[in] timeout 发送数据的超时次数。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_send_data(struct alislinput_device *dev, unsigned char *data, unsigned long len, unsigned long timeout);

/*!
@brief 从面板设备接收数据。
@param[in] dev 需接收数据的面板设备节点。
@param[out] data 接收数据的地址。
@param[in] len 接收数据的长度。
@param[in] timeout 接收数据的超时次数。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_receive_data(struct alislinput_device *dev, unsigned char *data, unsigned long len, unsigned long timeout);

/*!
@brief 清除前面板设备按键缓存信息。
@param[in] dev 需清除按键缓存信息的前面板设备节点。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_buff_clear(struct alislinput_device *dev);

/*!
@brief 设置前面板设备按键repeat属性。
@param[in] dev 需设置按键repeat属性的前面板设备节点。
@param[in] enable_repeat 0，禁止repeat；1，使能repeat。
*/
void alislinput_buff_set_repeat(struct alislinput_device *dev, unsigned char enable_repeat);

/*!
@brief 获得前面板设备按键repeat属性。
@param[in] dev 需获得按键repeat属性的前面板设备节点。
@return unsigned char  按键repeat属性。
@retval  0     禁止repeat。
@retval  1     使能repeat。
*/
unsigned char alislinput_buff_get_repeat(struct alislinput_device *dev);

/*!未实现
*/
long alislinput_buff_queue_tail(struct alislinput_key *key);

/*!
@brief 配置红外设备按键信息发送的时间间隔。
@param[in] dev 需配置按键信息时间间隔的前面板设备节点。
@param[in] delay 当用户按住红外设备某个按键不放时，Linux input系统发送第二次按键信息的时间间隔。
@param[in] interval 当用户按住红外设备某个按键不放时，Linux input系统发送第三次及其以后按键信息的时间间隔。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_config_ir_rep_interval(struct alislinput_device *dev, unsigned long delay, unsigned long interval);

/*!
@brief 配置面板设备按键信息发送的时间间隔。
@param[in] dev 需配置按键信息时间间隔的前面板设备节点。
@param[in] delay 当用户按住面板某个按键不放时，Linux input系统发送第二次按键信息的时间间隔。
@param[in] interval 当用户按住面板某个按键不放时，Linux input系统发送第三次及其以后按键信息的时间间隔。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_config_pan_rep_interval(struct alislinput_device *dev, unsigned long delay, unsigned long interval);

/*!
@brief 配置红外设备按键物理键值到逻辑键值的映射表。
@param[in] dev 需配置按键物理键值到逻辑键值的映射表的前面板设备节点。
@param[in] phy_code 	0: read 返回的code为linux 逻辑键值；1: read 返回的 code 为物理键值在映射表中的索引值；2: read 返回的code为物理键值。
@param[in] map 映射表的起始地址。
@param[in] map_len 映射表的长度。
@param[in] unit_len 映射表以byte为单位的长度。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_config_key_map(struct alislinput_device *dev, unsigned char phy_code, unsigned char *map, unsigned long map_len, unsigned long unit_len);

/*!
@brief 删除红外设备按键物理键值到逻辑键值的映射表。
@param[in] dev 需配置按键物理键值到逻辑键值的映射表的前面板设备节点。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_del_key_map(struct alislinput_device *dev);

/*!
@brief 配置面板设备按键物理键值到逻辑键值的映射表。
@param[in] dev 需配置按键物理键值到逻辑键值的映射表的前面板设备节点。
@param[in] phy_code 	0: read 返回的code为linux 逻辑键值；1: read 返回的 code 为物理键值在映射表中的索引值；2: read 返回的code为物理键值。
@param[in] map 映射表的起始地址。
@param[in] map_len 映射表的长度。
@param[in] unit_len 映射表以byte为单位的长度。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_config_panel_map(struct alislinput_device *dev, unsigned char phy_code, unsigned char *map, unsigned long map_len, unsigned long unit_len);

/*!
@brief 配置红外设备需要识别的红外编码格式。
@param[in] dev 需配置红外编码格式的前面板设备节点。
@param[in] format 需要识别的红外编码格式。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_config_ir_format(struct alislinput_device *dev, unsigned long format);

/*!
@brief 获得面板设备的名字。
@param[out] dev_name 接收面板设备名字的地址。
@return long。
@retval  0       成功。
@retval  非0  失败。
*/
long alislinput_get_panel_name(char *dev_name);

//alisl_retcode alislinput_open(alisl_handle *handle, int id);
//
//alisl_retcode alislinput_close(alisl_handle handle);
//
//alisl_retcode alislinput_get_key(alisl_handle handle,
//		struct input_event *key_event, int *type);
//
//alisl_retcode alislinput_config_ir_map(alisl_handle handle, char phy_code,
//		char *map, int map_len, int unit_len);
//
//alisl_retcode alislinput_config_panel_map(alisl_handle handle, char phy_code,
//		char *map, int map_len, int unit_len);

long alislinput_callback_register(struct alislinput_device *dev,
		alislinput_callback callback);

#endif

/*!
 @}
 */
#ifdef __cplusplus
}
#endif
#endif /* __ALISLINPUT_H__ */
