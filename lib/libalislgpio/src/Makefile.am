## Makefile.am for gpio
AM_CFLAGS = -DSL_MOD=ALIPLTF_GPIO

AM_CPPFLAGS = \
	-I$(top_srcdir)/include 		          \
	-I$(srcdir)/../include 			          \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislgpio.la

libalislgpio_la_SOURCES = alislgpio.c
libalislgpio_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislgpio_la_LDFLAGS = \
	-version-info 0:0:0 -release 0.0

EXTRA_DIST = internal.h

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libalislgpio_fixed.a

all-local: libalislgpio_fixed.a

libalislgpio_fixed.a: .libs/libalislgpio.a
	rm -f libalislgpio_fixed.a
	${AR} cru libalislgpio_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislgpio_fixed.a
	cp -pf libalislgpio_fixed.a .libs/libalislgpio.a

.libs/libalislgpio.a: libalislgpio.la

else

clean-local:

all-local:

endif
