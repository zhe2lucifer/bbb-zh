## Makefile.am for template

AM_CPPFLAGS = \
	-I$(top_srcdir)/include     				\
	-I$(srcdir)/../include 						\
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	alislcickit

alislcickit_SOURCES = alislcickit.c
alislcickit_LDADD = $(builddir)/../src/libalislcic.la


alislcickit_LDFLAGS = \
	-pthread

instdir = $(prefix)/bin
dist_inst_SCRIPTS = alislcickit.sh

EXTRA_DIST = alislcickit.sh
