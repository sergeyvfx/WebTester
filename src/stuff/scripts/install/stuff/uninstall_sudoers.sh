#!/bin/sh

data=`cat /etc/sudoers | grep -v "^webtester"`;
echo "${data}" > /etc/sudoers
