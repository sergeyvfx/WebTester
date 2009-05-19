#!/bin/sh

#
# Script for building checker  writteon on C++
#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# This file can be distributed under the terms of the GNU GPL

g++ -L/home/webtester/lib -I/home/webtester/include \
  -ltestlib -ltestlib++ $@
