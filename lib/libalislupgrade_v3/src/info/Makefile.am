## Makefile.am for upgrade

AM_CPPFLAGS = \
	-I$(srcdir)/../include                            \
	-I$(srcdir)/../../include                         \
	-I$(srcdir)/../../../libalislstorage/include      \
	-I$(srcdir)/../../../libalislsysinfo/include      \
	-I$(srcdir)/../../../../include

lib_LTLIBRARIES = libalislupgradeinfo.la

libalislupgradeinfo_la_SOURCES = info.c

libalislupgradeinfo_la_LDFLAGS = \
	-lc      \
	-pthread

libalislupgradeinfo_la_LIBADD = \
	$(builddir)/../../../libalislsysinfo/src/libalislsysinfo.la  \
	$(builddir)/../../../libalislstorage/src/libalislstorage.la  \
	$(builddir)/../../../../src/libaliplatform.la
