/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *
 *	Implementation of TRNG examples application
 *
 *	All rights reserved.
 *
 *	2013-5-21, Created, Zhao Owen <Owen.Zhao@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <config.h>

/* TRNG header */
#include <alisltrng.h>

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
static const struct option longopts[] = {
	{"length",	1, NULL, 'l'},
	{"series",	0, NULL, 's'},
	{"count",	1, NULL, 'c'},
	{"multi",	2, NULL, 'm'},
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

#define ALITRNG_OPT_STRING	"l:sc:m::vV?"

/* Internal header */

static void usage(char *program)
{
#if HAVE_GETOPT_LONG
    fprintf (stderr, "usage: %s [-lscmvV?] [--length=1|8] "
							   "[--series] [--count=series_count ] [--multi] "
							   "[--verbose] [--version] [--help]\n", program);
#else
    fprintf (stderr, "usage: %s [-lscmvV?]\n", program);
#endif
    fprintf (stderr, "TRNG kit for function test\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "  -l, --length         length of random to be got[default=1]\n");
	fprintf (stderr, "  -s, --series         series data to be got\n");
	fprintf (stderr, "  -c, --count          count for series data\n");
	fprintf (stderr, "  -m, --multi          run in multiple thread mode\n");
	fprintf (stderr, "  -v, --verbose        display status information\n");
	fprintf (stderr, "  -V, --version        display trngkit version and exit\n");
	fprintf (stderr, "  -?, --help           display this help and exit\n");
}

typedef struct alisltrng_cmdparam {
	bool			series;
	bool			multiple;
	int				tasks;
	size_t			length;
	int				count;
	pid_t			child;
} alisltrng_cmdparam_t;

static void dump(alisltrng_cmdparam_t *param)
{
	printf("TRNG COMMAND PARAMETERS DUMP START ...\n");
	printf("-----------------------------------------\n");
	printf("Length....................%d\n", param->length);
	printf("Series Data Enable........%s\n", param->series ? "yes" : "no");
	printf("Count of Series Data......%d\n", param->count);
	printf("Multiple Process Test.....%s\n", param->multiple ? "yes" : "no");
	printf("Each Process Threads......%d\n", param->tasks);
	printf("-----------------------------------------\n");
	printf("END\n");
}

static int parse(alisltrng_cmdparam_t *param, int argc, char *argv[])
{
	int ret = -1;
	int c = 0;

	param->length = param->count = 0;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
#if HAVE_GETOPT_LONG
    while ((c = getopt_long(argc, argv, ALITRNG_OPT_STRING, longopts, NULL)) != -1)
#else
	while ((c = getopt(argc, argv, ALITRNG_OPT_STRING)) != -1)
#endif
	{
		ret = 0;
		switch (c) {
			case 'l':
				param->length = atoi(optarg);
				break;
			case 's':
				param->series = true;
				break;
			case 'c':
				param->count = atoi(optarg);
				break;
			case 'm':
				param->multiple = true;
				if (NULL != optarg)
					param->tasks = atoi(optarg);
				else
					param->tasks = 0;
				break;
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
	if ((0 == param->length) && \
		(!param->series && (0 == param->count)))
		usage(argv[0]);
	else
		dump(param);
	return ret;
}

#define ALISLTRNG_TEST_LOOP		32

static void *thread(void *params)
{
	unsigned char *buf = NULL;
	int length = 0;
	int i = 0;
	int loop = ALISLTRNG_TEST_LOOP;

	alisltrng_cmdparam_t *param;
	param = params;
	if (param->series)
		length = 8 * param->count;
	else
		length = param->length;

	buf = malloc(length);

	printf("TRNG RANDOM DATA GOT\n");
	while (loop--) {
		if (param->series)
			alisltrng_get_rand_series(buf, param->count);
		else
			alisltrng_get_rand_bytes(buf, param->length);

		for (i = 0; i < length; i++) {
			printf("%2x ", buf[i]);
			if (7 == (i % 8))
				printf("\n");
		}
		printf("\nTRNG INFO: %s\n", (param->multiple ? \
									(param->child ? "child" : "father") : \
									"single"));
		i = 0;
		sleep(1);
	}

	free(buf);
	return NULL;
}

/**
 * Function Name: main
 *
 * \example
 *
 * 2013-5-21, Created, Zhao Owen <Owen.Zhao@alitech.com>
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	alisltrng_cmdparam_t param;
	pid_t child = 0;
	int status;
	pthread_t *threadid = NULL;
	int i = 0;

	memset(&param, 0x00, sizeof(alisltrng_cmdparam_t));

	if (parse(&param, argc, argv) < 0) {
		return -1;
	}

	if (param.multiple) {
		child = fork();
		if (-1 == child)
			printf("TRNG ERR: Fork child failure, only 1 process is run\n");
		if (0 == child)
			printf("TRNG INFO: I'm the child\n");
		else
			printf("TRNG INFO: I'm the father, I have child %ld\n", (long)child);
	}

	param.child = child;
	if (param.tasks) {
		threadid = (pthread_t *)malloc(param.tasks * sizeof(pthread_t));

		for (i = 0; i < param.tasks; i++)
			pthread_create(&threadid[i], NULL, thread, &param);

		for (i = 0; i < param.tasks; i++)
			pthread_join(threadid[i], NULL);

		free(threadid);
	} else {
		thread(&param);
	}

	printf("Successful to run a TRNG test\n");

	if (child)
		waitpid(child, &status, WUNTRACED | WCONTINUED);

	return ret;
}
