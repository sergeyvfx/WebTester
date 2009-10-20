m4_define([VERSION_NUMBER], m4_esyscmd([cat WT-VERSION-FILE | tr \\n "\\0" | sed -r 's/WT_VERSION\=//']))

AC_INIT(webtester, [VERSION_NUMBER])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

AM_PROG_LIBTOOL

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

AC_ARG_ENABLE(homedir,        AC_HELP_STRING([--enable-homedir], [Set the home directory of WebTester (default=/home/webtester)]),homedir="$enableval",homedir="/home/webtester")
AC_ARG_ENABLE(pidfile,        AC_HELP_STRING([--enable-pidfile], [Set the file name to save PID of WebTester (default=/home/webtester/var/run/webtester.pid)]),pidfile="$enableval",pidfile="/home/webtester/var/run/webtester.pid")
AC_ARG_ENABLE(super_pidfile,  AC_HELP_STRING([--enable-supervisor_pidfile], [Set the file name to save PID of WebTester sUpervisor (default=/home/webtester/var/run/supervisor.pid)]),super_pidfile="$enableval",super_pidfile="/home/webtester/var/run/supervisor.pid")
AC_ARG_ENABLE(logfile,        AC_HELP_STRING([--enable-logfile], [Set the file name to save log of WebTester (default=/home/webtester/var/log/webtester.log)]),logfile="$enableval",logfile="/home/webtester/var/log/webtester.log")
AC_ARG_ENABLE(debug,          AC_HELP_STRING([--enable-debug],   [Compile debug parts of source]),USER_DEBUG="yes",USER_DEBUG="no")
AC_ARG_ENABLE(gui,            AC_HELP_STRING([--enable-gui],     [Compile WebTester Server GUI interface (default=no)]),build_gui="yes",build_gui="no")
AC_ARG_ENABLE(user_debug,     AC_HELP_STRING([--enable-debug],   [Compile debug parts of source]), USER_DEBUG="yes",USER_DEBUG="no")
AC_ARG_ENABLE(kernel,         AC_HELP_STRING([--enable-kernel],  [Set the release version of kernel to built with]),KERNEL_RELEASE="$enableval",KERNEL_RELEASE=`uname -r`)

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

dnl ===================
dnl Gettext stuff
dnl ===================

GETTEXT_PACKAGE=webtester
AC_SUBST(GETTEXT_PACKAGE)
ALL_LINGUAS="en_GB"


dnl ===================
dnl Check for libraries
dnl ===================

GLIB_REQ=2.6.0
GMODULE_REQ=2.0.0
GTHREAD_REQ=2.0.0
SOUP22_REQ=2.2.3
SOUP24_REQ=2.4.1
PCRE_REQ=6.7
GTK_REQ=2.6.0
ENCA_REQ=1.9

AC_SUBST(GLIB_REQ)

dnl plugin stuff
dnl ========================

dnl Ok, first we find out what flags we need to use.
AC_MSG_CHECKING([for what extension and flags to use for plugin compilation])
case "$target" in
  *)
    AC_MSG_RESULT([libdl-compatible: -fPIC -DPIC -shared, .so])
    PICFLAGS="-fPIC -DPIC"
    PICLDFLAGS="-fPIC -DPIC -shared"
    LIBLDFLAGS="-fPIC -DPIC -shared"
    AUDLDFLAGS="-Wl,-export-dynamic $RPATH_VAL"
    SHARED_SUFFIX=".so"
    ;;
esac
AC_SUBST(PICFLAGS)
AC_SUBST(PICLDFLAGS)
AC_SUBST(LIBLDFLAGS)
AC_SUBST(AUDLDFLAGS)
AC_SUBST(SHARED_SUFFIX)
AC_DEFINE_UNQUOTED(SHARED_SUFFIX, "${SHARED_SUFFIX}",
  [Define the shared module suffix extension on your platform.])

