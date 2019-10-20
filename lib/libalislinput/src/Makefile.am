
AM_CPPFLAGS		= -DSL_MOD=ALIPLTF_INPUT \
			  -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislinput.la

libalislinput_la_SOURCES	= \
	alislinput.c \
	alislinput_rfk.c

libalislinput_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislinput_la_LDFLAGS	= -version-info 0:0:0
