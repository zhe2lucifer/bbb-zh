## Makefile.am for libaliplatform


ACLOCAL_AMFLAGS = -I m4 -Wall

AM_CFLAGS = -Wall

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
	src			\
	lib			\
	include		\
	$(EXAMPLES_DIR)	\
	$(TESTS_DIR)		\
	$(DOCS_DIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaliplatform.pc

# Generate the HTML API documentation:
html-local:

.PHONY: html-local