dnl Check for glib
AC_MSG_CHECKING(if glib >= ${GLIB_REQ} exists)
if pkg-config --exists glib-2.0 ; then
  GLIB_CFLAGS=`pkg-config --cflags glib-2.0`
  GLIB_LIBS=`pkg-config --libs glib-2.0`
  GLIB_PREFIX=`pkg-config --variable=prefix glib-2.0`
  GLIB_VERSION=`pkg-config --modversion glib-2.0`
  AC_SUBST(GLIB_VERSION)
  AC_SUBST(GLIB_PREFIX)
  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)

  if pkg-config --exists glib-2.0 '>=' ${GLIB_REQ} ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_ERROR("only ${GLIB_VERSION} found")
  fi
else
  AC_MSG_ERROR(no)
fi

dnl Check for gthread
AC_MSG_CHECKING(if gthread >= ${GTHREAD_REQ} exists)
if pkg-config --exists gthread-2.0 ; then
  GLIB_CFLAGS=`pkg-config --cflags gthread-2.0`
  GLIB_LIBS=`pkg-config --libs gthread-2.0`
  GLIB_PREFIX=`pkg-config --variable=prefix gthread-2.0`
  GLIB_VERSION=`pkg-config --modversion gthread-2.0`
  AC_SUBST(GTHREAD_VERSION)
  AC_SUBST(GTHREAD_PREFIX)
  AC_SUBST(GTHREAD_CFLAGS)
  AC_SUBST(GTHREAD_LIBS)

  if pkg-config --exists gthread-2.0 '>=' ${GTHREAD_REQ} ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_ERROR("only ${GTHREAD_VERSION} found")
  fi
else
  AC_MSG_ERROR(no)
fi

dnl Check for gmodule
AC_MSG_CHECKING(if gmodule >= ${GMODULE_REQ} exists)
if pkg-config --exists gmodule-2.0 ; then
  if pkg-config --exists gmodule-2.0 '>=' ${GMODULE_REQ} ; then
    AC_MSG_RESULT(yes)
    GMODULE_AVAIL="yes"
  else
    AC_MSG_ERROR("only `pkg-config --modversion gmodule-2.0` found")
    GMODULE_AVAIL="no"
  fi
else
  AC_MSG_ERROR(no)
  GMODULE_AVAIL="no"
fi
AC_SUBST(GMODULE_AVAIL)

dnl Check for gtk

if test "x$build_gui" = "xyes"; then
  AC_MSG_CHECKING(if gtk+-2.0 >= ${GTK_REQ} exists)
  if pkg-config --exists gtk+-2.0 ; then
    GTK_CFLAGS=`pkg-config --cflags gtk+-2.0`
    GTK_LIBS=`pkg-config --libs gtk+-2.0`
    GTK_PREFIX=`pkg-config --variable=prefix gtk+-2.0`
    GTK_VERSION=`pkg-config --modversion gtk+-2.0`
    AC_SUBST(GTK_VERSION)
    AC_SUBST(GTK_PREFIX)
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)

    if pkg-config --exists gtk+-2.0 '>=' ${GTK_REQ} ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
      echo "gtk+-2.0 version only ${GTK_VERSION} found"
      build_gui="no"
    fi
  else
    AC_MSG_RESULT(no)
    build_gui="no"
  fi
fi
BUILD_GUI="${build_gui}"
AC_SUBST(BUILD_GUI)

dnl Check for libsoup
AC_MSG_CHECKING(if libsoup-2.4 >= ${SOUP24_REQ} exists)
if pkg-config --exists libsoup-2.4 ; then
    SOUP_CFLAGS=`pkg-config --cflags libsoup-2.4`
    SOUP_LIBS=`pkg-config --libs libsoup-2.4`
    SOUP_PREFIX=`pkg-config --variable=prefix libsoup-2.4`
    SOUP_VERSION=`pkg-config --modversion libsoup-2.4`

    if pkg-config --exists libsoup-2.4 '>=' ${SOUP24_REQ} ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE_UNQUOTED(LIBSOUP_24,"1",[Major version of libsoup])
    else
        AC_MSG_RESULT("only ${SOUP_VERSION} found")
    fi
else
    AC_MSG_RESULT(no)
fi

