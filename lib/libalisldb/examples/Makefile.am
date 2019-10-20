
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include \
			  -I$(srcdir)/../lib/sqlite-src-3071700

bin_PROGRAMS		= alisldbkit

alisldbkit_SOURCES	= alisldbkit.c

alisldbkit_LDADD	= $(builddir)/../src/libalisldb.la \
			  $(builddir)/../../../src/libaliplatform.la \
			  $(builddir)/../lib/sqlite-src-3071700/libsqlite3.la

alisldbkit_LDFLAGS	= -pthread
