## Makefile.am for libaliplatform

AM_CPPFLAGS = \
	-I$(srcdir)/../include				\
	-I$(srcdir)/../lib/libalisltrng/include	\
	-I$(srcdir)/../lib/libalislcic/include

lib_LTLIBRARIES = libaliplatform.la
#The sample code in 3527 mini build will call the function in aliplatform,which will call the fun in log.c, but log.c is not compiled
libaliplatform_la_SOURCES = platform.c flag.c log.c

if BUILD_DEBUG
#libaliplatform_la_SOURCES += log.c
AM_CFLAGS = -DENABLE_DEBUG
endif

libaliplatform_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE)

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libaliplatform_fixed.a

all-local: libaliplatform_fixed.a

libaliplatform_fixed.a: .libs/libaliplatform.a
	rm -f libaliplatform_fixed.a
	${AR} cru libaliplatform_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libaliplatform_fixed.a
	cp -pf libaliplatform_fixed.a .libs/libaliplatform.a

.libs/libaliplatform.a: libaliplatform.la

else

clean-local:

all-local:

endif
