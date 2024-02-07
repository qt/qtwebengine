# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# These are helper functions aim to create and handle properties on gn target

function(add_gn_target target config arch)
    add_custom_target(${target})
    list(REMOVE_ITEM ARGN ${target})
    list(REMOVE_ITEM ARGN ${config})
    list(REMOVE_ITEM ARGN ${arch})
    set_target_properties(${target} PROPERTIES
        ELEMENTS "${ARGN}"
        PREFIX "GN"
        CONFIG ${config}
        ARCH ${arch}
    )
endfunction()

function(extend_gn_target target)
    get_target_property(elements ${target} ELEMENTS)
    cmake_parse_arguments(PARSE_ARGV 1 GN "" "" "CONDITION;${elements}")
    _qt_internal_validate_all_args_are_parsed(GN)

    if("x${GN_CONDITION}" STREQUAL "x")
        set(GN_CONDITION ON)
    endif()
    qt_evaluate_config_expression(result ${GN_CONDITION})
    if(${result})
        message(DEBUG "extend_gn_target(${target} CONDITION ${GN_CONDITION} ...): Evaluated")
        set_properties_on_target_scope(${target})
    endif()
endfunction()

macro(set_properties_on_target_scope target)
    get_target_property(element_list ${target} ELEMENTS)
    get_target_property(prefix ${target} PREFIX)
    foreach(element IN LISTS element_list)
        if(${prefix}_${element})
            set_property(TARGET ${target} APPEND PROPERTY ${prefix}_${element} ${${prefix}_${element}})
        endif()
    endforeach()
endmacro()

function(extend_gn_list out_list)
    cmake_parse_arguments(PARSE_ARGV 1 GN "" "" "ARGS;CONDITION")
    _qt_internal_validate_all_args_are_parsed(GN)

    if("x${GN_CONDITION}" STREQUAL "x")
        set(GN_CONDITION ON)
    endif()
    qt_evaluate_config_expression(result ${GN_CONDITION})
    if(${result})
        set(value "true")
    else()
        set(value "false")
    endif()
    message(DEBUG "extend_gn_list(${out_list} ${GN_ARGS} CONDITION ${GN_CONDITION} ...): Evaluated to ${value}")
    foreach(gnArg ${GN_ARGS})
        set(${out_list} "${${out_list}}" "${gnArg}=${value}")
    endforeach()
    set(${out_list} "${${out_list}}" PARENT_SCOPE)
endfunction()
