## Makefile.am for alisltrng

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(srcdir)/../include \
	-I$(srcdir)/../../../include
bin_PROGRAMS = \
	alisltrngkit

alisltrngkit_SOURCES = alisltrngkit.c
alisltrngkit_LDADD = \
	$(builddir)/../src/libalisltrng.la \
	$(builddir)/../../../src/libaliplatform.la

alisltrngkit_LDFLAGS = \
	-pthread

instdir = $(prefix)/bin
dist_inst_SCRIPTS = alisltrngkit.sh

EXTRA_DIST = alisltrngkit.sh
