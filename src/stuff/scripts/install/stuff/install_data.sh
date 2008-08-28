#!/bin/sh

DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`

$PREFIX/stuff/mkdistdir.sh /webtester/var/data  webtester webtester 0775

echo -n "  Installing data for module Informatics..."
$PREFIX/stuff/mkdistdir.sh /webtester/var/data/Informatics  webtester webtester 0775 > /dev/null
$PREFIX/stuff/mkdistdir.sh /webtester/var/data/Informatics/chroot  webtester webtester 0775 > /dev/null
$PREFIX/stuff/mkdistdir.sh /webtester/var/data/Informatics/chroot/lib  webtester webtester 07752 > /dev/null
rm -rf $DIST_DIR/webtester/var/data/Informatics/chroot/lib/*
cp -P `ls -1 /lib | grep -E "^((libc)|(libm)|(ld)|(libdl)|(libgcc_s)|(libpthread))[\\.|\\-]" | awk ' { printf "/lib/%s ",$1 } '` $DIST_DIR/webtester/var/data/Informatics/chroot/lib
cp -P `ls -1 /usr/lib | grep -E "^((libstdc\\+\\+))[\\.|\\-]" | awk ' { printf "/usr/lib/%s ",$1 } '` $DIST_DIR/webtester/var/data/Informatics/chroot/lib
echo "ok."
