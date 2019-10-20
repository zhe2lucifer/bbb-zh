/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               alisldb.h
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/16/2013 08:44:21 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */
#ifndef __ALISLDB_H__
#define __ALISLDB_H__

#include <sqlite3.h>
#include <alipltfretcode.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

typedef enum alisldb_dbmltype {
	DBML_NONE,      /**< means none */
	DBML_BOOL,      /**< means bool */
	DBML_UINT8,     /**< means uint8_t */
	DBML_UINT16,    /**< means uint16_t */
	DBML_UINT32,    /**< means uint32_t */
	DBML_INT8,      /**< means int8_t */
	DBML_INT16,     /**< means int16_t */
	DBML_INT32,     /**< means int32_t */
	DBML_FLOAT,     /**< means float */
	DBML_DOUBLE,    /**< means double */
	DBML_STR,       /**< means ascii string */
	DBML_AUINT32,   /**< means array of uint32_t */
} alisldb_dbmltype_t;

typedef void (*alisldb_callback) (void *arg);

typedef const struct alisldb_dbmlsyntax {
	const char      *fieldname;
	enum alisldb_dbmltype  type;
	int             nb_elem;
	int             data_offset;
} alisldb_dbmlsyntax_t;

typedef struct alisldb_dbml {
	alisldb_dbmlsyntax_t *syntax;
	void            *data;
} alisldb_dbml;

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
alisl_retcode alisldb_lockdb(void);

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
alisl_retcode alisldb_unlockdb(void);

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
alisl_retcode alisldb_create_database(const char *pathdb, const char *pathsql);

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
alisl_retcode alisldb_factory_settings_bysql(const char *dbname);

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
alisl_retcode alisldb_factory_settings_bydb(const char *dbname);

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
				void *arg);

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
				struct alisldb_dbml *dbml);
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
				struct alisldb_dbml *dbml);
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
				const char *sql);
#ifdef __cplusplus
}
#endif

#endif /* __ALISLDB_H__ */
