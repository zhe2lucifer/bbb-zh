/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislcic.h
 *  @brief              declaration of ALi CI controller hardware interface
 *
 *  @version            1.0
 *  @date               07/04/2013 05:44:37 PM
 *  @revision           none
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 */

#ifndef __ALISLCIC_H__
#define __ALISLCIC_H__

/* system headers */
#include <inttypes.h>
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void* cicdev_handle;            /**< CI device struct handel */


enum cic_device_ioctrl_command {
    CIC_DRIVER_READMEM = 0,         /**< CIC driver read attribute memory command*/
    CIC_DRIVER_WRITEMEM,            /**< CIC driver write attribute memory command */
    CIC_DRIVER_READIO,              /**< CIC driver read byte from I/O command */
    CIC_DRIVER_WRITEIO,             /**< CIC driver write byte to I/O command  */
    CIC_DRIVER_TSIGNAL,             /**< CIC driver test a hardware PC card signal */
    CIC_DRIVER_SSIGNAL,             /**< CIC driver set or clear a PC card pin */
    CIC_DRIVER_REQMUTEX,             /**< When CI and ATA share pin, they need mutex protection*/
    CIC_DRIVER_SETPORT				/**< set socket communication port */
};

struct cic_io_command_memrw {
    int            slot;            /**< Slot */
    uint16_t       addr;            /**< Attribute memory address */
    uint16_t       size;            /**< Number of bytes of buffer */
    unsigned char* buffer;          /**< Pointer of the data buffer */
    uint16_t       *rsize;          /**< Number of bytes actually in data buffer */
};

struct cic_io_command_iorw {
    int            slot;            /**< Slot */
    uint16_t       reg;             /**< CI register */
    unsigned char* buffer;          /**< CI register value */
};

struct cic_io_command_signal {
    int             slot;             /**< Slot */
    unsigned char signal;           /**< CI signal */
    unsigned char status;           /**< CI signal status */
};

typedef void (*p_aliplsl_fun_cb)(int slot);

/**
 *  Function Name:      alislcic_construct
 *  @brief              construct some needed data or struct
 *
 *  @param handle       pointer to struct cic_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note
 */
alisl_retcode alislcic_construct(alisl_handle* handle);

/**
 *  Function Name:      alislcic_open
 *  @brief              This function is used to open a CI controller device
 *                      identified by device structure pointer *dev. After call
 *                      this function, upper layer can operate the CI controller
 *                      and CAM.
 *
 *  @param handle       The handle's device will be opened.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/05/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_open(alisl_handle handle);

/**
 *  Function Name:      alislcic_close
 *  @brief              This function used to close a CI controller device
 *                      identified by device structure pointer *dev. After call
 *                      this function, all operation for this device, include
 *                      CAM card operation, are invalid.
 *
 *  @param  handle      The handle's device will be closed.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/08/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_close(alisl_handle handle);

/**
 *  Function Name:      alislcic_read_io_data
 *  @brief              This function used to read "size" bytes data to *buffer
 *                      from the CI command interface data register which indicated
 *                      by *dev.
 *
 *
 *  @param  handle      CI controller device handle
 *  @param  slot        The slot index, value must between [0, MAX_CI_SLOT_NUM)
 *  @param  size        Data's size which will be readed (byte)
 *  @param  buffer      Buffer allocated by up layer used to store the read data.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note               It only read CI block data, but we also need detect
 *                      the other register's status include size_low. size_high
 *                      and status register. So before invoking this funciton ,up
 *                      layer must implement the CI physic layer read operation
 *                      (check status, get read data size and read data out, etc).
 *
 */
alisl_retcode alislcic_read_io_data(alisl_handle handle, uint32_t slot, 
                                    size_t size, unsigned char *buffer);

/**
 *  Function Name:      alislcic_write_io_data
 *  @brief              This function used to write "size" bytes data from *buffer
 *                      into the CI command interface data register which indicated
 *                      by *dev.
 *
 *
 *  @param  handle      CI controller device handle
 *  @param  slot        Ioctl message index
 *  @param  size        Data's size which will be writed (byte)
 *  @param  buffer      Buffer allocated by up layer used to store the write data.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note               It only write CI block data, but we also need detect
 *                      the other register's status include size_low. size_high
 *                      and status register. So before invoking this funciton ,up
 *                      layer must implement the CI physic layer write operation
 *                      (check status, get read data size and read data out, etc).
 *
 */
