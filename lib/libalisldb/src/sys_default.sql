/*
typedef struct audio_output {
	uint8_t         volumn;
	uint8_t         default_volumn;
	bool            install_beep;
	bool            is_ber_printf_on;
	bool            is_spdif_dolby_on;
	bool            is_mute_state;
} audio_output_t;
*/
DROP TABLE IF EXISTS `audio_output`;
CREATE TABLE audio_output (
	id                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	volumn            INTEGER UNSIGNED NULL,
	default_volumn    INTEGER UNSIGNED NULL,
	install_beep      INTEGER UNSIGNED NULL,
	is_ber_printf_on  INTEGER UNSIGNED NULL,
	is_spdif_dolby_on INTEGER UNSIGNED NULL,
	is_mute_state     INTEGER UNSIGNED NULL
	);

-- Initialize audio_output settings
INSERT INTO audio_output
       VALUES (0, 25, 30, 0, 1, 0, 0);

/*
typedef struct audio_description {
	bool    is_default;
	bool    is_service_enabled;
	bool    is_mode_on;
	uint8_t volumn;
	bool    is_default_mode_on;
	bool    use_as_default;
} audio_description_t;
*/
DROP TABLE IF EXISTS `audio_description`;
CREATE TABLE audio_description (
	id                 INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	is_default         INTEGER UNSIGNED NULL,
	is_service_enabled INTEGER UNSIGNED NULL,
	is_mode_on         INTEGER UNSIGNED NULL,
	volumn             INTEGER UNSIGNED NULL,
	is_default_mode_on INTEGER UNSIGNED NULL,
	use_as_default     INTEGER UNSIGNED NULL
	);

-- Initialize audio_output settings
INSERT INTO audio_description
       VALUES (0, 0, 0, 1, 25, 1, 0);

/*
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
*/
DROP TABLE IF EXISTS `test`;
CREATE TABLE test (
	id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	variable0 INTEGER UNSIGNED NULL,
	variable1 INTEGER UNSIGNED NULL,
	variable2 INTEGER UNSIGNED NULL,
	variable3 INTEGER UNSIGNED NULL,
	variable4 INTEGER NULL,
	variable5 INTEGER NULL,
	variable6 INTEGER NULL,
	variable7 REAL NULL,
	variable8 REAL NULL,
	variable9 TEXT NULL,
	variable10 TEXT NULL
	);

-- Initialize test settings
INSERT INTO test
       VALUES (0,
               1, 128, 65534, 655350, -25, -32767, -655350, 12.5, 25.6,
               'this is record 0',
	       '0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19');

INSERT INTO test
       VALUES (1,
               1, 127, 65533, 655349, -24, -32765, -655345, 11.5, 24.6,
               'this is record 1',
	       '10 11 12 13 14 15 16 17 18 19 110 111 112 113 114 115 116 117 118 119');
