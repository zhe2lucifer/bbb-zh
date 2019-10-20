## Makefile.am for fake

AM_CFLAGS = -DSL_MOD=ALIPLTF_FAKE

AM_CPPFLAGS = \
	-I$(top_srcdir)/include 		          \
	-I$(srcdir)/../include 			          \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislfake.la

libalislfake_la_SOURCES =  \
	alislfake.c   		  	\
	alislfake_wrapper.c

libalislfake_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislfake_la_LDFLAGS = \
	-version-info 0:0:0 -release 0.0

EXTRA_DIST = internal.h

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libalislfake_fixed.a

all-local: libalislfake_fixed.a

libalislfake_fixed.a: .libs/libalislfake.a
	rm -f libalislfake_fixed.a
	${AR} cru libalislfake_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislfake_fixed.a
	cp -pf libalislfake_fixed.a .libs/libalislfake.a

.libs/libalislfake.a: libalislfake.la

else

clean-local:

all-local:

endif
