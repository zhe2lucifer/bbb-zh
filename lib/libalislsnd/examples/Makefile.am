
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

bin_PROGRAMS		= alislsndkit

alislsndkit_SOURCES	= alislsndkit.c

alislsndkit_LDADD	= $(builddir)/../src/libalislsnd.la \
			  $(builddir)/../../../src/libaliplatform.la \
			  $(builddir)/../../libalislevent/src/libalislevent.la

alislsndkit_LDFLAGS	= -pthread
