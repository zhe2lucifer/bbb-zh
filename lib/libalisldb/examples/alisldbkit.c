/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               alisldbkit.c
 *  @brief              
 *
 *  @version            1.0
 *  @date               07/12/2013 09:10:22 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <alisldb.h>
#include <alisldb_sys.h>

#define MAX_VARIABLE9 32
#define MAX_VARIABLE10 20
typedef struct test {
	bool            variable0;
	uint8_t         variable1;
	uint16_t        variable2;
	uint32_t        variable3;
	int8_t          variable4;
	int16_t         variable5;
	int32_t         variable6;
	float           variable7;
	double          variable8;
	char            variable9[MAX_VARIABLE9];
	uint32_t        variable10[MAX_VARIABLE10];
} test_t;

static alisldb_dbmlsyntax_t test_syntax[] = {
	{ "variable0",  DBML_BOOL,    1, offsetof(test_t, variable0) },
	{ "variable1",  DBML_UINT8,   1, offsetof(test_t, variable1) },
	{ "variable2",  DBML_UINT16,  1, offsetof(test_t, variable2) },
	{ "variable3",  DBML_UINT32,  1, offsetof(test_t, variable3) },
	{ "variable4",  DBML_INT8,    1, offsetof(test_t, variable4) },
	{ "variable5",  DBML_INT16,   1, offsetof(test_t, variable5) },
	{ "variable6",  DBML_INT32,   1, offsetof(test_t, variable6) },
	{ "variable7",  DBML_FLOAT,   1, offsetof(test_t, variable7) },
	{ "variable8",  DBML_DOUBLE,  1, offsetof(test_t, variable8) },
	{ "variable9",  DBML_STR,     MAX_VARIABLE9, offsetof(test_t, variable9) },
	{ "variable10", DBML_AUINT32, MAX_VARIABLE10, offsetof(test_t, variable10) },
	{ 0 }
};

static void print_test(void *arg)
{
	struct test *t1 = (struct test *)arg;
	int i;

	printf("variable0: %d\n", t1->variable0);
	printf("variable1: %d\n", t1->variable1);
	printf("variable2: %d\n", t1->variable2);
	printf("variable3: %d\n", t1->variable3);
	printf("variable4: %d\n", t1->variable4);
	printf("variable5: %d\n", t1->variable5);
	printf("variable6: %d\n", t1->variable6);
	printf("variable7: %f\n", t1->variable7);
	printf("variable8: %lf\n", t1->variable8);
	printf("variable9: %s\n", t1->variable9);
	for (i=0; i<MAX_VARIABLE10; i++) {
		printf("variable10[%d]: %d\n", i, t1->variable10[i]);
	}

	return ;
}

