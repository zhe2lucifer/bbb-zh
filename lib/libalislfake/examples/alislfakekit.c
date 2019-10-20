/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file           alislfakekit.c
 *  @brief          Test function of FAKE share lib  
 *
 *  @version        1.0
 *  @date           05/31/2013 06:18:16 PM
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

/* Fake header */
#include <alislfake.h>

/* Priva header*/
#include "alislfakekit.h"

#if HAVE_GETOPT_LONG
#undef  _GUN_SOURCE
#define _GUN_SOURCE
#include <getopt.h>
static const struct option longopts[] = {
	{"tick",       0, NULL, 't'},
	{"ddr",        0, NULL, 'd'},
	{"show",       0, NULL, 's'},
	{"all",        0, NULL, 'a'},
	{"pid",        1, NULL, 'p'},
	{"multi",      2, NULL, 'm'},
	{"verbose",    0, NULL, 'v'},
	{"version",    0, NULL, 'V'},
	{"help",       0, NULL, '?'},
	{"NULL",       0, NULL,  0 },
};
#endif

#if HAVE_GETOPT || HAVE_GETOPT_LONG
extern char *optarg;
extern int optind, opterr, optopt;
#endif
#define FAKE_OPT_STRING "tdsap:m::vV?"

/**
 *  Function Name:  usage
 *  @brief          help info
 *
 *  @param          program 
 *
 *  @return         
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
static void usage(char *program)
{
#if HAVE_GETOPT_LONG
	fprintf(stderr, "usage: %s [-tdsapmvV?]" \
			"[--tick] [--ddr] [--show memory] [--all] [--pid]" \
			"[--verbose] [--version] [--help]\n",program);
#else
	fprintf(stderr, "usage: %s [-tsmmapvV?]\n", program);
#endif

	fprintf(stderr, "Test for FAKE function \n");

	fprintf(stderr, "  -t,  --tick           get tick\n");
	fprintf(stderr, "  -d,  --ddr            show start memory ddr\n");
	fprintf(stderr, "  -s,  --show memory    show memory map\n");
	fprintf(stderr, "  -a,  --all            show all task stack\n");
	fprintf(stderr, "  -p,  --pid            show one task stack by pid\n");

	fprintf(stderr, "  -m,  --multi          run in multiple thread mode\n"); 
	fprintf(stderr, "  -v,  --verbose        display status information\n");
	fprintf(stderr, "  -V,  --version        display FAKE version and exit\n");
	fprintf(stderr, "  -?,  --help           display this help and exit\n");
}

/**
 *  Function Name:  dump
 *  @brief          dump date for debug
 *
 *  @param      
 *
 *  @return         
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
static void dump(alislfake_cmdparam_t *param)
{
	printf("FAKE COMMAND PARAMETERS DUMP START ...\n");
	printf("-----------------------------------------\n");
	printf("Cmd Test .................%d\n", param->cmd);
	printf("Task Pid Test ............%d\n", param->pid);
	printf("Multiple Process Test.....%s\n", param->multiple? "yes" : "no");
	printf("Each Process Threads......%d\n", param->tasks);
	printf("-----------------------------------------\n");
	printf("END\n");
}

/**
 *  Function Name:  parse
 *  @brief          input interface of program
 *
 *  @param          param input cmd param
 *  @param          argc  input param
 *
 *  @return         
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
static int parse(alislfake_cmdparam_t *param, int argc, char *argv[])
{
	int ret = -1;
	int c = 0;

#if HAVE_GETOPT_LONG || HAVE_GETOPT
#if HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, FAKE_OPT_STRING, longopts, NULL)) != -1)
#else 
	while ((c = getopt(argc, argv, FAKE_OPT_STRING)) != -1)
#endif
	{
		ret = 0;
		switch (c) {
		case 't':
			param->cmd = ALISLFAKE_GET_TICKS;
			break;
		case 'd':
			param->cmd = ALISLFAKE_SHOW_START_MEM;
			break;
		case 's':
			param->cmd = ALISLFAKE_SHOW_MEM_MAP;
			break;
		case 'a':
			param->cmd = ALISLFAKE_SHOW_STACK_ALL;
			break;
		case 'p':
			param->cmd = ALISLFAKE_SHOW_STACK_PID;
			if (NULL != optarg)
				param->pid = atoi(optarg);
			else 
				param->pid = 0; 
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

	if (0 == param->cmd)
		usage(argv[0]);
	else
		dump(param);

	return ret;
}

/**
 *  Function Name:  thread
 *  @brief          thread of program
 *
 *  @param          cmdparam input cmd param      
 *
 *  @return         
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
static void *thread(void *cmdparam)
{
	unsigned long cmd;
	pid_t thread_pid = 0;
	alislfake_cmdparam_t *param = (alislfake_cmdparam_t *)cmdparam;

	cmd = param->cmd; 
	thread_pid = param->pid;
	pthread_mutex_t mutex_w;
	pthread_mutex_init(&mutex_w, NULL);

	switch (cmd) {
	case ALISLFAKE_GET_TICKS:
		pthread_mutex_lock(&mutex_w);

		printf("Test fun alislfake_get_tick start!\n");

		printf("Test example:a)Test usleep(1000) run time\n");

		alislfake_get_tick("The FAKE Test exam 1 Program start!\n", true, \
				ALISLFAKE_GET_TICK_IN);
		usleep(ALISLFAKE_TEST_DELAY_1000_US); 
		alislfake_get_tick("The FAKE Test exam 1 Program end!\n", true, \
				ALISLFAKE_GET_TICK_OUT);

		printf("Test example:b)Test usleep(10000) run time\n");

		alislfake_get_tick("The FAKE Test exam 2 Program start!\n", true, \
				ALISLFAKE_GET_TICK_IN);
		usleep(ALISLFAKE_TEST_DELAY_10000_US); 
		alislfake_get_tick("The FAKE Test exam 2 Program end!\n", true, \
				ALISLFAKE_GET_TICK_OUT);

		pthread_mutex_unlock(&mutex_w);
		break;
	case ALISLFAKE_SHOW_START_MEM:
		alislfake_show_mm_ddr();
		break;
	case ALISLFAKE_SHOW_MEM_MAP:
		alislfake_show_mem();
		break;
	case ALISLFAKE_SHOW_STACK_ALL:
		alislfake_show_stack_all();
		break;
	case ALISLFAKE_SHOW_STACK_PID:
		alislfake_show_stack_pid(thread_pid);
		break;
	default:
		printf("The cmd is err! Please input cmd again.\n");
		break;

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
 *  @return         
 *
 *  @author         Chen Jonathan <Jonathan.Chen@alitech.com>
 *  @date           05/31/2013, Created
 *
 *  @note
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	alislfake_cmdparam_t param;
	pid_t child = 0;
	int status;
	pthread_t *threadid = NULL;
	int i = 0;

	printf("Success to run a FAKE start!\n");

	memset(&param, 0x00, sizeof(alislfake_cmdparam_t));

	if (parse(&param, argc, argv) < 0) {
		return -1;
	}

	if (param.multiple) {
		child = fork();
		if(-1 == child)
			printf("ALISLFAKE ERR:  Fork child failure, only 1 process is run\n");
		if(0 == child)
			printf("ALISLFAKE INFO: I'm the child\n");
		else 
			printf("ALISLFAKE INFO: I'm the father, I have child %ld\n", (long)child);
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

	printf("Success to run a FAKE test!\n");

	if (child)
		waitpid(child, &status, WUNTRACED | WCONTINUED);

	return ret;
}
