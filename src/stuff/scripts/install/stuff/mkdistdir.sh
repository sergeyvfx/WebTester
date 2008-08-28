#!/bin/sh

# dir owner group mode

DIST_DIR=`$PREFIX/stuff/opt_get.sh DIST_DIR`

if ( `test ! -e "$DIST_DIR$1"` ); then
  echo -n "Creating dist directory \`$1\`... "
  if ( mkdir "$DIST_DIR$1" ); then
    echo "ok.";
    chown "$2:$3" "$DIST_DIR$1";
    chmod $4 "$DIST_DIR$1";
  else
    echo "failed!";
    exit -1;
  fi
fi

exit 0;
