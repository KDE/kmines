SUBDIRS  = generic data bitmaps solver
INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore -I$(top_srcdir)/kmines/generic $(all_includes)

bin_PROGRAMS       = kmines
kmines_LDFLAGS     = $(all_libraries) $(KDE_RPATH)
kmines_LDADD       = ./solver/libsolver.la ./generic/libkmines.la $(LIB_KDEGAMES)
kmines_SOURCES     = defines.cpp highscores.cpp \
                     dialogs.cpp frame.cpp field.cpp status.cpp main.cpp
kmines_METASOURCES = dialogs.moc field.moc status.moc main.moc

rcdir   = $(kde_datadir)/kmines
rc_DATA = kminesui.rc kminesconfig.rc

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
