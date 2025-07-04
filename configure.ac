dnl Process this file with autoconf to produce a configure script.
dnl Created originally by the Anjuta application wizard.

dnl required version of autoconf
AC_PREREQ([2.53])

AC_INIT(sound_effects_player, [0.131], [John_Sauter@systemeyescomputerstore.com], sound_effects_player, [git@github.com:ShowControl/sound_effects_player.git])
AC_DEFINE([MODIFICATION_DATE], ["May 25, 2025"], [last modification date])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])
AC_CONFIG_MACRO_DIR([m4])

dnl checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h])

dnl checks for typedefs, structures, and compiler characteristics.
AC_STRUCT_TIMEZONE
AC_TYPE_SIZE_T

dnl checks for library functions.
AC_FUNC_MKTIME
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([xml2], [xmlDocGetRootElement])
AC_CHECK_FUNCS([localtime_r])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strtol])

dnl set LDFLAGS and LIBS so we can depend on libtime.
LDFLAGS='-L/usr/local/lib'
AC_SEARCH_LIBS([time_current_tm],[time],[ ],[
  AC_MSG_ERROR([You need to have libtime installed.
  Libtime is a component of ShowControl.  You can get it
  from github at this URL: git@github.com:ShowControl/libtime.git
  or for Fedora you can install libtime-devel from copr at
  johnsauter / libtime.
  ])
])	
AC_CHECK_FUNCS([time_current_tm])

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=sound_effects_player
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT

LT_PREREQ([2.2.6])
LT_INIT

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

dnl required versions of gstreamer and plugins-base
GST_REQUIRED=1.14.0
GSTPB_REQUIRED=1.14.0


dnl Check for the required version of GStreamer core (and gst-plugins-base)
dnl This will export GST_CFLAGS and GST_LIBS variables for use in Makefile.am
dnl
dnl If you need libraries from gst-plugins-base here, also add:
dnl for libgstaudio-1.0: gstreamer-audio-1.0 >= $GST_REQUIRED
dnl for libgstvideo-1.0: gstreamer-video-1.0 >= $GST_REQUIRED
dnl for libgsttag-1.0: gstreamer-tag-1.0 >= $GST_REQUIRED
dnl for libgstpbutils-1.0: gstreamer-pbutils-1.0 >= $GST_REQUIRED
dnl for libgstfft-1.0: gstreamer-fft-1.0 >= $GST_REQUIRED
dnl for libgstinterfaces-1.0: gstreamer-interfaces-1.0 >= $GST_REQUIRED
dnl for libgstrtp-1.0: gstreamer-rtp-1.0 >= $GST_REQUIRED
dnl for libgstrtsp-1.0: gstreamer-rtsp-1.0 >= $GST_REQUIRED
dnl etc.
PKG_CHECK_MODULES(GST, [
  gstreamer-1.0 >= $GST_REQUIRED
  gstreamer-base-1.0 >= $GST_REQUIRED
  gstreamer-controller-1.0 >= $GST_REQUIRED
  gstreamer-audio-1.0 >= $GST_REQUIRED
], [
  AC_SUBST(GST_CFLAGS)
  AC_SUBST(GST_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the GStreamer development
      packages on your system. On debian-based systems these are
      libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev.
      on RPM-based systems gstreamer1-devel, gstreamer1-plugins-base-devel
      or similar. The minimum version required is $GST_REQUIRED.
  ])
])

PKG_CHECK_MODULES(SOUND_EFFECTS_PLAYER, \
[gtk+-3.0 >= 3.14 gstreamer-1.0 gtk+-3.0 gstreamer-allocators-1.0 \
gstreamer-plugins-base-1.0 gstreamer-base-1.0 gstreamer-app-1.0 \
gstreamer-controller-1.0 gstreamer-audio-1.0 libxml-2.0 gtk-doc >= 1.14])

dnl check if compiler understands -Wall (if yes, add -Wall to GST_CFLAGS)
AC_MSG_CHECKING([to see if compiler understands -Wall])
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([ ], [ ])], [
  GST_CFLAGS="$GST_CFLAGS -Wall"
  AC_MSG_RESULT([yes])
], [
  AC_MSG_RESULT([no])
])

dnl set the plugindir where plugins should be installed (for src/Makefile.am)
if test "x${prefix}" = "x$HOME"; then
  plugindir="$HOME/.gstreamer-1.0/plugins"
else
  plugindir="\$(libdir)/gstreamer-1.0"
fi
AC_SUBST(plugindir)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version \
-export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

# check for gtk-doc
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

dnl set license, copyright notice, package name and package origin
dnl for the gstreamer plugins
GST_LICENSE="LGPL"
AC_DEFINE_UNQUOTED(GST_LICENSE, "$GST_LICENSE", [Gstreamer license])
AC_SUBST(GST_LICENSE)
GST_PACKAGE_NAME="Gstreamer source release"
AC_DEFINE_UNQUOTED(GST_PACKAGE_NAME, "$GST_PACKAGE_NAME", \
[Gstreamer package name])
AC_SUBST(GST_PACKAGE_NAME)
GST_PACKAGE_ORIGIN="https://developer.gnome.org/gstreamer/"
AC_DEFINE_UNQUOTED(GST_PACKAGE_ORIGIN, "$GST_PACKAGE_ORIGIN", \
[Gstreamer package origin])
AC_SUBST(GST_PACKAGE_ORIGIN)

AC_CONFIG_FILES([
Makefile
src/Makefile
man/Makefile
po/Makefile.in
doc/Makefile
Docs/Makefile
icons/Makefile
sample/Makefile
examples/Makefile
examples/01/Makefile
examples/02/Makefile
examples/03/Makefile
examples/04/Makefile
])
AC_OUTPUT

# End of file configure.ac
