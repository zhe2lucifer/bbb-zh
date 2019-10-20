/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           wrapper.c
 *  @brief          wrapper function for trng device
 *
 *  @version        1.0
 *  @date           06/04/2013 02:49:14 PM
 *  @revision       none
 *
 *  @author         Zhao Owen <Owen.Zhao@alitech.com>
 */

/* System header */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <alipltflog.h>

/* TRNG header */
#include <alisltrng.h>

/* Internal header */
#include "internal.h"

/**
 *  Function Name:  alisltrng_get_rand
 *  @brief          get rand data from ALi trng device
 *
 *  @param          buf     return buf for the rand data
 *  @param          len     bytes of the rand to be got
 *
 *  @return         0       successful
 *  @return         others  trng error code
 *
 *  @author         Zhao Owen <Owen.Zhao@alitech.com>
 *  @date           06/04/2013, Created
 *
 *  @note           Time-sensitive function, do not add any print.
 */
alisl_retcode alisltrng_get_rand(alisltrng_dev_t *dev,
                                        unsigned char *buf,
                                        size_t len)
{
    alisl_retcode ret = 0;
    //alisltrng_ioparam_t param;

    if (NULL == dev || NULL == buf || 0 == len) {
        ret = -1;
        return ret;
    }

    dev->buf = buf;
    memset(buf, 0, len);
    //param.data = dev->buf;
 #if 1
    ret = read(dev->handle, buf, len);  
    if (-1 == ret) {
        SL_ERR("Cannot get %d random\n", len); 
    } else {
        ret = 0;
    }
#else
    switch (len) {
        case ALISLTRNG_BYTE_1:
            ret = ioctl(dev->handle, ALI_TRNG_GENERATE_BYTE, dev->buf);
            if (0 != ret) {
                SL_ERR("In %s Cannot get a byte random\n",
                                __func__);
            }
            break;
        case ALISLTRNG_BYTES_8:
            if (false == dev->series) {
                ret = ioctl(dev->handle, ALI_TRNG_GENERATE_64bits, dev->buf);
            } else {
                param.n_group = 1;
                ret = alisltrng_ioctl(dev, ALI_TRNG_GET_64bits, &param);
            }
            if (0 != ret) {
                SL_ERR("Cannot get 8 bytes random\n");
            }
            break;
        default:
            /* We may want to get a series random data
             * and need to care about the len is the multiple
             * of 8 bytes  -- Zhao Owen -- */
            if (dev->series && (0 == (len % 8))) {
                param.n_group = len / 8;
                ret = alisltrng_ioctl(dev, ALI_TRNG_GET_64bits, &param);
                if (0 != ret) {
                    SL_ERR("Cannot get 8 bytes series random %d\n", len / 8);
                }
            } else {
                SL_ERR("Not support len %d rand currently\n", len);
                ret = ERROR_INVAL;
            }
            break;
    }
#endif
//    if (NULL == buf) free(dev->buf);
//    dev->buf = NULL;

    return ret;
}

/**
 *  Function Name:  alisltrng_get_rand_bytes
 *  @brief          get bytes rand from ALi trng hardware
 *
 *  @param          buf     buf for the rand data
 *  @param          len     bytes number to be got
 *
 *  @return         0       successful
 *  @return         others  trng error code
 *
 *  @author         Zhao Owen <Owen.Zhao@alitech.com>
 *  @date           06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_get_rand_bytes(unsigned char *buf,
                                       alisltrng_bytes_t len)
{
    alisl_retcode ret = 0;
    alisltrng_dev_t dev;
    alisltrng_param_t param;

    param.series = false;
    memset(&dev, 0x00, sizeof(alisltrng_dev_t));
    alisltrng_construct(&dev, &param);

    ret = alisltrng_get_rand(&dev, buf, len);

    alisltrng_destruct(&dev);
    return ret;
}

/**
 *  Function Name:  alisltrng_get_rand_series
 *  @brief          get a series rand data, each have 8 bytes
 *
 *  @param          buf     buf for the rand series
 *  @param          count   series number to be got
 *
 *  @return         0       successful
 *  @return         others  trng error code
 *
 *  @author         Zhao Owen <Owen.Zhao@alitech.com>
 *  @date           06/04/2013, Created
 *
 *  @note
 */
alisl_retcode alisltrng_get_rand_series(unsigned char *buf, int count)
{
    alisl_retcode ret = 0;
    alisltrng_dev_t dev;
    alisltrng_param_t param;

    param.series = true;
    memset(&dev, 0x00, sizeof(alisltrng_dev_t));
    alisltrng_construct(&dev, &param);

    ret = alisltrng_get_rand(&dev, buf, 8 * count);

    alisltrng_destruct(&dev);
    return ret;
}

