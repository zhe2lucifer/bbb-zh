/**@file
 *  (c) Copyright 2016-2999  ALi Corp. ZHA Linux AUI Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisli2c.h
 *  @brief             
 *
 *  @version            1.0
 *  @date               09/30/2016 15:44:37 
 *  @revision           none
 *
 *  @author             Alfa.Shang  <Alfa.Shang@alitech.com>
 */

#ifndef __ALISLI2C_H__
#define __ALISLI2C_H__


/* system headers */
#include "alipltfretcode.h"

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <assert.h> 
#include <errno.h>
#include <string.h>


//#define SL_I2C_DEBUG
#define I2C_DEFAULT_TIMEOUT		1               //the default value that driver provided
#define I2C_DEFAULT_RETRY		3

#define i2c_printf              SL_DBG

#define WRITE                   0
#define ALI_RTN_SUCCESS         0
#define ALI_RTN_FAIL            -1  
#define MAX_DATA_LENGTH         16


#define IDX_0   "/dev/i2c-0"                     //define the the i2c-0 hardware i2c bus device 
#define IDX_1   "/dev/i2c-1"                     //define the the i2c-1 hardware i2c bus device
#define IDX_2   "/dev/i2c-2"                     //define the the i2c-2 hardware i2c bus device
#define IDX_3   "/dev/i2c-3"                     //define the the i2c-3 hardware i2c bus device
#define IDX_4   "/dev/i2c-4"                     //define the the i2c-4 gpio simulating i2c bus device
#define IDX_5   "/dev/i2c-5"                     //define the the i2c-5 gpio simulating i2c bus device

/**
@brief @struct i2c_handle
        Struct of the <b> I2c Module </b> to store the information of opened i2c bus device
*/
typedef struct i2c_handle{                  
/** 
	Member store the returned value of function @b open().
*/
	 int fd;                                    
}alisli2c_handle;

/**
@brief @struct alisli2c_attr
	Struct of the <b> I2c Module </b> to store the information of user input to configure
	the opened i2c bus device.
*/
typedef struct alisli2c_attr {               
      /**
      Member to specify which @b idx of i2c bus device is to open
																					  
      @note  the value range is :0-5, the 0-3 represent hardware i2c bus device; the 4-5 
               represent the gpio simulated i2c bus device.
                     0 -------------hardware i2c device
                     1--------------hardware i2c device
                     2--------------hardware i2c device
                     3--------------hardware i2c device
                     4--------------gpio simulated i2c device
                     5--------------gpio simulated i2c device
       */
    unsigned int uc_dev_idx; 

} alisli2c_attr;


/**
 *  Function Name:      alisli2c_read_data
 *  @brief              This function used to read "buf_len" bytes data to "*buf" from the "sub_addr" address of
 *                         the slave device, which address is "chip_addr", through the i2c bus device, which is pointed by handle.
 *                     

 *  @param    handle            point to the i2c handle, store the information of the opened i2c bus device.
 *  @param    chip_addr        the address of the slave device, which should be mounted on the opened i2c bus device.
 *  @param    sub_addr        the register address in the slave device, which store the data to be read.
 *  @param    *buf               the string pointer, which used to store the read data. the length of the read data should be less than 16 bytes
 *  @param    buf_len           the length of the data to be read. The value should be no more than 16 (bytes)
 *  @param    subaddr_flag    the sub address flag, "subaddr_flag = 1" means having data register, and the value of sub_addr is effective; 
 *                                       "subaddr_flag = 0" means having no data register, and the value of sub_addr is ineffective.
 *  @return    alisl_retcode

 *  @author    Alfa Shang  <alfa.shang@alitech.com>
 *  @date       09/30/2016, Created

 *  @note      Make sure the salve device is mounted on the opened i2c bus device, and the register address and the "buf_len" bytes addressbehind 
                   it should support read operation.
 */
alisl_retcode alisli2c_read_data( alisl_handle handle, unsigned int chip_addr,
                                   unsigned int sub_addr, unsigned char *buf, unsigned int buf_len, unsigned int subaddr_flag);

