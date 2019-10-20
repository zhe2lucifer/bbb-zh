/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldb_common.c
 *  @brief              common interfaces for ALi share library databases
 *
 *  @version            1.0
 *  @date               07/16/2013 08:43:12 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

/* system headers */
#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#undef _GNU_SOURCE
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
 
/* share library headers */
#include <alipltflog.h>
#include <alipltfretcode.h>
#include <alisldb.h>

/* local headers */
#include "internal.h"

#ifdef LOCAL_DEBUG
#define PREFIX "/zhsa012/usrhome/summer.xia/workspace/git/aliplatform/install"
#else
#define PREFIX ""
#endif

#define DBCONF "/etc/alisldb/alisldb.conf"
#define DEFAULT_SEMID_FILE "/tmp/.alisldb_semid"
#define MAX_PATH_LEN 256
#define MAX_SQL_LEN 4096

/*
 * The union for semctl may or may not be defined for us. This
 * code, devined in linux's semctl() manpage, is the proper way
 * to attain it if necessary.
 */
#if defined (__GNU_LIBRARY__) && !defined (_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val; /* value for SETVAL */
	struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array; /* array for GETALL,SETALL */
	struct seminfo *__buf; /* buffer for IPC_INFO */
};
#endif

static bool initialized = false;
static int semid = -1;

static int get_conf_value(const char *name, char *value, int size)
{
	char inbuf[MAX_PATH_LEN], dbconf[MAX_PATH_LEN];
	FILE *stream;
	int lineno = 0;

	memset(dbconf, 0, sizeof(dbconf));
	snprintf(dbconf, sizeof(dbconf), "%s%s", PREFIX, DBCONF);
	stream = fopen(dbconf, "r");
	if (stream == NULL) {
		return -1;
	}

	while (fgets(inbuf, sizeof(inbuf), stream) != NULL) {
		int len = strlen(inbuf);
		char *p;
		lineno++;
		if (inbuf[len - 1] != '\n') {
			SL_DBG("line %d is too long in config file", lineno);
			continue;
		}

		/* Remove the '\n' */
		inbuf[len - 1] = '\0';
		p = strstr(inbuf, "#");
		if (p != NULL)
			memset(p, '\0', strlen(p));
		p = strstr(inbuf, name);
		if (p == NULL)
			continue;
		p = strstr(inbuf, "=");
		if (p == NULL)
			continue;
		p++;
		while(*p == ' ' || *p == '\t') p++;
		if (*p == '\0')
			continue;

		if (strlen(PREFIX) != 0) {
			snprintf(value, size, "%s", PREFIX);
		}
		snprintf(value + strlen(PREFIX), size - strlen(PREFIX), "%s", p);

		fclose(stream);
		return 0;
	}
	
	fclose(stream);

	return -1;
}

static int mkdir_of_file(const char *path)
{
	char dir[MAX_PATH_LEN];
	char cmd[MAX_PATH_LEN];
	int res __attribute__ ((unused));
	int rc, cnt = 0;

	memset(dir, 0, sizeof(dir));
	memset(cmd, 0, sizeof(cmd));
	strncpy(dir, path, sizeof(dir));
	dirname(dir);
	snprintf(cmd, sizeof(cmd),"[ -d %s ] || mkdir -p %s ", dir, dir);
	rc = access(dir, F_OK);
	while (rc) {
		if (cnt++ > 5)
			return -1;

		res = system(cmd);
		usleep(1000);
		rc = access(dir, F_OK);
	}

	return 0;
}

#ifdef LOCAL_DEBUG
static char SEMID_FILE[MAX_PATH_LEN];
static void semdelete(void)
{
	SL_DBG("Process quit;\n");
	SL_DBG("Delete the semaphore %d\n", semid);
	SL_DBG("Delete file %s\n", SEMID_FILE);

	semctl(semid, 0, IPC_RMID, (union semun)0);
	unlink(SEMID_FILE);
}

static void sigdelete(int signum)
{
	/*
	 * Calling exit will conveniently trigger
	 * the normal delete item
	 */
	exit(0);
}
#endif

