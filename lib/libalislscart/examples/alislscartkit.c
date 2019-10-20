/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file           alislscartkit.c
 *  @brief          test function of SCART module 
 *
 *  @version        1.0
 *  @date           06/22/2013 02:32:25 PM
 *  @revision       none
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
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

/* SCART header */
#include <alislscart.h>

/* Private header */
#include "alislscartkit.h"

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
static const struct option longopts[] = {
	{"standby", 1, NULL, 's'},
	{"aspect",  1, NULL, 'a'},
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

#define ALISLSCART_OPT_STRING "s:a:m::vV?"

static void usage(char *program)
{
#if HAVE_GETOPT_LONG
	fprintf (stderr, "usage: %s [-samvV?] [--standby] [--aspect]"
                               "[--multi] [--verbose] [--help]\n", program);
#else
	fprintf (stderr, "usage: %s [-smvV?]\n", program);
#endif
	fprintf (stderr, "SCART kit for function test\n");
	fprintf (stderr, " -s, --standby    SCART entry standby. 0: off, 1: on\n");
	fprintf (stderr, " -a, --aspect     aspect of tv video.\n");
	fprintf (stderr, " -m, --multi      run in multiple thread mode.\n");
	fprintf (stderr, " -v, --verbose    display status information\n");
	fprintf (stderr, " -V, --version    display scartkit version and exit.\n");
	fprintf (stderr, " -?, --help       display this help and exit.\n");

}

static void dump(alislscart_cmdparam_t *param)
{
	printf("SCART COMMAND PARAMETERS DUMP START ...\n");
	printf("-----------------------------------------\n");
	printf("Standby......................%s\n", param->standby  ? "yes" : "no");
	printf("aspect.......................%s\n", param->aspect   ? "yes" : "no");
	printf("Mutiple Process Test.........%s\n", param->multiple ? "yes" : "no");
	printf("Each Process Threads.........%d\n", param->tasks);
	printf("-----------------------------------------\n");
	printf("END\n");
}

static int parse(alislscart_cmdparam_t *param, int argc, char *argv[])
{
	int ret = -1;
	int c = 0;

	param->standby = false;

#if HAVE_GETOPT_LONG || HAVE_GETOPT
#if HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, ALISLSCART_OPT_STRING, longopts, NULL)) != -1)
#else 
	while ((c = getopt(argc, argv, ALISLSCART_OPT_STRING)) != -1)
#endif
	{
		ret = 0;
		switch (c) {
		case 's':
			param->standby = true;
			param->sw = atoi(optarg);
			break;
		case 'a':
			param->aspect = true;
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
	//unsigned long cmd;
	//pid_t thread_pid = 0;
	alislscart_cmdparam_t *param = (alislscart_cmdparam_t *)cmdparam;
    
	if (param->standby == true) {
		if (param->sw == 0) {
			alislscart_power_onoff(ALISLSCART_PARAM_POWER_ON, 0);
		} else {
			alislscart_power_onoff(ALISLSCART_PARAM_POWER_OFF, 0);
		}
	}
	
	if (param->aspect == true) {
		if (param->sw == 0) {
			alislscart_set_tv_video_out_aspect(ALISLSCART_PARAM_ASPECT_4_3, 0);
		} else {
			alislscart_set_tv_video_out_aspect(ALISLSCART_PARAM_ASPECT_16_9, 0);
		}
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
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           20/6/2013, Created
 *
 *  @note
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	alislscart_cmdparam_t param;
	pid_t child = 0;
	int status;
	pthread_t *threadid = NULL;
	int i = 0;

	printf("Success to run a SCART start!\n");

	memset(&param, 0x00, sizeof(alislscart_cmdparam_t));

	if (parse(&param, argc, argv) < 0) {
		return -1;
	}

	if (param.multiple) {
		child = fork();
		if(-1 == child)
			printf("ALISLSCART ERR:  Fork child failure, only 1 process is run\n");
		if(0 == child)
			printf("ALISLSCART INFO: I'm the child\n");
		else
			printf("ALISLSCART INFO: I'm the father, I have child %ld\n", (long)child);
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

	printf("Success to run a SCART test!\n");

	if (child)
		waitpid(child, &status, WUNTRACED | WCONTINUED);

	return ret;
}
