function(assertTargets)
    qt_parse_all_arguments(arg "add_check_for_support"
        "" "" "MODULES;TARGETS" "${ARGN}"
    )
    foreach(module ${arg_MODULES})
        if(NOT DEFINED ${module}_SUPPORT)
            set(${module}_SUPPORT ON PARENT_SCOPE)
            set(${module}_SUPPORT ON)
        endif()
        if(${module}_SUPPORT)
            foreach(qtTarget ${arg_TARGETS})
                if(NOT TARGET Qt::${qtTarget})
                    set(${module}_ERROR "Missing required Qt::${qtTarget}." PARENT_SCOPE)
                    set(${module}_SUPPORT OFF PARENT_SCOPE)
                    break()
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

#TODO: remove me
function(add_implicit_dependencies target)
    if(TARGET ${target})
        list(REMOVE_ITEM ARGN ${target})
        foreach(qtTarget IN ITEMS ${ARGN})
            if(TARGET Qt::${qtTarget})
                add_dependencies(${target} Qt::${qtTarget})
            endif()
        endforeach()
    endif()
endfunction()

# TODO: this should be idealy in qtbase
function(add_check_for_support)
    qt_parse_all_arguments(arg "add_check_for_support"
        "" "" "MODULES;MESSAGE;CONDITION" "${ARGN}"
    )
    foreach(module ${arg_MODULES})
        if(NOT DEFINED ${module}_SUPPORT)
            set(${module}_SUPPORT ON PARENT_SCOPE)
            set(${module}_SUPPORT ON)
        endif()
        if(${module}_SUPPORT)
            if("x${arg_CONDITION}" STREQUAL x)
                set(arg_CONDITION ON)
            endif()
            qt_evaluate_config_expression(result ${arg_CONDITION})
            if(NOT ${result})
                set(${module}_SUPPORT OFF PARENT_SCOPE)
                set(${module}_ERROR ${arg_MESSAGE} PARENT_SCOPE)
            qt_configure_add_report_entry(TYPE WARNING
                MESSAGE "${module} won't be built. ${arg_MESSAGE}"
                CONDITION ON
            )
            endif()
        endif()
    endforeach()
endfunction()

function(get_qt_features outList module)
    get_cmake_property(variableList VARIABLES)
    set(_featureList "")
    foreach (variableKey ${variableList})
        unset(FOUND)
        string(REGEX MATCH QT_FEATURE_${module} FOUND ${variableKey})
        if(FOUND)
            list(APPEND _featureList "${variableKey}=${${variableKey}}")
        endif()
    endforeach()
    if("${${outList}}" STREQUAL "")
        set(${outList} ${_featureList} PARENT_SCOPE)
    else()
        set(${outList} "${${outList}}" "${_featureList}" PARENT_SCOPE)
    endif()
endfunction()

function(create_cxx_config cmakeTarget arch configFileName)
    if(NOT QT_SUPERBUILD AND QT_WILL_INSTALL)
        get_target_property(mocFilePath Qt6::moc IMPORTED_LOCATION)
    else()
        if(CMAKE_CROSSCOMPILING)
            set(mocFilePath "${QT_HOST_PATH}/${INSTALL_LIBEXECDIR}/moc${CMAKE_EXECUTABLE_SUFFIX}")
        else()
            set(mocFilePath "${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/moc${CMAKE_EXECUTABLE_SUFFIX}")
        endif()
    endif()
    file(GENERATE
          OUTPUT $<CONFIG>/${arch}/${configFileName}
          CONTENT "\
              set(GN_INCLUDES \"$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>\")\n\
              set(GN_DEFINES \"$<TARGET_PROPERTY:COMPILE_DEFINITIONS>\")\n\
              set(GN_LINK_OPTIONS \"$<TARGET_PROPERTY:LINK_OPTIONS>\")\n\
              set(GN_CXX_COMPILE_OPTIONS \"$<TARGET_PROPERTY:COMPILE_OPTIONS>\")\n\
              set(GN_MOC_PATH \"${mocFilePath}\")"
#             set(GN_LIBS $<TARGET_PROPERTY:LINK_LIBRARIES>)
          CONDITION $<COMPILE_LANGUAGE:CXX>
          TARGET ${cmakeTarget}
     )
endfunction()

function(create_c_config cmakeTarget arch configFileName)
    file(GENERATE
          OUTPUT $<CONFIG>/${arch}/${configFileName}
          CONTENT "set(GN_C_COMPILE_OPTIONS $<TARGET_PROPERTY:COMPILE_OPTIONS>)"
          CONDITION $<COMPILE_LANGUAGE:C>
          TARGET ${cmakeTarget})
endfunction()

function(create_gn_target_config target configFile)
    get_target_property(elementList ${target} ELEMENTS)
    get_target_property(prefix ${target} PREFIX)
    file(WRITE ${configFile}
        "set(PREFIX ${prefix})\nset(ELEMENTS ${elementList})\n"
    )
    foreach(element IN LISTS elementList)
         get_target_property(prop ${target} ${prefix}_${element})
         if(prop)
             file(APPEND ${configFile} "set(${prefix}_${element} ${prop})\n")
         endif()
    endforeach()
endfunction()

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

