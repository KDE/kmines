cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "90")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KMINES_BASE_VERSION once new features are added
set(KMINES_BASE_VERSION "4.0")
set(KMINES_VERSION "${KMINES_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kmines VERSION ${KMINES_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.89.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    I18n
    TextWidgets
    WidgetsAddons
    XmlGui
)
find_package(KF5DocTools ${KF5_MIN_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

find_package(KF5KDEGames 7.3.1 REQUIRED)

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055900
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory(data)
add_subdirectory(themes)
add_subdirectory(src)

ki18n_install(po)
if(KF5DocTools_FOUND)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
