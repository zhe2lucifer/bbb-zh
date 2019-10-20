AM_CFLAGS = -DSL_MOD=ALIPLTF_SBM

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislsbm.la

libalislsbm_la_SOURCES	= alislsbm.c \
				  internel.h

libalislsbm_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislsbm_la_LDFLAGS	= -version-info 0:0:0
