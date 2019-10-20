
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include \
			  -I$(srcdir)/../../libalisldis/include

bin_PROGRAMS		= alislhdmikit

instdir = $(prefix)/bin
dist_inst_SCRIPTS 	=  test_CTS.sh test_PHY.sh

EXTRA_DIST =  test_CTS.sh test_PHY.sh

alislhdmikit_SOURCES	= alislhdmikit.c hdmitest.c disptest.c videotest.c video.c audio.c hdmikumsg.c hdcptest.c

alislhdmikit_LDADD	= $(builddir)/../src/libalislhdmi.la \
			  $(builddir)/../../../src/libaliplatform.la \
			  $(builddir)/../../libalislevent/src/libalislevent.la \
			  $(builddir)/../../libalisldis/src/libalisldis.la

alislhdmikit_LDFLAGS	= -pthread
