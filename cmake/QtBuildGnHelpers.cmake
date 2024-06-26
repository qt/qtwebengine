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
    get_property(gn_cxx_compile_options DIRECTORY PROPERTY GN_CXX_COMPILE_OPTIONS)
    foreach(gn_cxx_compile_option ${gn_cxx_compile_options})
        list(APPEND GN_CFLAGS_CC \"${gn_cxx_compile_option}\")
    endforeach()
    list(REMOVE_DUPLICATES GN_CFLAGS_CC)

    # GN_CFLAGS_C
    get_property(gn_c_compile_options DIRECTORY PROPERTY GN_C_COMPILE_OPTIONS)
    foreach(gn_c_compile_option ${gn_c_compile_options})
        list(APPEND GN_CFLAGS_C \"${gn_c_compile_option}\")
    endforeach()
    list(REMOVE_DUPLICATES GN_CFLAGS_C)

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

