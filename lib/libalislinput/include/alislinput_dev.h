/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan_dev.h
*
*    Description:    This file contains pan_device structure define in HLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#ifndef __ALISLINPUT_DEV_H__
#define __ALISLINPUT_DEV_H__

#ifndef HLD_MAX_NAME_SIZE
#define HLD_MAX_NAME_SIZE 16
#endif

#ifdef __cplusplus
extern "C" {
#endif
//#include <hld/hld_dev.h>

/*! @addtogroup pan
 *  @{
 */

/*! @enum alislinput_device_panel_ioctrl_command
    @brief 前面板IO命令集。
*/
enum alislinput_device_panel_ioctrl_command
{
	PAN_DRIVER_ATTACH	= 0,			//!<Front panel driver attach command
	PAN_DRIVER_SUSPEND	= 1,			//!<Front panel driver suspend command
	PAN_DRIVER_RESUME	= 2,			//!<Front panel driver resume command
	PAN_DRIVER_DETACH	= 3,			//!<Front panel driver detach command
	PAN_DRIVER_READ_LNB_POWER = 4,		//!<Front panel driver NIM LNB power protect status
	PAN_DRIVER_WRITE_LED_ONOFF =5,
	PAN_DRIVER_UART_SELECT = 6,
	PAN_DRIVER_SET_GPIO	= 7,			//!<Front panel driver set gpio command
	PAN_DRIVER_GET_GPIO = 8,			//!<Front panel driver get gpio value
	PAN_DRIVER_STANDBY  = 9,        	//!<Front panel driver enter low power mode
	PAN_DRIVER_SK_DETECT_POLAR = 10,			//!<Front panel driver set key detect polor, only enable for shadow scan
	PAN_DRIVER_SET_HW_INFO	= 11,		//!<Front panel driver set hw info command
	PAN_DRIVER_GET_HW_INFO	= 12,		//!<Front panel driver get hw info command
	PAN_DRIVER_WRITE_REG	= 13,		//!<Front panel driver write register command
	PAN_DRIVER_READ_REG	= 14,			//!<Front panel driver read register command
	PAN_DRIVER_SET_I2C	= 15,			//!<Front panel driver set command, byte0, I2C ID; byte1, 1 gpio_i2c, 0 i2c byte2, SDA; byte3, SCL;
	PAN_DRIVER_SET_DIG_ADDR = 16,	//!<Front panel driver set command, byte0, dig0_addr; byte1, dig1_addr; byte2, dig2_addr; byte3, dig3_addr.
	PAN_DRIVER_SET_LED_BITMAP = 17,	//!<Front panel driver set command, set led bit map.
};

/*! @enum alislinput_device_ir_ioctrl_command
    @brief 前面板IO命令集。
*/
enum alislinput_device_ir_ioctrl_command
{
	PAN_DRIVER_IR_SET_STANDBY_KEY	= 0xfff0,		//!<Remote controller driver set standby key command
	PAN_DRIVER_IR_SET_ENDIAN			,			//!<Remote controller driver set endian command
	PAN_DRIVER_IR_SET_RESUME_KEY		,		//!<Remote controller driver set resume key command
	PAN_DRIVER_IR_SET_EMOUSE		    ,		//!<Remote controller driver set emulate mouse speed...
};

/*! @enum alislinput_key_type
    @brief 前面板按键类型。
*/
typedef enum alislinput_key_type
{
	PAN_KEY_TYPE_INVALID	= 0,	//!<Invalid key type
	PAN_KEY_TYPE_REMOTE	= 1,	//!<Remote controller
	PAN_KEY_TYPE_PANEL	= 2,	//!<Front panel
	PAN_KEY_TYPE_JOYSTICK	= 3,	//!<Game joy stick
	PAN_KEY_TYPE_KEYBOARD	= 4	//!<Key board
}alislinput_key_type_e;

/*! @enum alislinput_key_press
    @brief 前面板按键状态。
*/
typedef enum alislinput_key_press
{
	PAN_KEY_RELEASE		= 0,
	PAN_KEY_PRESSED		= 1,
	PAN_KEY_REPEAT		= 2
}alislinput_key_press_e;

/*! @enum alislinput_ir_protocol
    @brief 支持的红外遥控协议。
*/
typedef enum alislinput_ir_protocol
{
	IR_TYPE_NEC = 0,
	IR_TYPE_LAB,
	IR_TYPE_50560,
	IR_TYPE_KF,
	IR_TYPE_LOGIC,
	IR_TYPE_SRC,
	IR_TYPE_NSE,
	IR_TYPE_RC5,
	IR_TYPE_RC6,
	IR_TYPE_UNDEFINE
}alislinput_ir_protocol_e;

/*! @enum alislinput_device_status
    @brief 前面板设备状态。
*/
typedef enum alislinput_device_status
{
	PAN_DEV_ATTACH = 0,
	PAN_DEV_OPEN,
	PAN_DEV_CLOSE,
	PAN_DEV_DETACH
}alislinput_device_status_e;

/*! @struct alislinput_device_stats
    @brief 前面板设备状态。
*/
struct alislinput_device_stats
{
	unsigned short	display_num;		//!<Number of display data
};

/*!@struct alislinput_key
   @brief 前面板按键信息。
*/
struct alislinput_key
{
	unsigned char  type;			//!<The key type
	unsigned char  state;			//!<The key press state
	unsigned short count;			//!<The key counter
	unsigned long code;			//!<The value
};

/*!@struct alislinput_key_index
   @brief 前面板按键信息及其索引。
*/
struct alislinput_key_index
{
	struct alislinput_key  key_value;	//!<The pan key
	unsigned short key_index;		//!<The key index
};

/*!@struct alislinput_key_info
   @brief 前面板按键信息。
*/
struct alislinput_key_info
{
	unsigned long  code_high;			//!<The key value of high
	unsigned long  code_low;				//!<The key value of low
	alislinput_ir_protocol_e protocol;		//!<The ir protocol
	alislinput_key_press_e state;			//!<The key press state
	alislinput_key_type_e type;			//!<The key press type
};

/*!@struct alislinput_standby_key
   @brief 前面板待机按键信息。
*/
struct alislinput_standby_key{
	unsigned char enable;
	unsigned long key;
};

/*!@struct alislinput_ir_endian
   @brief 前面板红外协议大小端模式。
*/
struct alislinput_ir_endian{
	unsigned int enable;
	alislinput_ir_protocol_e protocol;
	unsigned int bit_msb_first;
	unsigned int byte_msb_first;
};

/*!@struct alislinput_device
   @brief 前面板驱动模块设备节点。
*/
struct alislinput_device
{
//	struct hld_device   *next;			//!<Next device structure
	unsigned long		type;				//!<Interface hardware type
	char		name[HLD_MAX_NAME_SIZE];	//!<Device name

	unsigned short		flags;				//!<Interface flags, status and ability

	unsigned short		led_num;			//!<Number of LED
	unsigned short		key_num;			//!<Number of input keys in Front panel

	/* Hardware privative structure */
	void		*priv;				//!<pointer to private data

	/*
	 *  Functions of this panel device
	 */
	long	(*init)();

	long	(*open)(struct alislinput_device *dev);

	long	(*stop)(struct alislinput_device *dev);

	void	(*display)(struct alislinput_device *dev, char *data, unsigned long len);

	long	(*send_data)(struct alislinput_device *dev, unsigned char *data, unsigned long len, unsigned long timeout);

	long	(*receive_data)(struct alislinput_device *dev, unsigned char *data, unsigned long len, unsigned long timeout);

	long	(*do_ioctl)(struct alislinput_device *dev, long cmd, unsigned long param);

	struct alislinput_device_stats* (*get_stats)(struct alislinput_device *dev);
};

/*!
 @}
 */
#ifdef __cplusplus
	}
#endif

#endif  /* __ALISLINPUT_DEV_H__ */
