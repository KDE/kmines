INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

lib_LTLIBRARIES             = libkdehighscores.la
libkdehighscores_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 1:0
libkdehighscores_la_LIBADD  = $(LIB_KDEGAMES) $(LIB_KIO)

libkdehighscores_la_SOURCES = gstring.cpp gsettings.cpp \
                              ghighscores_item.cpp ghighscores_internal.cpp \
                              ghighscores_tab.cpp ghighscores_gui.cpp \
                              ghighscores.cpp gmisc_ui.cpp gmisc.cpp
noinst_HEADERS = gstring.h gsettings.h \
                 ghighscores_item.h ghighscores_tab.h \
                 ghighscores_gui.h ghighscores_internal.h ghighscores.h \
                 gmisc_ui.h gmisc.h grid2d.h
METASOURCES = gsettings.moc ghighscores_tab.moc ghighscores_gui.moc \
              ghighscores.moc gmisc_ui.moc gmisc.moc

messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdehighscores.pot
