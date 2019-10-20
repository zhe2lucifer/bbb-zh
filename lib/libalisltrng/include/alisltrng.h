/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file			alisltrng.h
 *  @brief			declaration of ALi trng hardware interface
 *
 *  @version		1.0
 *  @date			06/04/2013 02:54:32 PM
 *  @revision		none
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 */

#ifndef __ALISLTRNG_INTERFACE__H_
#define __ALISLTRNG_INTERFACE__H_

#include <stdbool.h>

#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Support length of hardware */
typedef enum alisltrng_bytes {
	ALISLTRNG_BYTE_1	= 1,
	ALISLTRNG_BYTES_8	= 8,
} alisltrng_bytes_t;

/** Parameters of the devices initialization */
typedef struct alisltrng_param {
	int				reserved;
	/* Get a series random data */
	bool			series;
} alisltrng_param_t;

/** Device descriptor */
typedef struct alisltrng_dev {
	char			*dev_name;
	int				handle;
	unsigned char	*buf;
	/* Get a series random data */
	bool			series;
} alisltrng_dev_t;

/**
 *  Function Name:	alisltrng_construct
 *  @brief			construct ALi hardware trng descriptor
 *
 *  @param			dev		descriptor of the ALi trng
 *  @param			param	parameter used to initialize trng descriptor
 *
 *  @return			0		successful
 *  @return			others	error code
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/03/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_construct(alisltrng_dev_t *dev,
								  alisltrng_param_t *param);

/**
 *  Function Name:	alisltrng_destruct
 *  @brief			destroy the ALi trng descriptor
 *
 *  @param			dev		descriptor of the ALi trng
 *
 *  @return			always return 0 for successful
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_destruct(alisltrng_dev_t *dev);

/**
 *  Function Name:	alisltrng_ioctl
 *  @brief			command to ALi trng device
 *
 *  @param			dev		descriptor of ALi trng device
 *  @param			cmd		command to device
 *  @param			param	parameter of the command
 *
 *  @return			0		successful
 *  @return			others	trng error code
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_ioctl(alisltrng_dev_t *dev,
							  unsigned int cmd,
							  void *param);

/**
 *  Function Name:	alisltrng_get_rand_bytes
 *  @brief			get bytes rand from ALi trng hardware
 *
 *  @param			buf		buf for the rand data
 *  @param			len		bytes number to be got
 *
 *  @return			0		successful
 *  @return			others	trng error code
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_get_rand_bytes(unsigned char *buf,
									   alisltrng_bytes_t len);

/**
 *  Function Name:	alisltrng_get_rand_series
 *  @brief			get a series rand data, each have 8 bytes
 *
 *  @param			buf		buf for the rand series
 *  @param			count	series number to be got
 *
 *  @return			0		successful
 *  @return			others	trng error code
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_get_rand_series(unsigned char *buf, int count);

alisl_retcode alisltrng_get_rand(alisltrng_dev_t *dev,
		unsigned char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
