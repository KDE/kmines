bin_PROGRAMS   = kmines
INCLUDES       = -I$(top_srcdir)/libkdegames $(all_includes)
kmines_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kmines_LDADD   = $(top_builddir)/libkdegames/libkdegames.la -lkdeprint

kmines_SOURCES     = defines.cpp ghighscores.cpp highscores.cpp dialogs.cpp \
                     field.cpp status.cpp main.cpp
kmines_METASOURCES = dialogs.moc ghighscores.moc \
                     field.moc status.moc main.moc
noinst_HEADERS     = defines.h ghighscores.h highscores.h dialogs.h \
                     field.h status.h main.h

SUBDIRS = data bitmaps

rcdir   = $(kde_datadir)/kmines
rc_DATA = kminesui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(kmines_SOURCES) -o $(podir)/kmines.pot
