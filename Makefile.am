## ############################################################################
## This Makefile.am is intended to be generic for standard (small) applications
## contact hadacek@kde.org for comments/suggestions

## ######################
## # Building definitions

bin_PROGRAMS = kmines
INCLUDES = $(all_includes)
kmines_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kmines_LDADD = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)

kmines_SOURCES= dialogs.cpp field.cpp status.cpp main.cpp
BUILTSOURCES = dialogs.moc field.moc status.moc main.moc
DISTCLEANFILES = $(BUILTSOURCES)

noinst_HEADERS = defines.h main.h status.h version.h field.h dialogs.h

SUBDIRS = data

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../kdelibs/po/kde.pot $(kmines_SOURCES) && mv messages.po ../po/kmines.pot

## ##############
## # MOC building

main.cpp: main.moc
main.moc: main.h
	$(MOC) -o main.moc main.h

status.cpp: status.moc
status.moc: status.h
	$(MOC) -o status.moc status.h

field.cpp: field.moc
field.moc: field.h
	$(MOC) -o field.moc field.h

dialogs.cpp: dialogs.moc
dialogs.moc: dialogs.h
	$(MOC) -o dialogs.moc dialogs.h
