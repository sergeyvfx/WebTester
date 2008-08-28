#!/bin/sh

#
# WebTester Server uninstallation script
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
    ./stuff/echo.sh "===================================="
    ./stuff/echo.sh "Started WebTester Server uninstaller"
    ./stuff/echo.sh "===================================="
    echo
  }

# Initialize all required variables
init_variables;

# Print banner
banner;


#### MAIN STUFF ####

# Step 1: Remove all installed files

./stuff/echo.sh "Uninstalling installed binaries...."
./stuff/uninstall_bins.sh

./stuff/echo.sh "Uninstalling installed system binaries...."
./stuff/uninstall_sbins.sh

./stuff/echo.sh "Uninstalling installed libraries...."
./stuff/uninstall_libs.sh

./stuff/echo.sh "Removing installed files and directories..."
./stuff/rmdistdir.sh /webtester

# Step 2: Delete unwanted groups and users

./stuff/echo.sh "Deleting unwanted Samba's users..."
./stuff/delete_smb_user.sh webtester

./stuff/echo.sh "Deleting unwanted groups and users..."
./stuff/delete_user.sh webtester
./stuff/delete_user.sh webtester-nobody
./stuff/delete_group.sh webtester
./stuff/delete_group.sh webtester-nobody

unlink /etc/init.d/webtester 

