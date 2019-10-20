## Makefile.am for libalisldsc

ACLOCAL_AMFLAGS = -I m4 -Wall

EXTRA_DIST = autogen.sh

TEST_DIR =

if BUILD_DOCUMENT
DOCS_DIR = doc
else
DOCS_DIR =
endif

SUBDIRS = \
	src           \
	include       \
	$(TEST_DIR)   \
	$(DOCS_DIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalisldsc.pc