function(init_gn_config filePath)
    include(${filePath})
    set_directory_properties(PROPERTIES
        ELEMENTS "${ELEMENTS}"
        PREFIX "${PREFIX}"
    )
    applyToGnTarget(DIRECTORY)
endfunction()

function(read_gn_config filePath)
    include(${filePath})
    applyToGnTarget(DIRECTORY)
endfunction()

# this runs also in script mode, so we use than DIRECTORY
macro(applyToGnTarget)
    set(type ${ARGV0})
    set(target ${ARGV1})
    get_property(elementList ${type} ${target} PROPERTY ELEMENTS)
    get_property(prefix ${type} ${target} PROPERTY PREFIX)
    foreach(element IN LISTS elementList)
        if(${prefix}_${element})
            message(DEBUG "${prefix}_${element} = ${${prefix}_${element}}")
            set_property(${type} ${target} APPEND PROPERTY ${prefix}_${element} ${${prefix}_${element}})
        endif()
    endforeach()
endmacro()

function(extend_gn_target target)
    get_target_property(elements ${target} ELEMENTS)
    qt_parse_all_arguments(GN "extend_gn_target" "" "" "CONDITION;${elements}" "${ARGN}")
    if("x${GN_CONDITION}" STREQUAL x)
        set(GN_CONDITION ON)
    endif()
    qt_evaluate_config_expression(result ${GN_CONDITION})
    if(${result})
        message(DEBUG "extend_gn_target(${target} CONDITION ${GN_CONDITION} ...): Evaluated")
        applyToGnTarget(TARGET ${target})
    endif()
endfunction()

function(extend_gn_list outList)
    qt_parse_all_arguments(GN "extend_gn_list" "" "" "ARGS;CONDITION" "${ARGN}")
    if("x${GN_CONDITION}" STREQUAL x)
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