int main(int argc, char **argv)
{
	struct alisldb_dbml test_dbml;
	struct test t1;
	int i;

	if (alisldb_factory_settings_bydb("sys_db"))
		alisldb_factory_settings_bysql("sys_db");

	/*
	 * example1:
	 * get whole struct from table test
	 */
	printf("example1:\n"
		"get whole struct from table test\n");

	test_dbml.syntax = test_syntax;
	test_dbml.data = &t1;

	alisldb_get_value("sys_db", "select * from test", &test_dbml,
	                  print_test, &t1);

	/*
	 * example2:
	 * get some variables from table test
	 */
	printf("example2:\n"
		"get some variables from table test\n");
	struct t2 {
		int32_t         variable6;
		uint32_t        variable10[MAX_VARIABLE10];
	} t2;

	alisldb_dbmlsyntax_t t2_dbmlsyntax[] = {
		{ "variable6",  DBML_INT32,   1, offsetof(struct t2, variable6) },
		{ "variable10", DBML_AUINT32, MAX_VARIABLE10, offsetof(struct t2, variable10) },
		{ 0 }
	};

	test_dbml.syntax = t2_dbmlsyntax;
	test_dbml.data = &t2;

	alisldb_get_value("sys_db", "select * from test",
			&test_dbml, NULL, NULL);

	printf("variable6: %d\n", t2.variable6);
	for (i=0; i<MAX_VARIABLE10; i++) {
		printf("variable10[%d]: %d\n",
			i, t2.variable10[i]);
	}

	/*
	 * example3:
	 * set one variable to all record of table test
	 */
	printf("example3:\n"
	       "set one variable to all record of table test\n");
	alisldb_set_value("sys_db", "update test set variable0=0", NULL);

	/*
	 * example4:
	 * set one variable to one record of table test
	 */
	printf("example4:\n"
		"set one variable to one record of table test\n");

	alisldb_set_value("sys_db", "update test set variable0=1 where id=1", NULL);

	/*
	 * example5:
	 * set all variable to one record of table test
	 */
	printf("example5:\n"
		"set all variable to one record of table test\n");

	test_dbml.syntax = test_syntax;
	test_dbml.data = &t1;
	memset(&t1, 0, sizeof(t1));
	t1.variable0 = 0;
	t1.variable1 = 1;
	t1.variable2 = 2;
	t1.variable3 = 3;
	t1.variable4 = 4;
	t1.variable5 = 5;
	t1.variable6 = 6;
	t1.variable7 = 7;
	t1.variable8 = 8;
	snprintf(t1.variable9, sizeof(t1.variable9), "this record is updated");
	for (i=0; i<MAX_VARIABLE10; i++)
		t1.variable10[i] = 100 + i;

	alisldb_set_value("sys_db", "update test where id=1", &test_dbml);

	/*
	 * example6:
	 * set all variable to all record of table test
	 */
	printf("example6:\n"
		"set all variable to all record of table test\n");

	test_dbml.syntax = test_syntax;
	test_dbml.data = &t1;
	alisldb_set_value("sys_db", "update test", &test_dbml);

	/*
	 * example7:
	 * insert all variable to a new record of table test
	 */
	printf("example7:\n"
		"insert all variable to a new record of table test\n");
	test_dbml.syntax = test_syntax;
	test_dbml.data = &t1;
	alisldb_insert_value("sys_db", "insert into test", &test_dbml);

	/*
	 * example8:
	 * delete a record from table test
	 */
	printf("example8:\n"
		"delete a record from table test\n");
	alisldb_delete_value("sys_db", "delete from test where id=0");

	/*
	 * print the final result if table test
	 */
	printf("print the final result if table test\n");

	test_dbml.syntax = test_syntax;
	test_dbml.data = &t1;
	memset(&t1, 0, sizeof(t1));
	alisldb_get_value("sys_db", "select * from test", &test_dbml,
	                  print_test, &t1);

	/*
	 * example9:
	 * get audio_output parameters by apis
	 */
	struct audio_output ao;
	uint8_t volumn;

	volumn = alisldb_get_sys_volumn();
	printf("audio_output volumn is %d\n", volumn);

	/*
	 * example10:
	 * get audio_output parameters by alisldb_get_sys()
	 */
	memset(&ao, 0, sizeof(ao));
	alisldb_get_sys(audio_output, is_spdif_dolby_on, ao);
	alisldb_get_sys(audio_output, *, ao);
	printf("audio_output volumn is %d\n", ao.volumn);
	printf("audio_output.install_beep: %d\n", ao.install_beep);
	printf("audio_output.is_spdif_dolby_on: %d\n", ao.is_spdif_dolby_on);

	struct test tt;
	memset(&tt, 0, sizeof(tt));
	alisldb_get_sys(test, *, tt);

	/*
	 * example11:
	 * set audio_output parameters by apis
	 */
	volumn = 30;
	alisldb_set_sys_volumn(volumn);

	/*
	 * example12:
	 * set audio_output parameters by alisldb_set_sys()
	 */
	alisldb_get_sys(audio_output, *, ao);

	ao.install_beep = false;
	ao.is_spdif_dolby_on = true;
	ao.volumn = 40;

	alisldb_set_sys(audio_output, install_beep, ao);
	alisldb_set_sys(audio_output, is_spdif_dolby_on, ao);
	alisldb_set_sys(audio_output, volumn, ao);

	alisldb_set_sys(audio_output, *, ao);

	memset(&ao, 0, sizeof(ao));
	alisldb_get_sys(audio_output, *, ao);
	printf("audio_output.volumn: %d\n", ao.volumn);
	printf("audio_output.install_beep: %d\n", ao.install_beep);
	printf("audio_output.is_spdif_dolby_on: %d\n", ao.is_spdif_dolby_on);

	/*
	 * end
	 */
	printf("exit test!\n");

	return 0;
}
