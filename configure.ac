# configure.ac - Configuration for rippix
# Copyright (C) 2011,2012 Aljosha Papsch <papsch.al@googlemail.com>
# and ripperX developers (http://sourceforge.net/projects/ripperx)
#
# This file is part of Rippix.
#
# Rippix is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# Rippix is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see `http://www.gnu.org/licenses/'.

#
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.60)
AC_INIT([Rippix], [3.5], [papsch.al@googlemail.com], [rippix], [http://rippix.sourceforge.net])
AC_CONFIG_SRCDIR([src/common.h])
AC_CONFIG_AUX_DIR(config)

PACKAGE=rippix

#
# Rippix version
#
MAJOR_VERSION=3
MINOR_VERSION=5
VERSION=${MAJOR_VERSION}.${MINOR_VERSION}

#
# Required versions for pkg-config
#
GLIB_REQUIRED=2.6
GTK_REQUIRED=3.0
GCONF_REQUIRED=3.0

AM_INIT_AUTOMAKE([-Wall -Werror])
dnl AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
dnl AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$PKG_CONFIG = xno ; then
  AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/])
fi

AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
if test x$GCONFTOOL = xno ; then
   AC_MSG_ERROR([*** gconftool-2 not found])
fi

#
# libtool versioning
#
LT_RELEASE=$MAJOR_VERSION.$MINOR_VERSION
LT_REVISION=$INTERFACE_AGE
LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

#
# Internationalization
#
ALL_LINGUAS="en_GB es it gl de"
GETTEXT_PACKAGE=rippix
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define text domain.])

AM_GNU_GETTEXT_VERSION([0.15])
AM_GNU_GETTEXT([external])

#
# Specify a header configuration file
#
AC_CONFIG_HEADERS(config.h)

AM_MAINTAINER_MODE([enable])

#
# Define some macros
#

AC_DEFINE([_(string)], [gettext(string)], [abbrev for gettext function])
AC_DEFINE([CONFIG_OP_OPEN], [100], [Indicates to open the GConf database])
AC_DEFINE([CONFIG_OP_CLOSE], [101], [Indicates to close the GConf database])

#
# 
#
AC_PROG_CC_C99
AC_ISC_POSIX
AC_PROG_INSTALL
AC_PROG_CPP
AM_PROG_LIBTOOL
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(pty.h fcntl.h strings.h sys/ioctl.h unistd.h)

#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T

CFLAGS="$CFLAGS -Wall"
if test -z $GCONF_CONFIG_SOURCE; then
   GCONF_CONFIG_SOURCE=$(gconftool-2 --get-default-source)
fi
AC_SUBST(GCONF_CONFIG_SOURCE)

#
# Checks for library functions.
#
AC_CHECK_FUNCS(socket strcspn strdup strerror strstr)
AC_CHECK_FUNCS(openpty,, 
		AC_CHECK_LIB(util,openpty,
			[AC_DEFINE(HAVE_OPENPTY)]
			[LIBS="$LIBS -lutil -lz -lstdc++"]
))
AC_CHECK_FUNCS(bind_textdomain_codeset)

#
# Set PACKAGE_DATA_DIR in config.h.
#
if test "x${datarootdir}" = 'x${prefix}/share'; then
	if test "x${prefix}" = "xNONE"; then
    		AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}",[PACKAGE_DATA_DIR])
  	else
    		AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}",[PACKAGE_DATA_DIR])
  	fi
else
	AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datarootdir}/${PACKAGE}",[PACKAGE_DATA_DIR])
fi

#
# Library checks
#
AC_CHECK_LIB(xpg4, setlocale)

AC_CHECK_LIB(id3, ID3Tag_Link, [], [ID3_LIBS="" ac_have_id3="no"])
if test x"$ac_have_id3" = xno; then
	AC_MSG_ERROR([libid3 not found])
fi


#rx_save_LIBS=$LIBS
#LIBS="$LIBS $GTK_LIBS"

#LIBS=$rx_save_LIBS

#
# Checks for packages
#
PKG_CHECK_MODULES([DEPENDENCIES], [
                  glib-2.0 >= $GLIB_REQUIRED
                  gmodule-2.0
                  gthread-2.0
                  gtk+-3.0 >= $GTK_REQUIRED
                  gconf-2.0 >= $GCONF_REQUIRED
                  ])

#LIBS="$LIBS $GLIB_LIBS $GTK_LIBS $GCONF_LIBS"


dnl --disable-deprecated switch for GTK2 purification
AC_ARG_ENABLE(deprecated, [  --disable-deprecated  Disable deprecated GTK functions. ],
			  [GTK_CFLAGS="$GTK_CFLAGS -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"], [])

#AC_SUBST(GTK_CFLAGS)
#AC_SUBST(LIBS)

AC_OUTPUT([
Makefile
share/Makefile
po/Makefile.in
src/version.h
src/Makefile
config/Makefile
plugins/Makefile
ripperX.pc
])

dnl Output the configuration summary
echo ""
echo "$PACKAGE $VERSION"
echo ""
echo "The binary will be installed in $prefix/bin"
echo ""
echo "Configure finished, type 'make' to build."
