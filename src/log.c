#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <alipltfretcode.h>

#define SL_MOD 1
#include <alipltflog.h>
#undef SL_MOD

#define LOG_NONE (-1)

// magic setting for CI test
static int g_log_print_output_only = 0;
static char *sl_log_header[] = {
    "_", // 0
    "SL.LOG.ALERT   :", // 1
    "SL.LOG.CRITICAL:", // 2
    "SL.LOG.ERROR   :", // 3
    "SL.LOG.WARNING :", // 4
    "SL.LOG.NOTICE  :", // 5
    "SL.LOG.INFO    :", // 6
    "SL.LOG.DEBUG   :", // 7
};

#define PRINT_CONS(prio, str) if(g_log_print_output_only) printf("%s %s", sl_log_header[prio], str)
#define PRINT(prio, str) if(g_log_print_output_only==0) syslog(prio, "%s", str)

static int alisl_module_dbg_level[NUM_OF_SL_MODULES] = { LOG_NONE };

/* define module names */
#define ALISL_MOD_DEF(x) "SL_"#x,

static char *module_name[] = {
    #include "alisl_module.def"
};

#define LOG_MAX_LEN 512
#undef ALISL_MOD_DEF

int alisl_log_init(void)
{
    memset(alisl_module_dbg_level, 0, sizeof(alisl_module_dbg_level));
    return 0;
}

void alisl_log_set_priority(int module, int prio)
{
    // The following setting is for CI test only, use print to output the log, so that CI can get the output and send to server.
    if ((module == -1) && (prio == -1)) {
        // magic setting for CI test
        g_log_print_output_only = 1;
        return;
    }
    if ((module == -1) && (prio == 0)) {
        // magic setting for CI test
        g_log_print_output_only = 0;
        return;
    }

    if ((module < 0) || (module >= NUM_OF_SL_MODULES))
        return;

    alisl_module_dbg_level[module] = prio;
}

void alisl_log(int module, int prio, int line,
           const char *func, const char *fmt, ...)
{
    char str[LOG_MAX_LEN + 1] = {0};

    if (module >= NUM_OF_SL_MODULES)
        return;

    if (prio > alisl_module_dbg_level[module])
        return;

    snprintf(str, LOG_MAX_LEN, "%s %s:%d", module_name[module], func, line);

    // If print content is not NULL
    if (fmt) {
        va_list args;
        strcat(str, " ");
        va_start(args, fmt);
        vsnprintf(str + strlen(str), LOG_MAX_LEN - strlen(str), fmt, args);
        va_end(args);
    } else {
        strcat(str, " (null)");
    }

    if (str[strlen(str)-1] != '\n')
        strcat(str, "\n");

    // Temp solution before finish configuration the syslog output to console
    PRINT_CONS(prio, str);
    PRINT(prio, str);
}

void alisl_log_retcode(int module, int line,
               const char *func, int err, const char *fmt, ...)
{
    char str[LOG_MAX_LEN + 1] = {0};

    if (module >= NUM_OF_SL_MODULES)
        return;

    if (LOG_DEBUG > alisl_module_dbg_level[module])
        return;

    snprintf(str, LOG_MAX_LEN, "%s %s:%d err %d", module_name[module], func, line, err);
    if (fmt) {
        va_list args;
        strcat(str, " ");
        va_start(args, fmt);
        vsnprintf(str + strlen(str), LOG_MAX_LEN - strlen(str), fmt, args);
        va_end(args);
    } else {
        strcat(str, " (null)");
    }

    if (str[strlen(str)-1] != '\n')
        strcat(str, "\n");

    // Temp solution before finish configuration the syslog output to console
    PRINT_CONS(LOG_DEBUG, str);
    PRINT(LOG_DEBUG, str);
}

void alisl_dump(int module, int line, const char *func,
        const char *prompt, char *data, int len)
{
    int k, l = len;
    char str[LOG_MAX_LEN + 1], s[20];

    if (module >= NUM_OF_SL_MODULES)
        return;

    if (LOG_DEBUG > alisl_module_dbg_level[module])
        return;

    alisl_log(module, LOG_DEBUG, line, func, "%s", prompt);

    str[0] = 0;
    for(k = 0; k < l; k++) {
        sprintf(s, "0x%02X ",(unsigned char)data[k]);
        strncat(str, s, LOG_MAX_LEN - strlen(str));
        if ((k+1) % 16 == 0) {
            strcat(str, "\n");
            
            // Temp solution before finish configuration the syslog output to console
            PRINT_CONS(LOG_DEBUG, str);
            PRINT(LOG_DEBUG, str);
            str[0] = 0;
        }

    }
    if((k)%16!=0) {
        strcat(str, "\n");
        // Temp solution before finish configuration the syslog output to console
        PRINT_CONS(LOG_DEBUG, str);
        PRINT(LOG_DEBUG, str);
    }
}
