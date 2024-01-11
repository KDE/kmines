# SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

function(install_theme name)
    cmake_parse_arguments(ARG "" "SVG;PREVIEW" "" ${ARGN})
    if (NOT ARG_SVG)
        set(ARG_SVG "${name}.svg")
    endif()
    if (NOT ARG_PREVIEW)
        set(ARG_PREVIEW "${name}_preview.png")
    endif()
    set(svgz "${CMAKE_CURRENT_BINARY_DIR}/${ARG_SVG}z")
    generate_svgz(${ARG_SVG} ${svgz} "theme-")
    install(
        FILES
            ${name}.desktop
            ${ARG_PREVIEW}
            ${svgz}
        DESTINATION ${KDE_INSTALL_DATADIR}/kmines/themes
    )
endfunction()

install_theme(default SVG kmines_oxygen.svg)
install_theme(classic SVG kmines_classic.svg)
install_theme(green SVG kmines_green.svg PREVIEW green.png)
install_theme(graveyard-mayhem PREVIEW graveyard-mayhem-preview.png)
install_theme(clean_blue SVG kmines_clean_blue.svg)
install_theme(clean_blue_dark SVG kmines_clean_blue_dark.svg)
