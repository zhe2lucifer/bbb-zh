#ifndef __ALI_KEYSET_H
#define __ALI_KEYSET_H
/* system header file */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
/* system header file */

/* ALi specific header file */
#include <ali_keyset_common.h>
/* ALi specific header file */

/* macro definition start */
#ifndef	RET_SUCCESS
#define RET_SUCCESS    0
#endif

#ifndef	RET_FAILURE
#define RET_FAILURE    -1
#endif
/* macro definition end */

/* functions definition start */
int do_get_keycode(cmd_info_t *cmd_info, struct ali_str2key_tbl_t *str2key_tbl);
int do_get_scancode(cmd_info_t *cmd_info, struct ali_str2key_tbl_t *str2key_tbl);
int get_code(char *_evdevname, struct ali_str2key_tbl_t *str2key_tbl, unsigned long val, unsigned long cmd);

void parse_option(int argc, char **argv, cmd_info_t *cmd_info);
void command_process(cmd_info_t *cmd_info);
/* functions definition end */
#endif

