/**@file
 *  (c) Copyright 2014-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alislavsynckit.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               08/12/2014 09:12:59 AM
 *  @revision           none
 *
 *  @author             Wendy he <wendy.he@alitech.com>
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
#include <alislavsync.h>

static const char avsynckit_version[] = "version 1.0";
static const char avsynckit_args_doc[] = "";
static const char avsynckit_doc[] = "\navsynckit -- alislavsync test example codes";

static struct argp_option avsynckit_options[] = {
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

struct avsynckit_options {
	char *   args[3];
	uint32_t argnum;
};

static void avsynckit_print_version(FILE *stream, struct argp_state *state)
{
	(void)state;
	fprintf(stream, "%s\n", avsynckit_version);
}

static error_t avsynckit_parse_opt(int key, char* arg, struct argp_state *state)
{
	struct avsynckit_options *a = state->input;

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
	static struct argp argp = {0, avsynckit_parse_opt, NULL, NULL, 0, 0, 0};
	void *avsync_dev;
	alisl_retcode ret;
    enum avsync_sync_mode sync_mode;
    struct avsync_cur_status status;
    unsigned int pts;

	struct avsynckit_options options = {
		.argnum = 0,
	};

	argp.options  = avsynckit_options;
	argp.args_doc = avsynckit_args_doc;
	argp.doc      = avsynckit_doc;
#ifndef __UCLIBC__
	argp_program_version_hook = avsynckit_print_version;
#endif
	
	if (argp_parse (&argp, argc, argv, 0, 0, &options)) {
		return -1;
	}
	AGAIN:
	alislavsync_open(&avsync_dev);
	//alislavsync_set_av_sync_mode(avsync_dev, AVSYNC_PCR);
    alislavsync_get_av_sync_mode(avsync_dev, &sync_mode);
    printf("sync_mode: %d\n", sync_mode);
    alislavsync_get_status(avsync_dev, &status);
    printf("avsync status:\n");
    printf("device_status: %d\n", status.device_status);
    printf("vpts_offset: %u\n", status.vpts_offset);
    printf("apts_offset: %u\n", status.apts_offset);
    printf("v_sync_flg: %d\n", status.v_sync_flg);
    printf("a_sync_flg: %d\n", status.a_sync_flg);
    printf("cur_vpts: %u\n", status.cur_vpts);
    printf("cur_apts: %u\n", status.cur_apts);
    alislavsync_get_current_pts(avsync_dev, &pts);
    printf("cur pts: %u\n", pts);
    alislavsync_get_current_stc(avsync_dev, &pts);
    printf("cur sct: %u\n", pts);
    alislavsync_close(avsync_dev);
	return 0;
}
