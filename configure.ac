AC_INIT(obus, 1.0, johnmh@openblox.org, obus, [https://git.openblox.org/obus.git])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([1.11 foreign])
LT_INIT

AC_PROG_CC
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([LGLIB], [glib-2.0])
PKG_CHECK_MODULES([LZMQ], [libzmq])
PKG_CHECK_MODULES([LJSONC], [json-c])

AC_CONFIG_HEADERS(common/config.h)
AC_CONFIG_FILES([Makefile daemon/Makefile cli/Makefile])

AC_OUTPUT
