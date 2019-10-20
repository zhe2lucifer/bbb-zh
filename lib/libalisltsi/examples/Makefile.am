
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alisltsikit

alisltsikit_SOURCES	= alisltsikit.c

alisltsikit_LDADD	= $(builddir)/../src/libalisltsi.la \
			  $(builddir)/../../../src/libaliplatform.la

alisltsikit_LDFLAGS	= -pthread
