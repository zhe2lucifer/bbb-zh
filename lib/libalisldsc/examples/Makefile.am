
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alisldsckit

alisldsckit_SOURCES	= alisldsckit.c

alisldsckit_LDADD	= $(builddir)/../src/libalisldsc.la \
			  $(builddir)/../../../src/libaliplatform.la

alisldsckit_LDFLAGS	= -pthread
