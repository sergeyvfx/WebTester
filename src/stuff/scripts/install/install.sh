#!/bin/sh

#
# WebTester Server installation script
#
# Written (by Nazgul) under GPL. 02.09.2007
#

export PREFIX=`dirname $0`
curdir=`pwd`

init_variables()
  {
    DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`
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

# Initialize all required variables
init_variables;

# Print banner
banner;


#### MAIN STUFF ####

# Step 1: Create required groups and users
$PREFIX/stuff/echo.sh "Creating required groups and users..."
$PREFIX/stuff/create_group.sh webtester
$PREFIX/stuff/create_group.sh webtester-nobody
$PREFIX/stuff/create_user.sh webtester        /home/webtester /usr/sbin/nologin webtester true
$PREFIX/stuff/create_user.sh webtester-nobody /dev/null /usr/sbin/nologin webtester-nobody true

# !!DEBUG!! #
gpasswd -a nazgul webtester
#############

$PREFIX/stuff/echo.sh "Creating required Samba's users..."
$PREFIX/stuff/create_smb_user.sh webtester assword

# Step 2: Create dist directory tree
$PREFIX/stuff/echo.sh "Create dist directory tree..."
$PREFIX/stuff/mkdistdir.sh /webtester                      webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/bin                  webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/sbin                 webtester webtester 0750
$PREFIX/stuff/mkdistdir.sh /webtester/lib                  webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/lib/plugins          webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/lib/modules          webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/include              webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/include/libwebtester webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/conf                 webtester webtester 0770
$PREFIX/stuff/mkdistdir.sh /webtester/tmp                  webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/var                  webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/var/data             webtester webtester 0770
$PREFIX/stuff/mkdistdir.sh /webtester/var/storage          webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/var/storage/problems webtester www-data  0770
$PREFIX/stuff/mkdistdir.sh /webtester/var/run              webtester webtester 0775

# Step 3: Coping files
$PREFIX/stuff/echo.sh "Installing config files..."
$PREFIX/stuff/cpfile.sh /etc/webtester.conf  /conf/webtester.conf  webtester webtester 0660
$PREFIX/stuff/cpfile.sh /etc/gwebtester.conf /conf/gwebtester.conf webtester webtester 0664
$PREFIX/stuff/cpfile.sh /etc/lrvm.conf       /conf/lrvm.conf       webtester webtester 0640
$PREFIX/stuff/cpfile.sh /etc/users.conf      /conf/users.conf      webtester webtester 0640
$PREFIX/stuff/cpfile.sh /etc/ip_blacklist    /conf/ip_blacklist    webtester webtester 0660

$PREFIX/stuff/echo.sh "Installing binaries..."
$PREFIX/stuff/install_bins.sh

$PREFIX/stuff/echo.sh "Installing system binaries..."
$PREFIX/stuff/install_sbins.sh

$PREFIX/stuff/echo.sh "Installing libraries..."
$PREFIX/stuff/install_libs.sh

$PREFIX/stuff/echo.sh "Installing includes..."
$PREFIX/stuff/cpfile.sh /src/stuff/testlib/testlib.h     /include/testlib.h   webtester webtester 0664
$PREFIX/stuff/cpfile.sh /src/stuff/testlib++/testlib++.h /include/testlib++.h webtester webtester 0664
$PREFIX/stuff/cpfile.sh /src/libwebtester/*.h   /include/libwebtester webtester webtester 0664

$PREFIX/stuff/echo.sh "Installing plugins..."
$PREFIX/stuff/install_plugins.sh
$PREFIX/stuff/echo.sh "Installing modules..."
$PREFIX/stuff/install_modules.sh

# Step 4: Compile webtester
$PREFIX/stuff/echo.sh "Compiling and installing webtester launcher..."
gcc -o webtester -I$SRC_TOPDIR -I$SRC_TOPDIR/src $PREFIX/templates/_webtester.c 
mv ./webtester $DIST_DIR/webtester/webtester
chown root:root $DIST_DIR/webtester/webtester
chmod 06775 $DIST_DIR/webtester/webtester

# Step 5: Compile pascal testlib
fpc $SRC_TOPDIR/src/stuff/testlib.pas/testlib.pas > /dev/null
$PREFIX/stuff/mkdistdir.sh /webtester/var/fpc        webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/var/fpc/obj    webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/var/fpc/lib    webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/var/fpc/units  webtester webtester 0775

$PREFIX/stuff/cpfile.sh /src/stuff/testlib.pas/testlib.pas  /var/fpc/units/testlib.pas   webtester webtester 0664
$PREFIX/stuff/cpfile.sh /src/stuff/testlib.pas/testlib.ppu  /var/fpc/units/testlib.ppu   webtester webtester 0664
$PREFIX/stuff/cpfile.sh /src/stuff/testlib.pas/testlib.o    /var/fpc/units/testlib.o     webtester webtester 0664

$PREFIX/stuff/echo.sh "Copying scripts..."
$PREFIX/stuff/cpfile.sh /src/stuff/scripts/lrvm_killall.sh   /sbin/lrvm_killall.sh  webtester webtester 0750

$PREFIX/stuff/mkdistdir.sh /webtester/usr           webtester webtester 0775
$PREFIX/stuff/mkdistdir.sh /webtester/usr/scripts   webtester webtester 0775

$PREFIX/stuff/mkdistdir.sh /webtester/usr/scripts/check_all   webtester webtester 0775
$PREFIX/stuff/cpfile.sh /src/stuff/scripts/check_all/check_all.sh      /usr/scripts/check_all/check_all.sh    webtester webtester 0775
$PREFIX/stuff/cpfile.sh /src/stuff/scripts/check_all/check_entry.sh    /usr/scripts/check_all/check_entry.sh  webtester webtester 0775
$PREFIX/stuff/cpfile.sh /src/stuff/scripts/check_all/files.info        /usr/scripts/check_all/files.info      webtester webtester 0644
$PREFIX/stuff/cpfile.sh /src/stuff/scripts/check_all/tests.info        /usr/scripts/check_all/tests.info      webtester webtester 0644

$PREFIX/stuff/mkdistdir.sh /webtester/usr/scripts/init.d   webtester webtester 0775
$PREFIX/stuff/cpfile.sh /src/stuff/scripts/init.d/webtester            /usr/scripts/init.d/webtester  webtester webtester 0775

$PREFIX/stuff/echo.sh "Copying checkers..."
$PREFIX/stuff/mkdistdir.sh /webtester/usr/checkers   webtester webtester 0775
$PREFIX/stuff/cpfile.sh /src/stuff/checkers/c_bystring_cmp.cxx     /usr/checkers/c_bystring_cmp.cxx   webtester webtester 0644
$PREFIX/stuff/cpfile.sh /src/stuff/checkers/c_long_cmp.cxx         /usr/checkers/c_long_cmp.cxx       webtester webtester 0644
$PREFIX/stuff/cpfile.sh /src/stuff/checkers/c_string_cmp.cxx       /usr/checkers/c_string_cmp.cxx     webtester webtester 0644
$PREFIX/stuff/cpfile.sh /src/stuff/checkers/!list                  /usr/checkers/!list                webtester webtester 0644
$PREFIX/stuff/cpfile.sh /src/stuff/checkers/Makefile.inst          /usr/checkers/Makefile             webtester webtester 0644

$PREFIX/stuff/echo.sh "Compiling checkers..."
cd $DIST_DIR/webtester/usr/checkers
make
cd $curdir

$PREFIX/stuff/echo.sh "Installing helpers..."
$PREFIX/stuff/mkdistdir.sh /webtester/usr/helpers   webtester webtester 0775
$PREFIX/stuff/cpfile.sh /src/stuff/helpers/genpass.c     /usr/helpers/genpass.c    webtester webtester 0644
$PREFIX/stuff/cpfile.sh /src/stuff/helpers/ipcpassenc.c  /usr/helpers/ipcpassenc.c webtester webtester 0644
cd $DIST_DIR/webtester/usr/helpers
gcc -o genpass genpass.c
gcc -I$DIST_DIR/webtester/include -L$DIST_DIR/webtester/lib -lwebtester -o ipcpassenc ipcpassenc.c
cd $curdir

$PREFIX/stuff/echo.sh "Installing pixmaps..."
$PREFIX/stuff/install_pixmaps.sh

$PREFIX/stuff/echo.sh "Installing data..."
$PREFIX/stuff/install_data.sh

$PREFIX/stuff/echo.sh "Installing init.d scripts..."
cp $SRC_TOPDIR/src/stuff/scripts/init.d/webtester  /etc/init.d/webtester
chown root:root /etc/init.d/webtester
chmod 0775 /etc/init.d/webtester

#
$PREFIX/stuff/echo.sh "Copying other stuff..."
$PREFIX/stuff/cpfile.sh /AUTHORS    /AUTHORS   webtester webtester 0664
$PREFIX/stuff/cpfile.sh /BUGS       /BUGS      webtester webtester 0664
$PREFIX/stuff/cpfile.sh /ChangeLog  /ChangeLog webtester webtester 0664
$PREFIX/stuff/cpfile.sh /COPYING    /COPYING   webtester webtester 0664
