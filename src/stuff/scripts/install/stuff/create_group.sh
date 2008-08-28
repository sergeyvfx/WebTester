#!/bin/sh

# group

if [ `cat /etc/group | grep -c "$1:x:"` = 0 ]; then
  echo -n "Creating grouop \`$1\`...";
  if ( `groupadd "$1" > /dev/null 2>&1` ); then
    echo "ok.";
  else
    echo "failed!";
    exit -1;
  fi
fi

exit 0;
