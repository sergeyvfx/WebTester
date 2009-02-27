#!/bin/sh

if test -f Makefile; then
  make clean
  make distclean
fi

which dh_clean > /dev/null && dh_clean

dirs_to_delete="./debian/webtester autom4te.cache \
  ./debian/webtester-core ./debian/webtester-gui"

files_to_delete="./configure ./configure.in ./acinclude.m4 ./configure \
  ./intltool-extract ./intltool-merge ./intltool-update ./Makefile \
  WT-VERSION-FILE"

for f in $dirs_to_delete; do
  rm -rf "$f"
done

for f in $files_to_delete; do
  rm -f "$f"
done

which git > /dev/null && \
  (git checkout src/libwebtester/version.h \
          src/stuff/scripts/install/stuff/!config > /dev/null 2>&1;)
