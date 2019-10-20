
//#define _TDS_

#ifdef _TDS_
#include <sys_config.h>
#include <types.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <bus/sci/sci.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#endif

#ifdef _TDS_
#include "cli.h"
#else
#include <alipltflog.h>
#include <alislcli.h>
#endif

#ifdef _TDS_
static void set_key(void)
{
	return;
}
static void reset_key(void)
{
	return;
}
static int putchar(char c)
{
	sci_write(0, c);
}

char cli_getch(void)
{
	return sci_read(0);
}
#else
static struct termios tty;
static void set_key(void)
{
	struct termios new_tty;

	tcgetattr(0, &tty);
	new_tty = tty;
	new_tty.c_lflag &= ~(ICANON | ECHO | ISIG);
	new_tty.c_cc[VTIME] = 0;
	new_tty.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &new_tty);

	return;
}
static void reset_key(void)
{
	tcsetattr(0, TCSANOW, &tty);
	return;
}
char cli_getch(void)
{
	int i;
	char ch;

	switch (i=read(0, &ch, 1)) {
		case -1:
		case 0:
			return 0;
	}

	return ch;
}
#endif

#ifdef _TDS_
#define cli_error(fmt, args...)                     \
	do                                          \
	{                                           \
		if(g_debug_control & CLI_LOG_ERROR) \
		{                                   \
			libc_printf(fmt, ##args);   \
			libc_printf("\r\n");        \
		}                                   \
	}while(0)

#define cli_printf(fmt, args...)          \
	do                                \
	{                                 \
		libc_printf(fmt, ##args); \
	} while (0)
#else
#define cli_error(fmt, args...)                     \
	do                                          \
	{                                           \
		if(g_debug_control & CLI_LOG_ERROR) \
		{                                   \
			printf(fmt, ##args);        \
			printf("\r\n");             \
		}                                   \
	}while(0)

#define cli_printf(fmt, args...)          \
	do                                \
	{                                 \
		printf(fmt, ##args);      \
	} while (0)
#endif

#define CLI_OK                  0
#define CLI_ERROR               -1
#define CLI_QUIT                -2
#define CLI_ERROR_ARG           -3

#ifdef __GNUC__
# define UNUSED(d) d __attribute__ ((unused))
#else
# define UNUSED(d) d
#endif

#ifndef size_t
#define size_t unsigned int
#endif

#define free_z(p) do { if (p) { free(p); (p) = 0; } } while (0)

#ifdef _TDS_
#define memcpy(dest, src, len)   MEMCPY(dest, src, len)
#define memcmp(buf1,buf2, len)	 MEMCMP(buf1,buf2, len)
#define memset(dest, c, len)     MEMSET(dest, c, len)
#define memmove(dest, src, len)	 MEMMOVE(dest, src, len)

#define strcpy(dest,src)         STRCPY(dest,src)
#define strcmp(dest,src)         STRCMP(dest,src)
#define strlen(str)              STRLEN(str)
#endif

#ifndef CTRL
#define CTRL(c) (c - '@')
#endif

struct cli_def {
	struct cli_command *commands_root;
	struct cli_command *commands_current;
	char *history[MAX_HISTORY];
	char showprompt;
	char *promptchar;
	char *hostname;
	char *promptstring;

	/* temporary buffer for cli_command_name() to prevent leak */
	char *commandname;

	char ignore_check;
};

static struct cli_def  *g_cli = NULL;
static unsigned int     g_debug_control = CLI_LOG_ERROR;

#if 0

#define func(a) \
int cmd_##a(int argc, char *argv[]) \
{ \
	int i; \
 \
	cli_printf("called %s\n", __FUNCTION__); \
	cli_printf("%d arguments:\n", argc); \
	for (i = 0; i < argc; i++) \
		cli_printf("        %s\n", argv[i]); \
 \
	return CLI_OK; \
}

func(testa)
func(testb)
func(testb_a)
func(testb_a_a)
func(testb_b)
func(testb_b_a)

static void cli_task(DWORD arg1, DWORD arg2)
{
	struct cli_command *c;

	arg1 = arg1;
	arg2 = arg2;

	cli_init();
	cli_set_hostname("alidvbcli");

	cli_register_command(NULL,
			"testa",
			cmd_testa,
			CLI_CMD_MODE_SELF,
			"this is testa cmd");
	c = cli_register_command(NULL,
			"testb",
			NULL,
			CLI_CMD_MODE_SELF,
			"this is testb cmd");
	cli_register_command(c,
			"testb_a",
			cmd_testb_a,
			CLI_CMD_MODE_SELF,
			"this is testb_a cmd");
	c = cli_register_command(c,
			"testb_b",
			NULL,
			CLI_CMD_MODE_SELF,
			"this is testb_b cmd");
	cli_register_command(c,
			"testb_b_a",
			cmd_testb_b_a,
			CLI_CMD_MODE_SELF,
			"this is testb_b_a cmd");
	cli_register_command(cli_find_command("testb_a"),
			"testb_a_a",
			cmd_testb_a_a,
			CLI_CMD_MODE_SELF,
			"this is testb_a_a cmd");

	cli_loop();

	return ;
}

void dvb_cli_init(void)
{
	OSAL_T_CTSK t_ctsk;

	t_ctsk.stksz	= CLI_TASK_STACKSIZE * 2;
	t_ctsk.quantum	= CLI_TASK_QUANTUM;
	t_ctsk.itskpri	= CLI_TASK_PRIORITY;
	t_ctsk.name[0]	= 'C';
	t_ctsk.name[1]	= 'L';
	t_ctsk.name[2]	= 'I';
	t_ctsk.task = (FP)cli_task;
	CLI_TASK_ID = osal_task_create(&t_ctsk);

	if(CLI_TASK_ID == OSAL_INVALID_ID)
		return FALSE;

	return TRUE;
}

#endif

#ifdef __TDS__
static char *strdup(const char *s)
{
	char *new_string = NULL;

	if (s == NULL)
		return NULL;

	new_string = malloc(strlen(s) + 1);
	memset(new_string, 0, strlen(s) + 1);

	strcpy(new_string, s);

	return new_string;
}
#endif /* HAVE_STRDUP */

static int alislcli_asprintf(char **ret, const char *format, ...)
{
	va_list ap;
	int count;

	*ret = NULL;
	va_start(ap, format);
	count = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	if (count >= 0) {
		char* buffer = malloc(count + 1);
		if (buffer == NULL)
			return -1;

		memset(buffer, 0, count + 1);
		va_start(ap, format);
		count = vsnprintf(buffer, count + 1, format, ap);
		va_end(ap);

		if (count < 0)
			free(buffer);
		else
			*ret = buffer;
	}

	return count;
}

#ifdef _TDS_
static int nprintf(const char *string, int n)
{
	int i;

	for (i=0; i<n; i++) {
		putchar(*(string + i));
	}

	return n;
}
#else
static int nprintf(const char *string, int n)
{
	return write(0, string, n);
}
#endif

static int show_prompt(struct cli_def *cli)
{
	int len = 0;

	if (cli->hostname)
		len += nprintf(cli->hostname, strlen(cli->hostname));

	if (cli->promptstring)
		len += nprintf(cli->promptstring, strlen(cli->promptstring));

	return len + nprintf(cli->promptchar, strlen(cli->promptchar));
}

static void cli_clear_line(char *cmd, int l, int cursor)
{
	int i;
	UNUSED(size_t res);

	if (cursor < l) {
		for (i = 0; i < (l - cursor); i++)
			nprintf(" ", 1);
	}
	for (i = 0; i < l; i++) cmd[i] = '\b';
	for (; i < l * 2; i++) cmd[i] = ' ';
	for (; i < l * 3; i++) cmd[i] = '\b';
	nprintf(cmd, i);
	memset(cmd, 0, i);
	l = cursor = 0;
}

static void cli_set_promptstring(struct cli_def *cli, char *promptstring)
{
	free_z(cli->promptstring);
	if (promptstring)
		cli->promptstring = strdup(promptstring);
}

static void cli_set_promptchar(struct cli_def *cli, char *promptchar)
{
	free_z(cli->promptchar);
	cli->promptchar = strdup(promptchar);
}

static int cli_set_promptstr(struct cli_def *cli, char *str)
{
	if (str && *str) {
		char string[64];
		snprintf(string, sizeof(string), "(%s)", str);
		cli_set_promptstring(cli, string);
	} else {
		cli_set_promptstring(cli, NULL);
	}
        return 0;
}

static int cli_help(UNUSED(int argc), UNUSED(char *argv[]));
static int cli_set_commands_current(struct cli_def *cli, struct cli_command *command)
{
	cli->commands_current = command;
	cli_help(0, NULL);
        return 0;
}

static char *cli_command_name(struct cli_def *cli, struct cli_command *command)
{
	char *name = cli->commandname;
	char *o;
	UNUSED(int res);

	if (name)
		free(name);

	if (!(name = malloc(1)))
		return NULL;

	memset(name, 0, 1);

	while (command && command != cli->commands_current->parent) {
		o = name;
		res = alislcli_asprintf(&name, "%s%s%s", command->command, *o ? " " : "", o);
		command = command->parent;
		free(o);
	}
	cli->commandname = name;
	return name;
}

static char *cli_command_name2(struct cli_def *cli, struct cli_command *command)
{
	char *name = cli->commandname;
	char *o;
	UNUSED(int res);

	if (name)
		free(name);

	if (!(name = malloc(1)))
		return NULL;

	memset(name, 0, 1);

	while (command) {
		o = name;
		res = alislcli_asprintf(&name, "%s%s%s",
				command->command, *o ? "/" : "", o);
		command = command->parent;
		free(o);
	}
	cli->commandname = name;
	return name;
}

static int cli_show_help(struct cli_def *cli, struct cli_command *c, unsigned int mode)
{
	struct cli_command *p;

	for (p = c; p; p = p->next) {
		if (p->mode == mode) {
			cli_error("  %-20s %s",
				cli_command_name(cli, p),
				p->help ? : "");
		}
#if 0
		if (p->children != NULL)
			cli_show_help(cli, p->children, mode);
#endif
	}

	return CLI_OK;
}

static int cli_help(UNUSED(int argc), UNUSED(char *argv[]))
{
	struct cli_def *cli = g_cli;

	cli_error("\nCommands available:");
	cli_show_help(cli, cli->commands_root, CLI_CMD_MODE_ALL);
	cli_error("\n");
	cli_show_help(cli, cli->commands_current, CLI_CMD_MODE_SELF);

	return CLI_OK;
}

static int cli_exit(UNUSED(int argc), UNUSED(char *argv[]))
{
#ifdef _TDS_
	int ret = CLI_OK;
#else
	int ret = CLI_QUIT;
#endif
	struct cli_def *cli = g_cli;

	if (cli->commands_current->parent == NULL) {
		return ret;
	} else if (cli->commands_current->parent->parent == NULL) {
		cli_set_commands_current(cli, cli->commands_root);
		cli_set_promptstr(cli, NULL);
		return CLI_OK;
	} else {
		cli_set_commands_current(cli, cli->commands_current->parent->parent->children);
		cli_set_promptstr(cli, cli_command_name2(cli, cli->commands_current->parent));
		return CLI_OK;
	}
}

static int cli_history(UNUSED(int argc), UNUSED(char *argv[]))
{
	int i;
	struct cli_def *cli = g_cli;

	for (i=0; i<MAX_HISTORY; i++) {
		if (cli->history[i])
			cli_error("%3d. %s", i, cli->history[i]);
	}

	return CLI_OK;
}

static int dummy_callback(UNUSED(int argc), UNUSED(char *argv[]))
{
	return CLI_OK;
}

static struct cli_command *cli_register_command2(struct cli_def *cli,
	struct cli_command *parent, char *command,
	int (*callback)(int, char **), unsigned int mode, char *help)
{
	struct cli_command *c, *p;

	if (!command)
		return NULL;

	if (!(c = malloc(sizeof(struct cli_command))))
		return NULL;

	memset(c, 0, sizeof(struct cli_command));

	if (callback)
		c->callback = callback;
	else
		c->callback = dummy_callback;

	c->next = NULL;
	c->children = NULL;
	c->mode = mode;
	if (!(c->command = strdup(command))) {
		free(c);
		return NULL;
	}

	c->parent = parent;

	if (help) {
		if (!(c->help = strdup(help))) {
			free(c->command);
			free(c);
			return NULL;
		}
	}

	if (parent) {
		if (parent->children == NULL) {
			parent->children = c;
		} else {
			for (p = parent->children; p && p->next; p = p->next);
			if (p)
				p->next = c;
		}
	} else {
		if (cli->commands_root == NULL) {
			cli->commands_root = c;
			cli->commands_current = c;
		} else {
			for (p = cli->commands_root; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}

	return c;
}

static int cli_build_shortest(struct cli_def *cli, struct cli_command *commands)
{
	struct cli_command *c, *p;
	char *cp, *pp;
	int len;

	for (c = commands; c; c = c->next) {
		c->unique_len = 1;
		for (p = commands; p; p = p->next) {
			if (c == p)
				continue;

			cp = c->command;
			pp = p->command;
			len = 1;

			while (*cp && *pp && *cp++ == *pp++)
				len++;

			if (len > c->unique_len)
				c->unique_len = len;
		}

		if (c->children)
			cli_build_shortest(cli, c->children);
	}

	return CLI_OK;
}

static struct cli_command *_cli_register_command(struct cli_def *cli,
			struct cli_command *parent,
			char *command,
			int (*callback)(int, char **),
			unsigned int mode,
			char *help)
{
	struct cli_command *c;

	c = cli_register_command2(cli, parent, command, callback, mode, help);

	cli_build_shortest(cli, cli->commands_root);

	return c;
}

static int cli_add_history(struct cli_def *cli, char *cmd)
{
	int i;

	for (i = 0; i < MAX_HISTORY; i++) {
		if (!cli->history[i]) {
			if (i == 0 || strcasecmp(cli->history[i-1], cmd))
			if (!(cli->history[i] = strdup(cmd)))
				return CLI_ERROR;
			return CLI_OK;
		}
	}

	free(cli->history[0]);
	for (i = 0; i < MAX_HISTORY-1; i++)
		cli->history[i] = cli->history[i+1];

	if (!(cli->history[MAX_HISTORY - 1] = strdup(cmd)))
		return CLI_ERROR;

	return CLI_OK;
}

static void cli_free_history(struct cli_def *cli)
{
	int i;
	for (i = 0; i < MAX_HISTORY; i++) {
		if (cli->history[i])
			free_z(cli->history[i]);
	}
}

static int cli_parse_line(char *line, char *words[], int max_words)
{
	int nwords = 0;
	char *p = line;
	char *word_start = 0;
	int inquote = 0;

	while (*p) {
		if (!isspace(*p)) {
			word_start = p;
			break;
		}
		p++;
	}

	while (nwords < max_words - 1) {
		if (!*p ||
		    *p == inquote ||
		    (word_start && !inquote && (isspace(*p) || *p == '|'))) {
			if (word_start) {
				int len = p - word_start;
				words[nwords] = malloc(len + 1);
				if (words[nwords] == NULL)
					return 0;
				memcpy(words[nwords], word_start, len);
				words[nwords++][len] = 0;
			}

			if (!*p)
				break;

			if (inquote)
				p++; /* skip over trailing quote */

			inquote = 0;
			word_start = 0;
		} else if (*p == '"' || *p == '\'') {
			inquote = *p++;
			word_start = p;
		} else {
			if (!word_start) {
				if (*p == '|') {
					if (!(words[nwords++] = strdup("|")))
						return 0;
				}
				else if (!isspace(*p))
					word_start = p;
			}

			p++;
		}
	}

	return nwords;
}


static int cli_get_completions(struct cli_def *cli,
			char *command,
			char **completions,
			int max_completions)
{
	struct cli_command *c;
	struct cli_command *n;
	int num_words, i, k=0;
	char *words[128] = {0};

	if (!command) return 0;
	while (isspace(*command))
		command++;

	num_words = cli_parse_line(command,
				words,
				sizeof(words)/sizeof(words[0]));

	if (!command[0] || command[strlen(command)-1] == ' ')
		num_words++;

	if (!num_words)
			return 0;

	for (c = cli->commands_current, i = 0;
	     c && i < num_words && k < max_completions;
	     c = n) {
		n = c->next;

		if (words[i] &&
		    strncasecmp(c->command, words[i], strlen(words[i])))
			continue;

		if (i < num_words - 1) {
			if (strlen(words[i]) < c->unique_len)
					continue;

			n = c->children;
			i++;
			continue;
		}

		completions[k++] = c->command;
	}

	if (cli->commands_current != cli->commands_root) {
		for (c = cli->commands_root, i = 0;
		     c && i < num_words && k < max_completions;
		     c = n) {
			n = c->next;

			if (c->mode != CLI_CMD_MODE_ALL)
				continue;

			if (words[i] &&
			    strncasecmp(c->command, words[i], strlen(words[i])))
				continue;

			if (i < num_words - 1) {
				if (strlen(words[i]) < c->unique_len)
						continue;

				n = c->children;
				i++;
				continue;
			}

			completions[k++] = c->command;
		}
	}

	return k;
}

static int cli_find_command2(struct cli_def *cli,
			struct cli_command *commands,
			unsigned int mode,
			int num_words,
			char *words[],
			int start_word,
			int show_result)
{
	struct cli_command *c, *again = NULL;
	int c_words = num_words;

	/* Deal with ? for help */
	if (!words[start_word])
		return CLI_ERROR;

	if (words[start_word][strlen(words[start_word]) - 1] == '?') {
		int l = strlen(words[start_word])-1;

		for (c = commands; c; c = c->next) {
			if (strncasecmp(c->command, words[start_word], l) == 0
			    && (c->callback || c->children)
			    && (c->mode == mode)) {
				cli_error("  %-20s %s", c->command, c->help ? : "");
			}
		}

		return CLI_OK;
	}

	for (c = commands; c; c = c->next) {
		int rc = CLI_OK;

		if (c->mode != mode)
			continue;

		if (strncasecmp(c->command, words[start_word], c->unique_len))
			continue;

		if (strncasecmp(c->command, words[start_word], strlen(words[start_word])))
			continue;

		AGAIN:
		/* Found a word! */
		if (!c->children) {
			/* Last word */
			if (!c->callback) {
				cli_error("No callback for \"%s\"",
						cli_command_name(cli, c));
				return CLI_ERROR;
			}
		} else {
			if (start_word == c_words - 1) {
				if (c->callback) {
					goto CORRECT_CHECKS;
				}

				cli_error("Incomplete command");
				return CLI_ERROR;
			}
			rc = cli_find_command2(cli,
					c->children,
					mode,
					num_words,
					words,
					start_word + 1,
					show_result);
			if (rc == CLI_ERROR_ARG) {
				if (c->callback) {
					rc = CLI_OK;
					goto CORRECT_CHECKS;
				} else {
					if (show_result) {
						cli_error("Invalid %s \"%s\"", "command",
							words[start_word]);
					}
				}
			}
			return rc;
		}

		CORRECT_CHECKS:

		if (rc == CLI_OK) {
			if (c->children != NULL) {
				cli_set_commands_current(cli, c->children);
				cli_set_promptstr(cli, cli_command_name2(cli, c));
			}
			//rc = c->callback(c_words - start_word - 1,
					//words + start_word + 1);
			rc = c->callback(c_words - start_word,
					words + start_word);
		}

		return rc;
	}

	// drop out of config submode if we have matched command on MODE_CONFIG
	if (again) {
		c = again;
		cli_set_promptstr(cli, NULL);
		goto AGAIN;
	}

	if (start_word == 0) {
		if (show_result) {
			cli_error("Invalid %s \"%s\"", "command",
				words[start_word]);
		}
	}

	return CLI_ERROR_ARG;
}

static struct cli_command * cli_find_command_ext(struct cli_command *parent, char *command)
{
	struct cli_command *p, *n = NULL;

	for (p = parent; p; p = p->next) {
		if (strcmp(p->command, command) == 0) {
			return p;
		}
		if (p->children != NULL)
			n = cli_find_command_ext(p->children, command);
	}

	return n;
}

int _cli_run_command(struct cli_def *cli, char *command)
{
	int r;
	unsigned int num_words, i;
	char *words[128] = {0};

	if (!command) return CLI_ERROR;
	while (isspace(*command))
		command++;

	if (!*command) return CLI_OK;

	num_words = cli_parse_line(command,
				words,
				sizeof(words) / sizeof(words[0]));

	if (num_words) {
		r = cli_find_command2(cli,
				cli->commands_current,
				CLI_CMD_MODE_SELF,
				num_words,
				words,
				0,
				0);

		if (r != CLI_OK) {
			r = cli_find_command2(cli,
					cli->commands_root,
					CLI_CMD_MODE_ALL,
					num_words,
					words,
					0,
					1);
		}
	} else {
		r = CLI_ERROR;
	}

	for (i = 0; i < num_words; i++)
		free(words[i]);

	if (r == CLI_QUIT)
		return r;

	return CLI_OK;
}

static int cli_delete_command(struct cli_def *cli, struct cli_command *cmd)
{
	struct cli_command *p, *c, *start, *t;

	for (c = cmd->children; c;) {
		p = c->next;
		cli_delete_command(cli, c);
		c = p;
	}

	if (cmd->parent == NULL) {
		start = cli->commands_root;
	} else {
		start = cmd->parent->children;
	}

	for (t = start; t; t = t->next) {
		if (t == cmd && t == start) {
			if (start == cli->commands_root) {
				cli->commands_root = t->next;
			} else {
				cmd->parent->children = t->next;
			}

			if (cli->commands_current == t) {
				if (t->parent && t->parent->parent) {
					cli->commands_current = t->parent->parent->children;
					cli_set_promptstr(cli,
							  cli_command_name2(
							  	cli,
							  	cli->commands_current->parent));
				} else {
					cli->commands_current = cli->commands_root;
					cli_set_promptstr(cli, NULL);
				}
			}
		}

		if (t->next == cmd)
			t->next = cmd->next;
	}

	free(cmd->command);
	if (cmd->help) free(cmd->help);
	free(cmd);

	return 0;
}

static int _cli_loop(struct cli_def *cli)
{
	char c;
	int l, oldl = 0, is_telnet_option = 0, skip = 0, esc = 0;
	int cursor = 0, insertmode = 1;
	char *cmd = NULL, *oldcmd = 0;

	cli_build_shortest(cli, cli->commands_root);

	cli_free_history(cli);

	if ((cmd = malloc(MAX_CMD_BUFFER_LEN)) == NULL)
		return CLI_ERROR;

	memset(cmd, 0, MAX_CMD_BUFFER_LEN);

	/** print the main menu */
	cli_help(0, NULL);

	while (1) {
		signed int in_history = 0;
		int lastchar = 0;

		cli->showprompt = 1;

		if (oldcmd) {
			l = cursor = oldl;
			oldcmd[l] = 0;
			cli->showprompt = 1;
			oldcmd = NULL;
			oldl = 0;
		} else {
			memset(cmd, 0, MAX_CMD_BUFFER_LEN);
			l = 0;
			cursor = 0;
		}

		while (1) {
			if (cli->showprompt) {
				nprintf("\r\n", 2);

				show_prompt(cli);
				nprintf(cmd, l);
				if (cursor < l) {
					int n = l - cursor;
					while (n--)
						nprintf("\b", 1);
				}

				cli->showprompt = 0;
			}


			c = cli_getch();

			if (c == 0)
				continue;

			if (skip) {
				skip--;
				continue;
			}

			if (c == 255 && !is_telnet_option) {
				is_telnet_option++;
				continue;
			}

			if (is_telnet_option) {
				if (c >= 251 && c <= 254) {
					is_telnet_option = c;
					continue;
				}

				if (c != 255) {
					is_telnet_option = 0;
					continue;
				}

				is_telnet_option = 0;
			}

			/* handle ANSI arrows */
			if (esc) {
				if (esc == '[') {
					/* remap to readline control codes */
					switch (c) {
					case 'A': /* Up */
						c = CTRL('P');
						break;

					case 'B': /* Down */
						c = CTRL('N');
						break;

					case 'C': /* Right */
						c = CTRL('F');
						break;

					case 'D': /* Left */
						c = CTRL('B');
						break;

					default:
						c = 0;
					}

					esc = 0;
				} else {
					esc = (c == '[') ? c : 0;
					continue;
				}
			}

			if (c == 0) continue;

			if (c == '\r' || c == '\n') {
				nprintf("\r\n", 2);
				break;
			}

			if (c == 27) {
				esc = 1;
				continue;
			}

			if (c == CTRL('C')) {
				strncpy(cmd, "\n", MAX_CMD_BUFFER_LEN-1);
				break;
			}

			/* back word, backspace/delete */
			if (c == CTRL('W') || c == CTRL('H') || c == 0x7f) {
				int back = 0;

				if (c == CTRL('W')) { /* word */
					int nc = cursor;

					if (l == 0 || cursor == 0)
						continue;

					while (nc && cmd[nc - 1] == ' ') {
						nc--;
						back++;
					}

					while (nc && cmd[nc - 1] != ' ') {
						nc--;
						back++;
					}
				} else {/* char */
					if (l == 0 || cursor == 0) {
						nprintf("\a", 1);
						continue;
					}

					back = 1;
				}

				if (back) {
					while (back--) {
						if (l == cursor) {
							cmd[--cursor] = 0;
							nprintf("\b \b", 3);
						} else {
							int i;
							cursor--;
							for (i=cursor; i<=l; i++)
								cmd[i] = cmd[i+1];

							nprintf("\b", 1);
							nprintf(cmd + cursor,
								strlen(cmd + cursor));
							nprintf(" ", 1);

							for (i=0; i<=(int)strlen(cmd+cursor); i++)
								nprintf("\b", 1);
						}
						l--;
					}

					continue;
				}
			}

			/* redraw */
			if (c == CTRL('L')) {
				int i;
				int cursorback = l - cursor;

				nprintf("\r\n", 2);
				show_prompt(cli);
				nprintf(cmd, l);

				for (i = 0; i < cursorback; i++)
					nprintf("\b", 1);

				continue;
			}

			/* clear line */
			if (c == CTRL('U')) {
				cli_clear_line(cmd, l, cursor);
				l = cursor = 0;
				continue;
			}

			/* kill to EOL */
			if (c == CTRL('K')) {
				int c;

				if (cursor == l)
					continue;

				for (c = cursor; c < l; c++)
					nprintf(" ", 1);

				for (c = cursor; c < l; c++)
					nprintf("\b", 1);

				memset(cmd + cursor, 0, l - cursor);
				l = cursor;
				continue;
			}

			/* EOT */
			if (c == CTRL('D')) {
				if (l)
					continue;

				strncpy(cmd, "quit", MAX_CMD_BUFFER_LEN-1);
				l = cursor = strlen(cmd);
				nprintf("quit\r\n", l + 2);
				break;
			}

			/* disable */
			if (c == CTRL('Z')) {
				cli_clear_line(cmd, l, cursor);
				cli_set_promptstr(cli, NULL);
				cli->showprompt = 1;
				continue;
			}

			/* TAB completion */
			if (c == CTRL('I')) {
				char *completions[128];
				int num_completions = 0;

				if (cursor != l) continue;

				num_completions = cli_get_completions(cli, cmd, completions, 128);
				if (num_completions == 0) {
					nprintf("\a", 1);
				} else if (num_completions == 1) {
					// Single completion
					for (; l > 0; l--, cursor--) {
						if (cmd[l-1] == ' ' || cmd[l-1] == '|')
							break;
						nprintf("\b", 1);
					}
					strncpy((cmd + l), completions[0], MAX_CMD_BUFFER_LEN-1);
					l += strlen(completions[0]);
					cmd[l++] = ' ';
					cursor = l;
					nprintf(completions[0], strlen(completions[0]));
					nprintf(" ", 1);
				} else if (lastchar == CTRL('I')) {
					// double tab
					int i;
					nprintf("\r\n", 2);
					for (i = 0; i < num_completions; i++) {
						nprintf(completions[i], strlen(completions[i]));
						if (i % 4 == 3)
							nprintf("\r\n", 2);
						else
							nprintf("	 ", 1);
					}
					if (i % 4 != 3)
						nprintf("\r\n", 2);
					cli->showprompt = 1;
				} else {
					// More than one completion
					lastchar = c;
					nprintf("\a", 1);
				}
				continue;
			}

			/* history */
			if (c == CTRL('P') || c == CTRL('N')) {
				int history_found = 0;
				if (c == CTRL('P')) {// Up
					in_history--;
					if (in_history < 0) {
						for (in_history = MAX_HISTORY-1;
						     in_history >= 0;
						     in_history--) {
							if (cli->history[in_history]) {
								history_found = 1;
								break;
							}
						}
					} else {
						if (cli->history[in_history])
							history_found = 1;
					}
				} else {// Down
					in_history++;
					if (in_history >= MAX_HISTORY ||
					    !cli->history[in_history]) {
						int i = 0;
						for (i = 0; i < MAX_HISTORY; i++) {
							if (cli->history[i]) {
								in_history = i;
								history_found = 1;
								break;
							}
						}
					} else {
					    history_found = 1;
					}
				}
				if (history_found && cli->history[in_history]) {
					// Show history item
					cli_clear_line(cmd, l, cursor);
					memset(cmd, 0, MAX_CMD_BUFFER_LEN);
					strncpy(cmd,
						cli->history[in_history],
						MAX_CMD_BUFFER_LEN - 1);
					cmd[MAX_CMD_BUFFER_LEN - 1] = '\0';
					l = cursor = strlen(cmd);
					nprintf(cmd, l);
				}

				continue;
			}

			/* left/right cursor motion */
			if (c == CTRL('B') || c == CTRL('F')) {
				if (c == CTRL('B')) { /* Left */
					if (cursor) {
						nprintf("\b", 1);
						cursor--;
					}
				} else { /* Right */
					if (cursor < l) {
						nprintf(&cmd[cursor], 1);
						cursor++;
					}
				}

				continue;
			}

			/* start of line */
			if (c == CTRL('A')) {
				if (cursor) {
					nprintf("\r", 1);
					show_prompt(cli);

					cursor = 0;
				}

				continue;
			}

			/* end of line */
			if (c == CTRL('E')) {
				if (cursor < l) {
					nprintf(&cmd[cursor], l - cursor);
					cursor = l;
				}

				continue;
			}

			/* normal character typed */
			if (cursor == l) {
				 /* append to end of line */
				cmd[cursor] = c;
				if (l < MAX_CMD_BUFFER_LEN - 1) {
					l++;
					cursor++;
				} else {
					nprintf("\a", 1);
					continue;
				}
			} else {
				// Middle of text
				if (insertmode) {
					int i;
					// Move everything one character to the right
					if (l >= MAX_CMD_BUFFER_LEN - 2) l--;
					for (i = l; i >= cursor; i--)
						cmd[i + 1] = cmd[i];
					// Write what we've just added
					cmd[cursor] = c;

					nprintf(&cmd[cursor], l - cursor + 1);
					for (i = 0; i < (l - cursor + 1); i++)
						nprintf("\b", 1);
					l++;
				} else {
					cmd[cursor] = c;
				}
				cursor++;
			}

			if (c == '?' && cursor == l) {
				nprintf("\r\n", 2);
				oldcmd = cmd;
				oldl = cursor = l - 1;
				break;
			}
			nprintf(&c, 1);

			oldcmd = 0;
			oldl = 0;
			lastchar = c;
		}

		if (l < 0) break;
		//if (!strcasecmp(cmd, "quit")) break;

		if (l == 0) continue;
		if (cmd[l - 1] != '?' && strcasecmp(cmd, "history") != 0)
			cli_add_history(cli, cmd);

		if (_cli_run_command(cli, cmd) == CLI_QUIT)
			break;
	}

	cli_free_history(cli);
	free_z(cmd);

	return CLI_OK;
}

/****************************wrapper layer ************************************/
/****************************wrapper layer ************************************/

/**
 *  Function Name:      cli_set_hostname
 *  @brief              set the major prompt of cli, here we
 *                      call it hostname.
 *
 *  @param cli          the handler of cli
 *  @param hostname     the hostname or the major prompt of cli
 *
 *  @return             void
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note
 */
void cli_set_hostname(char *hostname)
{
	struct cli_def *cli = g_cli;

	free_z(cli->hostname);
	if (hostname && *hostname)
		cli->hostname = strdup(hostname);
}

/**
 *  Function Name:      cli_register_command
 *  @brief              register a command to cli
 *
 *  @param cli          the handler of cli
 *  @param parent       the parent command of this command
 *  @param command      the command name of this command
 *  @param callback     the callback function of this command
 *  @param help         the help string information of this command
 *
 *  @return cli_command the pointer of struct cli_command for this command
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note
 */
struct cli_command *cli_register_command(struct cli_command *parent,
		char *command,
		int (*callback)(int, char **),
		unsigned int mode,
		char *help)
{
	return _cli_register_command(g_cli, parent, command, callback, mode, help);
}

/**
 *  Function Name:      cli_unregister_command
 *  @brief              unregister command
 *
 *  @param cli          the handler of cli
 *  @param command      the command name of the command which would
 *                      be unregistered.
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note               This function would unregister all the children
 *                      of this command. And so is the children of its
 *                      children. And so on.
 */
int cli_unregister_command(char *command)
{
	struct cli_def *cli = g_cli;
	struct cli_command *c;

	if (!command) return -1;
	if (!cli->commands_root)
		return CLI_OK;

	c = cli_find_command(command);
	if (c == NULL)
		return -1;

	cli_delete_command(cli, c);

	return CLI_OK;
}

/**
 *  Function Name:      cli_run_command
 *  @brief              run a command
 *
 *  @param cli          the handler of cli
 *  @param command      the command that would run
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note
 */
int cli_run_command(char *command)
{
	return _cli_run_command(g_cli, command);
}

/**
 *  Function Name:      cli_find_command
 *  @brief              find a command by command name
 *
 *  @param cli          the handler of cli
 *  @param command      the name of the command
 *
 *  @return             struct cli_command *
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note
 */
struct cli_command * cli_find_command(char *command)
{
	return cli_find_command_ext(g_cli->commands_root, command);
}

/**
 *  Function Name:      cli_init
 *  @brief              initialize a cli
 *
 *  @param void
 *
 *  @return cli_def     a handler of cli
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note
 */
void cli_init(void)
{
	if (!(g_cli = malloc(sizeof(struct cli_def))))
		return ;

	memset(g_cli, 0, sizeof(struct cli_def));

	cli_set_promptchar(g_cli, ">");
	cli_set_promptstr(g_cli, NULL);

	cli_register_command(NULL,
			"help",
			cli_help,
			CLI_CMD_MODE_ALL,
			"Show available commands");

	cli_register_command(NULL,
			"exit",
			cli_exit,
			CLI_CMD_MODE_ALL,
			"Exit from current command set");

	cli_register_command(NULL,
			"history",
			cli_history,
			CLI_CMD_MODE_ALL,
			"Show history commands");


	return ;
}

/**
 *  Function Name:      cli_loop
 *  @brief              main loop of cli
 *
 *  @return
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/31/2013, Created
 *
 *  @note
 */
void cli_loop(void)
{
	set_key();
	_cli_loop(g_cli);
	reset_key();

	return ;
}

char cli_ignore_check_result(char ignore)
{
	if (g_cli != NULL) {
		g_cli->ignore_check = ignore;
	}
        return ignore;
}

char cli_check_result(const char *info, char success, char fail)
{
	char c = 0;

	if (g_cli == NULL || g_cli->ignore_check) {
		if (success >= 'A' && success <= 'Z') {
			c = success + 32;
		} else if (fail >= 'A' && fail <= 'Z') {
			c = fail + 32;
		}

		return c;
	}

	cli_printf("\n%s", info);
	cli_printf(" [%c/%c]\n", success, fail);

	while (1) {
		c = cli_getch();

		if (c == 0x0D || c == 0x0A) {
			if (success >= 'A' && success <= 'Z') {
				c = success + 32;
				break;
			} else if (fail >= 'A' && fail <= 'Z') {
				c = fail + 32;
				break;
			}
		} else {
			break;
		}
	}

	return c;
}
