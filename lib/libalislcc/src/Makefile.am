AM_CFLAGS = -DSL_MOD=ALIPLTF_CC

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislcc.la

libalislcc_la_SOURCES	= alislcc.c \
			  internel.h

libalislcc_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislcc_la_LDFLAGS	= -version-info 0:0:0
