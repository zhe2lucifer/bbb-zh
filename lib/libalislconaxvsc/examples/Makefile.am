
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislconaxvsckit

alislconaxvsckit_SOURCES	= alislconaxvsckit.c

alislconaxvsckit_LDADD	= $(builddir)/../src/libalislconaxvsc.la \
			  $(builddir)/../../../src/libaliplatform.la

alislconaxvsckit_LDFLAGS	= -pthread
