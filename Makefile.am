SUBDIRS  = data bitmaps solver
INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore $(all_includes)

bin_PROGRAMS       = kmines
kmines_LDFLAGS     = $(all_libraries) $(KDE_RPATH)
kmines_LDADD       = ./solver/libsolver.la $(LIB_KDEGAMES)
kmines_DEPENDENCIES = $(LIB_KDEGAMES_DEP)
kmines_SOURCES     = kzoommainwindow.cpp defines.cpp highscores.cpp settings.kcfgc \
                     dialogs.cpp frame.cpp field.cpp status.cpp main.cpp
kmines_METASOURCES = AUTO

rcdir   = $(kde_datadir)/kmines
rc_DATA = kminesui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp solver/*.cpp -o $(podir)/kmines.pot

# for system-wide highscore file
DESTBIN = $(DESTDIR)$(bindir)/$(bin_PROGRAMS)
DESTSCORES = $(HIGHSCORE_DIRECTORY)/$(bin_PROGRAMS).scores

install-data-local:
	@(test x$(HIGHSCORE_DIRECTORY) != x \
	&& echo "********************************************************" \
	&& echo "" \
	&& echo "This game is installed sgid \"games\" to use the" \
	&& echo "system-wide highscore file (in "$(HIGHSCORE_DIRECTORY)")." \
	&& echo "" \
	&& echo "If the system-wide highscore file does not exist, it is" \
        && echo "created with the correct ownership and permissions. See the" \
        && echo "INSTALL file in \"kdegames/libkdegames/highscore\" for details." \
	&& echo "" \
	&& echo "********************************************************" \
	) || true

install-exec-hook:
	@(test x$(HIGHSCORE_DIRECTORY) != x \
        && ((chown root:games $(DESTBIN) && chmod 2755  $(DESTBIN)) \
	|| echo "Error: Could not install the game with correct permissions !!" \
	)) || true

	@(test x$(HIGHSCORE_DIRECTORY) != x \
        && ((touch $(DESTSCORES) && chown root.games $(DESTSCORES) \
	&& chmod 0660 $(DESTSCORES)) \
	|| echo "Error: Could not create system-wide highscore file with correct permissions !!" \
	)) || true