alisl_retcode alislcic_write_io_data(alisl_handle handle, uint32_t slot, 
                                     size_t size, unsigned char *buffer);

/**
 *  Function Name:      alislcic_ioctl
 *  @brief              command to ALi cic device
 *
 *  @param handle       descriptor of ALi cic device
 *  @param cmd          command to device
 *  @param param        parameter of the command
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/16/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_ioctl(alisl_handle handle, unsigned int cmd, unsigned long param);

/**
 *  Function Name:      alislcic_destruct
 *  @brief              release pre-constructed data or structure
 *
 *  @param handle       pointer to struct cic_device
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/15/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_destruct(alisl_handle handle);

/**
 *  Function Name:      alislcic_write_mem
 *  @brief              write cic device memory data
 *
 *  @param handle       CI controller device handle
 *  @slot      			slot number
 *  @size       		data size writing to cic device
 *  @addr        		memory start offset
 *  @buffer        		data buffer pointer to writen data
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/15/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_write_mem(alisl_handle handle, uint32_t slot, 
                                 size_t size, unsigned short addr, unsigned char *buffer);

/**
 *  Function Name:      alislcic_read_mem
 *  @brief              read cic device memory data
 *
 *  @param handle       CI controller device handle
 *  @slot      			slot number
 *  @size       		data size read from cic device
 *  @addr        		memory start offset
 *  @buffer        		data buffer pointer to writen data

 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note               
 *
 */
alisl_retcode alislcic_read_mem(alisl_handle handle, uint32_t slot, 
                                size_t size, unsigned short addr, unsigned char *buffer);



/**
 *  Function Name:      alislcic_write_io_reg
 *  @brief              write cic device register
 *
 *  @param handle       CI controller device handle
 *  @slot      			slot number
 *  @reg       		 	register address
 *  @buffer        		data buffer pointer to writen data

 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/15/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_write_io_reg(alisl_handle handle, uint32_t slot, 
                                    uint32_t reg, unsigned char *buffer);

/**
 *  Function Name:      alislcic_read_io_reg
 *  @brief              read cic device register
 *
 *  @param handle       CI controller device handle
 *  @slot      			slot number
 *  @reg       		 	register address
 *  @buffer        		data buffer pointer to save read data

 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               07/15/2013, Created
 *
 *  @note
 */
alisl_retcode alislcic_read_io_reg(alisl_handle handle, uint32_t slot, 
                                   uint32_t reg, unsigned char *buffer);

/**
 *  Function Name:      alislcic_test_signal
 *  @brief              write cic device status
 *
 *
 *  @param  handle      CI controller device handle
 *  @param  slot        The slot index, value must between [0, 1]
 *  @param  signal      status type
 *  @param  buffer      Buffer allocated by up layer used to store the read data.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note               
 *
 */
 alisl_retcode alislcic_test_signal(alisl_handle handle, uint32_t slot, 
                                   uint32_t signal, unsigned char *status);

/**
 *  Function Name:      alislcic_set_signal
 *  @brief              set cic device status
 *
 *
 *  @param  handle      CI controller device handle
 *  @param  slot        The slot index, value must between [0, 1)
 *  @param  signal      status type
 *  @param  buffer      Buffer allocated by up layer used to store the writing data.
 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note               
 *
 */
alisl_retcode alislcic_set_signal(alisl_handle handle, uint32_t slot, 
                                  uint32_t signal, unsigned char status);


/**
 *  Function Name:      alislcic_register_callback
 *  @brief              register user callback
 *
 *
 *  @param  p_callback_init      user callback functon pointer

 *
 *  @return             alisl_retcode
 *
 *  @author             Adolph liu  <adolph.liu@alitech.com>
 *  @date               2015/07/21, Created
 *
 *  @note               
 *
 */

alisl_retcode alislcic_register_callback(p_aliplsl_fun_cb p_callback_init);


#ifdef __cplusplus
}
#endif

#endif /* __ALISLCIC_H__ */
