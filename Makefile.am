bin_PROGRAMS   = kmines
INCLUDES       = -I$(top_srcdir)/libkdegames $(all_includes)
kmines_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kmines_LDADD   = $(top_builddir)/libkdegames/libkdegames.la -lkdeprint

kmines_SOURCES     = defines.cpp gsettings.cpp ghighscores.cpp highscores.cpp \
                     dialogs.cpp field.cpp status.cpp main.cpp
kmines_METASOURCES = dialogs.moc gsettings.moc ghighscores.moc \
                     field.moc status.moc main.moc

SUBDIRS = data bitmaps

rcdir   = $(kde_datadir)/kmines
rc_DATA = kminesui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(kmines_SOURCES) -o $(podir)/kmines.pot

install-data-local:
	rm -rf $(kde_appsdir)/Games/kmines.desktop
