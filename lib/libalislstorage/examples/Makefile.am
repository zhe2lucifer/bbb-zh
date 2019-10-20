## Makefile.am for storage

AM_CPPFLAGS = \
	-I$(srcdir)/../include                  \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	alislstoragekit

alislstoragekit_SOURCES = alislstoragekit.c
alislstoragekit_LDADD = \
	$(builddir)/../src/libalislstorage.la                \
	$(builddir)/../../../src/libaliplatform.la

alislstoragekit_LDFLAGS = -lpthread