function(assertTargets errorResult supportResult)
    if(NOT DEFINED ${supportResult})
        set(${supportResult} ON PARENT_SCOPE)
        set(${supportResult} ON)
    endif()
    if(${${supportResult}})
       list(REMOVE_ITEM ARGN ${errorResult})
       list(REMOVE_ITEM ARGN ${supportResult})
       foreach(qtTarget IN ITEMS ${ARGN})
           if(NOT TARGET Qt::${qtTarget})
               set(${errorResult} "Missing required Qt::${qtTarget}." PARENT_SCOPE)
               set(${supportResult} OFF PARENT_SCOPE)
               return()
           endif()
       endforeach()
    endif()
endfunction()

# TODO: this should be idealy in qtbase
function(add_check_for_support errorResult supportResult)
    if(NOT DEFINED ${supportResult})
        set(${supportResult} ON PARENT_SCOPE)
        set(${supportResult} ON)
    endif()
    if (${${supportResult}})
        qt_parse_all_arguments(arg "add_check_for_support"
                "" "MODULE" "MESSAGE;CONDITION" "${ARGN}")
        if ("x${arg_CONDITION}" STREQUAL x)
            set(arg_CONDITION ON)
        endif()
        qt_evaluate_config_expression(result ${arg_CONDITION})
        if(NOT ${result})
            set(${supportResult} OFF PARENT_SCOPE)
            set(${errorResult} ${arg_MESSAGE} PARENT_SCOPE)
            qt_configure_add_report_entry(TYPE WARNING
                MESSAGE "${arg_MODULE} won't be built. ${arg_MESSAGE}"
                CONDITION ON
            )
        endif()
    endif()
endfunction()

function(get_qt_features outList module)
    get_cmake_property(variableList VARIABLES)
    set(_featureList "")
    foreach (variableKey ${variableList})
        unset(FOUND)
        string(REGEX MATCH QT_FEATURE_${module} FOUND ${variableKey})
        if (FOUND)
            list(APPEND _featureList "${variableKey}=${${variableKey}}")
        endif()
    endforeach()
    if ("${${outList}}" STREQUAL "")
        set(${outList} ${_featureList} PARENT_SCOPE)
    else()
        set(${outList} "${${outList}}" "${_featureList}" PARENT_SCOPE)
    endif()
endfunction()

function(get_configure_mode configureMode)
    if (NOT DEFINED WEBENGINE_REPO_BUILD)
        set(${configureMode} NO_CONFIG_HEADER_FILE NO_SYNC_QT PARENT_SCOPE)
    endif()
endfunction()

function(make_config_for_gn target configFileName)
    if(NOT DEFINED WEBENGINE_REPO_BUILD)
        file(GENERATE
            OUTPUT ${configFileName}.cxx.cmake
            CONTENT [[
                set(GN_INCLUDES_IN $<TARGET_PROPERTY:INCLUDE_DIRECTORIES>)
                set(GN_DEFINES_IN $<TARGET_PROPERTY:COMPILE_DEFINITIONS>)
                set(GN_LIBS_IN $<TARGET_PROPERTY:LINK_LIBRARIES>)
                set(GN_LINK_OPTIONS_IN $<TARGET_PROPERTY:LINK_OPTIONS>)
                set(GN_CXX_COMPILE_OPTIONS_IN $<TARGET_PROPERTY:COMPILE_OPTIONS>)]]
            CONDITION $<COMPILE_LANGUAGE:CXX>
            TARGET ${target})
        file(GENERATE
            OUTPUT ${configFileName}.c.cmake
            CONTENT [[ set(GN_C_COMPILE_OPTIONS_IN $<TARGET_PROPERTY:COMPILE_OPTIONS>)]]
            CONDITION $<COMPILE_LANGUAGE:C>
            TARGET ${target})
    endif()
endfunction()

function(make_install_only target)
    if(NOT DEFINED WEBENGINE_REPO_BUILD)
        set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endfunction()

macro(assertRunAsTopLevelBuild condition)
    if (NOT DEFINED WEBENGINE_REPO_BUILD AND ${condition})
        message(FATAL_ERROR "This cmake file should run as top level build.")
        return()
    endif()
endmacro()
