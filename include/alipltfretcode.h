/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file                       alipltfretcode.h
 *  @brief                      unified definition of return code type
 *
 *  @version                    1.0
 *  @date                       05/31/2013 03:02:41 PM
 *  @revision                   none
 *
 *  @author                     Zhao Owen <Owen.Zhao@alitech.com>
 */

#ifndef __ALIPLTFRETCODE_DEFINITION__H_
#define __ALIPLTFRETCODE_DEFINITION__H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Unified definition for return code */
typedef int alipltf_retcode;
typedef int alisl_retcode;

/*  Unified definition for module handle */
typedef void * alipltf_handle;
typedef void * alisl_handle;

/* modules ID */
#define ALISL_MOD_DEF(x) ALIPLTF_##x,
enum {
        #include "alisl_module.def"
        NUM_OF_SL_MODULES
};
#undef ALISL_MOD_DEF

#define ERROR_NONE      0
#define ERROR_NOMEM     1 /* memory malloc failed */
#define ERROR_NOPRIV    2 /* No private data struct */
#define ERROR_NOOP      3 /* No operation */
#define ERROR_OPEN      4 /* open failed */
#define ERROR_NOOPEN    5 /* device not opened */
#define ERROR_CFG       6 /* wrong config parameter */
#define ERROR_IOCTL     7 /* ioctl failed */
#define ERROR_TMOUT     8 /* timeout expired */
#define ERROR_WAIT      9 /* wait failed */
#define ERROR_NOPORTID  10 /* No portid specified */
#define ERROR_NOSTART   11 /* device not started */
#define ERROR_READ      12 /* read failed */
#define ERROR_WRITE     13 /* write failed */
#define ERROR_RESET     14 /* reset failed */
#define ERROR_NULLDEV   15 /* no dev data struct */
#define ERROR_INVAL     16 /* invalid parameter */
#define ERROR_PTCREATE  17 /* Create monitor pthread failed */
#define ERROR_PTCANCEL  18 /* Cancel monitor pthread failed */
#define ERROR_CHTYPE    19 /* invalid channel type */
#define ERROR_NOCH      20 /* no channel */
#define ERROR_NOFILTER  21 /* no filter */
#define ERROR_CHSTART   22 /* channel start failed */
#define ERROR_NOPID     23
#define ERROR_AVSTART   24
#define ERROR_ADDPID    25 /* adding PID failed */
#define ERROR_DELPID    26 /* deleting PID failed */
#define ERROR_STATUS    27 /* status error */
#define ERROR_CLOSE     28 /* close failed */
#define ERROR_MMAP      29 /* mmap failed */
#define ERROR_GETMEM    30 /* getting memory failed */
#define ERROR_RELMEM    31 /* releasing memory failed */
#define ERROR_GETMSG    32 /* get message failed */
#define ERROR_BADCB     33 /* invalid callback function */
#define ERROR_GETRES    34 /* getting resource failed */
#define ERROR_FAILED    35 /* failure */

#define ERROR_MKDIR     36
#define ERROR_MKDB      37
#define ERROR_INITDB    38
#define ERROR_NODB      39
#define ERROR_SEMGET    40
#define ERROR_SEMSET    41
#define ERROR_SEMOP     42
#define ERROR_SEMREAD   43
#define ERROR_NOSEM     44
#define ERROR_OPENDB    45
#define ERROR_EXEC      46

#ifdef __cplusplus
}
#endif

#endif
