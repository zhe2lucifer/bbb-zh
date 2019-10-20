## Makefile.am for libalislcic

AM_CFLAGS = -DSL_MOD=ALIPLTF_CIC

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                   \
	-I$(srcdir)/../include                    \
	-I$(srcdir)/../../../include              \
	-I$(srcdir)/../../libalislevent/include

lib_LTLIBRARIES = libalislcic.la

libalislcic_la_SOURCES =  \
	alislcic.c        \
	alislcicwrapper.c

libalislcic_la_LDFLAGS =  \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE)
libalislcic_la_LIBADD = \
    $(builddir)/../../../src/libaliplatform.la\
    $(builddir)/../../libalislevent/src/libalislevent.la

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libalislcic_fixed.a

all-local: libalislcic_fixed.a

libalislcic_fixed.a: .libs/libalislcic.a
	rm -f libalislcic_fixed.a
	${AR} cru libalislcic_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislcic_fixed.a
	cp -pf libalislcic_fixed.a .libs/libalislcic.a

.libs/libalislcic.a: libalislcic.la

else

clean-local:

all-local:

endif
