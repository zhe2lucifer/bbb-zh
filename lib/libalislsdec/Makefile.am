SUBDIRS			= src include

if BUILD_EXAMPLES
SUBDIRS			+= examples
endif

if BUILD_DOCUMENT
SUBDIRS			+= doc
endif

ACLOCAL_AMFLAGS		= -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislsdec.pc 
