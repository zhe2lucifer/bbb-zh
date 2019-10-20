AM_CFLAGS = -DSL_MOD=ALIPLTF_AVSYNC

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislavsync.la

libalislavsync_la_SOURCES	= alislavsync.c \
				  internel.h

libalislavsync_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislavsync_la_LDFLAGS	= -version-info 0:0:0
