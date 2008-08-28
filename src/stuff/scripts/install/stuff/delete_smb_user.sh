#!/bin/sh

# user

if [ `cat /etc/samba/smbpasswd | grep -c "$1:"` = 1 ]; then
  echo -n "Deleting Samba's user \`$1\`..."
  if ( `smbpasswd -x "$1" > /dev/null 2>&1` ); then
    echo "ok."
  else
    echo "failed!"
    exit -1;
  fi;
fi;

exit 0;
