project(kmines)

add_subdirectory( data ) 
add_subdirectory( themes ) 

set(libsolver_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/solver/bfield.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/solver/solver.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/solver/advFastRules.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/solver/adviseFast.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/solver/adviseFull.cpp
	)

add_subdirectory( solver ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames ${CMAKE_SOURCE_DIR}/libkdegames/highscore  )


########### next target ###############

set(kmines_SRCS ${libsolver_SRCS}
   defines.cpp 
   dialogs.cpp 
   kminestheme.cpp
   kminesthemeselector.cpp
   field.cpp 
   status.cpp 
   main.cpp )

kde4_automoc(${kmines_SRCS})

kde4_add_ui_files(kmines_SRCS kminesthemeselector.ui )

kde4_add_kcfg_files(kmines_SRCS settings.kcfgc )

kde4_add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines  ${KDE4_KDEUI_LIBS} kdegames ${KDE4_KNOTIFYCONFIG_LIBS} )

install(TARGETS kmines  DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES kminesui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmines )
