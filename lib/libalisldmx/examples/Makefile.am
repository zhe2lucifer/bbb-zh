
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alisldmxkit

alisldmxkit_SOURCES	= alisldmxkit.c

alisldmxkit_LDADD	= $(builddir)/../src/libalisldmx.la \
	$(builddir)/../../../src/libaliplatform.la

alisldmxkit_LDFLAGS	= -pthread
