#!/bin/sh

$PREFIX/stuff/uninstall_sudoers.sh

echo "webtester ALL=NOPASSWD:${DIST_DIR}/webtester/sbin/lrvm_killall.sh" >> \
  /etc/sudoers
