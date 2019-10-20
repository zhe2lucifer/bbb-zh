AM_CFLAGS = -DSL_MOD=ALIPLTF_VDEC

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

AM_CXXFLAGS = -lpthread

lib_LTLIBRARIES		= libalislvdec.la

libalislvdec_la_SOURCES	= alislvdec.c

libalislvdec_la_LIBADD = \
		$(builddir)/../../../src/libaliplatform.la

libalislvdec_la_LDFLAGS	= -lrt -version-info 0:0:0
