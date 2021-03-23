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
            OUTPUT $<CONFIG>/${configFileName}.cxx.cmake
            CONTENT [[
                set(GN_INCLUDES $<TARGET_PROPERTY:INCLUDE_DIRECTORIES>)
                set(GN_DEFINES $<TARGET_PROPERTY:COMPILE_DEFINITIONS>)
                set(GN_LIBS $<TARGET_PROPERTY:LINK_LIBRARIES>)
                set(GN_LINK_OPTIONS $<TARGET_PROPERTY:LINK_OPTIONS>)
                set(GN_CXX_COMPILE_OPTIONS $<TARGET_PROPERTY:COMPILE_OPTIONS>)]]
            CONDITION $<COMPILE_LANGUAGE:CXX>
            TARGET ${target})
        file(GENERATE
            OUTPUT $<CONFIG>/${configFileName}.c.cmake
            CONTENT [[ set(GN_C_COMPILE_OPTIONS $<TARGET_PROPERTY:COMPILE_OPTIONS>)]]
            CONDITION $<COMPILE_LANGUAGE:C>
            TARGET ${target})
    endif()
endfunction()

function(make_install_only target)
    if(NOT DEFINED WEBENGINE_REPO_BUILD)
        set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endfunction()

function(add_gn_target target)
    add_custom_target(${target})
    list(REMOVE_ITEM ARGN ${target})
    set_target_properties(${target} PROPERTIES
        ELEMENTS "${ARGN}"
        PREFIX "GN")
endfunction()

function(read_gn_target target filePath)
    include(${filePath})
    applyToGnTarget(${target})
endfunction()

macro(applyToGnTarget target)
    get_target_property(elementList ${target} ELEMENTS)
    get_target_property(prefix ${target} PREFIX)
    foreach(element IN LISTS elementList)
        if(${prefix}_${element})
             message(DEBUG "${prefix}_${element} = ${${prefix}_${element}}")
            set_property(TARGET ${target} APPEND PROPERTY ${prefix}_${element} ${${prefix}_${element}})
        endif()
    endforeach()
endmacro()

function(extend_gn_target target)
    get_target_property(elements ${target} ELEMENTS)
    qt_parse_all_arguments(GN "extend_gn_target" "" "" "CONDITION;${elements}" "${ARGN}")
    if ("x${GN_CONDITION}" STREQUAL x)
        set(GN_CONDITION ON)
    endif()
    qt_evaluate_config_expression(result ${GN_CONDITION})
    if(${result})
        message(DEBUG "extend_gn_target(${target} CONDITION ${GN_CONDITION} ...): Evaluated")
        applyToGnTarget(${target})
    endif()
endfunction()

function(extend_gn_list outList)
    qt_parse_all_arguments(GN "extend_gn_list" "" "" "ARGS;CONDITION" "${ARGN}")
    if ("x${GN_CONDITION}" STREQUAL x)
        set(GN_CONDITION ON)
    endif()
    qt_evaluate_config_expression(result ${GN_CONDITION})
    if(${result})
        set(value "true")
    else()
        set(value "false")
    endif()
    message(DEBUG "extend_gn_list(${outList} ${GN_ARGS} CONDITION ${GN_CONDITION} ...): Evaluated to ${value}")
    foreach(gnArg ${GN_ARGS})
        set(${outList} "${${outList}}" "${gnArg}=${value}")
    endforeach()
    set(${outList} "${${outList}}" PARENT_SCOPE)
endfunction()

