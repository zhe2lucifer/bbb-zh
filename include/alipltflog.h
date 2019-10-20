#ifndef __ALIPLTFLOG_H__
#define __ALIPLTFLOG_H__

/* SL_MOD is defined in the Makefile */
#if (defined ENABLE_DEBUG) && (defined SL_MOD)

#include <stdarg.h>
#include <alipltfretcode.h>
#include <syslog.h>

/* only called through below macro */
void alisl_log(int module, int prio, int line,
           const char *func, const char *fmt, ...);

/* only called through below macro */
void alisl_dump(int module, int line, const char *func,
        const char *prompt, char *data, int len);

/* only called through below macro */
void alisl_log_retcode(int module, int line,
               const char *func, int errcode, const char *fmt, ...);

#define SL_DBG(...) \
    alisl_log(SL_MOD, LOG_DEBUG, __LINE__, __func__, __VA_ARGS__)

#define SL_ERR(...) \
    alisl_log(SL_MOD, LOG_ERR, __LINE__, __func__, __VA_ARGS__)

#define SL_WARN(...) \
    alisl_log(SL_MOD, LOG_WARNING, __LINE__, __func__, __VA_ARGS__)

#define SL_INFO(...) \
    alisl_log(SL_MOD, LOG_INFO, __LINE__, __func__, __VA_ARGS__)

#define SL_DUMP(prompt, data, len) \
    alisl_dump(SL_MOD, __LINE__, __func__, prompt, data, len)

/* log the string associated to the error code */
#define SL_DBG_CODE(err, ...) \
    alisl_log_retcode(SL_MOD, __LINE__, __func__, err, __VA_ARGS__)

#else

#define alisl_log_level_refresh(void) do { } while(0)

#define SL_DBG(...) do { } while(0)
#define SL_ERR(...) do { } while(0)
#define SL_WARN(...) do { } while(0)
#define SL_INFO(...) do { } while(0)
#define SL_DUMP(...) do { } while(0)
#define SL_DBG_CODE(...) do { } while(0)

#endif /* ENABLE_DEBUG */

#endif
