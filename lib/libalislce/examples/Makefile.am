
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislcekit

alislcekit_SOURCES	= alislcekit.c

alislcekit_LDADD	= $(builddir)/../src/libalislce.la \
			  $(builddir)/../../../src/libaliplatform.la

alislcekit_LDFLAGS	= -pthread
