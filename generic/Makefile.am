INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

lib_LTLIBRARIES             = libkdehighscores.la
libkdehighscores_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined -version-info 1:0
libkdehighscores_la_LIBADD  = $(top_builddir)/libkdegames/libkdegames.la $(LIB_KIO)

libkdehighscores_la_SOURCES = gsettings.cpp \
                              ghighscores_item.cpp ghighscores_internal.cpp \
                              ghighscores_gui.cpp ghighscores.cpp \
                              gmisc_ui.cpp canvas.cpp 
noinst_HEADERS = gsettings.h ghighscores_item.h ghighscores.h \
                 ghighscores_gui.h ghighscores_internal.h gmisc_ui.h \
                 canvas.h grid2d.h
METASOURCES = gsettings.moc ghighscores_gui.moc gmisc_ui.moc canvas.moc

messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdehighscores.pot
