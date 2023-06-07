# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(qt6_add_webengine_dictionary)
    set(options)
    set(oneValueArgs TARGET SOURCE OUTPUT_DIRECTORY)
    set(multiValueArgs)

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT ARGS_SOURCE OR NOT EXISTS "${ARGS_SOURCE}" OR NOT IS_ABSOLUTE "${ARGS_SOURCE}")
        message(FATAL_ERROR "Function qt_add_webengine_dictionary requires an absolute path to SOURCE dictionary.")
    endif()

    if (NOT ARGS_OUTPUT_DIRECTORY)
        set(ARGS_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    get_target_property(isBundle ${ARGS_TARGET} MACOSX_BUNDLE)
    if(isMultiConfig)
        set(spellcheckerDir ${ARGS_OUTPUT_DIRECTORY}/dict/qtwebengine_dictionaries)
        set(copyCommand COMMAND ${CMAKE_COMMAND} -E copy_directory ${ARGS_OUTPUT_DIRECTORY}/dict
           ${ARGS_OUTPUT_DIRECTORY}/$<CONFIG>
        )
    elseif(isBundle)
        get_target_property(outputName ${ARGS_TARGET} OUTPUT_NAME)
        if(NOT outputName)
           set(outputName ${ARGS_TARGET})
        endif()
        set(spellcheckerDir "${ARGS_OUTPUT_DIRECTORY}/${outputName}.app/Contents/Resources/qtwebengine_dictionaries")
    else()
        set(spellcheckerDir ${ARGS_OUTPUT_DIRECTORY}/qtwebengine_dictionaries)
    endif()

    get_filename_component(dictName ${ARGS_SOURCE} NAME_WE)
    add_custom_command(
        OUTPUT ${spellcheckerDir}/${dictName}.bdic
        DEPENDS ${ARGS_SOURCE}
        COMMENT "Running qwebengine_convert_dict for ${ARGS_SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${spellcheckerDir}
        COMMAND ${CMAKE_COMMAND} -E env
        $<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qwebengine_convert_dict>
        ${ARGS_SOURCE} ${spellcheckerDir}/${dictName}.bdic
        ${copyCommand}
    )
    set(global_dict_target "qtwebengine_dictionaries")
    if(NOT TARGET ${global_dict_target})
        add_custom_target(${global_dict_target})
    endif()

    # in case of large project gen target should have unique name since it can collide, use TARGET
    if (ARGS_TARGET)
        add_custom_target(gen-${ARGS_TARGET}-${dictName} DEPENDS ${spellcheckerDir}/${dictName}.bdic)
        add_dependencies(${ARGS_TARGET} gen-${ARGS_TARGET}-${dictName})
        add_dependencies(${global_dict_target} gen-${ARGS_TARGET}-${dictName})
    else()
        add_custom_target(gen-${dictName} DEPENDS ${spellcheckerDir}/${dictName}.bdic)
        add_dependencies(${global_dict_target} gen-${dictName})
    endif()

endfunction()

if(NOT QT_NO_CREATE_VERSIONLESS_FUNCTIONS)
    function(qt_add_webengine_dictionary)
        qt6_add_webengine_dictionary(${ARGN})
    endfunction()
endif()
