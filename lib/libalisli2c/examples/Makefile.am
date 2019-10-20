## Makefile.am for template

AM_CPPFLAGS = \
	-I$(top_srcdir)/include     				\
	-I$(srcdir)/../include 						\
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	alisli2ckit

alisli2ckit_SOURCES = alisli2ckit.c
alisli2ckit_LDADD = \
	$(builddir)/../../../src/libaliplatform.la	\
	$(builddir)/../src/libalisli2c.la

alisli2ckit_LDFLAGS = \
	-pthread

instdir = $(prefix)/bin
dist_inst_SCRIPTS = alisli2ckit.sh

EXTRA_DIST = alisli2ckit.sh
