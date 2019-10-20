# Makefile.am for otp 
AM_CFLAGS = -DSL_MOD=ALIPLTF_OTP

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(srcdir)/../include \
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalislotp.la

libalislotp_la_SOURCES = \
	alislotp.c \
	alislotp_wrapper.c

libalislotp_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislotp_la_LDFLAGS = \
	-version-info 0:0:0 -release 0.0

EXTRA_DIST = internal.h

if BUILD_STATIC

clean-local:
	rm -f libalislotp_fixed.a

all-local: libalislotp_fixed.a

libalislotp_fixed.a: .libs/libalislotp.a
	rm -f libalislotp_fixed.a
	${AR} cru libalislotp_fixed.a `find . -name "*.o" | grep -v '.libs'`
	${RANLIB} libalislotp_fixed.a
	cp -pf libalislotp_fixed.a .libs/libalislotp.a

.libs/libalislotp.a: libalislotp.la

else

clean-local:

all-local:

endif
