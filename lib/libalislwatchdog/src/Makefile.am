# Makefile.am for watchdog
AM_CFLAGS = -DSL_MOD=ALIPLTF_WATCHDOG

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(srcdir)/../include \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislwatchdog.la

libalislwatchdog_la_SOURCES = \
	alislwatchdog.c \
	alislwatchdog_wrapper.c
	
libalislwatchdog_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislwatchdog_la_LDFLAGS = \
	-version-info 0:0:0 -release 0.0

EXTRA_DIST = internal.h

if BUILD_STATIC

clean-local:
	rm -f libalislwatchdog_fixed.a

all-local: libalislwatchdog_fixed.a

libalislwatchdog_fixed.a: .libs/libalislwatchdog.a
	rm -f libalislwatchdog_fixed.a
	${AR} cru libalislwatchdog_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislwatchdog_fixed.a
	cp -pf libalislwatchdog_fixed.a .libs/libalislwatchdog.a

.libs/libalislwatchdog.a: libalislwatchdog.la

else

clean-local:

all-local:

endif
