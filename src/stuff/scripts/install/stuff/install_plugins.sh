#!/bin/sh

cat ./stuff/!plugins | awk ' { printf "./stuff/install_plugin.sh %s\n",$1;  } ' > ./z_install_plugins.sh

chmod 0775 ./z_install_plugins.sh
./z_install_plugins.sh
rm ./z_install_plugins.sh