function(configure_gn_target target configType inFilePath outFilePath)

    # GN_CONFIG
    string(TOUPPER ${configType} GN_CONFIG)

    # GN_SOURCES GN_HEADERS
    get_target_property(gnSources ${target} GN_SOURCES)
    foreach(gnSourceFile ${gnSources})
        get_filename_component(gnSourcePath ${gnSourceFile} REALPATH)
        list(APPEND sourceList \"${gnSourcePath}\")
    endforeach()
    set(GN_HEADERS ${sourceList})
    set(GN_SOURCES ${sourceList})
    list(FILTER GN_HEADERS INCLUDE REGEX "^.+\\.h\"$")
    list(FILTER GN_SOURCES EXCLUDE REGEX "^.+\\.h\"$")

    # GN_DEFINES
    get_target_property(gnDefines ${target} GN_DEFINES)
    list(REMOVE_DUPLICATES gnDefines)
    foreach(gnDefine ${gnDefines})
        list(APPEND GN_ARGS_DEFINES \"-D${gnDefine}\")
        list(APPEND GN_DEFINES \"${gnDefine}\")
    endforeach()

    # GN_INCLUDES
    get_target_property(gnIncludes ${target} GN_INCLUDES)
    list(REMOVE_DUPLICATES gnIncludes)
    foreach(gnInclude ${gnIncludes})
        get_filename_component(gnInclude ${gnInclude} REALPATH)
        list(APPEND GN_ARGS_INCLUDES \"-I${gnInclude}\")
        list(APPEND GN_INCLUDE_DIRS \"${gnInclude}\")
    endforeach()

    # MOC
    get_target_property(GN_MOC_BIN_IN Qt6::moc IMPORTED_LOCATION)
    set(GN_ARGS_MOC_BIN \"${GN_MOC_BIN_IN}\")

    # GN_CFLAGS_CC
    get_target_property(gnCxxCompileOptions ${target} GN_CXX_COMPILE_OPTIONS)
    foreach(gnCxxCompileOption ${gnCxxCompileOptions})
        list(APPEND GN_CFLAGS_CC \"${gnCxxCompileOption}\")
    endforeach()
    list(REMOVE_DUPLICATES GN_CFLAGS_CC)

    # GN_CFLAGS_C
    get_target_property(gnCCompileOptions ${target} GN_C_COMPILE_OPTIONS)
    foreach(gnCCompileOption ${gnCCompileOptions})
        list(APPEND GN_CFLAGS_C \"${gnCCompileOption}\")
    endforeach()
    list(REMOVE_DUPLICATES GN_CFLAGS_C)

    # GN_SOURCE_ROOT
    get_filename_component(GN_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}" REALPATH)

    if(MACOS)
       recoverFrameworkBuild(GN_INCLUDE_DIRS GN_CFLAGS_C)
    endif()

    foreach(item GN_HEADERS GN_SOURCES GN_ARGS_DEFINES GN_DEFINES GN_ARGS_INCLUDES
       GN_INCLUDE_DIRS GN_CFLAGS_CC GN_CFLAGS_C)
       string(REPLACE ";" ",\n  " ${item} "${${item}}")
    endforeach()
    configure_file(${inFilePath} ${outFilePath} @ONLY)
endfunction()

# we had no qtsync on headers during configure, so take current interface from expression
# generator from our WebEngieCore target so we can apply it for our buildGn target
function(resolve_target_includes resultVar target)
    get_target_property(includeDirs ${target} INCLUDE_DIRECTORIES)
    foreach(includeDir  ${includeDirs})
        if (includeDir MATCHES "\\$<BUILD_INTERFACE:([^,>]+)>")
           list(APPEND includeDirList ${CMAKE_MATCH_1})
        endif()
    endforeach()
    set(${resultVar} ${includeDirList} PARENT_SCOPE)
endfunction()

function(get_install_config result)
    if(DEFINED CMAKE_BUILD_TYPE)
        set(${result} ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
    elseif(DEFINED CMAKE_CONFIGURATION_TYPES)
        if("Release" IN_LIST CMAKE_CONFIGURATION_TYPES)
            set(${result} "Release" PARENT_SCOPE)
        elseif("RelWithDebInfo" IN_LIST CMAKE_CONFIGURATION_TYPES)
            set(${result} "RelWithDebInfo" PARENT_SCOPE)
        elseif("Debug" IN_LIST CMAKE_CONFIGURATION_TYPE)
            set(${result} "Debug" PARENT_SCOPE)
        else()
            # assume MinSizeRel ?
            set(${result} "${CMAKE_CONFIGURATION_TYPES}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

macro(assertRunAsTopLevelBuild condition)
    if (NOT DEFINED WEBENGINE_REPO_BUILD AND ${condition})
        message(FATAL_ERROR "This cmake file should run as top level build.")
        return()
    endif()
endmacro()

# we need to pass -F or -iframework in case of frameworks builds, which gn treats as
# compiler flag and cmake as include dir, so swap it.
function(recoverFrameworkBuild includeDirs compilerFlags)
    foreach(includeDir ${${includeDirs}})
        if (includeDir MATCHES "^\"(.*/([^/]+)\\.framework)\"$")
           list(APPEND frameworkDirs \"-iframework${CMAKE_MATCH_1}/..\")
        else()
           list(APPEND newIncludeDirs ${includeDir})
        endif()
    endforeach()
    set(${includeDirs} ${newIncludeDirs} PARENT_SCOPE)
    set(${compilerFlags} ${${compilerFlags}} ${frameworkDirs} PARENT_SCOPE)
endfunction()

# we need to fix namespace ambiguity issues between Qt and Chromium like
# forward declarations of NSString.
function(get_forward_declaration_macro result)
    if(MACOS)
    set(${result} "Q_FORWARD_DECLARE_OBJC_CLASS(name)=class name;" PARENT_SCOPE)
else()
    set(${result} "Q_FORWARD_DECLARE_OBJC_CLASS=QT_FORWARD_DECLARE_CLASS" PARENT_SCOPE)
endif()
endfunction()

function(get_darwin_sdk_version result)
    if(APPLE)
        if(IOS)
            set(sdk_name "iphoneos")
        elseif(TVOS)
            set(sdk_name "appletvos")
        elseif(WATCHOS)
            set(sdk_name "watchos")
        else()
            # Default to macOS
            set(sdk_name "macosx")
        endif()
        set(xcrun_version_arg "--show-sdk-version")
        execute_process(COMMAND /usr/bin/xcrun --sdk ${sdk_name} ${xcrun_version_arg}
                        OUTPUT_VARIABLE sdk_version
                        ERROR_VARIABLE xcrun_error)
        if(NOT sdk_version)
            message(FATAL_ERROR
                    "Can't determine darwin ${sdk_name} SDK version. Error: ${xcrun_error}")
        endif()
        string(STRIP "${sdk_version}" sdk_version)
        set(${result} "${sdk_version}" PARENT_SCOPE)
    endif()
endfunction()

