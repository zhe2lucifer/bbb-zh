

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                  \
	-I$(srcdir)/../include                   \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
    alislstandbykit

alislstandbykit_SOURCES = alislstandbykit.c

alislstandbykit_LDADD = \
	$(builddir)/../src/libalislstandby.la \
	$(builddir)/../../libalislevent/src/libalislevent.la \
	$(builddir)/../../../src/libaliplatform.la

alislstandbykit_LDFLAGS = \
	-pthread
