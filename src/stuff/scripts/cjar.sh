#!/bin/sh

#
# Smart Java source file compilator for WebTester Server
#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# This file can be distributed under the terms of the GNU GPL
#

#
# Usage: cjar.sh <file-to-be-compiled.java>
#

if test "$#" -ne "1"; then
  echo "Usage: $0 <file-to-be-compiled.java>";
  exit 1;
fi

PATH="/usr/local/lib/j2sdk/bin:${PATH}"

# Temporary storage path to use
TMPPATH="/tmp/javac";

# Current temporary storage
# Need this because manu compilations may be started
# at the same time
CURTMP="${TMPPATH}/${USER}-`date +\"%F:%T\"`-${RANDOM}";

# Get name of class, where public static
# method `main` is implemented
#
# Parameters:
#  $1 - name of file to parse
getMainClass()
{
  #
  # TODO: Need some type of smart parsing here
  #

  echo "Main"
}

# Prepare mainfest file for jar-file
#
# Parameters:
#   $1 - name of class, where public static
#        method `main` is implemented
#   $2 - full manifest file name
buildManifest()
{
  echo "Main-Class: ${1}" > "${2}"
}

# Source file name validation
#
# Parameters:
#   $1 - initial source file name
#   $2 - name of class, where public static
#        method `main` is implemented
validateSourcefile()
{
  if test -f "${1}"; then
    result="${CURTMP}/${2}.java"
    cp "${1}" "${result}"
    echo "${result}"
  fi
}

# Main compilation procedure
#
# Parameters:
#   $1 - source file to compile
#   $2 - name of main class
#   $3 - name of output file name
compile()
{
  sourceFile="${1}"
  mainClass="${2}"
  outputFile="${3}.jar"
  manifestFile="${CURTMP}/mainfest"

  if (javac -d "${CURTMP}" "${sourceFile}"); then
    buildManifest ${mainClass} ${manifestFile}
    jar cvfm "${outputFile}" "${manifestFile}" \
      -C "${CURTMP}" "${mainClass}.class"
  else
    return 1;
  fi

  return 0;
}

# Finalize executing compiler
#
# Parameters:
#    $1 - exit code
finalize()
{
  # Totally remove current temporary storage
  rm -rf "${CURTMP}";

  # Remove temprary storage if avaliable
  rmdir --ignore-fail-on-non-empty "${TMPPATH}";

  exit ${1}
}

#
# Initialization
#

# Create temporary storage directory
mkdir -p ${CURTMP};

#
# Main stuff
#

mainClass=`getMainClass ${1}`
sourceFile=`validateSourcefile ${1} ${mainClass}`
outputFile=`echo "${1}" | sed -r s/\.java\$//`

if [ "x${sourceFile}" = "x" ]; then
  echo "Source file ${1} not found"
  finalize 1
fi

compile "${sourceFile}" "${mainClass}" "${outputFile}"

finalize $?