if test "x$SOUP_CFLAGS" = "x"; then
  AC_MSG_CHECKING(if libsoup-2.2 >= ${SOUP22_REQ} exists)
  if pkg-config --exists libsoup-2.2 ; then
      SOUP_CFLAGS=`pkg-config --cflags libsoup-2.2`
      SOUP_LIBS=`pkg-config --libs libsoup-2.2`
      SOUP_PREFIX=`pkg-config --variable=prefix libsoup-2.2`
      SOUP_VERSION=`pkg-config --modversion libsoup-2.2`

      if pkg-config --exists libsoup-2.2 '>=' ${SOUP22_REQ} ; then
          AC_MSG_RESULT(yes)
          AC_DEFINE_UNQUOTED(LIBSOUP_22,"1",[Major version of libsoup])
      else
          AC_MSG_RESULT("only ${SOUP_VERSION} found")
      fi
  else
      AC_MSG_RESULT(no)
  fi
fi

if test "x$SOUP_CFLAGS" = "x"; then
  echo "Unable to find sutable libsoup library"
  exit 1
fi

AC_SUBST(SOUP_VERSION)
AC_SUBST(SOUP_PREFIX)
AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)

dnl Check for libpcre
AC_MSG_CHECKING(if libpcre >= ${PCRE_REQ} exists)
if pkg-config --exists libpcre ; then
  PCRE_CFLAGS=`pkg-config --cflags libpcre`
  PCRE_LIBS=`pkg-config --libs libpcre`
  PCRE_PREFIX=`pkg-config --variable=prefix libpcre`
  PCRE_VERSION=`pkg-config --modversion libpcre`
  AC_SUBST(PCRE_VERSION)
  AC_SUBST(PCRE_PREFIX)
  AC_SUBST(PCRE_CFLAGS)
  AC_SUBST(PCRE_LIBS)

  if pkg-config --exists libpcre '>=' ${PCRE_REQ} ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_ERROR("only ${PCRE_VERSION} found")
  fi
else
  AC_MSG_ERROR(no)
fi

dnl Check for enca
AC_MSG_CHECKING(if enca >= ${ENCA_REQ} exists)
if pkg-config --exists enca ; then
  ENCA_CFLAGS=`pkg-config --cflags enca`
  ENCA_LIBS=`pkg-config --libs enca`
  ENCA_PREFIX=`pkg-config --variable=prefix enca`
  ENCA_VERSION=`pkg-config --modversion enca`
  AC_SUBST(ENCA_VERSION)
  AC_SUBST(ENCA_PREFIX)
  AC_SUBST(ENCA_CFLAGS)
  AC_SUBST(ENCA_LIBS)

  if pkg-config --exists enca '>=' ${ENCA_REQ} ; then
    AC_DEFINE_UNQUOTED(USE_ENCA, 1, [Use enca library])
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT("only ${ENCA_VERSION} found")
  fi
else
  AC_MSG_RESULT(no)
fi

dnl Check for smbclient
SMBCLIENT_CFLAGS=""
SMBCLIENT_LIBS="-lsmbclient -ldl"
AC_SUBST(SMBCLIENT_CFLAGS)
AC_SUBST(SMBCLIENT_LIBS)

dnl Check for kernel source
echo -n "Checking for kernel... "
KERNEL_SOURCE="/lib/modules/${KERNEL_RELEASE}/source"
if [[ -d ${KERNEL_SOURCE} ]]; then
  echo "ok."
else
  KERNEL_SOURCE="/lib/modules/${KERNEL_RELEASE}/build"
  if [[ -d ${KERNEL_SOURCE} ]]; then
    echo "ok."
  else
    echo "failed."
    exit -1;
  fi
fi

dnl =============================
dnl Checks for library functions
dnl =============================

AC_FUNC_MMAP

dnl =====================
dnl Set stuff in config.h
dnl =====================

dnl Get the prefix
if test "x${prefix}" = "xNONE"; then
  PREFIX="${ac_default_prefix}"
  AC_DEFINE_UNQUOTED(PREFIX, "${ac_default_prefix}", [definition of PREFIX])
else
  PREFIX="${prefix}"
  AC_DEFINE_UNQUOTED(PREFIX, "${prefix}", [definition of PREFIX])
