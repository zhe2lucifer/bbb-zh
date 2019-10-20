
AM_CFLAGS = -DSL_MOD=ALIPLTF_PE

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislpe.la

libalislpe_la_SOURCES	= alislpe.c \
				  internel.h

libalislpe_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislpe_la_LDFLAGS	= -version-info 0:0:0
