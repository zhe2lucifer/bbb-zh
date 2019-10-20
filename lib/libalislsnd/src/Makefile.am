AM_CFLAGS = -DSL_MOD=ALIPLTF_SND

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislsnd.la

libalislsnd_la_SOURCES	= alislsnd.c \
			  internel.h

libalislsnd_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislsnd_la_LDFLAGS	= -version-info 0:0:0
libalislsnd_la_LIBADD = \
    $(builddir)/../../../src/libaliplatform.la\
    $(builddir)/../../libalislevent/src/libalislevent.la
