

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                  \
	-I$(srcdir)/../include                   \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	alislsockit

alislsockit_SOURCES = alislsockit.c
alislsockit_LDADD = \
	$(builddir)/../src/libalislsoc.la \
	$(builddir)/../../../src/libaliplatform.la

alislsockit_LDFLAGS = \
	-pthread





