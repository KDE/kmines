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

ecm_qt_declare_logging_category(kmines
    HEADER kmines_debug.h
    IDENTIFIER KMINES_LOG
    CATEGORY_NAME org.kde.kdegames.kmines
    DESCRIPTION "KMines game"
    EXPORT KMINES
)

ki18n_wrap_ui(kmines customgame.ui generalopts.ui)

kconfig_add_kcfg_files(kmines settings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/data/*-apps-kmines.png")
ecm_add_app_icon(kmines ICONS ${ICONS_SRCS})

target_link_libraries(kmines 
    KF5KDEGames
    KF5::TextWidgets
    KF5::WidgetsAddons
    KF5::DBusAddons
    KF5::Crash
    KF5::XmlGui
)

install(TARGETS kmines  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_qt_install_logging_categories(
    EXPORT KMINES
    FILE kmines.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
