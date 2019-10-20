AM_CFLAGS = -DSL_MOD=ALIPLTF_TSI

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalisltsi.la

libalisltsi_la_SOURCES	= alisltsi.c \
			  internel.h 

libalisltsi_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalisltsi_la_LDFLAGS	= -lrt -version-info 0:0:0
