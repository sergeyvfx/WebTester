#!/bin/sh

if [ `cat /etc/group | grep -c "$1:x:"` != 0 ]; then
  echo -n "Deleting group \`$1\`... "
  if ( `groupdel "$1"` ); then
    echo "ok.";
  else
    echo "failed!";
    exit -1;
  fi
fi

exit 0;