static alisl_retcode create_semaphore(const char *semid_file)
{
	union semun sunion;
	FILE *stream;

	if (mkdir_of_file(semid_file) == -1) {
		SL_DBG("Can not create dir of %s\n", semid_file);
		semid = -1;
		initialized = true;
		return ERROR_MKDIR;
	}

	stream = fopen(semid_file, "w+");
	if (stream == NULL) {
		SL_DBG("Can not create file %s\n", semid_file);
		SL_DBG("Please check the permission of its directory");
		semid = -1;
		initialized = true;
		return ERROR_OPEN;
	}

	semid = semget(IPC_PRIVATE, 1, SHM_R | SHM_W);
	if (semid == -1) {
		SL_DBG("Can not get global semaphore!\n");
		initialized = true;
		fprintf(stream, "%d", semid);
		fclose(stream);
		return ERROR_SEMGET;
	}

	sunion.val = 1;
	if (semctl(semid, 0, SETVAL, sunion) == -1) {
		SL_DBG("Initialize semaphore failed!\n");
		SL_DBG("Can not use global semaphore!\n");
		/* delete the semaphore manually */
		semctl(semid, 0, IPC_RMID, (union semun)0);
		semid = -1;
		initialized = true;
		fprintf(stream, "%d", semid);
		fclose(stream);
		return ERROR_SEMSET;
	}

#ifdef LOCAL_DEBUG
	strncpy(SEMID_FILE, semid_file, sizeof(SEMID_FILE));
	atexit(&semdelete);
	signal(SIGINT, &sigdelete);
#endif

	SL_DBG("Save global semaphore to:\n");
	SL_DBG("%s\n", semid_file);
	SL_DBG("semid = %d\n", semid);
	fprintf(stream, "%d", semid);

	initialized = true;

	fflush(stream);
	fclose(stream);

	return 0;
}

static alisl_retcode read_semaphore(FILE *stream)
{
	if (fscanf(stream, "%d", &semid) != 1) {
		semid = -1;
		initialized = true;
		return ERROR_SEMREAD;
	}

	return 0;
}

static alisl_retcode module_initialize(void)
{
	char semid_file[MAX_PATH_LEN];
	alisl_retcode rc;
	FILE *stream;

	if (initialized == true) {
		return 0;
	}

	memset(semid_file, 0, sizeof(semid_file));
	if (get_conf_value("semid_file", semid_file, sizeof(semid_file)) != 0) {
		SL_DBG("Read conf file to get semid_file failed!\n");
		SL_DBG("Use default value %s%s", PREFIX, DEFAULT_SEMID_FILE);
		snprintf(semid_file, sizeof(semid_file), "%s%s",
			PREFIX, DEFAULT_SEMID_FILE);
	}

	stream = fopen(semid_file, "r");
	if (stream == NULL) {
		SL_DBG("No global semaphore, create it!\n");
		rc = create_semaphore(semid_file);
	} else {
		rc = read_semaphore(stream);
		fclose(stream);
	}

	return rc;
}

static alisl_retcode _semop(int semid, struct sembuf *sops, unsigned nsops)
{
	if (semop(semid, sops, nsops) == -1) {
		SL_DBG("semop semid %d (%d operation) failed: %s",
		        semid, nsops, strerror(errno));
		return ERROR_SEMOP;
	}

	return 0;
}

static alisl_retcode lockdb(void)
{
	struct sembuf sb;
	alisl_retcode rc;

	if (unlikely(initialized == false)) {
		if ((rc = module_initialize()))
			return rc;
	}

	if (unlikely(semid == -1)) {
		SL_DBG("No semaphore to lockdb!\n");
		return ERROR_NOSEM;
	}

	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;

	return _semop(semid, &sb, 1);
}

static alisl_retcode unlockdb(void)
{
	struct sembuf sb;

	if (unlikely(semid == -1)) {
		SL_DBG("No semaphore to unlockdb!\n");
		return ERROR_NOSEM;
	}

	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;

	return _semop(semid, &sb, 1);
}

