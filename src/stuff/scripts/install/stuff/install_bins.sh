#!/bin/sh

./stuff/install_bin.sh /src/librun/lrvm  lrvm             root      root      06775
./stuff/install_bin.sh /src/webtester    webtester.bin    webtester webtester 0775   false
./stuff/install_bin.sh /src/frontend     gwebtester       webtester webtester 0775   false
