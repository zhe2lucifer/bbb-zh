dnl
dnl libalislsm configure.ac
dnl
AC_INIT([libalislsnd], [0.0.0], [ze.hong@alitech.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
LT_INIT
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])

AC_ARG_ENABLE([examples],
              [AC_HELP_STRING([--enable-examples],[enable example build [default=no]])],
              [
                  case "${enableval}" in
                  yes|no)
                      enable_examples=${enableval} ;;
                       *)
                      AC_MSG_ERROR(bad value ${enableval} for --enable-examples) ;;
                  esac
              ],
              [enable_examples=no])

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],[enable debug info [default=no]])],
              [
                  case "${enableval}" in
                  yes|no)
                      enable_debug=${enableval} ;;
                       *)
                      AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
                  esac
              ],
              [enable_debug=no])

AC_ARG_ENABLE([docs],
              [AC_HELP_STRING([--enable-docs],[enable docs info [default=no]])],
              [
                  case "${enableval}" in
                  yes|no)
                      enable_docs=${enableval} ;;
                       *)
                      AC_MSG_ERROR(bad value ${enableval} for --enable-docs) ;;
                  esac
              ],
              [enable_docs=no])

if test "$enable_debug" = "yes"; then
	CFLAGS="-g3 -O0 -fno-inline -Wno-inline $CFLAGS"
	AC_DEFINE(ENABLE_DEBUG, 1, [Define this macro if you want to print debug message.])
fi

if test "$enable_docs" = "yes"; then
	AC_CHECK_PROG(DOXYGEN, [doxygen], [true], [false])
fi

AM_CONDITIONAL(BUILD_DEBUG, test "$enable_debug" = "yes")
AM_CONDITIONAL(BUILD_EXAMPLES, test "$enable_examples" = "yes")
AM_CONDITIONAL(BUILD_DOCUMENT, test "$DOXYGEN" = "true")
AM_CONDITIONAL(BUILD_BYPLATFORM, test "yes" = "no")

AC_CONFIG_FILES([
Makefile
libalislsnd.pc
include/Makefile
src/Makefile
examples/Makefile
doc/Makefile
doc/doxygen/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([])
AC_MSG_RESULT([examples    : ${enable_examples}])
AC_MSG_RESULT([debug       : ${enable_debug}])
AC_MSG_RESULT([docs        : ${DOXYGEN}])
