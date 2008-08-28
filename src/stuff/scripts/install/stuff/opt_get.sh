#!/bin/sh

cat ./stuff/!config | grep "^$1" | sed -e "s/^$1 //"
