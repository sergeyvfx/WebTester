#!/bin/sh

#
# This file is a part of WebTester project
#
# Copyright (C) 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
#
# Thids file can be distributed under the terms of the GNU GPL
#

kill -s 9 `pidof lrvm`> /dev/null 2>&1
