#!/bin/sh

# lib

if ( `test -e /usr/lib/$1 ` ); then
  echo -n "Uninstalling library \`$1\`...";
  if ( `rm -f /usr/lib/$1` ); then
    echo "ok.";
  else
    echo "failed!";
    exit -1;
  fi  
fi

exit -1;
