# SPDX-FileCopyrightText: 2021, 2023 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(7Zip)
set_package_properties(7Zip PROPERTIES
    PURPOSE "For installing SVG files as SVGZ"
)

if(WIN32)
    set_package_properties(7Zip PROPERTIES
        TYPE REQUIRED
    )
else()
    set_package_properties(7Zip PROPERTIES
        TYPE OPTIONAL
    )
    if(NOT TARGET 7Zip::7Zip)
        find_package(gzip)
        set_package_properties(gzip PROPERTIES
            TYPE REQUIRED
            PURPOSE "For installing SVG files as SVGZ (less efficient fallback for 7Zip)"
        )
    endif()
endif()

function(generate_svgz svg_file svgz_file target_prefix)
    if (NOT IS_ABSOLUTE ${svg_file})
        set(svg_file "${CMAKE_CURRENT_SOURCE_DIR}/${svg_file}")
    endif()
    if (NOT EXISTS ${svg_file})
        message(FATAL_ERROR "No such file found: ${svg_file}")
    endif()
    get_filename_component(_fileName "${svg_file}" NAME)

    if(TARGET 7Zip::7Zip)
        add_custom_command(
            OUTPUT ${svgz_file}
            COMMAND 7Zip::7Zip
            ARGS
                a
                -bd # silence logging
                -mx9 # compress best
                -tgzip
                ${svgz_file} ${svg_file}
            DEPENDS ${svg_file}
            COMMENT "Gzipping ${_fileName}"
        )
    else()
        add_custom_command(
            OUTPUT ${svgz_file}
            COMMAND gzip::gzip
            ARGS
                -9 # compress best
                -n # no original name and timestamp stored, for reproducibility
                -c # write to stdout
                ${svg_file} > ${svgz_file}
            DEPENDS ${svg_file}
            COMMENT "Gzipping ${_fileName}"
        )
    endif()

    add_custom_target("${target_prefix}${_fileName}z" ALL DEPENDS ${svgz_file})
endfunction()
