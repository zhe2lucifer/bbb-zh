/**@file
 *  (c) Copyright 2014-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislsbmkit.c
 *  @brief
 *
 *  @version            1.0
 *  @date               08/13/2014 15:16:00 PM
 *  @revision           none
 *
 *  @author             Wendy He <wendy.he@alitech.com>
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
#include <alislsbm.h>

static const char sbmkit_version[] = "version 1.0";
static const char sbmkit_args_doc[] = "";
static const char sbmkit_doc[] = "\nsbmkit -- alislsbm test example codes";

static struct argp_option sbmkit_options[] = {
	{ 0,0,0,0,0,0 }
};

struct sbmkit_options {
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

static void sbmkit_print_version(FILE *stream, struct argp_state *state)
{
	(void)state;
	fprintf(stream, "%s\n", sbmkit_version);
}

static error_t sbmkit_parse_opt(int key, char* arg, struct argp_state *state)
{
	struct sbmkit_options *a = state->input;

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
	static struct argp argp = {0, sbmkit_parse_opt, NULL, NULL, 0, 0, 0};
	void *sbm_dev;
	alisl_retcode ret;
	unsigned long size = -1;
    enum sbm_id id = SBM_ID_SBM5;
    struct sbm_buf_config config;

	struct sbmkit_options options = {
		.argnum = 0,
	};

	argp.options  = sbmkit_options;
	argp.args_doc = sbmkit_args_doc;
	argp.doc      = sbmkit_doc;
#ifndef __UCLIBC__
	argp_program_version_hook = sbmkit_print_version;
#endif

	if (argp_parse (&argp, argc, argv, 0, 0, &options)) {
		return -1;
	}
	AGAIN:
	alislsbm_open(&sbm_dev, id);
	config.buffer_addr = 0x02da4000 + 0xc00000 - 0x300000 - 0x180000;
	config.buffer_size = 0x180000;
    config.block_size = 0x20000;
    config.reserve_size = 128*1024;
    config.wrap_mode = SBM_WRAP_MODE_PACKET;
    config.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;
	ret = alislsbm_create(sbm_dev, &config);
	printf("create ret: %d\n", ret);
	ret = alislsbm_get_valid_size(sbm_dev, &size);
	printf("valid size: %d, ret: %d\n", size, ret);
	ret = alislsbm_get_free_size(sbm_dev, &size);
	printf("free size: %d, ret: %d\n", size, ret);
	ret = alislsbm_get_total_size(sbm_dev, &size);
	printf("total size: %d, ret: %d\n", size, ret);
	ret = alislsbm_get_pkt_num(sbm_dev, (unsigned int*)&size);
	printf("pkt num: %d, ret: %d\n", size, ret);
	printf("\n***   destroy   ***\n");
	alislsbm_destroy(sbm_dev);
	printf("destroy ret: %d\n", ret);
	ret = alislsbm_get_valid_size(sbm_dev, &size);
	printf("valid size: %d, ret: %d\n", size, ret);
	ret = alislsbm_get_free_size(sbm_dev, &size);
	printf("free size: %d, ret: %d\n", size, ret);
	ret = alislsbm_get_total_size(sbm_dev, &size);
	printf("total size: %d, ret: %d\n", size, ret);
	ret = alislsbm_get_pkt_num(sbm_dev, (unsigned int*)&size);
	printf("pkt num: %d, ret: %d\n", size, ret);
	alislsbm_open(&sbm_dev, id);
    alislsbm_close(sbm_dev);
    alislsbm_close(sbm_dev);
	return 0;
}
