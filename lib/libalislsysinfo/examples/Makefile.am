## Makefile.am for sysinfo

AM_CPPFLAGS = \
	-I$(srcdir)/../include                       \
	-I$(srcdir)/../../libalislstorage/include    \
	-I$(srcdir)/../../../include

bin_PROGRAMS = alislsysinfokit

alislsysinfokit_SOURCES = alislsysinfokit.c
alislsysinfokit_LDADD = \
	$(builddir)/../src/libalislsysinfo.la                 \
	$(builddir)/../../../src/libaliplatform.la  \
	$(builddir)/../../libalislstorage/src/libalislstorage.la

alislsysinfokit_LDFLAGS = -lpthread