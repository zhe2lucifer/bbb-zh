## Makefile.am for upgrade

AM_CPPFLAGS = \
	-I$(srcdir)/../include                     \
	-I$(srcdir)/../../libalislstorage/include  \
	-I$(srcdir)/../../../include

bin_PROGRAMS = alislupgradekit

alislupgradekit_SOURCES = alislupgradekit.c
alislupgradekit_LDADD = \
	$(builddir)/../src/libalislupgrade.la                     \
	$(builddir)/../../libalislstorage/src/libalislstorage.la  \
	$(builddir)/../../../src/libaliplatform.la

