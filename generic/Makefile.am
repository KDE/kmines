INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

noinst_LTLIBRARIES             = libkdehighscores.la
libkdehighscores_la_LDFLAGS = $(all_libraries)
libkdehighscores_la_LIBADD  = $(LIB_KDEGAMES) $(LIB_KIO)

libkdehighscores_la_SOURCES = gstring.cpp gsettings.cpp \
                              ghighscores_item.cpp ghighscores_internal.cpp \
                              ghighscores_tab.cpp ghighscores_gui.cpp \
                              ghighscores.cpp
noinst_HEADERS = gstring.h gsettings.h \
                 ghighscores_item.h ghighscores_tab.h \
                 ghighscores_gui.h ghighscores_internal.h ghighscores.h
METASOURCES = gsettings.moc ghighscores_tab.moc ghighscores_gui.moc

messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/libkdehighscores.pot
