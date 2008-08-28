#!/bin/sh

# user pass

if [ `cat /etc/samba/smbpasswd | grep -c "$1:"` = 0 ]; then
  echo -n "Creating Samba's user \`$1\`..."
  echo "$2" > __t_smb_pass; echo "$2" >> __t_smb_pass; 
  if ( `smbpasswd -a "$1" -s < __t_smb_pass > /dev/null 2>&1` ); then
    rm __t_smb_pass;
    echo "ok."
  else
    rm __t_smb_pass;
    echo "failed!"
    exit -1;
  fi;
fi

exit 0;
