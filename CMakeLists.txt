project(kmines)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.2.0")

find_package(ECM 1.3.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Test Qml)
find_package(KF5 REQUIRED COMPONENTS 
  CoreAddons 
  Config 
  WidgetsAddons 
  Config 
  DBusAddons 
  I18n 
  ConfigWidgets 
  TextWidgets 
  XmlGui 
  KIO 
  NotifyConfig)
    
find_package(KF5KDEGames 4.9.0 REQUIRED)
find_package(Phonon4Qt5)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${QDBUS_INCLUDE_DIRS})

add_subdirectory( data ) 
add_subdirectory( themes ) 
add_subdirectory( doc )

include_directories( ${CMAKE_SOURCE_DIR}/KF5KDEGames/highscore  )


########### next target ###############

set(kmines_SRCS
   mainwindow.cpp
   cellitem.cpp
   borderitem.cpp
   minefielditem.cpp
   scene.cpp
   main.cpp )

ki18n_wrap_ui(kmines_SRCS customgame.ui generalopts.ui)
kconfig_add_kcfg_files(kmines_SRCS settings.kcfgc )
#kde4_add_app_icon(kmines_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/data/*-apps-kmines.png")
add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines 
  KF5::TextWidgets 
  KF5::WidgetsAddons
  KF5::DBusAddons 
  Qt5::Qml 
  KF5::XmlGui
  KF5KDEGames)

install(TARGETS kmines  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES kminesui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmines )
install( FILES kmines.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
