## Makefile.am for upgrade
AM_CFLAGS = -DSL_MOD=ALIPLTF_UPGRADE

AM_CPPFLAGS = \
	-I$(srcdir)/../../../include                      \
	-I$(srcdir)/../../../src/include                  \
	-I$(srcdir)/../../../../libalislstorage/include   \
	-I$(srcdir)/../../../../../include

lib_LTLIBRARIES = libalislupgradenet.la

libalislupgradenet_la_SOURCES = \
	net.c net_interface.c

# otherwise libcurl in buildroot

libalislupgradenet_la_LDFLAGS = \
	-lc      \
	-pthread	\
	-lcurl
