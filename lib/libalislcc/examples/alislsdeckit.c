/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislsdeckit.c
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
#include <alipltflog.h>
#include <alislsdec.h>

static const char sdeckit_version[] = "version 1.0";
static const char sdeckit_args_doc[] = "";
static const char sdeckit_doc[] = "\nsmckit -- alislsdec test example codes";

static struct argp_option sdeckit_options[] = {
	{ 0,0,0,0,0,0 }
};

struct sdeckit_options {
	char *   args[3];
	uint32_t argnum;
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

static void sdeckit_print_version(FILE *stream, struct argp_state *state)
{
	(void)state;
	fprintf(stream, "%s\n", sdeckit_version);
}

static error_t sdeckit_parse_opt(int key, char* arg, struct argp_state *state)
{
	struct sdeckit_options *a = state->input;

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
	static struct argp argp = {0, sdeckit_parse_opt, NULL, NULL, 0, 0, 0};
	void *sdec_dev;
	void *sdec_dev2;
	alisl_retcode ret;

	struct sdeckit_options options = {
		.argnum = 0,
	};

	argp.options  = sdeckit_options;
	argp.args_doc = sdeckit_args_doc;
	argp.doc      = sdeckit_doc;
#ifndef __UCLIBC__
	argp_program_version_hook = sdeckit_print_version;
#endif
	
	if (argp_parse (&argp, argc, argv, 0, 0, &options)) {
		return -1;
	}

	alislsdec_open(&sdec_dev);
	alislsdec_open(&sdec_dev2);

	return 0;
}
