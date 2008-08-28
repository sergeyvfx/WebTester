#!/bin/sh

$PREFIX/stuff/install_lib.sh /src/libwebtester    libwebtester.so   webtester webtester 0775
$PREFIX/stuff/install_lib.sh /src/librun          librun.so         webtester webtester 0775
$PREFIX/stuff/install_lib.sh /src/stuff/testlib   libtestlib.so     webtester webtester 0775
$PREFIX/stuff/install_lib.sh /src/stuff/testlib++ libtestlib++.so   webtester webtester 0775
