## Makefile.am for libalislfake

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh

if BUILD_EXAMPLES
EXAMPLES_DIR = examples
else
EXAMPLES_DIR = 
endif

if BUILD_DOCUMENT
DOCS_DIR = doc
else
DOCS_DIR = 
endif

SUBDIRS = \
	src					\
	include 			\
	$(EXAMPLES_DIR) 	\
	$(DOCS_DIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislfake.pc

# Generate the HTML API documentation:
html-local:
	make -C doc/doxygen html-local

.PHONY: html-local
