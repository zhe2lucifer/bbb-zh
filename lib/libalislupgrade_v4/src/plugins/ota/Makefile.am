## Makefile.am for upgrade
AM_CFLAGS = -DSL_MOD=ALIPLTF_UPGRADE

AM_CPPFLAGS = \
	-I$(srcdir)/../../../include                      \
	-I$(srcdir)/../../../src/include                  \
	-I$(srcdir)/../../../../libalislstorage/include   \
	-I$(srcdir)/../../../../libalisldmx/include       \
	-I$(srcdir)/../../../../../include

lib_LTLIBRARIES = libalislupgradeota.la

libalislupgradeota_la_SOURCES = \
	aliota.c lib_ota.c lib_ota_dbc.c

libalislupgradeota_la_LIBADD = $(builddir)/../../../../libalisldmx/src/libalisldmx.la

libalislupgradeota_la_LDFLAGS = \
	-lc      \
	-pthread
