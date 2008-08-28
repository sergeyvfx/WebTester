#!/bin/sh

# file mode

DIST_DIR=`./stuff/opt_get.sh DIST_DIR`

chmod $2 "$DIST_DIR$1";

exit 0;
