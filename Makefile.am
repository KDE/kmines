SUBDIRS  = generic data bitmaps
INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/kmines/generic $(all_includes)

bin_PROGRAMS       = kmines
kmines_LDFLAGS     = $(all_libraries) $(KDE_RPATH)
kmines_LDADD       = ./generic/libkdehighscores.la -lkdeprint
kmines_SOURCES     = defines.cpp highscores.cpp \
                     dialogs.cpp field.cpp status.cpp main.cpp
kmines_METASOURCES = dialogs.moc field.moc status.moc main.moc

rcdir   = $(kde_datadir)/kmines
rc_DATA = kminesui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp `find . -name \*.h -o -name \*.cpp` -o $(podir)/kmines.pot

install-data-local:
	rm -rf $(kde_appsdir)/Games/kmines.desktop
