#!/bin/sh

#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# Thids file can be distributed under the terms of the GNU GPL
#

#
# Part of the check_all.sh script
#
# Written (by Nazgul) under GPL. 03.10.07
#

echo -e "Running solution at tests' set..."


# Preremove input and output files
rm -f $2 $3;

# Some initialization
i=1;
all_points="";
total=0;

for points in $1; do
  _test=$i;

  if ( `test  $i -le 9` ); then
    _test="0$i";
  fi;

  _ans="$_test.ans";
  _test="$_test.tst";

  # Copy test file
  cp -f $_test $2;

  # Run solution
  ./solution > /dev/null 2>&1;

  # Run checker
  echo -ne "\tTest $i: ";
  if ( `test "$all_points" != ""` ); then
    all_points="$all_points + ";
  fi;
  ./checker $2 $3 $_ans -s
  if ( `test "$?" = "0"` ); then
    all_points="$all_points$points";
    let "total += $points";
  else
    all_points=$all_points"0";
  fi;

  # Remove input and output files
  rm -f $2 $3;
  let "i += 1";
done;

echo -e "Points: $all_points = $total";
