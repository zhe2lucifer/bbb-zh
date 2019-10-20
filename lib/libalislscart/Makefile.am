## Makefiel for libalislscart

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh

if BUILD_EXAMPLES
TEST_DIR = examples
else
TEST_DIR =
endif

if BUILD_DOCUMENT
DOCS_DIR = doc
else
DOCS_DIR =
endif

SUBDIRS =   \
	src     \
	include \
	$(DOCS_DIR) \
	$(TEST_DIR)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libalislscart.pc

# Generate teh HTML API documentation:

html-local:
	make -C doc/doxygen html-local

.PHONY: html-local
