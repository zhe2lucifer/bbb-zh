
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include \
			  -I$(srcdir)/../../libalisltsi/include \
			  -I$(srcdir)/../../libalisldis/include \
			  -I$(srcdir)/../../libalislvdec/include \
			  -I$(srcdir)/../../libalislsnd/include \
			  -I$(srcdir)/../../libalisldmx/include
			  -I$(srcdir)/../../libalisldmx/include/ \
			  -I$(srcdir)/../../libalisltsi/include/

bin_PROGRAMS		= alisltsgkit

alisltsgkit_SOURCES	= alisltsgkit.c

alisltsgkit_LDADD	= $(builddir)/../src/libalisltsg.la \
			  $(builddir)/../../../src/libaliplatform.la \
			  $(builddir)/../../libalisltsi/src/libalisltsi.la \
			  $(builddir)/../../libalisldis/src/libalisldis.la \
			  $(builddir)/../../libalislvdec/src/libalislvdec.la \
			  $(builddir)/../../libalislsnd/src/libalislsnd.la \
			  $(builddir)/../../libalisldmx/src/libalisldmx.la \
			  $(builddir)/../../libalislevent/src/libalislevent.la

alisltsgkit_LDFLAGS	= -pthread
