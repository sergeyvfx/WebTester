#!/bin/sh

# lib

if ( `test -e /usr/bin/$1 ` ); then
  echo -n "Uninstalling binary \`$1\`...";
  if ( `rm -f /usr/bin/$1` ); then
    echo "ok.";
  else
    echo "failed!";
    exit -1;
  fi  
fi

exit -1;
