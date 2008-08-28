#!/bin/sh

cat $PREFIX/stuff/!config | grep "^$1" | sed -e "s/^$1 //"
