# Makefile for libalislsoc

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh

SUBDIRS = src include

if BUILD_EXAMPLES
SUBDIRS += examples
endif

if BUILD_DOCUMENT
SUBDIRS += doc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislsoc.pc

# Generate the HTML API documentation:
html-local:
	make -C doc/doxygen html-local

.PHONY: html-local
