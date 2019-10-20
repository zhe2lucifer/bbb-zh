
AM_CFLAGS = -DSL_MOD=CLI

AM_CPPFLAGS		= \
	-I$(top_srcdir)/include                   \
	-I$(srcdir)/../include                    \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislcli.la

libalislcli_la_SOURCES	= libalislcli.c

libalislcli_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislcli_la_LDFLAGS	= -version-info 0:0:0
