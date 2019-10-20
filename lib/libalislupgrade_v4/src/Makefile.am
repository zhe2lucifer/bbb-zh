## Makefile.am for upgrade

SUBDIRS = plugins

AM_CFLAGS = -DSL_MOD=ALIPLTF_UPGRADE

AM_CPPFLAGS = \
	-I$(srcdir)/../include                        \
	-I$(srcdir)/include                           \
	-I$(srcdir)/../../libalislstorage/include     \
	-I$(srcdir)/../../../include                  

lib_LTLIBRARIES = libalislupgrade.la

libalislupgrade_la_SOURCES = \
	alislupgradeparser.c \
	object.c             \
	alislupgrade.c       \
	writter.c            \
	tune.c               \
	fastCRC.c

libalislupgrade_la_LDFLAGS = \
	-lc                  \
	-pthread             \
	-ldl

libalislupgrade_la_LIBADD = \
	$(builddir)/../../libalislstorage/src/libalislstorage.la  \
	$(builddir)/../../../src/libaliplatform.la  \
	$(builddir)/plugins/ota/libalislupgradeota.la \
	$(builddir)/plugins/net/libalislupgradenet.la