cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "20")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KMINES_BASE_VERSION once new features are added
set(KMINES_BASE_VERSION "4.0")
set(KMINES_VERSION "${KMINES_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kmines VERSION ${KMINES_VERSION})

set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.30.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    I18n
    TextWidgets
    WidgetsAddons
    XmlGui
)
    
find_package(KF5KDEGames 4.9.0 REQUIRED)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050700
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x051E00
)
if (${KF5Config_VERSION} STRGREATER "5.56.0")
    add_definitions(-DQT_NO_FOREACH)
endif()

add_subdirectory(data)
add_subdirectory(themes)
add_subdirectory(doc)

########### next target ###############

set(kmines_SRCS
    mainwindow.cpp
    cellitem.cpp
    borderitem.cpp
    minefielditem.cpp
    scene.cpp
    main.cpp
)
ecm_qt_declare_logging_category(kmines_SRCS HEADER kmines_debug.h IDENTIFIER KMINES_LOG CATEGORY_NAME org.kde.kdegames.kmines)

ecm_setup_version(${KMINES_VERSION}
    VARIABLE_PREFIX KMINES
    VERSION_HEADER kmines_version.h
)

qt5_add_resources(kmines_SRCS kmines.qrc)
ki18n_wrap_ui(kmines_SRCS customgame.ui generalopts.ui)
kconfig_add_kcfg_files(kmines_SRCS settings.kcfgc )
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/data/*-apps-kmines.png")
ecm_add_app_icon(kmines_SRCS ICONS ${ICONS_SRCS})
add_executable(kmines ${kmines_SRCS})

target_link_libraries(kmines 
    KF5::TextWidgets
    KF5::WidgetsAddons
    KF5::DBusAddons
    KF5::Crash
    KF5::XmlGui
    KF5KDEGames
)

install(TARGETS kmines  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
if (ECM_VERSION VERSION_GREATER "5.58.0")
    install(FILES kmines.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kmines.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
