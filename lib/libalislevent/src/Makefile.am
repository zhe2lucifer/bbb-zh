AM_CFLAGS = -DSL_MOD=ALIPLTF_EVENT

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislevent.la

libalislevent_la_SOURCES	= \
	alislevent.c

libalislevent_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislevent_la_LDFLAGS	= -version-info 0:0:0
