## Makefile.am for upgrade

AM_CPPFLAGS = \
	-I$(srcdir)/../include                     \
	-I$(srcdir)/../../libalislstorage/include  \
	-I$(srcdir)/../../libalislsysinfo/include  \
	-I$(srcdir)/../../../include

bin_PROGRAMS = alislupgradekit alislupgradeinfokit

alislupgradekit_SOURCES = alislupgradekit.c
alislupgradekit_LDADD = \
	$(builddir)/../src/libalislupgrade.la                     \
	$(builddir)/../src/info/libalislupgradeinfo.la            \
	$(builddir)/../../libalislstorage/src/libalislstorage.la  \
	$(builddir)/../../libalislsysinfo/src/libalislsysinfo.la  \
	$(builddir)/../../../src/libaliplatform.la

alislupgradeinfokit_SOURCES = alislupgradeinfokit.c
alislupgradeinfokit_LDADD = \
	$(builddir)/../src/libalislupgrade.la                     \
	$(builddir)/../src/info/libalislupgradeinfo.la            \
	$(builddir)/../../libalislstorage/src/libalislstorage.la  \
	$(builddir)/../../libalislsysinfo/src/libalislsysinfo.la  \
	$(builddir)/../../../src/libaliplatform.la
