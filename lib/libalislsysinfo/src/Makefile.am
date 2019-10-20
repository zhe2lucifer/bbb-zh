## Makefile.am for sysinfo
AM_CFLAGS = -DSL_MOD=ALIPLTF_SYSINFO

AM_CPPFLAGS = \
	-I$(srcdir)/../include                       \
	-I$(srcdir)/../../libalislstorage/include    \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislsysinfo.la

libalislsysinfo_la_SOURCES =  \
	alislsysinfo.c        \
	alislsysinfoparser.c

libalislsysinfo_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislsysinfo_la_LDFLAGS = -lc

#
## and now rebuild the static version with the *correct* object files
#
if BUILD_STATIC

clean-local:
	rm -f libalislsysinfo_fixed.a

all-local: libalislsysinfo_fixed.a

libalislsysinfo_fixed.a: .libs/libalislsysinfo.a
	rm -f libalislsysinfo_fixed.a
	${AR} cru libalislsysinfo_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislsysinfo_fixed.a
	cp -pf libalislsysinfo_fixed.a .libs/libalislsysinfo.a

.libs/libalislsysinfo.a:libalislsysinfo.la

else

clean-local:

all-local:

endif
