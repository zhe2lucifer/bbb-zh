AM_CFLAGS = -DSL_MOD=ALIPLTF_DIS

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES	= libalisldis.la
libalisldis_la_SOURCES	= alisldis.c
libalisldis_la_LIBADD	= $(builddir)/../../../src/libaliplatform.la

libalisldis_la_LDFLAGS	= -lrt -version-info 0:0:0
