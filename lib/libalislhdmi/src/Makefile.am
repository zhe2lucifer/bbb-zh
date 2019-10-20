AM_CFLAGS = -DSL_MOD=ALIPLTF_HDMI

AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalislhdmi.la

libalislhdmi_la_SOURCES	= alislhdmi.c

libalislhdmi_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalislhdmi_la_LDFLAGS	= -lrt -version-info 0:0:0
