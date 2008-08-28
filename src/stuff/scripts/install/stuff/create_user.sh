#!/bin/bash

# login home_dir  sh  group  lock

if [ `cat /etc/passwd | grep -c "$1:x:"` = 0 ]; then
  echo -n "Creating user \`$1\`... ";
  if ( `useradd "$1" -d "$2" -s "$3" -g "$4" -m >/dev/null 2>&1 ` ); then
    echo "ok.";
    if ( `test "x$5" = "xtrue"` ); then
      echo -n "Locking local user \`$1\`... ";
      if ( `passwd -l "$1" > /dev/null 2>&1` ); then
        echo "ok.";
      else
        echo "failed!";
        exit -1;
      fi
    fi
  else
    echo "failed!";
    exit -1;
  fi
fi

exit 0;
