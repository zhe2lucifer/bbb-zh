/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislgpio.h
 *  @brief              ALi gpio function interfaces
 *
 *  @version            1.0
 *  @date               07/17/2016 12:05:43 PM
 *  @revision           none
 *
 *  @author             Steven Zhang <steven.zhang@alitech.com>
 */

#ifndef __ALISLGPIO__H_
#define __ALISLPPIO__H_

/* system headers */
#include <inttypes.h>
#include <stdbool.h>

/* share library headers */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum gpio_interrupt_type {
		/**
		No interrupt.
		*/
		GPIO_INTERRUPT_DISABLED = 0,
		/**
		Interrupt on a 0->1 transition.
		*/
		GPIO_INTERRUPT_RISING_EDGE,
		/**
		Interrupt on a 1->0 transition.
		*/
		GPIO_INTERRUPT_FALLING_EDGE,
		/**
		Interrupt on both a 0->1 and a 1->0 transition.
		*/
		GPIO_INTERRUPT_EDGE,
		/**
		Max interrupt type number.
		*/
		GPIO_INTERRUPT_MAX,
	
} gpio_interrupt_type;

typedef void(* gpio_cbfuc)(int gpio_index, gpio_interrupt_type interrupt_type);

typedef struct gpio_io_reg_callback_param 
{
	gpio_interrupt_type irq_type;
    gpio_cbfuc p_cb;
    void *pv_param;
	int gpio_index;
}gpio_io_reg_callback_param;

/**
 *  Function Name:      		alislgpio_open
 *  @brief                   	open actual hardware device
 *
 *  @param handle      	 	pointer to module handle
 *
 *  @return                 	alisl_retcode
 *
 *  @author                 	Steven Zhang <steven.zhang@alitech.com>
 *  @date                  		2016.07.14, Created
 *
 *  @note
 */
alisl_retcode alislgpio_open(alisl_handle *handle);

/**
 *  Function Name:      		alislgpio_close
 *  @brief                   	open actual hardware device
 *
 *  @param handle       	pointer to module handle
 *
 *  @return                		alisl_retcode
 *
 *  @author               		Steven Zhang <steven.zhang@alitech.com>
 *  @date                  		2016.07.14, Created
 *
 *  @note
 */

alisl_retcode alislgpio_close(alisl_handle *handle);

/**
 *  Function Name:      		alislgpio_callback_reg
 *  @brief                  		set the gpio reg call back funtion, it will be falling or rising.
 *  @author                	Steven Zhang <steven.zhang@alitech.com>
 *  @date                  		2016.07.14, Created

 *  @param[in]           		hdl is gpio device pointer
 *  @param[in]           		gpio_io_reg_callback_param is the param of interrupt callback will be register.
 *  @param[out]
 *  @return                		alisl_retcode
 *  @note
 *
 */
alisl_retcode alislgpio_callback_reg(alisl_handle hdl, gpio_io_reg_callback_param *regcb_param);

/**
 *  Function Name:     		alislgpio_callback_unreg
 *  @brief                 		set the gpio unreg call back funtion, it will be falling or rising.
 *  @author              		Steven Zhang <steven.zhang@alitech.com>
 *  @date                 		2016.07.14, Created

 *  @param[in]          	 	hdl is gpio device pointer
 *  @param[in]           		index: the index of gpio will be unregister.
 *  @param[out]
 *  @return               		alisl_retcode
 *  @note
 *
 */
alisl_retcode alislgpio_callback_unreg(alisl_handle hdl, unsigned int index);


#ifdef __cplusplus
}
#endif

#endif /* __ALISLGPIO__H_ */
