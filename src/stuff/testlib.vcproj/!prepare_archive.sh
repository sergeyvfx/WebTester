#!/bin/sh

#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# This file can be distributed under the terms of the GNU GPL
#

./!clean.sh
./!copy_files.sh

zip -r -9 checker.vcproj.zip ./checker ./src ./testlib \
  ./testlib++ ./checker.sln ./checker.suo

./!clean.sh
