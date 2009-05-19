#!/bin/sh

#
# Script for building checker  writteon on Pascal
#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# This file can be distributed under the terms of the GNU GPL

fpc -Fu/home/webtester/var/fpc/units -Fl/home/webtester/var/fpc/lib \
  -Fo/home/webtester/var/fpc/obj $@
