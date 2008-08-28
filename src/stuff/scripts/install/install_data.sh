#!/bin/sh

#
# WebTester Server data installation script
#
# Written (by Nazgul) under GPL. 02.09.2007
#

export PREFIX=`dirname $0`
curdir=`pwd`

init_variables()
  {
    DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`
    SRC_TOPDIR=`$PREFIX/stuff/opt_get.sh SRC_TOPDIR`
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

$PREFIX/stuff/echo.sh "Installing data..."
$PREFIX/stuff/install_data.sh
