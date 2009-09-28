#!/bin/sh

$PREFIX/stuff/uninstall_sudoers.sh

echo "webtester ALL=NOPASSWD:${PREFIX}/webtester/sbin/lrvm_killall.sh" >> \
  /etc/sudoers
