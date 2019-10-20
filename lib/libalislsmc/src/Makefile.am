AM_CFLAGS = -DSL_MOD=ALIPLTF_SMC

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislsmc.la

libalislsmc_la_SOURCES	= alislsmc.c \
			  internel.h

libalislsmc_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislsmc_la_LDFLAGS	= -version-info 0:0:0
