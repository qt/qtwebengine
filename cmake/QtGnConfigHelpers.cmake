# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# These are functions aim to create cmake files with configuration needed by GnGen
#  * gn_config_target.cmake
#  * gn_config_c.cmake
#  * gn_config_cxx.cmake
#  * gn_static.cmake

function(get_config_filenames c_config cxx_config static_config target_config)
    set(${target_config} gn_config_target.cmake PARENT_SCOPE)
    set(${cxx_config} gn_config_cxx.cmake PARENT_SCOPE)
    set(${c_config} gn_config_c.cmake PARENT_SCOPE)
    set(${static_config} gn_static.cmake PARENT_SCOPE)
endfunction()

function(create_cxx_configs cmake_target arch)
    get_config_filenames(c_config_file_name cxx_config_file_name static_config_file_name target_config_file_name)
    create_c_config(${cmake_target} ${arch} ${c_config_file_name})
    create_cxx_config(${cmake_target} ${arch} ${cxx_config_file_name})
    create_static_config(${cmake_target} ${arch} ${static_config_file_name})
endfunction()

function(create_cxx_config cmake_target arch config_file_name)
    if(NOT QT_SUPERBUILD AND QT_WILL_INSTALL)
        get_target_property(moc_file_path Qt6::moc IMPORTED_LOCATION)
    else()
        if(CMAKE_CROSSCOMPILING)
            set(moc_file_path "${QT_HOST_PATH}/${INSTALL_LIBEXECDIR}/moc${CMAKE_EXECUTABLE_SUFFIX}")
        else()
            set(moc_file_path "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/moc${CMAKE_EXECUTABLE_SUFFIX}")
        endif()
    endif()
    file(GENERATE
        OUTPUT $<CONFIG>/${arch}/${config_file_name}
        CONTENT "\
            set(GN_INCLUDES \"$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>\")\n\
            set(GN_DEFINES \"$<TARGET_PROPERTY:COMPILE_DEFINITIONS>\")\n\
            set(GN_LINK_OPTIONS \"$<TARGET_PROPERTY:LINK_OPTIONS>\")\n\
            set(GN_CXX_COMPILE_OPTIONS \"$<TARGET_PROPERTY:COMPILE_OPTIONS>\")\n\
            set(GN_MOC_PATH \"${moc_file_path}\")"
#           set(GN_LIBS $<TARGET_PROPERTY:LINK_LIBRARIES>)
        CONDITION $<COMPILE_LANGUAGE:CXX>
        TARGET ${cmake_target}
    )
endfunction()

function(create_static_config cmake_target arch config_file_name)
    list(APPEND libs Png Jpeg Harfbuzz Freetype Zlib)
    foreach(lib IN LISTS libs)
        string(TOUPPER ${lib} out)
        set(lib Qt::${lib}Private)
        list(APPEND contents "set(GN_${out}_INCLUDES \"$<$<STREQUAL:$<TARGET_NAME_IF_EXISTS:${lib}>,${lib}>:$<TARGET_PROPERTY:${lib},INTERFACE_INCLUDE_DIRECTORIES>>\")")
    endforeach()
    list(JOIN contents "\n" contents)
    file(GENERATE
        OUTPUT $<CONFIG>/${arch}/${config_file_name}
        CONTENT "${contents}"
    )
endfunction()

function(create_c_config cmake_target arch config_file_name)
    file(GENERATE
          OUTPUT $<CONFIG>/${arch}/${config_file_name}
          CONTENT "set(GN_C_COMPILE_OPTIONS $<TARGET_PROPERTY:COMPILE_OPTIONS>)"
          CONDITION $<COMPILE_LANGUAGE:C>
          TARGET ${cmake_target})
endfunction()

function(create_gn_target_config target config_file_name)
    get_target_property(element_list ${target} ELEMENTS)
    get_target_property(prefix ${target} PREFIX)
    file(WRITE ${config_file_name}
        "set(PREFIX ${prefix})\nset(ELEMENTS ${element_list})\n"
    )
    foreach(element IN LISTS element_list)
         get_target_property(prop ${target} ${prefix}_${element})
         if(prop)
             file(APPEND ${config_file_name} "set(${prefix}_${element} ${prop})\n")
         endif()
    endforeach()
endfunction()

