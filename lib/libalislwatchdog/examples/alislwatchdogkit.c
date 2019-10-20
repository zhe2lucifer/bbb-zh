/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alislwatchdogkit.c
 *  @brief              test watchdog base funcation
 *
 *  @version            1.0
 *  @date               07/06/2013 10:14:08 AM
 *  @revision           none
 *
 *  @author             Jonathan Chen <jonathan.chen@alitech.com>
 */

/* System Header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <alislwatchdog.h>
#include <config.h>

/* Kit Private header */
#include <alislwatchdogkit.h>

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>

static const struct option longopts[] = {
	{"reboot" , 1, NULL, 'r'},
	{"start"  , 0, NULL, 'S'},
	{"stop"   , 0, NULL, 's'},
	{"feed"   , 1, NULL, 'f'},
	{"multi"  , 2, NULL, 'm'},
	{"verbose", 0, NULL, 'v'},
	{"version", 0, NULL, 'V'},
	{"help"   , 0, NULL, '?'},
	{NULL     , 0, NULL,  0 },
};
#endif

#if HAVE_GETOPT || HAVE_GETOPT_LONG
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#define ALISLWATCHDOG_OPT_STRING "r:Ssf:m::vV?"

static void usage(char *program)
{
#if HAVE_GETOPT_LONG
	fprintf(stderr, "usage: %s [-rSsfmvV?] [-reboot] [-start] [-stop] [-feed]"
	                           "[-multi] [-verbose] [-version] [-help]\n", program);
#else
	fprintf(stderr, "usage: %s [-rSsfmvV?]\n", program);
#endif
	fprintf(stderr, "Watchdog kit for  function test.\n");
	fprintf(stderr, " -r, --reboot     reboot watchdog.\n");
	fprintf(stderr, " -S, --start      start watchdog.\n");
	fprintf(stderr, " -s, --stop       stop watchdog.\n");
	fprintf(stderr, " -f, --feed       feed watchdog.\n");
	fprintf(stderr, " -m, --multi      run in multiple thread mode.\n");
	fprintf(stderr, " -v, --verbose    display status information.\n");
	fprintf(stderr, " -V, --version    display watchdogkit version and exit.\n");
	fprintf(stderr, " -?, --help       display this help and exit.\n");
}

static void dump(alislwatchdog_cmdparam_t *param)
{
	printf("Watchdog command parameters dump start ...\n");
	printf("----------------------------------------------\n");
	printf("reboot dog........%s\n", param->reboot ? "yes" : "no");
	printf("start  dog........%s\n", param->start  ? "yes" : "no");
	printf("stop   dog........%s\n", param->stop   ? "yes" : "no");
	printf("feed   dog........%s\n", param->feed   ? "yes" : "no");
	printf("Mutiple Process Test   ...%s\n", param->multiple ? "yes" : "no");
	printf("Each Proces threads  ...%d\n",   param->tasks);
	printf("----------------------------------------------\n");
	printf("Watchdog command parameters dump end.\n");
}

static int parse(alislwatchdog_cmdparam_t *param, int argc, char *argv[])
{
	int ret = -1;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
	char c;
#endif
	param->multiple = false;
	param->tasks = 0;
	param->reboot = false;
	param->start = false;
	param->feed = false;
	printf("--->test parse start :%d\n",__LINE__);
#if HAVE_GETOPT_LONG || HAVE_GETOPT
#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, ALISLWATCHDOG_OPT_STRING, longopts, NULL)) != -1)
#else
	while ((c = getopt(argc, argv, ALISLWATCHDOG_OPT_STRING)) != -1)
#endif
	{
		ret = 0;
		switch (c) {
		case 'r':
			param->reboot = true;
			param->time = atoi(optarg);
			break;
		case 'S':
			param->start = true;
			break;
		case 's':
			param->stop  = true;
			break;
		case 'f':
			param->feed  = true;
			param->time = atoi(optarg);
			break;
		case 'm':
			param->multiple = true;
			if (NULL != optarg) {
				param->tasks = atoi(optarg);
			} else {
				param->tasks = 0;
			}
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
	printf("--->test parse end :%d\n",__LINE__);
	if (param->reboot == false)
		usage(argv[0]);
	else
		dump(param);
	return ret;
}

static void *thread(void *cmdparam)
{
	alislwatchdog_cmdparam_t *param = (alislwatchdog_cmdparam_t *)cmdparam;

	if (param->reboot == true) {
		alislwatchdog_reboot(param->time);
	}

	if (param->start == true) {
		alislwatchdog_start();
	}

	if (param->stop == true) {
		alislwatchdog_stop();
	}

	if (param->feed == true) {
		alislwatchdog_feed_dog(param->time);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	alislwatchdog_cmdparam_t param;
	pid_t child = 0;
	int status;
	pthread_t *threadid = NULL;
	int i = 0;

	printf("Success to run a watchdog start!\n");

	memset(&param, 0x00, sizeof(alislwatchdog_cmdparam_t));

	if (parse(&param, argc, argv) < 0) {
		return -1;
	}

	if (param.multiple) {
		child = fork();
		if (-1 == child) {
			printf("Fork child failure, only 1 process is run\n");
		}

		if (0 == child) {
			printf("I am a child\n");
		}

		else {
			printf("I am the father, I have child %ld\n", (long)child);
		}
	}

	param.child = child;

	printf("Task number: %d\n", param.tasks);

	if (param.tasks) {
		threadid = (pthread_t *)malloc(param.tasks * sizeof(pthread_t));

		for (i = 0; i < param.tasks; i++) {
			pthread_create(&threadid[i], NULL, thread, &param);
		}

		for (i = 0; i < param.tasks; i++) {
			pthread_join(threadid[i], NULL);
		}
		free(threadid);
	} else {
		printf("Task test: %d\n", param.tasks);

		thread(&param);
	}

	printf("Success to run a watchdog test!\n");

	if (child)
		waitpid(child, &status, WUNTRACED | WCONTINUED);

	return ret;
}
