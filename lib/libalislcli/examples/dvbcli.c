/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               dvbcli.c
 *  @brief              a simple example of the usage of libcli
 *
 *  @version            1.0
 *  @date               08/01/2013 04:23:34 PM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

/* system head files */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* share library head files */
#include <alislcli.h>

#ifdef __GNUC__
# define UNUSED(d) d __attribute__ ((unused))
#else
# define UNUSED(d) d
#endif

#define func(a) \
int cmd_##a(int argc, char *argv[]) \
{ \
	int i; \
 \
	printf("called %s\n", __FUNCTION__); \
	printf("%d arguments:\n", argc); \
	for (i = 0; i < argc; i++) \
		printf("        %s\n", argv[i]); \
 \
	return 0; \
}

func(testa)
func(testb)
func(testb_a)
func(testb_a_a)
func(testb_b)
func(testb_b_a)

int main(int argc, char **argv)
{
	struct cli_command *c;
	int rc;

	cli_init();
	cli_set_hostname("alidvbcli");

	cli_register_command(NULL,
			"testa", cmd_testa, CLI_CMD_MODE_SELF, "this is testa cmd");
	c = cli_register_command(NULL,
			"testb", NULL, CLI_CMD_MODE_SELF, "this is testb cmd");
	cli_register_command(c, 
			"testb_a", cmd_testb_a, CLI_CMD_MODE_SELF, "this is testb_a cmd");
	c = cli_register_command(c,
			"testb_b", NULL, CLI_CMD_MODE_SELF, "this is testb_b cmd");
	cli_register_command(c,
			"testb_b_a", cmd_testb_b_a, CLI_CMD_MODE_SELF, "this is testb_b_a cmd");

	cli_register_command(cli_find_command("testb_a"),
			"testb_a_a", cmd_testb_a_a, CLI_CMD_MODE_SELF, "this is testb_a_a cmd");

	cli_loop();

	return 0;
}
