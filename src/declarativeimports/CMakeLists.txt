project(corebindings)

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames )

set(corebindings_SRCS
    corebindingsplugin.cpp
    canvasitem.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${corebindings_SRCS})

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin kdegames ${QT_QTDECLARATIVE_LIBRARY})

install(TARGETS corebindingsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/games/core)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/games/core)
