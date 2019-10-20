AM_CFLAGS = -DSL_MOD=ALIPLTF_VBI

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislvbi.la

libalislvbi_la_SOURCES	= alislvbi.c \
			  internel.h

libalislvbi_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislvbi_la_LDFLAGS	= -version-info 0:0:0
