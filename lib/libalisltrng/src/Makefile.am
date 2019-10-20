## Makefile.am for libalisltrng

AM_CFLAGS = -DSL_MOD=ALIPLTF_TRNG

AM_CPPFLAGS = \
	-I$(top_srcdir)/include		\
	-I$(srcdir)/../include			\
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalisltrng.la

libalisltrng_la_SOURCES = \
	alisltrng.c \
	alisltrngwrapper.c

libalisltrng_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalisltrng_la_LDFLAGS = \
	-version-info 0:0:0

EXTRA_DIST = internal.h
