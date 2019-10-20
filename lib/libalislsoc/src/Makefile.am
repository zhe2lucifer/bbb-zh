# Makefile.am for soc 
AM_CFLAGS = -DSL_MOD=ALIPLTF_SOC

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(srcdir)/../include \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislsoc.la

libalislsoc_la_SOURCES = \
	alislsoc.c \
	alislsoc_wrapper.c

libalislsoc_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislsoc_la_LDFLAGS = \
	-version-info 0:0:0 -release 0.0

EXTRA_DIST = internal.h

if BUILD_STATIC

clean-local:
	rm -f libalislsoc_fixed.a

all-local: libalislsoc_fixed.a

libalislsoc_fixed.a: .libs/libalislsoc.a
	rm -f libalislsoc_fixed.a
	${AR} cru libalislsoc_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislsoc_fixed.a
	cp -pf libalislsoc_fixed.a .libs/libalislsoc.a

.libs/libalislsoc.a: libalislsoc.la

else

clean-local:

all-local:

endif