function(configure_gn_target sourceDir inFilePath outFilePath)
    # FIXME: GN_CONFIG
    set(GN_CONFIG NOTUSED)

    # GN_SOURCES GN_HEADERS
    get_property(gnSources DIRECTORY PROPERTY GN_SOURCES)
    foreach(gnSourceFile ${gnSources})
        get_filename_component(gnSourcePath ${sourceDir}/${gnSourceFile} REALPATH)
        list(APPEND sourceList \"${gnSourcePath}\")
    endforeach()
    set(GN_HEADERS ${sourceList})
    set(GN_SOURCES ${sourceList})
    list(FILTER GN_HEADERS INCLUDE REGEX "^.+\\.h\"$")
    list(FILTER GN_SOURCES EXCLUDE REGEX "^.+\\.h\"$")

    # GN_DEFINES
    get_property(gnDefines DIRECTORY PROPERTY GN_DEFINES)
    list(REMOVE_DUPLICATES gnDefines)
    foreach(gnDefine ${gnDefines})
        list(APPEND GN_ARGS_DEFINES \"-D${gnDefine}\")
        list(APPEND GN_DEFINES \"${gnDefine}\")
    endforeach()

    # GN_INCLUDES
    get_property(gnIncludes DIRECTORY PROPERTY GN_INCLUDES)
    list(REMOVE_DUPLICATES gnIncludes)
    foreach(gnInclude ${gnIncludes})
        get_filename_component(gnInclude ${gnInclude} REALPATH)
        list(APPEND GN_ARGS_INCLUDES \"-I${gnInclude}\")
        list(APPEND GN_INCLUDE_DIRS \"${gnInclude}\")
    endforeach()

    # MOC
    get_property(mocFilePath DIRECTORY PROPERTY GN_MOC_PATH)
    set(GN_ARGS_MOC_BIN \"${mocFilePath}\")

    # GN_CFLAGS_CC
    get_property(gnCxxCompileOptions DIRECTORY PROPERTY GN_CXX_COMPILE_OPTIONS)
    foreach(gnCxxCompileOption ${gnCxxCompileOptions})
        list(APPEND GN_CFLAGS_CC \"${gnCxxCompileOption}\")
    endforeach()
    list(REMOVE_DUPLICATES GN_CFLAGS_CC)

    # GN_CFLAGS_C
    get_property(gnCCompileOptions DIRECTORY PROPERTY GN_C_COMPILE_OPTIONS)
    foreach(gnCCompileOption ${gnCCompileOptions})
        list(APPEND GN_CFLAGS_C \"${gnCCompileOption}\")
    endforeach()
    list(REMOVE_DUPLICATES GN_CFLAGS_C)

    # GN_SOURCE_ROOT
    get_filename_component(GN_SOURCE_ROOT "${sourceDir}" REALPATH)

    # GN_RSP_PREFIX
    get_property(GN_RSP_PREFIX DIRECTORY PROPERTY GN_RSP_PREFIX)

    if(APPLE) # this runs in scrpit mode without qt-cmake so on MACOS here
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
        if(includeDir MATCHES "\\$<BUILD_INTERFACE:([^,>]+)>")
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

macro(assertRunAsTopLevelBuild)
    if(NOT DEFINED WEBENGINE_REPO_BUILD)
        message(FATAL_ERROR "This cmake file should run as top level build.")
        return()
    endif()
endmacro()

# we need to pass -F or -iframework in case of frameworks builds, which gn treats as
# compiler flag and cmake as include dir, so swap it.
function(recoverFrameworkBuild includeDirs compilerFlags)
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

# we need to fix namespace ambiguity issues between Qt and Chromium like
# forward declarations of NSString.
function(get_forward_declaration_macro result)
    if(MACOS)
        set(${result} " \"Q_FORWARD_DECLARE_OBJC_CLASS(name)=class name;\" " PARENT_SCOPE)
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

function(add_ninja_target target cmakeTarget ninjaTarget config arch buildDir)
    string(TOUPPER ${config} cfg)
    add_custom_target(${target} DEPENDS ${buildDir}/${config}/${arch}/${ninjaTarget}.stamp)
    set_target_properties(${target} PROPERTIES
        CONFIG ${config}
        ARCH ${arch}
        CMAKE_TARGET ${cmakeTarget}
        NINJA_TARGET ${ninjaTarget}
    )
endfunction()

function(copy_response_files target)
    get_target_property(config ${target} CONFIG)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    list(REMOVE_ITEM ARGN ${target})
    foreach(rsp IN ITEMS ${ARGN})
        set(rsp_dst "CMakeFiles_${ninjaTarget}_${config}_${rsp}.rsp")
        set(rsp_src "${${rsp}_rsp}")
        if(NOT QT_SUPERBUILD)
            set(rsp_output ${PROJECT_BINARY_DIR}/${rsp_dst})
        else()
            set(rsp_output ${PROJECT_BINARY_DIR}/../${rsp_dst})
        endif()
        add_custom_command(
            OUTPUT ${rsp_output}
            COMMAND ${CMAKE_COMMAND} -E copy ${rsp_src} ${rsp_output}
            DEPENDS ${rsp_src}
            USES_TERMINAL
        )
        set(${rsp}_rsp ${rsp_dst} PARENT_SCOPE)
        add_custom_target(${cmakeTarget}_${rsp}_copy_${config}
            DEPENDS ${rsp_output}
        )
        add_dependencies(${cmakeTarget} ${cmakeTarget}_${rsp}_copy_${config})
    endforeach()
endfunction()

function(extend_cmake_target target buildDir)
    get_target_property(config ${target} CONFIG)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    string(TOUPPER ${cmakeTarget} tg)
    string(TOUPPER ${config} cfg)
    set(objects_rsp "${buildDir}/${ninjaTarget}_objects.rsp")
    set(archives_rsp "${buildDir}/${ninjaTarget}_archives.rsp")
    set(libs_rsp "${buildDir}/${ninjaTarget}_libs.rsp")
    if(LINUX)
         target_link_options(${cmakeTarget} PRIVATE
             "$<$<CONFIG:${config}>:@${objects_rsp}>"
         )
         target_link_libraries(${cmakeTarget} PRIVATE
             "-Wl,--start-group $<$<CONFIG:${config}>:@${archives_rsp}> -Wl,--end-group"
         )
         # linker here options are just to prevent processing it by cmake
         target_link_libraries(${cmakeTarget} PRIVATE
             "-Wl,--no-fatal-warnings $<$<CONFIG:${config}>:@${libs_rsp}> -Wl,--no-fatal-warnings"
         )

    endif()
    if(MACOS)
        target_link_options(${cmakeTarget} PRIVATE
            "$<$<CONFIG:${config}>:@${objects_rsp}>"
            "$<$<CONFIG:${config}>:@${archives_rsp}>"
            "$<$<CONFIG:${config}>:@${libs_rsp}>"
        )
    endif()
    if(WIN32)
        copy_response_files(${target} objects archives libs)
        target_link_options(${cmakeTarget} PRIVATE
            "$<$<CONFIG:${config}>:@${objects_rsp}>"
            "$<$<CONFIG:${config}>:@${archives_rsp}>"
            "$<$<CONFIG:${config}>:@${libs_rsp}>"
        )
        # we need libs rsp also when linking process with sandbox lib
        set_property(TARGET ${cmakeTarget} PROPERTY LIBS_RSP ${libs_rsp})
    endif()
endfunction()

function(add_rsp_command target buildDir)
    get_target_property(config ${target} CONFIG)
    get_target_property(arch ${target} ARCH)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    string(TOUPPER ${config} cfg)
    add_custom_command(
        OUTPUT ${buildDir}/${cmakeTarget}.a
        BYPRODUCTS
            ${buildDir}/${cmakeTarget}.o
        COMMAND clang++ -r -nostdlib -arch ${arch}
            -o ${buildDir}/${cmakeTarget}.o
            -Wl,-keep_private_externs
            @${buildDir}/${ninjaTarget}_objects.rsp
            -Wl,-all_load
            @${buildDir}/${ninjaTarget}_archives.rsp
        COMMAND ar -cr
            ${buildDir}/${cmakeTarget}.a
            ${buildDir}/${cmakeTarget}.o
        DEPENDS
            ${buildDir}/${ninjaTarget}.stamp
        WORKING_DIRECTORY "${buildDir}/../../.."
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
endfunction()

function(add_lipo_command target buildDir)
    get_target_property(config ${target} CONFIG)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    set(libs_rsp "${buildDir}/x86_64/${ninjaTarget}_libs.rsp")
    # Lipo the object files together to a single fat archive
    add_library(${cmakeTarget}_${config} STATIC IMPORTED GLOBAL)
    add_custom_command(
        OUTPUT ${buildDir}/lib${cmakeTarget}.a
        COMMAND lipo -create
            -output ${buildDir}/lib${cmakeTarget}.a
        ARGS
            ${buildDir}/arm64/${cmakeTarget}.a
            ${buildDir}/x86_64/${cmakeTarget}.a
        DEPENDS
            ${buildDir}/arm64/${cmakeTarget}.a
            ${buildDir}/x86_64/${cmakeTarget}.a
        USES_TERMINAL
        VERBATIM
    )
    set_property(TARGET ${cmakeTarget}_${config}
        PROPERTY IMPORTED_LOCATION ${buildDir}/lib${cmakeTarget}.a
    )
    add_custom_target(lipo_${cmakeTarget}_${config} DEPENDS
        ${buildDir}/lib${cmakeTarget}.a
    )
    add_dependencies(${cmakeTarget}_${config} lipo_${cmakeTarget}_${config})
    target_link_libraries(${cmakeTarget} PRIVATE ${cmakeTarget}_${config})

    # Just link with dynamic libs once
    # TODO: this is evil hack, since cmake has no idea about libs
    target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${libs_rsp}>")
endfunction()

function(qt_internal_add_external_project_dependency_to_root_project name)
    set(independent_args)
    cmake_policy(PUSH)
    if(POLICY CMP0114)
        set(independent_args INDEPENDENT TRUE)
        cmake_policy(SET CMP0114 NEW)
    endif()

    # Force configure step to re-run after we configure the root project
    set(reconfigure_check_file ${CMAKE_CURRENT_BINARY_DIR}/reconfigure_${name}.stamp)
    file(TOUCH ${reconfigure_check_file})
    ExternalProject_Add_Step(${name} reconfigure-check
        DEPENDERS configure
        DEPENDS   ${reconfigure_check_file}
        ${independent_args}
    )

    cmake_policy(POP)
endfunction()

function(get_gn_arch result arch)
    if(arch STREQUAL "i386")
        set(${result} "x86" PARENT_SCOPE)
    elseif(arch STREQUAL "x86_64")
        set(${result} "x64" PARENT_SCOPE)
    elseif(arch STREQUAL "arm")
        set(${result} "arm" PARENT_SCOPE)
    elseif(arch STREQUAL "arm64")
        set(${result} "arm64" PARENT_SCOPE)
    elseif(arch STREQUAL "mipsel")
        set(${result} "mipsel" PARENT_SCOPE)
    elseif(arch STREQUAL "mipsel64")
        set(${result} "mips64el" PARENT_SCOPE)
    else()
        message(DEBUG "Unsupported architecture: ${arch}")
    endif()
endfunction()

function(get_v8_arch result targetArch hostArch)
    set(list32 i386 arm mipsel)
    if(hostArch STREQUAL targetArch)
        set(${result} "${targetArch}" PARENT_SCOPE)
    elseif(targetArch IN_LIST list32)
        # 32bit target which needs a 32bit compatible host
        if(hostArch STREQUAL "x86_64")
            set(${result} "i386" PARENT_SCOPE)
        elseif(hostArch STREQUAL "arm64")
            set(${result} "arm" PARENT_SCOPE)
        elseif(hostArch STREQUAL "mips64")
            set(${result} "mipsel" PARENT_SCOPE)
        elseif(hostArch STREQUAL "mipsel64")
            set(${result} "mipsel" PARENT_SCOPE)
        else()
            message(DEBUG "Unsupported architecture: ${hostArch}")
        endif()
    else()
        # assume 64bit target which matches 64bit host
        set(${result} "${hostArch}" PARENT_SCOPE)
    endif()
endfunction()

function(get_gn_os result)
    if(WIN32)
        set(${result} "win" PARENT_SCOPE)
    elseif(LINUX)
        set(${result} "linux" PARENT_SCOPE)
    elseif(MACOS)
        set(${result} "mac" PARENT_SCOPE)
    elseif(IOS)
        set(${result} "ios" PARENT_SCOPE)
    else()
        message(DEBUG "Unrecognized OS")
    endif()
endfunction()

function(get_gn_is_clang result)
    if(CLANG)
        set(${result} "true" PARENT_SCOPE)
    else()
        set(${result} "false" PARENT_SCOPE)
    endif()
endfunction()

function(configure_gn_toolchain name binTargetCpu v8TargetCpu toolchainIn toolchainOut)
    set(GN_TOOLCHAIN ${name})
    get_gn_os(GN_OS)
    get_gn_is_clang(GN_IS_CLANG)
    get_gn_arch(GN_CPU ${binTargetCpu})
    get_gn_arch(GN_V8_CPU ${v8TargetCpu})
    configure_file(${toolchainIn} ${toolchainOut}/BUILD.gn @ONLY)
endfunction()

function(create_pkg_config_wrapper wrapperName wrapperCmd)
    file(WRITE ${wrapperName}
        "#!/bin/sh\n"
        "unset PKG_CONFIG_LIBDIR\n"
        "unset PKG_CONFIG_PATH\n"
        "unset PKG_CONFIG_SYSROOT_DIR\n"
        "exec ${wrapperCmd} \"$@\""
    )
    file(CHMOD ${wrapperName} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
endfunction()

function(extract_cflag result cflag)
    set(i 1)
    while(NOT "x${CMAKE_CXX_COMPILER_ARG${i}}" STREQUAL "x")
        list(APPEND cflags ${CMAKE_CXX_COMPILER_ARG${i}})
        math(EXPR i "${i} + 1")
    endwhile()
    list(APPEND cflags ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
    string(REPLACE ";" " " cflags "${cflags}")
    message(DEBUG "Found cflags: ${cflags}")
    if(cflags MATCHES "-${cflag}=([^ ]+)")
        set(${result} ${CMAKE_MATCH_1} PARENT_SCOPE)
        return()
    endif()
    if(cflags MATCHES "-${cflag}")
       set(${result} ON PARENT_SCOPE)
    else()
       set(${result} OFF PARENT_SCOPE)
    endif()
endfunction()

function(extend_gn_list_cflag outList)
    qt_parse_all_arguments(GN "extend_gn_list_cflag" "" "" "ARG;CFLAG" "${ARGN}")
    extract_cflag(cflag "${GN_CFLAG}")
    if(cflag)
        set(${outList} "${${outList}}" "${GN_ARG}=\"${cflag}\"" PARENT_SCOPE)
    endif()
endfunction()

function(get_arm_version result cflag)
    if(cflag MATCHES "^armv([0-9])")
        set(${result} ${CMAKE_MATCH_1} PARENT_SCOPE)
    endif()
endfunction()

function(check_thumb result)
    extract_cflag(thumb "mthumb")
    if(thumb)
        set(${result} TRUE PARENT_SCOPE)
        return()
    else()
        extract_cflag(marm "marm")
        if(marm)
            set(${result} FALSE PARENT_SCOPE)
            return()
        else()
            extract_cflag(march "march")
            get_arm_version(arm_version ${march})
            if(arm_version GREATER_EQUAL 7)
                set(${result} TRUE PARENT_SCOPE)
                return()
            endif()
        endif()
    endif()
    set(${result} FALSE PARENT_SCOPE)
endfunction()

macro(create_pkg_config_host_wrapper buildDir)
    find_package(PkgConfigHost)
        if(CMAKE_CROSSCOMPILING)
            create_pkg_config_wrapper("${buildDir}/pkg-config-host_wrapper.sh" "${PKG_CONFIG_HOST_EXECUTABLE}")
            set(PKG_CONFIG_HOST_EXECUTABLE "${buildDir}/pkg-config-host_wrapper.sh")
        endif()
endmacro()

macro(setup_toolchains)
    if(NOT CMAKE_CROSSCOMPILING) # delivered by hostBuild project
        configure_gn_toolchain(host ${TEST_architecture_arch} ${TEST_architecture_arch}
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
            ${buildDir}/host_toolchain)
        configure_gn_toolchain(v8 ${TEST_architecture_arch} ${TEST_architecture_arch}
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
            ${buildDir}/v8_toolchain)
    endif()
    configure_gn_toolchain(target ${TEST_architecture_arch} ${TEST_architecture_arch}
        ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
        ${buildDir}/target_toolchain)
endmacro()

macro(append_build_type_setup)
    list(APPEND gnArgArg
        use_qt=true
        init_stack_vars=false
        is_component_build=false
        is_shared=true
        use_sysroot=false
        forbid_non_component_debug_builds=false
        enable_debugallocation=false
        treat_warnings_as_errors=false
        use_allocator_shim=false
        use_allocator="none"
        use_custom_libcxx=false
    )
    if(${config} STREQUAL "Debug")
        list(APPEND gnArgArg is_debug=true symbol_level=2)
        if(WIN32)
            list(APPEND gnArgArg enable_iterator_debugging=true v8_optimized_debug=false)
        endif()
    elseif(${config} STREQUAL "Release")
        list(APPEND gnArgArg is_debug=false symbol_level=0)
    elseif(${config} STREQUAL "RelWithDebInfo")
        list(APPEND gnArgArg is_debug=false)
        if(WIN32 AND NOT CLANG)
            list(APPEND gnArgArg symbol_level=2)
        else()
            list(APPEND gnArgArg symbol_level=1)
        endif()
    elseif(${config} STREQUAL "MinSizeRel")
        list(APPEND gnArgArg is_debug=false symbol_level=0 optimize_for_size=true)
    endif()
    if(FEATURE_developer_build OR (${config} STREQUAL "Debug") OR QT_FEATURE_webengine_sanitizer)
        list(APPEND gnArgArg
             is_official_build=false
             from_here_uses_location_builtins=false
        )
    else()
        list(APPEND gnArgArg is_official_build=true)
        if(NOT CLANG OR NOT QT_FEATURE_use_lld_linker)
            list(APPEND gnArgArg
                use_thin_lto=false
            )
        endif()
    endif()
    extend_gn_list(gnArgArg
        ARGS is_unsafe_developer_build
        CONDITION FEATURE_developer_build
    )

    if(NOT QT_FEATURE_webengine_full_debug_info)
        list(APPEND gnArgArg blink_symbol_level=0 remove_v8base_debug_symbols=true)
    endif()

    extend_gn_list(gnArgArg ARGS use_jumbo_build CONDITION QT_FEATURE_webengine_jumbo_build)
    if(QT_FEATURE_webengine_jumbo_build)
        list(APPEND gnArgArg jumbo_file_merge_limit=${QT_FEATURE_webengine_jumbo_file_merge_limit})
        if(QT_FEATURE_webengine_jumbo_file_merge_limit LESS_EQUAL 8)
            list(APPEND gnArgArg jumbo_build_excluded=[\"browser\"])
        endif()
    endif()

    extend_gn_list(gnArgArg
        ARGS enable_precompiled_headers
        CONDITION BUILD_WITH_PCH
    )
    extend_gn_list(gnArgArg
        ARGS dcheck_always_on
        CONDITION QT_FEATURE_force_asserts
    )
endmacro()

macro(append_compiler_linker_sdk_setup)
    if(CMAKE_CXX_COMPILER_LAUNCHER)
        list(APPEND gnArgArg cc_wrapper="${CMAKE_CXX_COMPILER_LAUNCHER}")
    endif()

    extend_gn_list(gnArgArg ARGS is_clang CONDITION CLANG)
    if(CLANG)
        if(MACOS)
            get_darwin_sdk_version(macSdkVersion)
            # macOS needs to use the objcxx compiler as the cxx compiler is just a link
            get_filename_component(clangBasePath ${CMAKE_OBJCXX_COMPILER} DIRECTORY)
            get_filename_component(clangBasePath ${clangBasePath} DIRECTORY)
        else()
            get_filename_component(clangBasePath ${CMAKE_CXX_COMPILER} DIRECTORY)
            get_filename_component(clangBasePath ${clangBasePath} DIRECTORY)
        endif()

        list(APPEND gnArgArg
            clang_base_path="${clangBasePath}"
            clang_use_chrome_plugins=false
            fatal_linker_warnings=false
        )

        if(MACOS)
            list(APPEND gnArgArg
                use_system_xcode=true
                mac_deployment_target="${CMAKE_OSX_DEPLOYMENT_TARGET}"
                mac_sdk_min="${macSdkVersion}"
            )
        endif()
    else()
        if(QT_FEATURE_use_lld_linker)
            get_filename_component(clangBasePath ${CMAKE_LINKER} DIRECTORY)
            get_filename_component(clangBasePath ${clangBasePath} DIRECTORY)
            list(APPEND gnArgArg
                clang_base_path="${clangBasePath}"
                fatal_linker_warnings=false
            )
        endif()
    endif()

    if(WIN32)
        get_filename_component(windowsSdkPath $ENV{WINDOWSSDKDIR} DIRECTORY)
        get_filename_component(visualStudioPath $ENV{VSINSTALLDIR} DIRECTORY)
        list(APPEND gnArgArg
            win_linker_timing=true
            use_incremental_linking=false
            visual_studio_version=2019
            visual_studio_path=\"${visualStudioPath}\"
            windows_sdk_path=\"${windowsSdkPath}\"
        )
    endif()
    get_gn_arch(cpu ${TEST_architecture_arch})
    if(LINUX AND CMAKE_CROSSCOMPILING AND cpu STREQUAL "arm")

        extend_gn_list_cflag(gnArgArg
            ARG arm_tune
            CFLAG mtune
        )
        extend_gn_list_cflag(gnArgArg
            ARG arm_float_abi
            CFLAG mfloat-abi
        )
        extend_gn_list_cflag(gnArgArg
            ARG arm_arch
            CFLAG march
        )
        extend_gn_list_cflag(gnArgArg
            ARG arm_cpu
            CFLAG mcpu
        )
        extract_cflag(cflag "mfpu")
        get_arm_version(arm_version "${cflag}")
        extend_gn_list(gnArgArg
            ARGS arm_use_neon
            CONDITION (arm_version GREATER_EQUAL 8) OR ("${cflag}" MATCHES ".*neon.*")
        )
        if(arm_version EQUAL 7 AND NOT "${cflag}" MATCHES ".*neon.*")
            # If the toolchain does not explicitly specify to use NEON instructions
            # we use arm_neon_optional for ARMv7
            list(APPEND gnArgArg arm_optionally_use_neon=true)
        endif()
        check_thumb(armThumb)
        extend_gn_list(gnArgArg
            ARGS arm_use_thumb
            CONDITION armThumb
        )
    endif()
    extend_gn_list(gnArgArg
        ARGS use_gold
        CONDITION QT_FEATURE_use_gold_linker
    )
    extend_gn_list(gnArgArg
        ARGS use_lld
        CONDITION QT_FEATURE_use_lld_linker
    )
endmacro()

macro(append_sanitizer_setup)
    if(QT_FEATURE_webengine_sanitizer)
        extend_gn_list(gnArgArg
            ARGS is_asan
            CONDITION address IN_LIST ECM_ENABLE_SANITIZERS
        )
        extend_gn_list(gnArgArg
            ARGS is_tsan
            CONDITION thread IN_LIST ECM_ENABLE_SANITIZERS
        )
        extend_gn_list(gnArgArg
            ARGS is_msan
            CONDITION memory IN_LIST ECM_ENABLE_SANITIZERS
        )
        extend_gn_list(gnArgArg
            ARGS is_ubsan is_ubsan_vptr
            CONDITION undefined IN_LIST ECM_ENABLE_SANITIZERS
        )
    endif()
endmacro()

macro(append_toolchain_setup)
    if(LINUX)
        list(APPEND gnArgArg
            custom_toolchain="${buildDir}/target_toolchain:target"
            host_toolchain="${buildDir}/host_toolchain:host"
            v8_snapshot_toolchain="${buildDir}/v8_toolchain:v8"
        )
        get_gn_arch(cpu ${TEST_architecture_arch})
        if(CMAKE_CROSSCOMPILING)
            list(APPEND gnArgArg target_cpu="${cpu}")
        else()
            list(APPEND gnArgArg host_cpu="${cpu}")
        endif()
        if(CMAKE_SYSROOT)
            list(APPEND gnArgArg target_sysroot="${CMAKE_SYSROOT}")
        endif()
    else()
        get_gn_arch(cpu ${arch})
        list(APPEND gnArgArg target_cpu="${cpu}")
    endif()
endmacro()


macro(append_pkg_config_setup)
    if(LINUX)
        list(APPEND gnArgArg
            pkg_config="${PKG_CONFIG_EXECUTABLE}"
            host_pkg_config="${PKG_CONFIG_HOST_EXECUTABLE}"
        )
    endif()
endmacro()

function(add_ninja_command)
    qt_parse_all_arguments(arg "add_ninja_command"
        "" "TARGET;OUTPUT;BUILDDIR;MODULE" "BYPRODUCTS" "${ARGN}"
    )
    string(REPLACE " " ";" NINJAFLAGS "$ENV{NINJAFLAGS}")
    list(TRANSFORM arg_BYPRODUCTS PREPEND "${arg_BUILDDIR}/")
    add_custom_command(
        OUTPUT
            ${arg_BUILDDIR}/${arg_OUTPUT}
            ${arg_BUILDDIR}/${arg_TARGET} # use generator expression in CMAKE 3.20
        BYPRODUCTS ${arg_BYPRODUCTS}
        COMMENT "Running ninja for ${arg_TARGET} in ${arg_BUILDDIR}"
        COMMAND Ninja::ninja
            ${NINJAFLAGS}
            -C ${arg_BUILDDIR}
            ${arg_TARGET}
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
        DEPENDS run_${arg_MODULE}_NinjaReady
    )
endfunction()

function(get_configs result)
    if(QT_GENERATOR_IS_MULTI_CONFIG)
        set(${result} ${CMAKE_CONFIGURATION_TYPES} PARENT_SCOPE)
    else()
        set(${result} ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
    endif()
endfunction()

function(get_architectures result)
    if(NOT QT_IS_MACOS_UNIVERSAL)
        set(${result} ${CMAKE_SYSTEM_PROCESSOR} PARENT_SCOPE)
    else()
        set(${result} ${CMAKE_OSX_ARCHITECTURES} PARENT_SCOPE)
    endif()
endfunction()

function(add_gn_build_aritfacts_to_target cmakeTarget ninjaTarget module buildDir)
    # config loop is a workaround to be able to add_custom_command per config
    # note this is fixed in CMAKE.3.20 and should be cleaned up when 3.20 is
    # the minimum cmake we support
    get_configs(configs)
    get_architectures(archs)
    if(NOT configs)
        message(FATAL_ERROR "No valid configurations found !")
    endif()
    if(NOT archs)
        message(FATAL_ERROR "No valid architectures found. In case of cross-compiling make sure you have CMAKE_SYSTEM_PROCESSOR in your toolchain file.")
    endif()
    foreach(config ${configs})
        foreach(arch ${archs})
            set(target ${ninjaTarget}_${config}_${arch})
            add_ninja_target(${target} ${cmakeTarget} ${ninjaTarget} ${config} ${arch} ${buildDir})
            add_ninja_command(
                TARGET ${ninjaTarget}
                OUTPUT ${ninjaTarget}.stamp
                BUILDDIR ${buildDir}/${config}/${arch}
                MODULE ${module}
            )
            add_dependencies(run_${module}_NinjaDone ${target})
            set_target_properties(${cmakeTarget} PROPERTIES
                LINK_DEPENDS ${buildDir}/${config}/${arch}/${ninjaTarget}.stamp
            )
            if(QT_IS_MACOS_UNIVERSAL)
                add_rsp_command(${target} ${buildDir}/${config}/${arch})
            else()
                extend_cmake_target(${target} ${buildDir}/${config}/${arch})
            endif()
        endforeach()
        if(QT_IS_MACOS_UNIVERSAL)
            set(arch ${CMAKE_SYSTEM_PROCESSOR})
            set(target ${ninjaTarget}_${config}_${arch})
            add_lipo_command(${target} ${buildDir}/${config})
        endif()
    endforeach()
endfunction()

function(get_config_filenames c_config cxx_config target_config)
    set(${target_config} gn_config_target.cmake PARENT_SCOPE)
    set(${cxx_config} gn_config_cxx.cmake PARENT_SCOPE)
    set(${c_config} gn_config_c.cmake PARENT_SCOPE)
endfunction()

function(add_gn_command)
    qt_parse_all_arguments(arg "add_gn_command"
        "" "CMAKE_TARGET;GN_TARGET;MODULE;BUILDDIR" "NINJA_TARGETS;GN_ARGS" "${ARGN}"
    )

    get_config_filenames(cConfigFileName cxxConfigFileName targetConfigFileName)
    set(gnArgArgFile ${arg_BUILDDIR}/args.gn)

    list(JOIN arg_GN_ARGS "\n" arg_GN_ARGS)
    file(WRITE ${gnArgArgFile} ${arg_GN_ARGS})

    foreach(ninjaTarget ${arg_NINJA_TARGETS})
        list(APPEND output ${ninjaTarget}_objects.rsp ${ninjaTarget}_archives.rsp ${ninjaTarget}_libs.rsp)
    endforeach()
    list(TRANSFORM output PREPEND "${arg_BUILDDIR}/")

    if(QT_HOST_PATH)
      set(QT_HOST_GN_PATH ${QT_HOST_PATH}/${INSTALL_LIBEXECDIR})
    endif()

    add_custom_command(
        OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND}
             -DBUILD_DIR=${arg_BUILDDIR}
             -DSOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
             -DMODULE=${arg_MODULE}
             -DQT_HOST_GN_PATH=${QT_HOST_GN_PATH}
             -P ${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Gn.cmake
        WORKING_DIRECTORY ${WEBENGINE_ROOT_BUILD_DIR}
        COMMENT "Run gn for target ${arg_CMAKE_TARGET} in ${arg_BUILDDIR}"
        DEPENDS ${gnArgArgFile} run_${arg_MODULE}_GnReady
        "${WEBENGINE_ROOT_SOURCE_DIR}/src/${arg_MODULE}/configure/BUILD.root.gn.in"
        USES_TERMINAL
    )
    add_custom_target(runGn_${arg_GN_TARGET}
        DEPENDS #TODO this is fixed in cmake 3.20 so we could simply use GN_TARGET and not create new one
            ${output}
            ${arg_BUILDDIR}/${cxxConfigFileName}
            ${arg_BUILDDIR}/${cConfigFileName}
            ${arg_BUILDDIR}/${targetConfigFileName}
    )
    add_dependencies(run_${arg_MODULE}_GnDone runGn_${arg_GN_TARGET})
    create_gn_target_config(${arg_GN_TARGET} ${arg_BUILDDIR}/${targetConfigFileName})
endfunction()

function(create_cxx_configs cmakeTarget arch)
    get_config_filenames(cConfigFileName cxxConfigFileName targetConfigFileName)
    create_c_config(${cmakeTarget} ${arch} ${cConfigFileName})
    create_cxx_config(${cmakeTarget} ${arch} ${cxxConfigFileName})
endfunction()

# targets to gather per config / architecture targets
function(addSyncTargets module)
    add_custom_target(run_${module}_GnReady)
    add_custom_target(run_${module}_GnDone)
    add_custom_target(run_${module}_NinjaReady)
    add_custom_target(run_${module}_NinjaDone)
    # make nicer log so all gn has to finish before any ninja build starts
    add_dependencies(run_${module}_NinjaReady run_${module}_GnDone)
endfunction()

function(addCopyCommand target src dst)
    add_custom_command(
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${dst}
        COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
        TARGET ${target}
        DEPENDS ${src}
        USES_TERMINAL
    )
endfunction()

function(addCopyDirCommand target src dst)
    add_custom_command(
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${src} ${dst}
        TARGET ${target}
        DEPENDS ${src}
        USES_TERMINAL
    )
endfunction()

function(check_for_ulimit)
    message("-- Checking 'ulimit -n'")
    execute_process(COMMAND bash -c "ulimit -n"
        OUTPUT_VARIABLE ulimitOutput
    )
    string(REGEX MATCHALL "[0-9]+" limit "${ulimitOutput}")
    message(" -- Open files limit ${limit}")
    if(NOT (QT_FEATURE_use_gold_linker OR QT_FEATURE_use_lld_linker) AND ulimitOutput LESS 4096)
        if(NOT ${CMAKE_VERSION} VERSION_LESS "3.21.0")
            message(" -- Creating linker launcher")
            file(GENERATE OUTPUT ${PROJECT_BINARY_DIR}/linker_ulimit.sh
                CONTENT "#!/bin/bash\nulimit -n 4096\nexec \"$@\""
                FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            )
            set(COIN_BUG_699 ON PARENT_SCOPE)
        else()
            set(PRINT_BFD_LINKER_WARNING ON PARENT_SCOPE)
        endif()
    endif()
endfunction()

function(add_build feature value)
    list(APPEND cmakeArgs
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DMATRIX_SUBBUILD=ON"
        "-DFEATURE_${feature}=${value}"
    )
    if(CMAKE_C_COMPILER_LAUNCHER)
        list(APPEND cmakeArgs "-DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}")
    endif()
    if(CMAKE_CXX_COMPILER_LAUNCHER)
        list(APPEND cmakeArgs "-DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}")
    endif()

    externalproject_add(${feature}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${feature}-${value}
        PREFIX ${feature}-${value}
        CMAKE_ARGS ${cmakeArgs}
        USES_TERMINAL_BUILD ON
        USES_TERMINAL_CONFIGURE ON
        BUILD_ALWAYS TRUE
        INSTALL_COMMAND ""
    )
    get_property(depTracker GLOBAL PROPERTY MATRIX_DEPENDENCY_TRACKER)
    foreach(dep ${depTracker})
        add_dependencies(${feature} ${dep})
    endforeach()
    set(depTracker "${depTracker}" ${feature})
    set_property(GLOBAL PROPERTY MATRIX_DEPENDENCY_TRACKER "${depTracker}")
endfunction()
