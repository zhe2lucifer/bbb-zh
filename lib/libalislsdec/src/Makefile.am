AM_CFLAGS = -DSL_MOD=ALIPLTF_SDEC

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislsdec.la

libalislsdec_la_SOURCES	= alislsdec.c \
			  internel.h

libalislsdec_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislsdec_la_LDFLAGS	= -version-info 0:0:0
