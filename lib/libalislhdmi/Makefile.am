SUBDIRS			= src include

if BUILD_EXAMPLES
SUBDIRS			+= examples
endif

if BUILD_DOCUMENT
SUBDIRS			+= doc
endif

if BUILD_TESTS
TESTS_DIR = tests
else
TESTS_DIR = 
endif

ACLOCAL_AMFLAGS		= -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislhdmi.pc 
