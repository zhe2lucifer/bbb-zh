
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislavsynckit

alislavsynckit_SOURCES	= alislavsynckit.c

alislavsynckit_LDADD	= $(builddir)/../src/libalislavsync.la \
			  $(builddir)/../../../src/libaliplatform.la

alislavsynckit_LDFLAGS	= -pthread
