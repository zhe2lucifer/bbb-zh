
# self include dir:          $(srcdir)/../include
# platform top include dir:  $(srcdir)/../../../include
# other module include dir:  $(srcdir)/../../other_module/include

# self lib dir:              $(builddir)/../src
# other module lib dir:      $(builddir)/../../other_module/src

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include 

bin_PROGRAMS		= alislsdeckit

alislsdeckit_SOURCES	= alislsdeckit.c

alislsdeckit_LDADD	= $(builddir)/../src/libalislsdec.la

alislsdeckit_LDFLAGS	= -pthread
