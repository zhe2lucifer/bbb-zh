
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislpekit

alislpekit_SOURCES	= alislpekit.c

alislpekit_LDADD	= $(builddir)/../src/libalislpe.la \
			  $(builddir)/../../../src/libaliplatform.la

alislpekit_LDFLAGS	= -pthread
