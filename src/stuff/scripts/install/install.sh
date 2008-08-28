#!/bin/sh

#
# WebTester Server installation script
#
# Written (by Nazgul) under GPL. 02.09.2007
#

init_variables()
  {
    DIST_DIR=`./stuff/opt_get.sh DIST_DIR`
    SRC_TOPDIR=`./stuff/opt_get.sh SRC_TOPDIR`
  }

banner()
  {
    ./stuff/echo.sh "=================================="
    ./stuff/echo.sh "Started WebTester Server installer"
    ./stuff/echo.sh "=================================="
    echo
  }

# Initialize all required variables
init_variables;

# Print banner
banner;


#### MAIN STUFF ####

# Step 1: Create required groups and users
./stuff/echo.sh "Creating required groups and users..."
./stuff/create_group.sh webtester
./stuff/create_group.sh webtester-nobody
./stuff/create_user.sh webtester        /home/webtester /usr/sbin/nologin webtester true
./stuff/create_user.sh webtester-nobody /dev/null /usr/sbin/nologin webtester-nobody true

# !!DEBUG!! #
gpasswd -a nazgul webtester
#############

./stuff/echo.sh "Creating required Samba's users..."
./stuff/create_smb_user.sh webtester assword

# Step 2: Create dist directory tree
./stuff/echo.sh "Create dist directory tree..."
./stuff/mkdistdir.sh /webtester                      webtester webtester 0775
./stuff/mkdistdir.sh /webtester/bin                  webtester webtester 0775
./stuff/mkdistdir.sh /webtester/sbin                 webtester webtester 0750
./stuff/mkdistdir.sh /webtester/lib                  webtester webtester 0775
./stuff/mkdistdir.sh /webtester/lib/plugins          webtester webtester 0775
./stuff/mkdistdir.sh /webtester/lib/modules          webtester webtester 0775
./stuff/mkdistdir.sh /webtester/include              webtester webtester 0775
./stuff/mkdistdir.sh /webtester/conf                 webtester webtester 0770
./stuff/mkdistdir.sh /webtester/tmp                  webtester webtester 0775
./stuff/mkdistdir.sh /webtester/var                  webtester webtester 0775
./stuff/mkdistdir.sh /webtester/var/data             webtester webtester 0770
./stuff/mkdistdir.sh /webtester/var/storage          webtester webtester 0775
./stuff/mkdistdir.sh /webtester/var/storage/problems webtester www-data  0770
./stuff/mkdistdir.sh /webtester/var/run              webtester webtester 0775

# Step 3: Coping files
./stuff/echo.sh "Installing config files..."
./stuff/cpfile.sh /etc/webtester.conf /conf/webtester.conf webtester webtester 0660
./stuff/cpfile.sh /etc/lrvm.conf      /conf/lrvm.conf      webtester webtester 0660

./stuff/echo.sh "Installing binaries..."
./stuff/install_bins.sh

./stuff/echo.sh "Installing system binaries..."
./stuff/install_sbins.sh

./stuff/echo.sh "Installing libraries..."
./stuff/install_libs.sh

./stuff/echo.sh "Installing includes..."
./stuff/cpfile.sh /src/stuff/testlib/testlib.h     /include/testlib.h   webtester webtester 0664
./stuff/cpfile.sh /src/stuff/testlib++/testlib++.h /include/testlib++.h webtester webtester 0664

./stuff/echo.sh "Installing plugins..."
./stuff/install_plugins.sh
./stuff/echo.sh "Installing modules..."
./stuff/install_modules.sh

# Step 4: Compile webtester
./stuff/echo.sh "Compiling and installing webtester launcher..."
gcc -o webtester ./templates/_webtester.c 
mv ./webtester $DIST_DIR/webtester/webtester
chown root:root $DIST_DIR/webtester/webtester
chmod 06775 $DIST_DIR/webtester/webtester

# Step 5: Compile pascal testlib
fpc $SRC_TOPDIR/src/stuff/testlib.pas/testlib.pas > /dev/null
./stuff/mkdistdir.sh /webtester/var/fpc        webtester webtester 0775
./stuff/mkdistdir.sh /webtester/var/fpc/obj    webtester webtester 0775
./stuff/mkdistdir.sh /webtester/var/fpc/lib    webtester webtester 0775
./stuff/mkdistdir.sh /webtester/var/fpc/units  webtester webtester 0775

./stuff/cpfile.sh /src/stuff/testlib.pas/testlib.pas  /var/fpc/units/testlib.pas   webtester webtester 0664
./stuff/cpfile.sh /src/stuff/testlib.pas/testlib.ppu  /var/fpc/units/testlib.ppu   webtester webtester 0664
./stuff/cpfile.sh /src/stuff/testlib.pas/testlib.o    /var/fpc/units/testlib.o     webtester webtester 0664

./stuff/echo.sh "Copying scripts..."
./stuff/mkdistdir.sh /webtester/usr           webtester webtester 0775
./stuff/mkdistdir.sh /webtester/usr/scripts   webtester webtester 0775

./stuff/mkdistdir.sh /webtester/usr/scripts/check_all   webtester webtester 0775
./stuff/cpfile.sh /src/stuff/scripts/check_all/check_all.sh      /usr/scripts/check_all/check_all.sh    webtester webtester 0775
./stuff/cpfile.sh /src/stuff/scripts/check_all/check_entry.sh    /usr/scripts/check_all/check_entry.sh  webtester webtester 0775
./stuff/cpfile.sh /src/stuff/scripts/check_all/files.info        /usr/scripts/check_all/files.info      webtester webtester 0644
./stuff/cpfile.sh /src/stuff/scripts/check_all/tests.info        /usr/scripts/check_all/tests.info      webtester webtester 0644

./stuff/echo.sh "Copying checkers..."
./stuff/mkdistdir.sh /webtester/usr/checkers   webtester webtester 0775
./stuff/cpfile.sh /src/stuff/checkers/c_bystring_cmp.cxx     /usr/checkers/c_bystring_cmp.cxx   webtester webtester 0644
./stuff/cpfile.sh /src/stuff/checkers/c_long_cmp.cxx         /usr/checkers/c_long_cmp.cxx       webtester webtester 0644
./stuff/cpfile.sh /src/stuff/checkers/c_string_cmp.cxx       /usr/checkers/c_string_cmp.cxx     webtester webtester 0644
./stuff/cpfile.sh /src/stuff/checkers/!list                  /usr/checkers/!list                webtester webtester 0644
./stuff/cpfile.sh /src/stuff/checkers/Makefile.inst          /usr/checkers/Makefile             webtester webtester 0644

./stuff/echo.sh "Compiling checkers..."
cd $DIST_DIR/webtester/usr/checkers
make
cd $SRC_TOPDIR/src/stuff/scripts/install

./stuff/echo.sh "Installing helpers..."
./stuff/mkdistdir.sh /webtester/usr/helpers   webtester webtester 0775
./stuff/cpfile.sh /src/stuff/helpers/genpass.c     /usr/helpers/genpass.c   webtester webtester 0644
cd $DIST_DIR/webtester/usr/helpers
gcc -o genpass genpass.c
cd $SRC_TOPDIR/src/stuff/scripts/install


#
./stuff/echo.sh "Copying other stuff..."
./stuff/cpfile.sh /AUTHORS    /AUTHORS   webtester webtester 0664
./stuff/cpfile.sh /BUGS       /BUGS      webtester webtester 0664
./stuff/cpfile.sh /ChangeLog  /ChangeLog webtester webtester 0664
./stuff/cpfile.sh /COPYING    /COPYING   webtester webtester 0664
