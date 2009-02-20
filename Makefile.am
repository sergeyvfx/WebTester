.SILENT:
ACLOCAL_AMFLAGS = -I m4

CFLAGS =
LDFLAGS = 

SUBDIRS = src etc

distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

EXTRA_DIST = 

DISTCLEANFILES = 

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

pkgconfigdir=${libdir}/pkgconfig
pkgconfig_DATA=webtester.pc

install:
	src/stuff/scripts/install/install.sh --dist-dir $$DESTDIR/home

install-gui:
	src/stuff/scripts/install/install.sh --dist-dir \
	  $$DESTDIR/home --gui-only

install-pkg:
	src/stuff/scripts/install/install.sh PACKAGE --dist-dir $$DESTDIR/home

install-gui-pkg:
	src/stuff/scripts/install/install.sh PACKAGE --dist-dir \
	  $$DESTDIR/home --gui-only

install-libwt-pkg:
	src/stuff/scripts/install/install.sh PACKAGE --dist-dir \
	  $$DESTDIR/home --libwt-only
