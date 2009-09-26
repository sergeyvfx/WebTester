#!/bin/sh

$PREFIX/stuff/uninstall_sudoers.sh

echo "webtester ALL=NOPASSWD:${INSTALL_DIR}/sbin/lrvm_killall.sh" >> \
  /etc/sudoers
