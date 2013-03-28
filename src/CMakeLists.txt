include_directories( ${CMAKE_SOURCE_DIR}/libkdegames/highscore  )

set(kmines_SRCS
   mainwindow.cpp
   canvaswidget.cpp
   main.cpp )

kde4_add_ui_files(kmines_SRCS customgame.ui generalopts.ui)

kde4_add_kcfg_files(kmines_SRCS settings.kcfgc )

kde4_add_app_icon(kmines_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../data/hi*-app-kmines.png")
kde4_add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines  kdegames ${QT_QTDECLARATIVE_LIBRARY} ${KDECLARATIVE_LIBRARIES})

install(TARGETS kmines  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kminesui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmines )
install( FILES kmines.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( DIRECTORY qml DESTINATION ${DATA_INSTALL_DIR}/kmines )
