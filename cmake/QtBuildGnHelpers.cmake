# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# These are helper functions aim to create BUILD.gn

function(init_gn_config file_path)
    include(${file_path})
    set_directory_properties(PROPERTIES
        ELEMENTS "${ELEMENTS}"
        PREFIX "${PREFIX}"
    )
    set_properties_on_directory_scope()
endfunction()

function(read_gn_config file_path)
    include(${file_path})
    set_properties_on_directory_scope()
endfunction()

macro(set_properties_on_directory_scope)
    get_directory_property(element_list ELEMENTS)
    get_directory_property(prefix PREFIX)
    foreach(element IN LISTS element_list)
        if(${prefix}_${element})
            set_property(DIRECTORY APPEND PROPERTY ${prefix}_${element} ${${prefix}_${element}})
        endif()
    endforeach()
endmacro()

# we need to pass -F or -iframework in case of frameworks builds, which gn treats as
# compiler flag and cmake as include dir, so swap it.
function(recover_framework_build includeDirs compilerFlags)
    foreach(includeDir ${${includeDirs}})
        if(includeDir MATCHES "^\"(.*/([^/]+)\\.framework)\"$")
            list(APPEND frameworkDirs \"-iframework${CMAKE_MATCH_1}/..\")
        else()
            list(APPEND newIncludeDirs ${includeDir})
        endif()
    endforeach()
    set(${includeDirs} ${newIncludeDirs} PARENT_SCOPE)
    set(${compilerFlags} ${${compilerFlags}} ${frameworkDirs} PARENT_SCOPE)
endfunction()

# CMake uses a "SHELL:" prefix to group options and avoid unwanted option de-duplication; we
# need to strip these after manually de-duplicating, but before passing to GN.
# See https://cmake.org/cmake/help/latest/command/target_compile_options.html#option-de-duplication
function(transform_cmake_compile_options_for_gn out_var compile_options_var)
    get_property(flags_var DIRECTORY PROPERTY ${compile_options_var})

    list(REMOVE_DUPLICATES flags_var)
    set(out_flags "")
    foreach(elem IN LISTS flags_var)
        if(elem MATCHES "^SHELL:(.*)")
            # Split on spaces and enclose each argument with quotes.
            string(REPLACE " " "\";\"" elem "${CMAKE_MATCH_1}")
        endif()
        list(APPEND out_flags "\"${elem}\"")
    endforeach()

    set(${out_var} ${out_flags} PARENT_SCOPE)
endfunction()

function(configure_gn_target source_dir in_file_path out_file_path path_mode)

    # GN_SOURCES GN_HEADERS
    get_property(gn_sources DIRECTORY PROPERTY GN_SOURCES)
    foreach(gn_source_file ${gn_sources})
        get_filename_component(gn_source_path ${source_dir}/${gn_source_file} ${path_mode})
        list(APPEND source_list \"${gn_source_path}\")
    endforeach()
    set(GN_HEADERS ${source_list})
    set(GN_SOURCES ${source_list})
    list(FILTER GN_HEADERS INCLUDE REGEX "^.+\\.h\"$")
    list(FILTER GN_SOURCES EXCLUDE REGEX "^.+\\.h\"$")

    # GN_DEFINES
    get_property(gn_defines DIRECTORY PROPERTY GN_DEFINES)
    list(REMOVE_DUPLICATES gn_defines)
    foreach(gn_define ${gn_defines})
        list(APPEND GN_ARGS_DEFINES \"-D${gn_define}\")
        list(APPEND GN_DEFINES \"${gn_define}\")
    endforeach()

    # GN_INCLUDES
    get_property(gn_includes DIRECTORY PROPERTY GN_INCLUDES)
    list(REMOVE_DUPLICATES gn_includes)
    foreach(gn_include ${gn_includes})
        get_filename_component(gn_include ${gn_include} ${path_mode})
        list(APPEND GN_ARGS_INCLUDES \"-I${gn_include}\")
        list(APPEND GN_INCLUDE_DIRS \"${gn_include}\")
    endforeach()

    # MOC
    get_property(moc_file_path DIRECTORY PROPERTY GN_MOC_PATH)
    set(GN_ARGS_MOC_BIN \"${moc_file_path}\")

    # GN_CFLAGS_CC
    transform_cmake_compile_options_for_gn(GN_CFLAGS_CC GN_CXX_COMPILE_OPTIONS)

    # GN_CFLAGS_C
    transform_cmake_compile_options_for_gn(GN_CFLAGS_C GN_C_COMPILE_OPTIONS)

    # GN_SOURCE_ROOT
    get_filename_component(GN_SOURCE_ROOT "${source_dir}" ${path_mode})

    if(APPLE) # this runs in scrpit mode without qt-cmake so no MACOS here
        recover_framework_build(GN_INCLUDE_DIRS GN_CFLAGS_C)
    endif()

    # Static setup
    set(libs PNG JPEG FREETYPE HARFBUZZ ZLIB)
    foreach(lib ${libs})
        get_property(static_includes DIRECTORY PROPERTY GN_${lib}_INCLUDES)
        foreach(is ${static_includes})
            list(APPEND GN_${lib}_INCLUDES \"${is}\")
        endforeach()
    endforeach()
    foreach(item GN_HEADERS GN_SOURCES GN_ARGS_DEFINES GN_DEFINES GN_ARGS_INCLUDES
        GN_INCLUDE_DIRS GN_CFLAGS_CC GN_CFLAGS_C GN_PNG_INCLUDES GN_JPEG_INCLUDES
        GN_FREETYPE_INCLUDES GN_HARFBUZZ_INCLUDES GN_ZLIB_INCLUDES)
        string(REPLACE ";" ",\n  " ${item} "${${item}}")
    endforeach()
    configure_file(${in_file_path} ${out_file_path} @ONLY)
endfunction()

