## Makefile.am for alislfakekit

AM_CPPFLAGS = \
	-I$(top_srcdir)/include 	              \
	-I$(srcdir)/../include 		              \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	 alislfakekit

alislfakekit_SOURCES = alislfakekit.c
alislfakekit_LDADD =   \
         $(builddir)/../src/libalislfake.la \
	     $(builddir)/../../../src/libaliplatform.la

alislfakekit_LDFLAGS = \
	-pthread 

#instdir = $(prefix)/bin
#dist_inst_SCRIPTS = alislfakekit.sh

#EXTRA_DIST = alislfakekit.sh
