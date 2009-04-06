#!/bin/sh

#
# WebTester Server installation script
#
# Written (by Nazgul) under GPL. 02.09.2007
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#

export PREFIX=`dirname $0`
curdir=`pwd`

#
# Parse command line arguments
#

DIST_INST=true
DIST_DIR=""
GUI_ONLY=false
LIBWT_ONLY=false

while test "$#" -ne 0; do
  case "$1" in
    PACKAGE)
      DIST_INST=false
      ;;

    --gui-only)
      GUI_ONLY=true
      ;;

    --libwt-only)
      LIBWT_ONLY=true
      ;;

    --dist-dir)
      DIST_DIR="$2"
      shift
      ;;
  esac;

  shift
done;

export DIST_INST=$DIST_INST

init_variables()
  {
    if test "x$DIST_DIR" = "x"; then
      DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`
    fi
    SRC_TOPDIR="$PREFIX/../../../.."

    echo "DIST_DIR $DIST_DIR" > "$PREFIX/stuff/!config"
    echo "SRC_TOPDIR $SRC_TOPDIR" >> "$PREFIX/stuff/!config"
  }

banner()
  {
    $PREFIX/stuff/echo.sh "=================================="
    $PREFIX/stuff/echo.sh "Started WebTester Server installer"
    $PREFIX/stuff/echo.sh "=================================="
    echo
  }

function install_gui()
  {
    $PREFIX/stuff/echo.sh "Create dist directory tree..."
    $PREFIX/stuff/mkdistdir.sh /webtester        webtester webtester 0775
    $PREFIX/stuff/mkdistdir.sh /webtester/bin    webtester webtester 0775
    $PREFIX/stuff/mkdistdir.sh /webtester/conf   webtester webtester 0775

    $PREFIX/stuff/echo.sh "Installing config files..."
    $PREFIX/stuff/cpfile.sh /etc/gwebtester.conf /conf/gwebtester.conf \
      webtester webtester 0664

    $PREFIX/stuff/echo.sh "Installing binaries..."
    $PREFIX/stuff/install_bin.sh /src/frontend gwebtester \
      webtester webtester 0775   false

    ${ECHO} "Installing pixmaps..."
    $PREFIX/stuff/install_pixmaps.sh

    exit 0;
  }

function install_libwt()
  {
    $PREFIX/stuff/echo.sh "Create dist directory tree..."
    $PREFIX/stuff/mkdistdir.sh /webtester        webtester webtester 0775
    $PREFIX/stuff/mkdistdir.sh /webtester/lib    webtester webtester 0775

    $PREFIX/stuff/echo.sh "Installing libraries..."
    $PREFIX/stuff/install_lib.sh /src/libwebtester libwebtester.so \
      webtester webtester 0775
    exit 0;
  }

#
#
#

# Initialize all required variables
init_variables;

# Print banner
banner;

#
# Some help commands
#

export ECHO="$PREFIX/stuff/echo.sh"
export CREATE_GROUP="$PREFIX/stuff/create_group.sh"
export CREATE_USER="$PREFIX/stuff/create_user.sh"
export CREATE_SMB_USER="$PREFIX/stuff/create_smb_user.sh"
export MKDISTDIR="$PREFIX/stuff/mkdistdir.sh "
export CPFILE="$PREFIX/stuff/cpfile.sh"

#### MAIN STUFF ####

# Stop WebTester before installation
if $DIST_INST && test -f /etc/init.d/webtester; then
  /etc/init.d/webtester stop
fi

#
# Step 1: Create required groups and users
#

($DIST_INST && (
  ${ECHO} "Creating required groups and users...";
  ${CREATE_GROUP} webtester;
  ${CREATE_GROUP} webtester-nobody;

  ${CREATE_USER}  webtester        /home/webtester /usr/sbin/nologin \
    webtester true;

  ${CREATE_USER}  webtester-nobody /dev/null /usr/sbin/nologin \
    webtester-nobody true;

  ${ECHO} "Creating required Samba's users...";

  #
  # TODO: Replace this password
  #

  ${CREATE_SMB_USER} webtester assword;
))

${GUI_ONLY} && install_gui && exit 0
${LIBWT_ONLY} && install_libwt && exit 0

# Step 2: Create dist directory tree
${ECHO} "Create dist directory tree..."
${MKDISTDIR} /webtester                      webtester webtester 0775
${MKDISTDIR} /webtester/bin                  webtester webtester 0775
${MKDISTDIR} /webtester/sbin                 webtester webtester 0750
${MKDISTDIR} /webtester/lib                  webtester webtester 0775
${MKDISTDIR} /webtester/lib/plugins          webtester webtester 0775
${MKDISTDIR} /webtester/lib/modules          webtester webtester 0775
${MKDISTDIR} /webtester/include              webtester webtester 0775
${MKDISTDIR} /webtester/include/libwebtester webtester webtester 0775
${MKDISTDIR} /webtester/conf                 webtester webtester 0775
${MKDISTDIR} /webtester/tmp                  webtester webtester 0775
${MKDISTDIR} /webtester/var                  webtester webtester 0775
${MKDISTDIR} /webtester/var/data             webtester webtester 0770
${MKDISTDIR} /webtester/var/storage          webtester webtester 0775
${MKDISTDIR} /webtester/var/storage/problems webtester www-data  0770
${MKDISTDIR} /webtester/var/run              webtester webtester 0775
${MKDISTDIR} /webtester/usr                  webtester webtester 0775
${MKDISTDIR} /webtester/usr/scripts          webtester webtester 0775
${MKDISTDIR} /webtester/usr/src              webtester webtester 0775
${MKDISTDIR} /webtester/usr/src/librun       webtester webtester 0775

# Step 3: Coping files
${ECHO} "Installing config files..."
${CPFILE} /etc/webtester.conf  /conf/webtester.conf  webtester webtester 0660
($DIST_INST && ${CPFILE} /etc/gwebtester.conf \
  /conf/gwebtester.conf webtester webtester 0664)
${CPFILE} /etc/lrvm.conf       /conf/lrvm.conf       webtester webtester 0640
${CPFILE} /etc/users.conf      /conf/users.conf      webtester webtester 0640
${CPFILE} /etc/ip_blacklist    /conf/ip_blacklist    webtester webtester 0660

${ECHO} "Installing binaries..."
$PREFIX/stuff/install_bins.sh

${ECHO} "Installing system binaries..."
$PREFIX/stuff/install_sbins.sh

${ECHO} "Installing libraries..."
$PREFIX/stuff/install_libs.sh

${ECHO} "Installing includes..."
${CPFILE} /src/stuff/testlib/testlib.h     /include/testlib.h    \
  webtester webtester 0664

${CPFILE} /src/stuff/testlib++/testlib++.h /include/testlib++.h  \
  webtester webtester 0664

${CPFILE} /src/libwebtester/*.h            /include/libwebtester/ \
  webtester webtester 0664

${ECHO} "Installing plugins..."
$PREFIX/stuff/install_plugins.sh
${ECHO} "Installing modules..."
$PREFIX/stuff/install_modules.sh

#
# Step 4: Compile webtester launcher
#
${ECHO} "Compiling and installing webtester launcher..."
gcc -o webtester -I$SRC_TOPDIR -I$SRC_TOPDIR/src $PREFIX/templates/_webtester.c 
mv ./webtester $DIST_DIR/webtester/webtester
chown root:root $DIST_DIR/webtester/webtester
chmod 6775 $DIST_DIR/webtester/webtester

#
# Step 5: Install pascal testlib
#
${ECHO} "Installing pascal testlib..."
${MKDISTDIR} /webtester/var/fpc        webtester webtester 0775
${MKDISTDIR} /webtester/var/fpc/obj    webtester webtester 0775
${MKDISTDIR} /webtester/var/fpc/lib    webtester webtester 0775
${MKDISTDIR} /webtester/var/fpc/units  webtester webtester 0775
${MKDISTDIR} /webtester/var/kylix      webtester webtester 0775
${MKDISTDIR} /webtester/var/kylix/lib  webtester webtester 0775
${MKDISTDIR} /webtester/var/kylix/bin  webtester webtester 0775

${CPFILE} /src/stuff/testlib.pas/testlib.pas  /var/fpc/units/testlib.pas   \
  webtester webtester 0664
${CPFILE} /src/stuff/testlib.pas/testlib.ppu  /var/fpc/units/testlib.ppu   \
  webtester webtester 0664
${CPFILE} /src/stuff/testlib.pas/testlib.o    /var/fpc/units/testlib.o     \
  webtester webtester 0664
${CPFILE} /src/stuff/testlib.pas/testlib.dcu  /var/kylix/lib/testlib.dcu   \
  webtester webtester 0664
${CPFILE} /src/stuff/testlib.pas/testlib.pas  /var/kylix/lib/testlib.pas   \
  webtester webtester 0664
${CPFILE} /src/stuff/scripts/dcc.sh  /var/kylix/bin/dcc.sh   \
  webtester webtester 0774

${ECHO} "Installing files for Java..."
${MKDISTDIR} /webtester/var/java      webtester webtester 0775
${CPFILE} /src/stuff/Bootstrap/dist/Bootstrap.jar  /var/java/Bootstrap.jar   \
  webtester webtester 0664

${ECHO} "Copying scripts..."
${CPFILE} /src/stuff/scripts/lrvm_killall.sh   /sbin/lrvm_killall.sh  \
  webtester webtester 0750

${MKDISTDIR} /webtester/usr/scripts/check_all   webtester webtester 0775
${CPFILE} /src/stuff/scripts/check_all/check_all.sh    \
  /usr/scripts/check_all/check_all.sh    webtester webtester 0775
${CPFILE} /src/stuff/scripts/check_all/check_entry.sh  \
  /usr/scripts/check_all/check_entry.sh  webtester webtester 0775
${CPFILE} /src/stuff/scripts/check_all/files.info      \
  /usr/scripts/check_all/files.info      webtester webtester 0644
${CPFILE} /src/stuff/scripts/check_all/tests.info      \
  /usr/scripts/check_all/tests.info      webtester webtester 0644

${MKDISTDIR} /webtester/usr/scripts/init.d   webtester webtester 0775
${CPFILE} /src/stuff/scripts/init.d/webtester  \
  /usr/scripts/init.d/webtester  webtester webtester 0775

${ECHO} "Copying checkers..."
${MKDISTDIR} /webtester/usr/checkers   webtester webtester 0775
${CPFILE} /src/stuff/checkers/c_bystring_cmp.cxx \
  /usr/checkers/c_bystring_cmp.cxx   webtester webtester 0644
${CPFILE} /src/stuff/checkers/c_long_cmp.cxx     \
  /usr/checkers/c_long_cmp.cxx       webtester webtester 0644
${CPFILE} /src/stuff/checkers/c_string_cmp.cxx   \
  /usr/checkers/c_string_cmp.cxx     webtester webtester 0644
${CPFILE} /src/stuff/checkers/!list              \
  /usr/checkers/!list                webtester webtester 0644
${CPFILE} /src/stuff/checkers/Makefile.inst      \
  /usr/checkers/Makefile             webtester webtester 0644

${ECHO} "Compiling checkers..."
cd $DIST_DIR/webtester/usr/checkers
make
cd $curdir

${ECHO} "Installing helpers..."
${MKDISTDIR} /webtester/usr/helpers   webtester webtester 0775
${CPFILE} /src/stuff/helpers/genpass.c     /usr/helpers/genpass.c    \
  webtester webtester 0644
${CPFILE} /src/stuff/helpers/ipcpassenc.c  \
  /usr/helpers/ipcpassenc.c webtester webtester 0644

cp $SRC_TOPDIR/src/libwebtester/libwebtester.so $DIST_DIR/webtester/usr/helpers
cd $DIST_DIR/webtester/usr/helpers
gcc -o genpass genpass.c
gcc -I$DIST_DIR/webtester/include -L. -lwebtester -o ipcpassenc ipcpassenc.c
cd $curdir
rm $DIST_DIR/webtester/usr/helpers/libwebtester.so

${DIST_INST} && ${ECHO} "Installing pixmaps..."
${DIST_INST} && $PREFIX/stuff/install_pixmaps.sh

${ECHO} "Installing data..."
$PREFIX/stuff/install_data.sh

($DIST_INST && (cp $SRC_TOPDIR/src/stuff/scripts/init.d/webtester  \
                     /etc/init.d/webtester;
  chown root:root /etc/init.d/webtester;
  chmod 0775 /etc/init.d/webtester;
))

# Install as system service
($DIST_INST && (
  update_rcd="no";

  while true; do
    echo -n "Start WebTester Server at system startup? [y/n]: ";
    read choise;

    if test "x${choise}" = "xy"; then
      update_rcd="yes";
      break;
    fi;

    if test "x${choise}" = "xn"; then
      update_rcd="no";
      break;
    fi;
  done;

  if test "x$update_rcd" = "xyes"; then
    update-rc.d -f webtester remove;
    update-rc.d webtester start 99 2 3 4 5 . stop 01 0 1 6 .;
  fi
))

${ECHO} "Installing sources..."
${CPFILE} /src/librun/*.c /usr/src/librun  webtester webtester 0775
${CPFILE} /src/librun/*.h /usr/src/librun  webtester webtester 0775
${CPFILE} /src/stuff/scripts/install/templates/Makefile.librun \
  /usr/src/librun/Makefile  webtester webtester 0664

#
${ECHO} "Copying other stuff..."
${CPFILE} /AUTHORS    /AUTHORS   webtester webtester 0664
${CPFILE} /BUGS       /BUGS      webtester webtester 0664
${CPFILE} /ChangeLog  /ChangeLog webtester webtester 0664
${CPFILE} /COPYING    /COPYING   webtester webtester 0664
