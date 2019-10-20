# Makefile.am for scart
AM_CFLAGS = -DSL_MOD=ALIPLTF_SCART

 AM_CPPFLAGS = \
	-I$(top_srcdir)/include                \
	-I$(srcdir)/../include                 \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislscart.la

libalislscart_la_SOURCES = \
	alislscart.c           \
	alislscart_wrapper.c

libalislscart_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislscart_la_LDFLAGS = \
    -version-info 0:0:0 -release 0.0

EXTRA_DIST = internal.h

if BUILD_STATIC

clean-local:
	rm -f libalislscart_fixed.a

all-local: libalislscart_fixed.a

libalislscart_fixed.a: .libs/libalislscart.a
	rm -f libalisledfake_fixed.a
	${AR} cru libalislscart_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislscart_fixed.a
	cp -pf libalislscart_fixed.a .libs/libalislscart.a

.libs/libalislscart.a: libalislscart.la

else

clean-local:

all-local:

endif
