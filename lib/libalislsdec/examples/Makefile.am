
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislsdeckit

alislsdeckit_SOURCES	= alislsdeckit.c

alislsdeckit_LDADD	= $(builddir)/../src/libalislsdec.la \
			  $(builddir)/../../../src/libaliplatform.la

alislsdeckit_LDFLAGS	= -pthread
