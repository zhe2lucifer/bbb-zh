/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislwatchdog.h
 *  @brief
 *
 *  @version            1.0
 *  @date               2017/2/15 15:05:40 PM
 *  @revision           none
 */

#ifndef __ALISLWATCHDOG_INTERFACE__H_
#define __ALISLWATCHDOG_INTERFACE__H_

/* Platform header */
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif


alisl_retcode alislwatchdog_reboot(unsigned int reboot_time);
alisl_retcode alislwatchdog_start(void);
alisl_retcode alislwatchdog_stop(void);
alisl_retcode alislwatchdog_feed_dog(unsigned int time_us);
int alislwatchdog_get_time_left(void);
void alislwatchdog_set_duration_time(unsigned int duration_time);


/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

typedef enum alislwatchdog_status {
    ALISLWATCHDOG_INACTIVATE,
    ALISLWATCHDOG_ACTIVATE
} alislwatchdog_status_t;

typedef enum alislwatchdog_cmd {
    ALISLWATCHDOG_CMD_REBOOT_DOG,
    ALISLWATCHDOG_CMD_START_DOG,
    ALISLWATCHDOG_CMD_FEED_DOG,
    ALISLWATCHDOG_CMD_STOP_DOG,
    ALISLWATCHDOG_CMD_SET_TIMEOUT,
    ALISLWATCHDOG_CMD_GET_TIMELEFT
} alislwatchdog_cmd_t;

typedef enum alislwatchdog_io_param {
    ALISLWATCHDOG_PARAM_DISABLE = 0x0001,
    ALISLWATCHDOG_PARAM_ENABLE  = 0x0002
} alislwatchdog_io_param_t;

typedef struct alislwatchdog_param {
    unsigned char status;
    unsigned char rev;
} alislwatchdog_param_t;

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
