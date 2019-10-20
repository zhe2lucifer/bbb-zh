
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislsbmkit

alislsbmkit_SOURCES	= alislsbmkit.c

alislsbmkit_LDADD	= $(builddir)/../src/libalislsbm.la \
			  $(builddir)/../../../src/libaliplatform.la

alislsbmkit_LDFLAGS	= -pthread
