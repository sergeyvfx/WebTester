#!/bin/sh

$PREFIX/stuff/mkdistdir.sh /webtester/usr/pixmaps           webtester  webtester  775
$PREFIX/stuff/mkdistdir.sh /webtester/usr/pixmaps/frontend  webtester  webtester  775
$PREFIX/stuff/cpfile.sh /src/frontend/pixmaps/* /usr/pixmaps/frontend webtester webtester 0664
$PREFIX/stuff/chmod.sh  /webtester/usr/pixmaps/frontend  775