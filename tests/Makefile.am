## Makefile.am for aliplatform library unitest

AM_CPPFLAGS = \
	-I$(top_srcdir)/include                                         \
	-I$(srcdir)                                                     \
	-I$(srcdir)/include                                             \
	-I$(srcdir)/../lib/libalislcli/include                          \
	-I$(srcdir)/../lib/libaliunity/include                          \
	-I$(srcdir)/../lib/libalislnim/include                          \
	-I$(srcdir)/../lib/libalislsnd/include                          \
	-I$(srcdir)/../lib/libalisldis/include                          \
	-I$(srcdir)/../lib/libalislvdec/include                         \
	-I$(srcdir)/../lib/libalisltsi/include                          \
	-I$(srcdir)/../lib/libalisltsg/include                          \
	-I$(srcdir)/../lib/libalisldmx/include                          \
	-I$(srcdir)/../lib/libalislhdmi/include                         \
	-I$(srcdir)/../lib/libalislstorage/include                      \
	-I$(srcdir)/../lib/libalislsysinfo/include                      \
	-I$(srcdir)/../lib/libalislsdec/include                      	\
	-I$(srcdir)/../lib/libalislvbi/include                          \
	-I$(srcdir)/../lib/libalisldis/tests                            \
	-I$(srcdir)/../lib/libalislvdec/tests                           \
	-I$(srcdir)/../lib/libalisldmx/tests                            \
	-I$(srcdir)/../lib/libalislsysinfo/tests                        \
	-I$(srcdir)/../lib/libalislotp/tests                            \
	-I$(srcdir)/../lib/libalislotp/include                          \
	-I$(srcdir)/../lib/libalislfake/tests                           \
	-I$(srcdir)/../lib/libalislhdmi/tests                           \
	-I$(srcdir)/../lib/libalislfake/include                         \
	-I$(srcdir)/../lib/libalislsnd/tests                            \
	-I$(srcdir)/../lib/libalislsoc/tests                            \
	-I$(srcdir)/../lib/libalislsoc/include                          \
	-I$(srcdir)/../lib/libalislstorage/tests                        \
	-I$(srcdir)/../lib/libalislsmc/include                          \
	-I$(srcdir)/../lib/libalislvbi/tests                            \
	-I$(srcdir)/../lib/libalislnim/tests                            \
	-I$(srcdir)/../lib/libalisltsi/tests                            \
	-I$(srcdir)/../lib/libalisltsg/tests                            \
	-I$(srcdir)/../lib/libalislsmc/tests                            \
	-I$(srcdir)/../lib/libalislavsync/include                            \
	-I$(srcdir)/../lib/libalislsdec/tests

bin_PROGRAMS = \
	pltftest

pltftest_SOURCES =                                                  \
	ini_config.c                                                    \
	sltest_common.c                                                 \
	$(builddir)/../lib/libalisldis/tests/alisldis_test.c            \
	$(builddir)/../lib/libalislvdec/tests/alislvdec_test.c          \
	$(builddir)/../lib/libalisldmx/tests/alisldmx_test.c            \
	$(builddir)/../lib/libalislotp/tests/alislotp_test.c            \
	$(builddir)/../lib/libalislfake/tests/alislfake_test.c          \
	$(builddir)/../lib/libalislhdmi/tests/alislhdmi_test.c          \
	$(builddir)/../lib/libalislsnd/tests/alislsnd_test.c            \
	$(builddir)/../lib/libalislsysinfo/tests/alislsysinfo_test.c    \
	$(builddir)/../lib/libalislsoc/tests/alislsoc_test.c            \
	$(builddir)/../lib/libalislstorage/tests/alislsto_test.c        \
	$(builddir)/../lib/libalislsdec/tests/alislsdec_test.c			\
	$(builddir)/../lib/libalislvbi/tests/alislvbi_test.c            \
	$(builddir)/../lib/libalislnim/tests/alislnim_test.c            \
	$(builddir)/../lib/libalisltsi/tests/alisltsi_test.c            \
	$(builddir)/../lib/libalisltsg/tests/alisltsg_test.c            \
	$(builddir)/../lib/libalislsmc/tests/alislsmc_test.c            \
	pltftest.c

pltftest_LDADD = \
	$(builddir)/../src/libaliplatform.la                            \
	$(builddir)/../lib/libalislcli/src/libalislcli.la               \
	$(builddir)/../lib/libaliunity/src/libaliunity.la               \
	$(builddir)/../lib/libalisldmx/src/libalisldmx.la               \
	$(builddir)/../lib/libalislvdec/src/libalislvdec.la             \
	$(builddir)/../lib/libalislsnd/src/libalislsnd.la               \
	$(builddir)/../lib/libalislvbi/src/libalislvbi.la               \
	$(builddir)/../lib/libalisltsi/src/libalisltsi.la               \
	$(builddir)/../lib/libalisldis/src/libalisldis.la               \
	$(builddir)/../lib/libalislnim/src/libalislnim.la               \
	$(builddir)/../lib/libalisltsi/src/libalisltsi.la               \
	$(builddir)/../lib/libalisltsg/src/libalisltsg.la               \
	$(builddir)/../lib/libalislotp/src/libalislotp.la               \
	$(builddir)/../lib/libalislfake/src/libalislfake.la             \
	$(builddir)/../lib/libalislsoc/src/libalislsoc.la               \
	$(builddir)/../lib/libalislhdmi/src/libalislhdmi.la             \
	$(builddir)/../lib/libalislsysinfo/src/libalislsysinfo.la       \
	$(builddir)/../lib/libalislstorage/src/libalislstorage.la       \
	$(builddir)/../lib/libalislsmc/src/libalislsmc.la               \
	$(builddir)/../lib/libalislavsync/src/libalislavsync.la               \
	$(builddir)/../lib/libalislsdec/src/libalislsdec.la

pltftest_LDFLAGS = \
	-pthread

data_DATA = ./board_config/*
