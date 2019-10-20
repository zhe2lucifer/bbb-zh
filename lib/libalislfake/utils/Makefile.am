## Makefile anfor alislfake

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                 \
	-I$(srcdir)/../include                  \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	showstack   \
	showmem

showstack_SOURCES = \
	showstack.c     

showstack_LDADD = \
	$(builddir)/../../../src/libaliplatform.la \
	$(builddir)/../src/libalislfake.la

showmem_SOURCES = \
	showmem.c		

showmem_LDADD = \
	$(builddir)/../../../src/libaliplatform.la \
	$(builddir)/../src/libalislfake.la
