## Makefile.am for libalislgpio

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

if BUILD_TESTS
TESTS_DIR = tests
else
TESTS_DIR = 
endif

SUBDIRS = \
	src \
	include \
	$(DOCS_DIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislgpio.pc

# Generate the HTML API documentation:
html-local:
	make -C doc/doxygen html-local

.PHONY: html-local
