AM_CFLAGS = -DSL_MOD=ALIPLTF_DMX

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalisldmx.la

libalisldmx_la_SOURCES	= alisldmx.c \
			  internel.h \
			  misc.h

libalisldmx_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalisldmx_la_LDFLAGS	= -lrt -version-info 0:0:0
