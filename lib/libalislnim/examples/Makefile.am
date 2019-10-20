
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislnimkit

alislnimkit_SOURCES	= alislnimkit.c

alislnimkit_LDADD	= $(builddir)/../src/libalislnim.la \
			  $(builddir)/../../../src/libaliplatform.la

alislnimkit_LDFLAGS	= -pthread
