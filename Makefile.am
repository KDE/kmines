bin_PROGRAMS   = kmines
INCLUDES       = $(all_includes)
kmines_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kmines_LDADD   = $(LIB_KDEUI)

kmines_SOURCES     = defines.cpp dialogs.cpp field.cpp status.cpp main.cpp
kmines_METASOURCES = USE_AUTOMOC
noinst_HEADERS     = defines.h dialogs.h field.h status.h main.h

SUBDIRS = data bitmaps

rcdir   = $(kde_datadir)/kmines
rc_DATA = kminesui.rc

messages:
	$(XGETTEXT) $(kmines_SOURCES) -o $(podir)/kmines.pot
