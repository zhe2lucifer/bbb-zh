## Makefile.am for libalisli2c


AM_CPPFLAGS =  -DSL_MOD=ALIPLTF_I2C\
	-I$(top_srcdir)/include                   \
	-I$(srcdir)/../include                    \
	-I$(srcdir)/../../../include              \
	-I$(srcdir)/../../libalislevent/include

lib_LTLIBRARIES = libalisli2c.la

libalisli2c_la_SOURCES =  \
	alisli2c.c        \
	alisli2cwrapper.c

libalisli2c_la_LDFLAGS =  \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE)
libalisli2c_la_LIBADD = \
    $(builddir)/../../../src/libaliplatform.la\
    $(builddir)/../../libalislevent/src/libalislevent.la

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libalisli2c_fixed.a

all-local: libalisli2c_fixed.a

libalisli2c_fixed.a: .libs/libalisli2c.a
	rm -f libalisli2c_fixed.a
	${AR} cru libalisli2c_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalisli2c_fixed.a
	cp -pf libalisli2c_fixed.a .libs/libalisli2c.a

.libs/libalisli2c.a: libalisli2c.la

else

clean-local:

all-local:

endif
