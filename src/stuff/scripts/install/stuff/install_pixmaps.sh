#!/bin/sh

./stuff/mkdistdir.sh /webtester/usr/pixmaps           webtester  webtester  775
./stuff/mkdistdir.sh /webtester/usr/pixmaps/frontend  webtester  webtester  775
./stuff/cpfile.sh /src/frontend/pixmaps/* /usr/pixmaps/frontend webtester webtester 0664
./stuff/chmod.sh  /webtester/usr/pixmaps/frontend  775