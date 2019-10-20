## Makefile.am for libalislce
AM_CFLAGS = -DSL_MOD=ALIPLTF_CE

AM_CPPFLAGS = \
	-I$(top_srcdir)/include	\
	-I$(srcdir)/../include		\
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislce.la

libalislce_la_SOURCES =	\
	alisl_ce_api.c  \
	alislcf.c

libalislce_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislce_la_LDFLAGS = \
	-version-info 0:0:0

EXTRA_DIST = ce_internal.h


