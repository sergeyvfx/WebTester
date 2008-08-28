#!/bin/sh

if [ `cat /etc/passwd | grep -c "$1:x:"` != 0 ]; then
  echo -n "Deleting user \`$1\`... "
  if ( `userdel "$1" > /dev/null 2>&1` ); then
    echo "ok.";
  else
    echo "failed!";
    exit -1;
  fi
fi

exit 0;