fi
AC_SUBST(PREFIX)


dnl Set PACKAGE_LOCALE_DIR in config.h.
AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${PREFIX}/${DATADIRNAME}/locale", [definition of PACKAGE_LOCALE_DIR])

dnl Set PIXMAPS_DIR in config.h.
AC_DEFINE_UNQUOTED(PIXMAPS_DIR, "${PREFIX}/share/pixmaps/${PACKAGE}", [pixmaps location])


dnl Set DATA_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [Directory of data files])
  else
    AC_DEFINE_UNQUOTED(DATA_DIR, "${prefix}/share/${PACKAGE}", [Directory of data files])
  fi
else
  AC_DEFINE_UNQUOTED(DATA_DIR, "${datadir}/${PACKAGE}", [Directory of data files])
fi

if test "x${USER_DEBUG}" = 'xyes'; then
  user_debug=1
else
  user_debug=0
fi

dnl Set HELP_DIR in config.h.
dnl AC_DEFINE_UNQUOTED(HELP_DIR, "${PREFIX}/share/${PACKAGE}/help", "")

# Host
AC_DEFINE_UNQUOTED(HOST, "${host}", [definition of HOST])

# Home directory
AC_DEFINE_UNQUOTED(HOME_DIRECTORY,"${homedir}",[Define for home directory])

# PID file
AC_DEFINE_UNQUOTED(PID_FILE,"${pidfile}",[Define for pid file])

# Supervisor PID file
AC_DEFINE_UNQUOTED(SUPERVISOR_PID_FILE,"${super_pidfile}",[Define for supervisor pid file])

# Log file
AC_DEFINE_UNQUOTED(LOG_FILE,"${logfile}",[Define for log file])

AC_DEFINE_UNQUOTED(USER_DEBUG,${user_debug},[Define for user debugging])

AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[Package name for gettext])

# Kernel release
AC_DEFINE_UNQUOTED(KERNEL_RELEASE, "${KERNEL_RELEASE}", [definition of kernel release])
AC_DEFINE_UNQUOTED(KERNEL_SOURCE,  "${KERNEL_SOURCE}",  [definition of kernel source dir])

KERNEL_CFLAGS="-I${KERNEL_SOURCE}"
AC_SUBST(KERNEL_CFLAGS)

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

dnl =====================
dnl Write some output
dnl =====================

AC_OUTPUT([
Makefile
webtester.pc
mk/rules.mk
src/Makefile
src/libwebtester/Makefile
src/librun/Makefile
src/librun/lrvm/Makefile
src/librun/kill/Makefile
src/stuff/testlib/Makefile
src/stuff/testlib/testlib.pc
src/stuff/testlib++/Makefile
src/stuff/testlib++/testlib++.pc
src/webtester/Makefile
src/webtester/modules/nullmodule/Makefile
src/webtester/modules/informatics/Makefile
src/webtester/plugins/nullplugin/Makefile
src/webtester/plugins/pipe/Makefile
src/webtester/plugins/webinterface/Makefile
src/webtester/plugins/rundll/Makefile
src/frontend/Makefile
])


echo "
Configuration for $PACKAGE $VERSION :
-----------------------------------

  Host System Type ................ : ${host}
  Compiler ........................ : $CC $CPPFLAGS
  CFLAGS .......................... : ${PICFLAGS} ${CFLAGS}
  LDFLAGS ......................... : ${PICLDFLAGS} ${LDFLAGS}
  Linker .......................... : $CC $LDFLAGS ${LIBLDFLAGS} $LIBS
  GLIB version .................... : ${GLIB_VERSION} in ${GLIB_PREFIX}
  Home directory .................. : ${homedir}
  PID file ........................ : ${pidfile}
  Supervisor PID file ............. : ${super_pidfile}
  Log file ........................ : ${logfile}
  Build WebTester Frontend ........ : ${build_gui}
  Building for kernel release ..... : ${KERNEL_RELEASE}

  Now type 'make' to build $PACKAGE $VERSION,
  and then type 'make install' for installation.
"
