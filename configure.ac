#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
m4_define(SYLNOTIFY_MAJOR, 0)
m4_define(SYLNOTIFY_MINOR, 3)
m4_define(SYLNOTIFY_MICRO, 0)
m4_define(SYLNOTIFY_VERSION,
	  [SYLNOTIFY_MAJOR.SYLNOTIFY_MINOR.SYLNOTIFY_MICRO])
AC_INIT([sylnotify], [SYLNOTIFY_VERSION])
AC_CONFIG_SRCDIR([src/sylnotify.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(MAJOR_VERSION, [SYLNOTIFY_MAJOR])
AC_SUBST(MINOR_VERSION, [SYLNOTIFY_MINOR])
AC_SUBST(MICRO_VERSION, [SYLNOTIFY_MICRO])

AM_INIT_AUTOMAKE

AC_DEFINE([SYLPF_ID], [PACKAGE_NAME], [""])
AC_DEFINE([SYLPF_OPTION], [sylnotify_option], [""])


# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
AM_PATH_GLIB_2_0(2.8.0,, AC_MSG_ERROR(Test for GLib failed. See the 'INSTALL' for help.))
AM_PATH_GTK_2_0(2.8.0,, AC_MSG_ERROR(Test for Gtk failed. See the 'INSTALL' for help.))
GETTEXT_PACKAGE=sylnotify
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define text domain.])
AM_GLIB_GNU_GETTEXT

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Checks for debug
AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug],
                              [use debug flags (default=no)])],
  [plugin_debug="$enableval"],
  [plugin_debug="no"])
if test "x$plugin_debug" != "xno"; then
  plugin_debug="yes"
  if test "$CLANG" = "yes"; then
    CFLAGS="$CFLAGS -O0 -g"
    CXXFLAGS="$CXXFLAGS -O0 -g"
  elif test "$GCC" = "yes"; then
    CFLAGS="$CFLAGS -O0 -g3"
    CXXFLAGS="$CXXFLAGS -O0 -g3"
  fi
fi
AC_SUBST(plugin_debug)

m4_include(macros/sylplugin_factory.m4)

PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTK], [gtk+-2.0])
PKG_CHECK_MODULES([SYLPF], [sylplugin-factory])

AC_ARG_ENABLE(libnotify,
              [AS_HELP_STRING([--enable-libnotify],
                              [Enable libnotify for notification (default=no)])],
  [use_libnotify="$enableval"],
  [use_libnotify="no"])
if test "x$use_libnotify" != "xno"; then
  use_libnotify="yes"
  PKG_CHECK_MODULES([LIBNOTIFY], [libnotify])
  AC_DEFINE([WITH_LIBNOTIFY], [1], [Enabled libnotify])
  AM_CONDITIONAL(WITH_LIBNOTIFY, test x$use_libnotify != xno)
fi

AC_ARG_ENABLE(appindicator,
              [AS_HELP_STRING([--enable-appindicator],
                              [Enable libappindicator for notification (default=no)])],
  [use_appindicator="$enableval"],
  [use_appindicator="no"])
if test "x$use_appindicator" != "xno"; then
  PKG_CHECK_MODULES([LIBAPPINDICATOR], [appindicator-0.1])
  AC_DEFINE([WITH_LIBAPPINDICATOR], [1], [Enabled libappindicator])
  AM_CONDITIONAL(WITH_LIBAPPINDICATOR, test x$use_appindicator != xno)
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile
])
AC_OUTPUT

echo
echo "Notifications"
echo "     libnotify: $use_libnotify"
echo "  appindicator: $use_appindicator"
echo
