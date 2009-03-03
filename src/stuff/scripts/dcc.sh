#!/bin/sh

#
# Smart Kylix source file compilator for WebTester Server
#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# Thids file can be distributed under the terms of the GNU GPL

set -e

ARGS=""
OUTPUT_FILE=""
DCC="`dirname $0`/dcc"
MAIN_SOURCE=""

#
# Parse command line arguments
#
while test "$#" -ne 0; do
  case "$1" in
    -o)
      OUTPUT_FILE="$2";
      shift;
    ;;
    *)
      if [ `echo "$1" | grep -E -c ".*\.(dpr|pas)\$"` = 1 ]; then
        if test "x${MAIN_SOURCE}" = "x"; then
          MAIN_SOURCE="$1"
        fi
      fi
      ARGS="${ARGS} $1"
    ;;
  esac;

  shift
done

#
# Compile source files
#
${DCC} ${ARGS}

#
# Rename generated by Kylix binary file
#
if test "x${OUTPUT_FILE}" != "x"; then
  BINARY_NAME=`echo "${MAIN_SOURCE}" | sed s/\.[A-Z]*$//i`
  if test "x${BINARY_NAME}" != "x${OUTPUT_FILE}"; then
    mv ${BINARY_NAME} ${OUTPUT_FILE}
  fi
fi;
