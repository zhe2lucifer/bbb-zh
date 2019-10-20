
AM_CPPFLAGS		= -I$(srcdir)/../include \
			  -I$(srcdir)/../../../include

lib_LTLIBRARIES		= libalisldb.la

libalisldb_la_SOURCES	= alisldb_common.c \
			  alisldb_sys.c

libalisldb_la_LIBADD = $(builddir)/../../../src/libaliplatform.la
libalisldb_la_LDFLAGS	= -version-info 0:0:0

alisldbconfdir		= $(prefix)/etc/alisldb
alisldbconf_DATA	= alisldb.conf \
			  sys_default.sql \
			  dvb_default.sql \
			  ott_default.sql \
			  hbbtv_default.sql
