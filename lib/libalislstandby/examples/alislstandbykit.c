/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file           alislstandbykit.c
 *  @brief          test function of STANDBY module 
 *
 *  @version        1.0
 *  @date           06/22/2013 02:32:25 PM
 *  @revision       none
 *
 *  @author         Alan Zhang <Alan.Zhang@alitech.com>
 */

/* System header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <config.h>

/* STANDBY header */
#include <alislstandby.h>

/* Private header */
#include "alislstandbykit.h"

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GUN_SOURCE
#include <getopt.h>
static const struct option longopts[] = {
	{"standby", 1, NULL, 's'},
	{"tv2sat",  1, NULL, 't'},
	{"multi",   2, NULL, 'm'},
	{"verbose", 0, NULL, 'v'},
	{"version", 0, NULL, 'V'},
	{"help",    0, NULL, '?'},
	{NULL,      0, NULL,  0 },
};
#endif

#if HAVE_GETOPT || HAVE_GETOPT_LONG
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#define ALISLSTANDBY_OPT_STRING "s:t:m::vV?"

static void usage(char *program)
{
#if HAVE_GETOPT_LONG
	fprintf (stderr, "usage: %s [-stmvV?] [--standby] [tv2sat]"
                               "[--multi] [--verbose] [--help]\n", program);
#else
	fprintf (stderr, "usage: %s [-smvV?]\n", program);
#endif
	fprintf (stderr, "STANDBY kit for function test\n");
	fprintf (stderr, " -s, --standby    STANDBY entry standby. 0: off, 1: on\n");
	fprintf (stderr, " -t, --tv2sat     tv and sat swtich    0: sat, 1: tv\n");
	fprintf (stderr, " -m, --multi      run in multiple thread mode\n");
	fprintf (stderr, " -v, --verbose    display status information\n");
	fprintf (stderr, " -V, --version    display standbykit version and exit\n");
	fprintf (stderr, " -?, --help       display this help and exit\n");

}

static void dump(alislstandby_cmdparam_t *param)
{
	printf("STANDBY COMMAND PARAMETERS DUMP START ...\n");
	printf("-----------------------------------------\n");
	printf("Standby......................%s\n", param->standby  ? "yes" : "no");
	printf("Tv and Sat Switch............%s\n", param->tv2sat   ? "yes" : "no");
	printf("Mutiple Process Test.........%s\n", param->multiple ? "yes" : "no");
	printf("Each Process Threads.........%d\n", param->tasks);
	printf("-----------------------------------------\n");
	printf("END\n");
}

static int parse(alislstandby_cmdparam_t *param, int argc, char *argv[])
{
	int ret = -1;
	char c;

	param->standby = false;

#if HAVE_GETOPT_LONG || HAVE_GETOPT
#if HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, ALISLSTANDBY_OPT_STRING, longopts, NULL)) != -1)
#else 
	while ((c = getopt(argc, argv, ALISLSTANDBY_OPT_STRING)) != -1)
#endif
	{
		ret = 0;
		switch (c) {
		case 's':
			param->standby = true;
			param->sw = atoi(optarg);
			break;
		case 't':
			param->tv2sat = true;
			param->sw = atoi(optarg);
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

	if (param->standby == false)
		usage(argv[0]);
	else
		dump(param);
	return ret;
}

static void *thread(void *cmdparam)
{
	alislstandby_cmdparam_t *param = (alislstandby_cmdparam_t *)cmdparam;

	if (param->standby == true) {
		//alislstandby_power_onoff(param->sw);
	}

	if (param->tv2sat == true) {
		//alislstandby_tvsat_switch(param->sw, ALISLSTANDBY_PARAM_ASPECT_4_3);
	}
	return NULL;
}

 /**
 *  Function Name:  main
 *  @brief          main function of program
 *
 *  \example
 *  @param          argc  input param
 *  @param          argv  input param
 *  @return         if success return 0 else return -1
 *
 *  @author         Alan Zhang <Alan.Zhang@alitech.com>
 *  @date           20/6/2013, Created
 *
 *  @note
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	alislstandby_cmdparam_t param;
	pid_t child = 0;
	int status;
	pthread_t *threadid = NULL;
	int i = 0;

	printf("Success to run a STANDBY start!\n");

	memset(&param, 0x00, sizeof(alislstandby_cmdparam_t));

	if (parse(&param, argc, argv) < 0) {
		return -1;
	}

	if (param.multiple) {
		child = fork();
		if(-1 == child)
			printf("ALISLSTANDBY ERR:  Fork child failure, only 1 process is run\n");
		if(0 == child)
			printf("ALISLSTANDBY INFO: I'm the child\n");
		else
			printf("ALISLSTANDBY INFO: I'm the father, I have child %ld\n", (long)child);
	}

	param.child = child;

	printf("tasknum : %d \n", param.tasks);

	if (param.tasks) {
		threadid = (pthread_t *)malloc(param.tasks * sizeof(pthread_t));

		for(i = 0; i < param.tasks; i++)
			pthread_create(&threadid[i], NULL, thread, &param);

		for(i = 0; i < param.tasks; i++)
			pthread_join(threadid[i], NULL);

			free(threadid);
	} else {
		thread(&param);
	}

	printf("Success to run a STANDBY test!\n");

	if (child)
		waitpid(child, &status, WUNTRACED | WCONTINUED);

	return ret;
}
