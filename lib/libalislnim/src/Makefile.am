AM_CFLAGS = -DSL_MOD=ALIPLTF_NIM

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislnim.la

libalislnim_la_LIBADD = \
	$(builddir)/../../../src/libaliplatform.la

libalislnim_la_SOURCES	=	\
				alislnim.c \
				internel.h

libalislnim_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislnim_la_LDFLAGS	= -lrt -version-info 0:0:0
