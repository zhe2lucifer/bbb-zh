## Makefile.am for libalislconaxvsc
AM_CFLAGS = -DSL_MOD=ALIPLTF_CONAXVSC

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(srcdir)/../include \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislconaxvsc.la

libalislconaxvsc_la_SOURCES = 	\
	alislconaxvsc.c

libalislconaxvsc_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislconaxvsc_la_LDFLAGS = \
	-version-info 0:0:0

EXTRA_DIST = internal.h


