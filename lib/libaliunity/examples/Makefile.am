
AM_CPPFLAGS		= -I$(srcdir)/../include

bin_PROGRAMS		= aliunity

aliunity_SOURCES	= main_entry.c \
			  testunity_fixture.c \
			  unity_fixture_TestRunner.c \
			  unity_fixture_Test.c \
			  unity_output_Spy.c

aliunity_LDADD		= $(builddir)/../src/libaliunity.la
