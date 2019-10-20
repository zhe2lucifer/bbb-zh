/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alisldmxkit.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               06/15/2013 09:12:59 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

/* system headers */
#include <argp.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alisldmx.h>

static const char dmxkit_version[] = "version 1.0";
static const char dmxkit_args_doc[] = "";
static const char dmxkit_doc[] = "\nsmckit -- alisldmx test example codes";

static struct argp_option dmxkit_options[] = {
	{ 0,0,0,0,0,0 }
};
#ifdef __UCLIBC__
/* Parsing state.  This is provided to parsing functions called by argp,
   which may examine and, as noted, modify fields.  */

error_t argp_parse (const struct argp *__restrict __argp,
			   int __argc, char **__restrict __argv,
			   unsigned __flags, int *__restrict __arg_index,
			   void *__restrict __input)
{
	return 0;
}


void argp_usage (__const struct argp_state *__state)
{
	
}

#endif

struct dmxkit_options {
	char *   args[3];
	uint32_t argnum;
};

static void dmxkit_print_version(FILE *stream, struct argp_state *state)
{
	(void)state;
	fprintf(stream, "%s\n", dmxkit_version);
}

static error_t dmxkit_parse_opt(int key, char* arg, struct argp_state *state)
{
	struct dmxkit_options *a = state->input;

	switch (key) {
#if 0
	case 'n':
		a->num = strtoul(arg, NULL, 0);
		break;
	
	case 'w':
		a->width = strtoul(arg, NULL, 0);
		if (a->width !=1 &&
		    a->width !=2 &&
		    a->width !=4 &&
		    a->width !=6 &&
		    a->width !=8) {
			return -EINVAL;
		}
		break;
#endif	
	case ARGP_KEY_ARG:
		if (state->arg_num >= a->argnum) {
			/* Too many arg. */
			argp_usage(state);
			return -EINVAL;
		} else {
			a->args[state->arg_num] = arg;
		}
		break;
	
	case ARGP_KEY_END:
		if (state->arg_num < a->argnum) {
			/* Not enough arg. */
			argp_usage(state);
			return -EINVAL;
		}
		break;
	
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

int main (int argc, char **argv)
{
	static struct argp argp = {0, dmxkit_parse_opt, NULL, NULL, 0, 0, 0};
	void *dmx_dev;
	alisl_retcode ret;

	struct dmxkit_options options = {
		.argnum = 0,
	};

	argp.options  = dmxkit_options;
	argp.args_doc = dmxkit_args_doc;
	argp.doc      = dmxkit_doc;
#ifndef __UCLIBC__
	argp_program_version_hook = dmxkit_print_version;
#endif
	
	if (argp_parse (&argp, argc, argv, 0, 0, &options)) {
		return -1;
	}

	alisldmx_open(&dmx_dev, 0, 0);
	alisldmx_close(dmx_dev);

	return 0;
}
