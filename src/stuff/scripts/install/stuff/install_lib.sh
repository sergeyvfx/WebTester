#!/bin/sh

# src_dir file user grouop mode

SRC_TOPDIR=`$PREFIX/stuff/opt_get.sh SRC_TOPDIR`
DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`

echo -n "Installing library \`$2\`... "
if ( `cp $SRC_TOPDIR$1/$2 $DIST_DIR/webtester/lib/$2 > /dev/null 2>&1` ); then
  echo "ok.";
  ${DIST_INST} && chown $3:$4 "$DIST_DIR/webtester/lib/$2";
  chmod $5 "$DIST_DIR/webtester/lib/$2";
  ${DST_INST} && ln -s "$DIST_DIR/webtester/lib/$2" \
    /usr/lib/$2 > /dev/null 2>&1;
else
  echo "failed!";
  exit -1;
fi

exit 0;