/**
 *  Function Name:      alisli2c_write_data
 *  @brief              This function used to write "buf_len" bytes data "*buf" to "sub_addr" register address of
 *                         the slave device, which address is "chip_addr", through the i2c bus device, which is pointed by handle.
                   
 *  @param    handle            point to the i2c handle, store the information of the opened i2c bus device.
 *  @param    chip_addr        the address of the slave device, which should be mounted on the opened i2c bus device.
 *  @param    sub_addr        the register address in the slave device, which store the data to be wrote.
 *  @param    *buf               the string pointer, which used to store the wrote data. the length of the wrote data should be less than 16 bytes
 *  @param    buf_len           the length of the data to be wrote. The value should be no more than 16 (bytes).
 *  @param    subaddr_flag    the sub address flag, "subaddr_flag = 1" means having data register, and the value of sub_addr is effective; 
 *                                       "subaddr_flag = 0" means having no data register, and the value of sub_addr is ineffective.

 *  @return    alisl_retcode

 *  @author    Alfa Shang  <alfa.shang@alitech.com>
 *  @date       09/30/2016, Created

 *  @note      Make sure the salve device is mounted on the opened i2c bus device, and the register address and the "buf_len" bytes address behind 
 *                  it should support read operation.
 */
 
alisl_retcode alisli2c_write_data( alisl_handle handle, unsigned int chip_addr,
                                   unsigned int sub_addr, unsigned char *buf, unsigned int buf_len, unsigned int subaddr_flag);

/**
 *  Function Name:      alisli2c_write_read
 *  @brief              This function is used to write "write_buf_len" bytes data "*write_buf" to "sub_addr"data register address 
 *                     of the slave device, which address is "chip_addr", through the i2c bus device, which is pointed by handle.
 *                         And this function can be used to read "read_buf_len" bytes data to "*read_buf" from the "sub_addr" address
 *                     of the slave device, which address is "chip_addr", through the i2c bus device, which is pointed by handle. 
 *                         
                   
 *  @param    handle            point to the i2c handle, store the information of the opened i2c bus device.
 *  @param    chip_addr        the address of the slave device, which should be mounted on the opened i2c bus device.
 *  @param    sub_addr        the register address in the slave device, which store the data to be wrote.
 *  @param    *write_buf               the string pointer, which used to store the wrote data. the length of the wrote data should no more than 16 bytes
 *  @param    write_buf_len           the length of the data to be wrote. The value should no more than 16 (bytes)
 *  @param    *read_buf               the string pointer, which used to store the wrote data. the length of the wrote data should no more than 16 bytes
 *  @param    read_buf_len           the length of the data to be wrote. The value should be no more than 16 (bytes).
 *  @param    subaddr_flag    the sub address flag, "subaddr_flag = 1" means having data register, and the value of sub_addr is effective; 
 *                                       "subaddr_flag = 0" means having no data register, and the value of sub_addr is ineffective.

 *  @return    alisl_retcode

 *  @author    Alfa Shang  <alfa.shang@alitech.com>
 *  @date       09/30/2016, Created

 *  @note      Make sure the salve device is mounted on the opened i2c bus device, and the register address and the "buf_len" bytes address behind 
 *                  it should support read operation.
 */

alisl_retcode alisli2c_write_read(alisl_handle handle, unsigned int chip_addr, unsigned int sub_addr, 
	                            unsigned char *write_buf, unsigned int write_buf_len, unsigned char *read_buf, 
	                            unsigned int read_buf_len, unsigned int subaddr_flag);

/**
 *  Function Name:      alisli2c_open
 *  @brief              This function is used to open i2c bus device. And set the timeout and retry times
 *  
 *  @param p_attr        The struct contains the needed information to open i2c bus device.
 *  @param handle       The handle's device will be opened.
 *
 *  @return             alisl_retcode
 *
 *  @author             Alfa Shang  <alfa.shang@alitech.com>
 *  @date               09/30/2016, Created
 *
 *  @note
 */
alisl_retcode alisli2c_open(alisli2c_attr *p_attr, alisl_handle *handle);

/**
 *  Function Name:      alisli2c_close
 *  @brief              This function is used to close i2c bus device. And set the timeout and retry times
 *  
 *  @param handle       The handle's device will be closed.
 *
 *  @return             alisl_retcode
 *
 *  @author             Alfa Shang  <alfa.shang@alitech.com>
 *  @date               09/30/2016, Created
 *
 *  @note
 */
alisl_retcode alisli2c_close(alisl_handle handle);

#endif
