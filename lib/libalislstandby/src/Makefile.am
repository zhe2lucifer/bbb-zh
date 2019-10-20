## Makefile.am for libalislstandby

AM_CFLAGS = -DSL_MOD=ALIPLTF_PM

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislstandby.la

libalislstandby_la_SOURCES =  \
	alislstandby.c         \
	alislstandbywrapper.c  \
	internal.h 			   \
	ali_keyset.h

libalislstandby_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislstandby_la_LDFLAGS = -version-info 0:0:0	\
							-ldl

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libalislstandby_fixed.a

all-local: libalislstandby_fixed.a

libalislstandby_fixed.a: .libs/libalislstandby.a
	rm -f libalislstandby_fixed.a
	${AR} cru libalislstandby_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislstandby_fixed.a
	cp -pf libalislstandby_fixed.a .libs/libalislstandby.a

.libs/libalislstandby.a: libalislstandby.la

else

clean-local:

all-local:

endif
