## ############################################################################
## This Makefile.am is intended to be generic for standard (small) applications
## contact hadacek@kde.org for comments/suggestions

## ######################
## # Building definitions

bin_PROGRAMS = kmines
INCLUDES = $(all_includes)
kmines_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kmines_LDADD =       $(LIB_KDEUI)

kmines_SOURCES= dialogs.cpp field.cpp status.cpp main.cpp
METASOURCES = dialogs.moc field.moc status.moc main.moc

noinst_HEADERS = defines.h main.h status.h version.h field.h dialogs.h

SUBDIRS = data

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kmines_SOURCES) && mv messages.po ../po/kmines.pot