/**
 *  Function Name:      alisldb_lockdb
 *  @brief              lock database access engine. Process access safe.
 *
 *  @param void         no input parameter
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/16/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_lockdb(void)
{
	return lockdb();
}

/**
 *  Function Name:      alisldb_unlockdb
 *  @brief              unlock database access engine. Process access safe.
 *
 *  @param void         no input parameter
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/16/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_unlockdb(void)
{
	return unlockdb();
}

/**
 *  Function Name:      alisldb_create_database
 *  @brief              Create a database.
 *
 *  @param pathdb       The path of database where it will located.
 *  @param pathsql      The path of sql file which will be used to
 *                      initialize the database.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/16/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_create_database(const char *pathdb, const char *pathsql)
{
	char cmd[MAX_PATH_LEN + MAX_PATH_LEN];
	int rc, cnt = 0;

	memset(cmd, 0, sizeof(cmd));

	if (mkdir_of_file(pathdb) == -1) {
		SL_DBG("Can not create directory of %s!\n", pathdb);
		return ERROR_MKDIR;
	}

	rc = access(pathdb, F_OK);
	if (rc == 0) {
		SL_DBG("Database %s already exist!\n", pathdb);
		return 0;
	}

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "sqlite3 %s \"\"", pathdb);
	lockdb();
	rc = system(cmd);
	unlockdb();
	if (rc) {
		SL_DBG("create database fail!\n");
		return ERROR_MKDB;
	}

	if (pathsql != NULL) {
		snprintf(cmd, sizeof(cmd), "sqlite3 %s \".read %s\"",
			 pathdb, pathsql);
		lockdb();
		rc = system(cmd);
		unlockdb();
		if (rc) {
			SL_DBG("Initialize databases fail!\n");
			return ERROR_INITDB;
		}
	}

	return 0;
}

/**
 *  Function Name:      alisldb_factory_settings_bysql
 *  @brief              make a database rollback to factory settings
 *                      initialized the database by its default sql file.
 *
 *  @param dbname       the database name that specify which database
 *                      will be set to factory settings.\n
 *                      the database name should be one of those read
 *                      from /etc/alisdb/alisldb.conf\n
 *                      example:\n
 *                      sys_db\n
 *                      dvb_db\n
 *                      ott_db\n
 *                      hbbtv_db\n
 *                      \n
 *                      And the default setting for the database is also
 *                      from sql file which specified by /etc/alisldb/alisldb.conf
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/17/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_factory_settings_bysql(const char *dbname)
{
	char pathdb[MAX_PATH_LEN];
	char pathsql[MAX_PATH_LEN];
	char cmd[MAX_PATH_LEN];
	char sqlname[32];
	int res __attribute__ ((unused));
	alisl_retcode rc;

	memset(pathdb, 0, sizeof(pathdb));
	if (get_conf_value(dbname, pathdb, sizeof(pathdb)) == -1) {
		return ERROR_NODB;
	}

	memset(sqlname, 0, sizeof(sqlname));
	snprintf(sqlname, sizeof(sqlname), "%s_default_sql", dbname);

	if (get_conf_value(sqlname, pathsql, sizeof(pathsql)) == -1) {
		return ERROR_NODB;
	}

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "mv %s %s.bak > /dev/null 2>&1",
			pathdb, pathdb);
	res = system(cmd);

	rc = alisldb_create_database(pathdb, pathsql);
	if (rc) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "mv %s.bak %s > /dev/null 2>&1",
				pathdb, pathdb);
		res = system(cmd);
		return rc;
	} else {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "rm -rf %s.bak > /dev/null 2>&1",
				pathdb);
		res = system(cmd);
		return 0;
	}
}


/**
 *  Function Name:      alisldb_factory_settings_bydb
 *  @brief              make a database rollback to factory settings
 *                      initialized the database by its default db file.
 *
 *  @param dbname       the database name that specify which database
 *                      will be set to factory settings.\n
 *                      the database name should be one of those read
 *                      from /etc/alisdb/alisldb.conf\n
 *                      example:\n
 *                      sys_db\n
 *                      dvb_db\n
 *                      ott_db\n
 *                      hbbtv_db\n
 *                      \n
 *                      And the default setting for the database is also
 *                      from configure file /etc/alisldb/alisldb.conf
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/21/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_factory_settings_bydb(const char *dbname)
{
	char pathdb[MAX_PATH_LEN];
	char pathdefdb[MAX_PATH_LEN];
	char cmd[MAX_PATH_LEN];
	char defdbname[32];
	int res __attribute__ ((unused));
	int rc;

	memset(pathdb, 0, sizeof(pathdb));
	if (get_conf_value(dbname, pathdb, sizeof(pathdb)) == -1) {
		return ERROR_NODB;
	}

	memset(defdbname, 0, sizeof(defdbname));
	snprintf(defdbname, sizeof(defdbname), "%s_default_db", dbname);

	if (get_conf_value(defdbname, pathdefdb, sizeof(pathdefdb)) == -1) {
		return ERROR_NODB;
	}

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "mv %s %s.bak > /dev/null 2>&1",
			pathdb, pathdb);
	res = system(cmd);

	snprintf(cmd, sizeof(cmd), "cp -rf %s %s > /dev/null 2>&1",
			pathdefdb, pathdb);
	rc = system(cmd);
	if (rc) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "mv %s.bak %s > /dev/null 2>&1",
				pathdb, pathdb);
		res = system(cmd);
		return rc;
	} else {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "rm -rf %s.bak > /dev/null 2>&1",
				pathdb);
		res = system(cmd);
		return 0;
	}
	
	return 0;
}

static int eval_callback(void *arg,
		int fieldcnt, char **fieldvalue, char **fieldname)
{
	struct callback_param *cb = (struct callback_param *)arg;
	struct alisldb_dbml *dbml = (struct alisldb_dbml *)cb->arg;
	alisldb_dbmlsyntax_t *syntax;
	char *endptr, *startptr;
	bool gotdata = false;
	void *data;
	int i, j;

	for (i=0; i<fieldcnt; i++) {
		syntax = dbml->syntax;
		while (syntax != NULL && 
		       syntax->fieldname != NULL &&
		       strcmp(syntax->fieldname, fieldname[i]))
			syntax++;

		if (syntax == NULL || syntax->fieldname == NULL)
			continue;

		gotdata = true;

		switch (syntax->type) {
		case DBML_NONE:
			break;
		case DBML_BOOL:
			data = dbml->data + syntax->data_offset;
			*(bool *)data = strtoul(fieldvalue[i], &endptr, 0);
			break;
                case DBML_UINT8:
			data = dbml->data + syntax->data_offset;
			*(uint8_t *)data = strtoul(fieldvalue[i], &endptr, 0);
			break;
                case DBML_UINT16:
			data = dbml->data + syntax->data_offset;
			*(uint16_t *)data = strtoul(fieldvalue[i], &endptr, 0);
			break;
                case DBML_UINT32:
			data = dbml->data + syntax->data_offset;
			*(uint32_t *)data = strtoul(fieldvalue[i], &endptr, 0);
			break;
                case DBML_INT8:
			data = dbml->data + syntax->data_offset;
			*(int8_t *)data = strtol(fieldvalue[i], &endptr, 0);
			break;
                case DBML_INT16:
			data = dbml->data + syntax->data_offset;
			*(int16_t *)data = strtol(fieldvalue[i], &endptr, 0);
			break;
                case DBML_INT32:
			data = dbml->data + syntax->data_offset;
			*(int32_t *)data = strtol(fieldvalue[i], &endptr, 0);
			break;
                case DBML_FLOAT:
			data = dbml->data + syntax->data_offset;
			*(float *)data = strtod(fieldvalue[i], &endptr);
			break;
                case DBML_DOUBLE:
			data = dbml->data + syntax->data_offset;
			*(double *)data = strtod(fieldvalue[i], &endptr);
			break;
                case DBML_STR:
			data = dbml->data + syntax->data_offset;
			strncpy(data, fieldvalue[i], syntax->nb_elem);
			break;
                case DBML_AUINT32:
			data = dbml->data + syntax->data_offset;
			startptr = fieldvalue[i];
			for (j=0; j<syntax->nb_elem; j++) {
				if (*startptr == '\0')
					break;
				*(uint32_t *)data = strtoul(startptr, &endptr, 0);
				data += sizeof(uint32_t);
				startptr = endptr;
			}
			break;
		default:
			break;
		}
	}

	if (gotdata == true && cb->callback != NULL) {
		cb->callback(cb->cb_arg);
	}

	return 0;
}

/**
 *  Function Name:      alisldb_get_value
 *  @brief              get value from a database
 *
 *  @param dbname       database name
 *  @param sql          sql statement that specify which record should
 *                      be filtered. \n
 *                      You can filter one record or a serials of records. \n
 *                      You can also filter all of the fields of a record or
 *                      filter some of the fields of a record.
 *  @param dbml         how to store and where to store the detailed records.
 *  @param callback     when got one record, this callback function will
 *                      be called.
 *  @param arg          callback function parameters.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/18/2013, Created
 *
 *  @note               After every record was filtered and the data was
 *                      precessed, the callback function will be called.
 */
