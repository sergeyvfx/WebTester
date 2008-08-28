#!/bin/sh

# plugin

SRC_TOPDIR=`./stuff/opt_get.sh SRC_TOPDIR`
DIST_DIR=`./stuff/opt_get.sh DIST_DIR`

echo -n "Installing plugin \`$1\`... "
if ( `cp $SRC_TOPDIR/src/webtester/plugins/$1/lib$1.so $DIST_DIR/webtester/lib/plugins/lib$1.so > /dev/null 2>&1` ); then
  echo "ok.";
  chown webtester:webtester "$DIST_DIR/webtester/lib/plugins/lib$1.so";
  chmod 0775 "$DIST_DIR/webtester/lib/plugins/lib$1.so"
else
  echo "failed!";
  exit -1;
fi

exit 0;
