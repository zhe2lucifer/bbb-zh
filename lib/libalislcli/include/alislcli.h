/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               cli.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               01/13/2014 10:29:59 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */
#ifndef __CLI_H_
#define __CLI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define CLI_CMD_MODE_SELF       0
#define CLI_CMD_MODE_ALL        1

#define CLI_LOG_ERROR           (1<<1)

#define MAX_HISTORY             40
#define MAX_CMD_BUFFER_LEN      1024

struct cli_command {
	char                   *command;
	int                     (*callback)(int argc, char **argv);
	unsigned int            unique_len;
	char                   *help;
	struct cli_command     *next;
	struct cli_command     *children;
	struct cli_command     *parent;
	unsigned int            mode;
};


void                    dvb_cli_init(void);
void                    cli_init(void);
void                    cli_loop(void);
void                    cli_set_hostname(char *hostname);
int                     cli_run_command(char *command);
struct cli_command *    cli_find_command(char *command);
int                     cli_unregister_command(char *command);
struct cli_command *    cli_register_command(struct cli_command *parent,
						char *command,
						int (*callback)(int, char **),
						unsigned int mode,
						char *help);
char                    cli_check_result(const char *info, char success, char fail);
char                    cli_ignore_check_result(char ignore);
#ifdef __cplusplus
}
#endif

#endif
