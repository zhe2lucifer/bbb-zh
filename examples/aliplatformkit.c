/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file			aliplatformkit.c
 *  @brief			example for ALi platform interface
 *
 *  @version		1.0
 *  @date			06/03/2013 02:50:23 PM
 *  @revision		none
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 */

/* System header */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

/* ALi platform log header */
#include <config.h>
#include <alipltfintf.h>
#include <alipltfretcode.h>

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
static const struct option longopts[] = {
	{"verbose", 0, NULL, 'v'},
	{"version", 0, NULL, 'V'},
	{"help",	0, NULL, '?'},
	{NULL,		0, NULL, 0	},
};
#endif
#if HAVE_GETOPT || HAVE_GETOPT_LONG
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#define ALIPLTF_OPT_STRING	"vV?"

/* Internal header */

static void usage(char *program)
{
#if HAVE_GETOPT_LONG
    fprintf (stderr, "usage: %s [-vV?] "
							   "[--verbose] [--version] [--help]\n", program);
#else
    fprintf (stderr, "usage: %s [-vV?]\n", program);
#endif
    fprintf (stderr, "ALi platform kit for function test\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "  -v, --verbose        display status information\n");
	fprintf (stderr, "  -V, --version        display aliplatformkit version and exit\n");
	fprintf (stderr, "  -?, --help           display this help and exit\n");
}

typedef struct alipltf_cmdparam {
	pid_t			child;
	bool			multiple;
	int				tasks;
} alipltf_cmdparam_t;

static int parse(alipltf_cmdparam_t *param, int argc, char *argv[])
{
	int ret = -1;
	int c = 0;

#if HAVE_GETOPT_LONG || HAVE_GETOPT
#if HAVE_GETOPT_LONG
    while ((c = getopt_long(argc, argv, ALIPLTF_OPT_STRING, longopts, NULL)) != -1)
#else
	while ((c = getopt(argc, argv, ALIPLTF_OPT_STRING)) != -1)
#endif
	{
		ret = 0;
		switch (c) {
			case 'v':
				return -1;
			case 'V':
				return -1;
			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}
#endif

	if (ret < 0)
		usage(argv[0]);

	return ret;
}

static void *thread(void *param)
{
}

/**
 *  Function Name:	main
 *  @brief			
 *
 *  @param
 *  @param
 *
 *  @return
 *
 *  @author			Zhao Owen <Owen.Zhao@alitech.com>
 *  @date			06/03/2013, Created
 *
 *  @note
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	alipltf_cmdparam_t param;
	pid_t child = 0;
	int status;
	pthread_t *threadid = NULL;
	int i = 0;

	memset(&param, 0x00, sizeof(alipltf_cmdparam_t));

	if ((argc > 1) && (parse(&param, argc, argv) < 0)) {
		return -1;
	}

	if (param.multiple) {
		child = fork();
		if (-1 == child)
			printf("PLTF LOG ERR: Fork child failure, only 1 process is run\n");
		if (0 == child)
			printf("PLTF LOG INFO: I'm the child\n");
		else 
			printf("PLTF LOG INFO: I'm the father, I have child %ld\n", (unsigned long)child);
	}

	param.child = child;
	if (param.tasks) {
		threadid = (pthread_t *)malloc(param.tasks * sizeof(pthread_t));

		for (i = 0; i < param.tasks; i++)
			pthread_create(&threadid[i], NULL, &thread, &param);

		for (i = 0; i < param.tasks; i++)
			pthread_join(threadid[i], NULL);

		free(threadid);
	} else {
		thread(&param);
	}

	printf("Successful to run a PLTF LOG test\n");

	if (child)
		waitpid(child, &status, WUNTRACED | WCONTINUED);
over:
	return ret;
}
