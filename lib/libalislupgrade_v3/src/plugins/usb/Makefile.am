## Makefile.am for upgrade

AM_CPPFLAGS = \
	-I$(srcdir)/../../../include                      \
	-I$(srcdir)/../../../src/include                  \
	-I$(srcdir)/../../../../libalislstorage/include   \
	-I$(srcdir)/../../../../libalislsysinfo/include   \
	-I$(srcdir)/../../../../../include

lib_LTLIBRARIES = libalislupgradeusb.la

libalislupgradeusb_la_SOURCES = \
	usb.c

libalislupgradeusb_la_LDFLAGS = \
	-lc       \
	-pthread