alisl_retcode alisldb_get_value(const char *dbname,
				const char *sql,
				struct alisldb_dbml *dbml,
				alisldb_callback callback,
				void *arg)
{
	static sqlite3 *db = NULL;
	char pathdb[256];
	char *zErrMsg = 0;
	struct callback_param cb;
	int rc;

	memset(pathdb, 0, sizeof(pathdb));
	if (get_conf_value(dbname, pathdb, sizeof(pathdb)) == -1) {
		return ERROR_NODB;
	}

	if (strcasestr(sql, "select") != sql) {
		return ERROR_INVAL;
	}

	lockdb();
	rc = sqlite3_open(pathdb, &db);
	if (rc) {
		SL_DBG("%s", sqlite3_errmsg(db));
		unlockdb();
		return ERROR_OPENDB;
	}

	cb.arg = dbml;
	cb.callback = callback;
	cb.cb_arg = arg;
	rc = sqlite3_exec(db, sql, eval_callback, (void *)&cb, &zErrMsg);

	sqlite3_close(db);

	unlockdb();

	if (rc == SQLITE_OK)
		return 0;
	else
		return ERROR_EXEC;
}

static int prepare_update_sql(const char *sql, struct alisldb_dbml *dbml,
			char *mysql, size_t size)
{
	alisldb_dbmlsyntax_t *syntax;
	char *ptr, *psql;
	void *data;
	uint32_t n;
	int i;

	psql = mysql;

	ptr = strcasestr(sql, "where");
	if (ptr != NULL)
		n = (uint32_t)(ptr - sql);
	else
		n = strlen(sql);

	memcpy(psql, sql, n);
	psql += n;
	*psql = ' ';
	psql++;

	snprintf(psql, sizeof(psql), "set");
	psql += strlen(psql);
	*psql = ' ';
	psql++;

	for (syntax = dbml->syntax;
	     syntax != NULL && syntax->fieldname != NULL;
	     syntax++) {
		switch (syntax->type) {
		case DBML_NONE:
			break;
		case DBML_BOOL:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %d,", syntax->fieldname, *(bool *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_UINT8:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%s = %d,", syntax->fieldname, *(uint8_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_UINT16:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%s = %d,", syntax->fieldname, *(uint16_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_UINT32:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %d,", syntax->fieldname, *(uint32_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_INT8:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %d,", syntax->fieldname, *(int8_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_INT16:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %d,", syntax->fieldname, *(int16_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_INT32:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %d,", syntax->fieldname, *(int32_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_FLOAT:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %f,", syntax->fieldname, *(float *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_DOUBLE:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = %f,", syntax->fieldname, *(double *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_STR:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = '%s',", syntax->fieldname, (char *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_AUINT32:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%s = '", syntax->fieldname);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			for (i=0; i<syntax->nb_elem; i++) {
				snprintf(psql, sizeof(psql),"%d", *(uint32_t *)data);
				data += sizeof(uint32_t);
				psql += strlen(psql);
				*psql = ' ';
				psql++;
			}
			snprintf(psql, sizeof(psql),"',", syntax->fieldname);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
		default:
			break;
		}
	}

	if (*(psql - 1) == ',') {
		*(psql - 1) = ' ';
	}
	if (*(psql - 2) == ',') {
		*(psql - 2) = ' ';
	}

	if (ptr != NULL) {
		n = (uint32_t)(ptr - sql);
		n = strlen(sql) - n;
		memcpy(psql, ptr, n);
		psql += n;
	}

	*psql = '\0';

	return 0;
}

/**
 *  Function Name:      alisldb_set_value
 *  @brief              update a record or a serials of records
 *
 *  @param dbname       database name
 *  @param sql          A partial of sql statement that specify which record
 *                      should be updated. In this case, the fields and the
 *                      value of the fields that would be updated were
 *                      specified by struct dbml. \n
 *                      --or-- \n
 *                      An integral sql statement. In this case, THE PARAMETER
 *                      DBML SHOULD BE SET TO --NULL--, and this function would
 *                      just excute the sql statement.
 *  @param dbml         how to update the detailed record.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_set_value(const char *dbname,
				const char *sql,
				struct alisldb_dbml *dbml)
{
	static sqlite3 *db = NULL;
	char pathdb[256];
	char *zErrMsg = 0;
	struct callback_param cb;
	char *mysql;
	int rc;

	memset(pathdb, 0, sizeof(pathdb));
	if (get_conf_value(dbname, pathdb, sizeof(pathdb)) == -1) {
		return ERROR_NODB;
	}

	if (strcasestr(sql, "update") != sql) {
		return ERROR_INVAL;
	}

	lockdb();
	rc = sqlite3_open(pathdb, &db);
	if (rc) {
		SL_DBG("%s", sqlite3_errmsg(db));
		unlockdb();
		return ERROR_OPENDB;
	}

	if (dbml == NULL) {
		rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	} else {
		mysql = malloc(MAX_SQL_LEN);
		prepare_update_sql(sql, dbml, mysql, MAX_SQL_LEN);
		if (strlen(mysql) > MAX_SQL_LEN) {
			while(1) {
			    SL_ERR("sql memory overflow!!!!!!!!\n");
				usleep(500000);
			}
		}

		rc = sqlite3_exec(db, mysql, NULL, NULL, &zErrMsg);
		free(mysql);
	}

	sqlite3_close(db);

	unlockdb();

	if (rc == SQLITE_OK)
		return 0;
	else
		return ERROR_EXEC;
}

static int _print_fieldvalue(struct alisldb_dbml *dbml, char **sql)
{
	alisldb_dbmlsyntax_t *syntax;
	char *psql;
	void *data;
	int i;

	psql = *sql;

	for (syntax = dbml->syntax;
	     syntax != NULL && syntax->fieldname != NULL;
	     syntax++) {
		switch (syntax->type) {
		case DBML_NONE:
			break;
		case DBML_BOOL:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql), "%d,", *(bool *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_UINT8:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%d,", *(uint8_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_UINT16:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%d,", *(uint16_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_UINT32:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%d,", *(uint32_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_INT8:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%d,", *(int8_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_INT16:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%d,", *(int16_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_INT32:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%d,", *(int32_t *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_FLOAT:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%f,", *(float *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_DOUBLE:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"%f,", *(double *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_STR:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"'%s',", (char *)data);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
                case DBML_AUINT32:
			data = dbml->data + syntax->data_offset;
			snprintf(psql, sizeof(psql),"'");
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			for (i=0; i<syntax->nb_elem; i++) {
				snprintf(psql, sizeof(psql),"%d", *(uint32_t *)data);
				data += sizeof(uint32_t);
				psql += strlen(psql);
				*psql = ' ';
				psql++;
			}
			snprintf(psql, sizeof(psql),"',", syntax->fieldname);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
		default:
			break;
		}
	}

	if (*(psql - 1) == ',') {
		*(psql - 1) = ' ';
	}
	if (*(psql - 2) == ',') {
		*(psql - 2) = ' ';
	}

	*sql = psql;

	return 0;
}

static int _print_fieldname(struct alisldb_dbml *dbml, char **sql)
{
	alisldb_dbmlsyntax_t *syntax;
	char *psql;

	psql = *sql;

	for (syntax = dbml->syntax;
	     syntax != NULL && syntax->fieldname != NULL;
	     syntax++) {
		switch (syntax->type) {
		case DBML_NONE:
			break;
		case DBML_BOOL:
                case DBML_UINT8:
                case DBML_UINT16:
                case DBML_UINT32:
                case DBML_INT8:
                case DBML_INT16:
                case DBML_INT32:
                case DBML_FLOAT:
                case DBML_DOUBLE:
                case DBML_STR:
                case DBML_AUINT32:
			snprintf(psql, sizeof(psql),"%s,", syntax->fieldname);
			psql += strlen(psql);
			*psql = ' ';
			psql++;
			break;
		default:
			break;
		}
	}

	if (*(psql - 1) == ',') {
		*(psql - 1) = ' ';
	}
	if (*(psql - 2) == ',') {
		*(psql - 2) = ' ';
	}

	*sql = psql;

	return 0;
}

static int prepare_insert_sql(const char *sql, struct alisldb_dbml *dbml,
			char *mysql, size_t size)
{
	alisldb_dbmlsyntax_t *syntax;
	char *psql;
	void *data;
	int i;

	psql = mysql;

	memcpy(psql, sql, strlen(sql));
	psql += strlen(sql);
	*psql = ' ';
	psql++;

	snprintf(psql, sizeof(psql),"(");
	psql += strlen(psql);
	*psql = ' ';
	psql++;

	_print_fieldname(dbml, &psql);

	snprintf(psql, sizeof(psql),")");
	psql += strlen(psql);
	*psql = ' ';
	psql++;

	snprintf(psql, sizeof(psql),"values (");
	psql += strlen(psql);
	*psql = ' ';
	psql++;

	_print_fieldvalue(dbml, &psql);

	snprintf(psql, sizeof(psql),")");
	psql += strlen(psql);
	*psql = ' ';
	psql++;

	*psql = '\0';

	return 0;
}

/**
 *  Function Name:      alisldb_insert_value
 *  @brief              insert a record into table
 *
 *  @param dbname       database name
 *  @param sql          "insert into xxx" \n
 *                      In this case, all the fields and values are
 *                      specifed by struct dbml.\n
 *                      --or--\n
 *                      "insert into xxx (field1, field2, ...)
 *                                values (value1, value2, ...)" \n
 *                      In this case, this function will just excute
 *                      the sql statement.
 *  @param dbml         how to update the detailed record.
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_insert_value(const char *dbname,
				const char *sql,
				struct alisldb_dbml *dbml)
{
	static sqlite3 *db = NULL;
	char pathdb[256];
	char *zErrMsg = 0;
	char *mysql;
	int rc;

	memset(pathdb, 0, sizeof(pathdb));
	if (get_conf_value(dbname, pathdb, sizeof(pathdb)) == -1) {
		return ERROR_NODB;
	}

	if (strcasestr(sql, "insert into") != sql) {
		return ERROR_INVAL;
	}

	if (strcasestr(sql, "values") == NULL && dbml == NULL) {
		return ERROR_INVAL;
	}

	if (strcasestr(sql, "values") != NULL && dbml != NULL) {
		return ERROR_INVAL;
	}

	lockdb();
	rc = sqlite3_open(pathdb, &db);
	if (rc) {
		SL_DBG("%s", sqlite3_errmsg(db));
		unlockdb();
		return ERROR_OPENDB;
	}

	if (dbml == NULL) {
		rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
	} else {
		mysql = malloc(MAX_SQL_LEN);
		prepare_insert_sql(sql, dbml, mysql, MAX_SQL_LEN);
		if (strlen(mysql) > MAX_SQL_LEN) {
			while(1) {
			    SL_ERR("sql memory overflow!!!!!!!!\n");
				usleep(500000);
			}
		}
		rc = sqlite3_exec(db, mysql, NULL, NULL, &zErrMsg);
		rc = SQLITE_OK;
		free(mysql);
	}

	sqlite3_close(db);

	unlockdb();

	if (rc == SQLITE_OK)
		return 0;
	else
		return ERROR_EXEC;
}

/**
 *  Function Name:      alisldb_delete_value
 *  @brief              delete a record from table
 *
 *  @param dbname       database name
 *  @param sql          sql statemenst \n
 *                      example: \n
 *                      delete all records: "delete from xxx" \n
 *                      delete one records: "delete from xxx where id=0"
 *
 *  @return             alisl_retcode
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 *  @date               07/19/2013, Created
 *
 *  @note
 */
alisl_retcode alisldb_delete_value(const char *dbname,
				const char *sql)
{
	static sqlite3 *db = NULL;
	char pathdb[256];
	char *zErrMsg = 0;
	int rc;

	memset(pathdb, 0, sizeof(pathdb));
	if (get_conf_value(dbname, pathdb, sizeof(pathdb)) == -1) {
		return ERROR_NODB;
	}

	if (strcasestr(sql, "delete") != sql) {
		return ERROR_INVAL;
	}

	if (strcasestr(sql, "from") == NULL) {
		return ERROR_INVAL;
	}

	lockdb();
	rc = sqlite3_open(pathdb, &db);
	if (rc) {
		SL_DBG("%s", sqlite3_errmsg(db));
		unlockdb();
		return ERROR_OPENDB;
	}

	rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);

	sqlite3_close(db);

	unlockdb();

	if (rc == SQLITE_OK)
		return 0;
	else
		return ERROR_EXEC;
}

