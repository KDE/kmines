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

messages:
	$(XGETTEXT) -C -ktranslate -x ../../kdelibs/po/kde.pot $(kmines_SOURCES) && mv messages.po ../po/kmines.pot

## ##########################
## # Installation definitions

icondir = $(kde_icondir)
icon_DATA = kmines.xpm
minidir = $(kde_minidir)
appsdir = $(kde_appsdir)/Games
apps_DATA = kmines.kdelnk
htmldir = $(kde_htmldir)/en/kmines
html_DATA = index.html index-1.html index-2.html logotp3.gif

install-data-local:
	$(mkinstalldirs) $(minidir)
	$(INSTALL_DATA) mini-kmines.xpm $(minidir)/kmines.xpm

uninstall-local:
	rm -f $(minidir)/kmines.xpm
	rm -rf $(htmldir)

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
