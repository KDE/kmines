INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

noinst_LTLIBRARIES = libkmines.la
libkmines_la_LDFLAGS = $(all_libraries)
libkmines_la_LIBADD  = $(LIB_KDEGAMES) $(LIB_KIO)

libkmines_la_SOURCES = gstring.cpp gsettings.cpp
noinst_HEADERS = gstring.h gsettings.h
METASOURCES = gsettings.moc
