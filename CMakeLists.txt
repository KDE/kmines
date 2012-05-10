project(kmines)

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames REQUIRED)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${QDBUS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${KDEGAMES_INCLUDE_DIRS})
endif(NOT COMPILING_KDEGAMES)

add_subdirectory( data ) 
add_subdirectory( themes ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames/highscore  )


########### next target ###############

set(kmines_SRCS
   mainwindow.cpp
   cellitem.cpp
   borderitem.cpp
   minefielditem.cpp
   scene.cpp
   main.cpp )

kde4_add_ui_files(kmines_SRCS customgame.ui generalopts.ui)

kde4_add_kcfg_files(kmines_SRCS settings.kcfgc )

kde4_add_app_icon(kmines_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/data/hi*-app-kmines.png")
kde4_add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines  kdegames)

install(TARGETS kmines  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kminesui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmines )
install( FILES kmines.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
