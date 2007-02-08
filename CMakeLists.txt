project(kmines)

add_subdirectory( data ) 
add_subdirectory( themes ) 
message(STATUS "TODO: someone knowing cmake has to fix the solver tests")
#add_subdirectory( solver ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames ${CMAKE_SOURCE_DIR}/libkdegames/highscore  )


########### next target ###############

set(libsolver_SRCS solver/bfield.cpp solver/solver.cpp solver/advFastRules.cpp solver/adviseFast.cpp solver/adviseFull.cpp)

set(kmines_SRCS ${libsolver_SRCS}
   defines.cpp 
   highscores.cpp 
   dialogs.cpp 
   frame.cpp 
   field.cpp 
   status.cpp 
   main.cpp )

kde4_automoc(${kmines_SRCS})

kde4_add_kcfg_files(kmines_SRCS settings.kcfgc )

kde4_add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines  ${KDE4_KDEUI_LIBS} kdegames ${KDE4_KNOTIFYCONFIG_LIBS} )

install(TARGETS kmines  DESTINATION ${BIN_INSTALL_DIR} )

message(STATUS "TODO: install highscore files")

########### install files ###############

install( FILES kminesui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmines )


#install-data-local:
#	@(test x$(HIGHSCORE_DIRECTORY) != x \
#	&& echo "********************************************************" \
#	&& echo "" \
#	&& echo "This game is installed sgid \"games\" to use the" \
#	&& echo "system-wide highscore file (in "$(HIGHSCORE_DIRECTORY)")." \
#	&& echo "" \
#	&& echo "If the system-wide highscore file does not exist, it is" \
#        && echo "created with the correct ownership and permissions. See the" \
#        && echo "INSTALL file in \"kdegames/libkdegames/highscore\" for details." \
#	&& echo "" \
#	&& echo "********************************************************" \
#	) || true
#
#install-exec-hook:
#	@(test x$(HIGHSCORE_DIRECTORY) != x \
#        && ((chown $(highscore_user):$(highscore_group) $(DESTBIN)) \
#	|| echo "Error: Could not install the game with correct permissions !!" \
#	)) || true
#
#	@(test x$(HIGHSCORE_DIRECTORY) != x \
#		&& ((mkdir -p $(DESTHIGHSCORES) && chown $(highscore_user):$(highscore_group) $(DESTHIGHSCORES) \
#		&& chmod 750 $(DESTHIGHSCORES)) \
#	|| echo "Error: Could not create the highscore directory with correct permissions !!" \
#	)) || true
#
#	@(test x$(HIGHSCORE_DIRECTORY) != x \
#        && ((chown $(highscore_user):$(highscore_group) $(DESTBIN)) \
#	|| echo "Error: Could not install the game with correct permissions !!" \
#	)) || true
#
#	@(test ${setgid} = true \
#        && ((chmod 2755  $(DESTBIN)) \
#	|| echo "Error: Could not install the game with correct permissions !!" \
#	)) || true
#
#	@(test x$(HIGHSCORE_DIRECTORY) != x \
#        && ((touch $(DESTSCORES) && chown $(highscore_user):$(highscore_group) $(DESTSCORES) \
#	&& chmod 0660 $(DESTSCORES)) \
#	|| echo "Error: Could not create system-wide highscore file with correct permissions !!" \
#	)) || true
#
