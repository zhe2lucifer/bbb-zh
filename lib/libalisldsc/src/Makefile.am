## Makefile.am for libalisldsc
AM_CFLAGS = -DSL_MOD=ALIPLTF_DSC

AM_CPPFLAGS = \
	-I$(top_srcdir)/include 					\
	-I$(srcdir)/../include 						\
	-I$(srcdir)/../../../include

lib_LTLIBRARIES = libalisldsc.la

libalisldsc_la_SOURCES = 	\
	alisl_algo_api.c			\
	alisl_sha_api.c         \
	alisldsc.c \
	alisl_pvr_api.c  

if BUILD_VMXPLUS	
libalisldsc_la_SOURCES += alislvmx.c
endif

libalisldsc_la_LIBADD = $(builddir)/../../../src/libaliplatform.la

libalisldsc_la_LDFLAGS = \
	-version-info 0:0:0

EXTRA_DIST = dsc_internal.h

