# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.2c from Makefile.am

# Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ../..
prefix = /usr/local/kde
exec_prefix = ${prefix}

bindir = /usr/local/kde/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

pkgdatadir = $(datadir)/kdegames
pkglibdir = $(libdir)/kdegames
pkgincludedir = $(includedir)/kdegames

top_builddir = ../..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/ginstall -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = true
PRE_INSTALL = true
POST_INSTALL = true
NORMAL_UNINSTALL = true
PRE_UNINSTALL = true
POST_UNINSTALL = true
build_alias = i586-pc-linux-gnulibc1
build_triplet = i586-pc-linux-gnulibc1
host_alias = i586-pc-linux-gnulibc1
host_triplet = i586-pc-linux-gnulibc1
target_alias = i586-pc-linux-gnulibc1
target_triplet = i586-pc-linux-gnulibc1
CC = gcc
CPP = gcc -E
CXX = g++
GLINC = @GLINC@
GLLIB = @GLLIB@
GMSGFMT = /usr/local/bin/msgfmt
KDE_EXTRA_RPATH = 
KDE_INCLUDES = -I/usr/local/kde/include
KDE_LDFLAGS = -L/usr/local/kde/lib
KDE_RPATH = -rpath $(kde_libraries) -rpath $(qt_libraries) -rpath $(x_libraries)
LD = /usr/bin/ld
LIBCOMPAT = 
LIBCRYPT = 
LIBDL = @LIBDL@
LIBJPEG = -ljpeg
LIBKHTMLW = -lkhtmlw -ljscript
LIBOBJS = @LIBOBJS@
LIBSOCKET = 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBUCB = 
LN_S = ln -s
MAKEINFO = makeinfo
MOC = /usr/local/gui/qt/bin/moc
MSGFMT = /usr/local/bin/msgfmt
NM = /usr/bin/nm -B
PACKAGE = kdegames
PAMINC = @PAMINC@
PAMLIBPATHS = @PAMLIBPATHS@
PAMLIBS = @PAMLIBS@
QT_INCLUDES = -I/usr/local/gui/qt/include
QT_LDFLAGS = -L/usr/local/gui/qt/lib
RANLIB = ranlib
TOPSUBDIRS =  kabalone kasteroids kmahjongg kmines kpat kpoker kreversi ksame kshisen ksnake ktetris po
USE_NLS = yes
VERSION = 0.1
XGETTEXT = /usr/local/bin/xgettext
XPMINC = @XPMINC@
XPMLIB = @XPMLIB@
X_INCLUDES = 
X_LDFLAGS = -L/usr/X11/lib
all_includes = -I/usr/include/g++ -I/usr/local/kde/include -I/usr/local/gui/qt/include 
all_libraries = -L/usr/local/kde/lib -L/usr/local/gui/qt/lib -L/usr/X11/lib
install_root = 
kde_appsdir = /usr/local/kde/share/applnk
kde_bindir = /usr/local/kde/bin
kde_cgidir = /usr/local/kde/cgi-bin
kde_confdir = /usr/local/kde/share/config
kde_datadir = /usr/local/kde/share/apps
kde_htmldir = /usr/local/kde/share/doc/HTML
kde_icondir = /usr/local/kde/share/icons
kde_includes = /usr/local/kde/include
kde_libraries = /usr/local/kde/lib
kde_locale = /usr/local/kde/share/locale
kde_mimedir = /usr/local/kde/share/mimelnk
kde_minidir = /usr/local/kde/share/icons/mini
kde_partsdir = /usr/local/kde/parts
kde_sounddir = /usr/local/kde/share/sounds
kde_toolbardir = /usr/local/kde/share/toolbar
kde_wallpaperdir = /usr/local/kde/share/wallpapers
qt_includes = /usr/local/gui/qt/include
qt_libraries = /usr/local/gui/qt/lib
topdir = /usr/local/cvs/kdegames
x_includes = .
x_libraries = /usr/X11/lib

icondir = $(kde_icondir)
icon_DATA = kmines.xpm
minidir = $(kde_minidir)
appsdir = $(kde_appsdir)/Games
apps_DATA = kmines.kdelnk
htmldir = $(kde_htmldir)/en/kmines
html_DATA = index.html index-1.html index-2.html logotp3.gif
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../config.h
CONFIG_CLEAN_FILES = 
DATA =  $(apps_DATA) $(html_DATA) $(icon_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP = --best
default: all

.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu kmines/data/Makefile

Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-appsDATA: $(apps_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(appsdir)
	@list='$(apps_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(appsdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(appsdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(appsdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(appsdir)/$$p; \
	  fi; fi; \
	done

uninstall-appsDATA:
	$(NORMAL_UNINSTALL)
	list='$(apps_DATA)'; for p in $$list; do \
	  rm -f $(appsdir)/$$p; \
	done

install-htmlDATA: $(html_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(htmldir)
	@list='$(html_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(htmldir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(htmldir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(htmldir)/$$p"; \
	    $(INSTALL_DATA) $$p $(htmldir)/$$p; \
	  fi; fi; \
	done

uninstall-htmlDATA:
	$(NORMAL_UNINSTALL)
	list='$(html_DATA)'; for p in $$list; do \
	  rm -f $(htmldir)/$$p; \
	done

install-iconDATA: $(icon_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(icondir)
	@list='$(icon_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(icondir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(icondir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(icondir)/$$p"; \
	    $(INSTALL_DATA) $$p $(icondir)/$$p; \
	  fi; fi; \
	done

uninstall-iconDATA:
	$(NORMAL_UNINSTALL)
	list='$(icon_DATA)'; for p in $$list; do \
	  rm -f $(icondir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = kmines/data

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu kmines/data/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  test -f $(distdir)/$$file \
	  || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $$d/$$file $(distdir)/$$file; \
	done
info:
dvi:
check: all
	$(MAKE)
installcheck:
install-exec: 
	@$(NORMAL_INSTALL)

install-data: install-appsDATA install-htmlDATA install-iconDATA install-data-local
	@$(NORMAL_INSTALL)

install: install-exec install-data all
	@:

uninstall: uninstall-appsDATA uninstall-htmlDATA uninstall-iconDATA uninstall-local

all: Makefile $(DATA)

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' INSTALL_SCRIPT='$(INSTALL_PROGRAM)' install
installdirs:
	$(mkinstalldirs)  $(appsdir) $(htmldir) $(icondir)


mostlyclean-generic:
	-test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(DISTCLEANFILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	-test -z "$(MAINTAINERCLEANFILES)" || rm -f $(MAINTAINERCLEANFILES)
	-test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean:  mostlyclean-generic

clean:  clean-generic mostlyclean

distclean:  distclean-generic clean
	-rm -f config.status
	-rm -f libtool

maintainer-clean:  maintainer-clean-generic distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: default uninstall-appsDATA install-appsDATA uninstall-htmlDATA \
install-htmlDATA uninstall-iconDATA install-iconDATA tags distdir info \
dvi installcheck install-exec install-data install uninstall all \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


install-data-local:
	$(mkinstalldirs) $(minidir)
	$(INSTALL_DATA) mini-kmines.xpm $(minidir)/kmines.xpm

uninstall-local:
	rm -f $(minidir)/kmines.xpm
	rm -rf $(htmldir)

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
