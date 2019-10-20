## Makefile.am for storage
AM_CFLAGS = -DSL_MOD=ALIPLTF_STORAGE

AM_CPPFLAGS = \
	-I$(srcdir)/../include                   \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislstorage.la

libalislstorage_la_SOURCES =  \
	alislstorage.c        \
	alislstorageparser.c

libalislstorage_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislstorage_la_LDFLAGS = -lc

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC
 
clean-local:
	rm -f libalislstorage_fixed.a

all-local: libalislstorage_fixed.a
 
libalislstorage_fixed.a: .libs/libalislstorage.a
	rm -f libalislstorage_fixed.a
	${AR} cru libalislstorage_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislstorage_fixed.a
	cp -pf libalislstorage_fixed.a .libs/libalislstorage.a

.libs/libalislstorage.a: libalislstorage.la

else

clean-local:

all-local:

endif
