

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                  \
	-I$(srcdir)/../include                   \
	-I$(srcdir)/../../../include

bin_PROGRAMS = \
	alislwatchdogkit

alislwatchdogkit_SOURCES = alislwatchdogkit.c
alislwatchdogkit_LDADD = \
	$(builddir)/../src/libalislwatchdog.la \
	$(builddir)/../../../src/libaliplatform.la \
	$(builddir)/../../libalislstandby/src/libalislstandby.la \
	$(builddir)/../../libalislevent/src/libalislevent.la

alislwatchdogkit_LDFLAGS = \
	-pthread
