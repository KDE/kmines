## ############################################################################
## This Makefile.am is intended to be generic for standard (small) applications
## contact hadacek@kde.org for comments/suggestions

## ######################
## # Building definitions

bin_PROGRAMS   = kmines
INCLUDES       = $(all_includes)
kmines_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kmines_LDADD   = $(LIB_KDEUI)

kmines_SOURCES = defines.cpp dialogs.cpp field.cpp status.cpp main.cpp
kmines_METASOURCES    = USE_AUTOMOC

noinst_HEADERS = defines.h dialogs.h field.h status.h main.h

SUBDIRS = data

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kmines_SOURCES) && mv messages.po ../po/kmines.pot
