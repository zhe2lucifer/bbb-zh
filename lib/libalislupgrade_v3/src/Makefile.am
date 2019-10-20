## Makefile.am for upgrade

SUBDIRS = plugins info

AM_CFLAGS = -DSL_MOD=ALIPLTF_UPGRADE

AM_CPPFLAGS = \
	-I$(srcdir)/../include                        \
	-I$(srcdir)/include                           \
	-I$(srcdir)/../../libalislstorage/include     \
	-I$(srcdir)/../../libalislsysinfo/include     \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislupgrade.la

libalislupgrade_la_SOURCES = \
	alislupgradeparser.c             \
	object.c             \
	alislupgrade.c       \
	writter.c            \
	tune.c

libalislupgrade_la_LDFLAGS = \
	-lc                  \
	-pthread             \
	-ldl

libalislupgrade_la_LIBADD = \
	$(builddir)/info/libalislupgradeinfo.la            \
	$(builddir)/../../libalislsysinfo/src/libalislsysinfo.la  \
	$(builddir)/../../libalislstorage/src/libalislstorage.la  \
	$(builddir)/../../../src/libaliplatform.la
