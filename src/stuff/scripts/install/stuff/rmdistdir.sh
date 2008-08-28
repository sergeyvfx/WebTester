#!/bin/sh

# dir

DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`

if ( `test -e "$DIST_DIR$1"` ); then
  echo -n "Recusively removing dist directory \`$1\`... "
  if ( rm -rf "$DIST_DIR$1" ); then
    echo "ok.";
  else
    echo "failed!";
    exit -1;
  fi
fi

exit 0;
