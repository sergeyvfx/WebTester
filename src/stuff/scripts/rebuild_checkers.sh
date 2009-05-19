#!/bin/sh

#
# Script for rebuilding checkers for all problems
#
# This script should be executed from problem's storage directory
# (by default /home/webtester/var/data/Informatics/problems)
#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# This file can be distributed under the terms of the GNU GPL

opt_get()
{
  cat "${1}/checker.conf" | sed 's/\r//g' | grep "${2}" |
    sed -r "s/\s*${2}\s*\"?(\w*)\"?\s*/\1/"
}

get_compiler_cmd()
{
  compiler=`opt_get ${1} 'CompilerID'`
  src=`opt_get ${1} 'SourceFile'`
  cmd=""

  case ${compiler} in
    G++)
      cmd="cc-checker-g++.sh -o checker ${src}"
    ;;

    FPC)
      cmd="cc-checker-fpc.sh -ochecker ${src}"
    ;;
  esac

  echo "${cmd}"
}

for i in *; do
  # Ignore non-directories
  if [ ! -d "${i}" ]; then
    continue;
  fi

  # If checker file inside problem's directory
  # is a symbolic link, we shouldn't recompile it
  # because this checker is not from uploaded archive
  if [ -h "${i}/checker" ]; then
    continue;
  fi

  echo -n "Rebuilding checker for problem ${i}... "

  if [ ! -f  "${i}/checker.conf" ]; then
    echo "failed! (no checker.conf file present)";
    continue;
  fi

  cmd=`get_compiler_cmd "${i}"`

  if [ -z "${cmd}" ]; then
    echo "failed! (compilation command is undefined)";
    continue;
  fi

  cd ${i}
    rm -f checker
    ${cmd} > /dev/null 2>&1
    chown webtester:webtester *
    if [ ! -f checker ]; then
      echo "failed! (compilation error)";
      cd ..;
      continue;
    fi
  cd ..

  echo "ok."
done
