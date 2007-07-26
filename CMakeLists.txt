project(kmines)

add_subdirectory( data ) 
add_subdirectory( themes ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames ${CMAKE_SOURCE_DIR}/libkdegames/highscore  )


########### next target ###############

set(kmines_SRCS
   mainwindow.cpp
   cellitem.cpp
   borderitem.cpp
   minefielditem.cpp
   scene.cpp
   renderer.cpp
   main.cpp )

kde4_add_ui_files(kmines_SRCS customgame.ui)

kde4_add_kcfg_files(kmines_SRCS settings.kcfgc )

kde4_add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines  kdegames ${KDE4_KNEWSTUFF2_LIBS} )

install(TARGETS kmines  DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES kminesui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmines )
install( FILES kmines.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
