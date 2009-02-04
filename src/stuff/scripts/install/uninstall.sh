#!/bin/sh

#
# WebTester Server uninstallation script
#
# Written (by Nazgul) under GPL. 02.09.2007
#
# Copyright (C) Sergey I. Sharybin <g.ulairi@gmail.com>
#

export PREFIX=`dirname $0`


init_variables()
  {
    DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`
    SRC_TOPDIR=`$PREFIX/stuff/opt_get.sh SRC_TOPDIR`
  }

banner()
  {
    $PREFIX/stuff/echo.sh "===================================="
    $PREFIX/stuff/echo.sh "Started WebTester Server uninstaller"
    $PREFIX/stuff/echo.sh "===================================="
    echo
  }

# Initialize all required variables
init_variables;

# Print banner
banner;


#### MAIN STUFF ####

# Step 1: Remove all installed files

$PREFIX/stuff/echo.sh "Uninstalling installed binaries...."
$PREFIX/stuff/uninstall_bins.sh

$PREFIX/stuff/echo.sh "Uninstalling installed system binaries...."
$PREFIX/stuff/uninstall_sbins.sh

$PREFIX/stuff/echo.sh "Uninstalling installed libraries...."
$PREFIX/stuff/uninstall_libs.sh

$PREFIX/stuff/echo.sh "Removing installed files and directories..."
$PREFIX/stuff/rmdistdir.sh /webtester

# Step 2: Delete unwanted groups and users

$PREFIX/stuff/echo.sh "Deleting unwanted Samba's users..."
$PREFIX/stuff/delete_smb_user.sh webtester

$PREFIX/stuff/echo.sh "Deleting unwanted groups and users..."
$PREFIX/stuff/delete_user.sh webtester
$PREFIX/stuff/delete_user.sh webtester-nobody
$PREFIX/stuff/delete_group.sh webtester
$PREFIX/stuff/delete_group.sh webtester-nobody

unlink /etc/init.d/webtester 
update-rc.d -f webtester remove
