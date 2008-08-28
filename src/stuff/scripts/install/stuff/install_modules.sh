#!/bin/sh

cat $PREFIX/stuff/!modules | awk ' { printf "$PREFIX/stuff/install_module.sh %s\n",$1;  } ' > ./z_install_modules.sh

chmod 0775 ./z_install_modules.sh
./z_install_modules.sh
rm ./z_install_modules.sh
