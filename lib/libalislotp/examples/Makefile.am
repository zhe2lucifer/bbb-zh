

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                  \
	-I$(srcdir)/../include                   \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	alislotpkit

alislotpkit_SOURCES = alislotpkit.c
alislotpkit_LDADD = \
	$(builddir)/../src/libalislotp.la \
	$(builddir)/../../../src/libaliplatform.la

alislotpkit_LDFLAGS = \
	-pthread





