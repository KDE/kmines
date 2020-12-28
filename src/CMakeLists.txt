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
file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/data/*-apps-kmines.png")
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

if (ECM_VERSION VERSION_GREATER "5.58.0")
    install(FILES kmines.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kmines.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()
