

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                  \
	-I$(srcdir)/../include                   \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
    alislscartkit

alislscartkit_SOURCES = alislscartkit.c
alislscartkit_LDADD = \
	$(builddir)/../src/libalislscart.la \
	$(builddir)/../../../src/libaliplatform.la

alislscartkit_LDFLAGS = \
	-pthread





