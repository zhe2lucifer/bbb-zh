## Makefile.am for aliplatform

AM_CPPFLAGS = \
	-I$(top_srcdir)/include 						\
	-I$(srcdir)/include

bin_PROGRAMS = \
	aliplatformkit

aliplatformkit_SOURCES = aliplatformkit.c
aliplatformkit_LDADD = \
	$(builddir)/../lib/libalisltrng/src/libalisltrng.la 			\
	$(builddir)/../src/libaliplatform.la

aliplatformkit_LDFLAGS = \
	-pthread

instdir = $(prefix)/bin
dist_inst_SCRIPTS = aliplatformkit.sh

EXTRA_DIST = aliplatformkit.sh
