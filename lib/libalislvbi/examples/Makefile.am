
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislvbikit

alislvbikit_SOURCES	= alislvbikit.c

alislvbikit_LDADD	= $(builddir)/../src/libalislvbi.la \
			  $(builddir)/../../../src/libaliplatform.la

alislvbikit_LDFLAGS	= -pthread
