## Makefile.am for upgrade

ACLOCAL_AMFLAGS = -I m4

#if BUILD_EXAMPLES
TESTS_DIR = examples
#else
#TESTS_DIR = 
#endif

if BUILD_DOCUMENT
DOCS_DIR = doc
else
DOCS_DIR = 
endif

SUBDIRS = \
	src             \
	include         \
	$(DOCS_DIR)     \
	$(TESTS_DIR)


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislupgrade.pc

# Generate the HTML API documentation:
html-local:
	make -C doc/doxygen html-local

.PHONY: html-local

