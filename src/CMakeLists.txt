ecm_setup_version(${KMINES_VERSION}
    VARIABLE_PREFIX KMINES
    VERSION_HEADER kmines_version.h
)

add_executable(kmines)

target_sources(kmines PRIVATE
    borderitem.cpp
    borderitem.h
    cellitem.cpp
    cellitem.h
    commondefs.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    minefielditem.cpp
    minefielditem.h
    scene.cpp
    scene.h
    main.cpp

    kmines.qrc
)

if(APPLE)
    set_target_properties(kmines PROPERTIES
	    MACOSX_BUNDLE_DISPLAY_NAME "KMines"
	    MACOSX_BUNDLE_BUNDLE_NAME "KMines"
	    MACOSX_BUNDLE_LONG_VERSION_STRING "KMines ${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.kmines"
	MACOSX_BUNDLE_COPYRIGHT "2024 The KMines Developers")
endif()

ecm_qt_declare_logging_category(kmines
    HEADER kmines_debug.h
    IDENTIFIER KMINES_LOG
    CATEGORY_NAME kmines
    OLD_CATEGORY_NAMES org.kde.kdegames.kmines
    DESCRIPTION "KMines game"
    EXPORT KMINES
)

ki18n_wrap_ui(kmines customgame.ui generalopts.ui)

kconfig_add_kcfg_files(kmines settings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/data/*-apps-kmines.png")
ecm_add_app_icon(kmines ICONS ${ICONS_SRCS})

target_link_libraries(kmines 
    KDEGames6
    KF6::CoreAddons
    KF6::TextWidgets
    KF6::WidgetsAddons
    KF6::DBusAddons
    KF6::Crash
    KF6::XmlGui
    KF6::IconThemes
)

install(TARGETS kmines  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_qt_install_logging_categories(
    EXPORT KMINES
    FILE kmines.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
