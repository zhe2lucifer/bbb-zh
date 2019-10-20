AM_CFLAGS = -DSL_MOD=ALIPLTF_TSG

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalisltsg.la

libalisltsg_la_SOURCES	= alisltsg.c \
			  error.h  \
			  internel.h 

libalisltsg_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalisltsg_la_LDFLAGS	= -lrt -version-info 0:0:0
