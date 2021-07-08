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
    if (NOT DEFINED WEBENGINE_REPO_BUILD AND NOT DEFINED QT_SUPERBUILD)
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

function(extend_target_with_gn_objects target config cmakeFile stampFile)

     include(${buildDir}/${config}/${cmakeFile})

     string(TOUPPER ${config} cfg)
     add_library(GnObjects_${target}_${config} OBJECT IMPORTED GLOBAL)
     target_link_libraries(${target} PRIVATE $<$<CONFIG:${config}>:GnObjects_${target}_${config}>)
     add_custom_target(ninja_${target}_${config} DEPENDS ${buildDir}/${config}/${stampFile})
     add_dependencies(GnObjects_${target}_${config} ninja_${target}_${config})
     #TODO: remove GnObjects_${target}_${config} with CMAKE 3.20
     set_property(TARGET GnObjects_${target}_${config}
         PROPERTY IMPORTED_OBJECTS_${cfg} ${${cfg}_NINJA_OBJECTS}
     )
     set_source_files_properties(${${cfg}_NINJA_OBJECTS} PROPERTIES GENERATED TRUE)

     if(LINUX)
     target_link_libraries(${target}
         PRIVATE "-Wl,--start-group" "$<$<CONFIG:${config}>:${${cfg}_NINJA_ARCHIVES}>" "-Wl,--end-group")
     else()
     target_link_libraries(${target} PRIVATE "$<$<CONFIG:${config}>:${${cfg}_NINJA_ARCHIVES}>")
     endif()

     target_link_libraries(${target} PUBLIC  "$<$<CONFIG:${config}>:${${cfg}_NINJA_LIBS}>")

     # we depend on stampFile, but ninja backend generator needs more (create once)
     if(stampFile)
         add_custom_command(OUTPUT ${${cfg}_NINJA_OBJECTS} ${${cfg}_NINJA_ARCHIVES}
             DEPENDS ${buildDir}/${config}/${stampFile}
         )
         add_custom_target(generate_${target}_${cfg}
             DEPENDS ${${cfg}_NINJA_OBJECTS} ${${cfg}_NINJA_ARCHIVES}
         )
     endif()
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
   if("${arch}" STREQUAL "i386")
      set(${result} "x86" PARENT_SCOPE)
   elseif("${arch}" STREQUAL "x86_64")
      set(${result} "x64" PARENT_SCOPE)
   elseif("${arch}" STREQUAL "arm")
      set(${result} "arm" PARENT_SCOPE)
   elseif("${arch}" STREQUAL "arm64")
      set(${result} "arm64" PARENT_SCOPE)
   elseif("${arch}" STREQUAL "mipsel")
      set(${result} "mipsel" PARENT_SCOPE)
   elseif("${arch}" STREQUAL "mipsel64")
      set(${result} "mips64el" PARENT_SCOPE)
   else()
      message(DEBUG "Unsupported achitecture: ${arch}")
   endif()
endfunction()

function(get_v8_arch result targetArch)
   set(list32 i386 arm mipsel)
   if("${targetArch}" IN_LIST list32)
       set(${result} "i386" PARENT_SCOPE)
   else()
       set(${result} "x86_64" PARENT_SCOPE)
   endif()
endfunction()

function(configure_gn_toolchain name cpuType v8CpuType toolchainIn toolchainOut)
    set(GN_TOOLCHAIN ${name})
    get_gn_arch(GN_CPU ${cpuType})
    get_gn_arch(GN_V8_CPU ${v8CpuType})
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
    message(DEBUG "Found cflags: ${cflags}")
    if (cflags MATCHES "-${cflag}=([^ ]+)")
       set(${result} ${CMAKE_MATCH_1} PARENT_SCOPE)
       return()
    endif()
    if (cflags MATCHES "-${cflag}")
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

macro(create_pkg_config_host_wrapper)
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
        remove_v8base_debug_symbols=true
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
    if(FEATURE_developer_build OR (${config} STREQUAL "Debug"))
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

    # FIXME: Make it configurable
    list(APPEND gnArgArg
        use_jumbo_build=true
        jumbo_file_merge_limit=8
        jumbo_build_excluded=["browser"]
    )

    extend_gn_list(gnArgArg
        ARGS enable_precompiled_headers
        CONDITION BUILD_WITH_PCH
    )
endmacro()

macro(append_compiler_linker_sdk_setup)
    if (CMAKE_CXX_COMPILER_LAUNCHER)
        list(APPEND gnArgArg cc_wrapper="${CMAKE_CXX_COMPILER_LAUNCHER}")
    endif()
    extend_gn_list(gnArgArg
        ARGS is_clang
        CONDITION CLANG
    )
    if(CLANG AND NOT MACOS)
        # For some reason this doesn't work for our macOS CIs
        get_filename_component(clangBasePath ${CMAKE_CXX_COMPILER} DIRECTORY)
        get_filename_component(clangBasePath ${clangBasePath} DIRECTORY)
        list(APPEND gnArgArg
            clang_base_path="${clangBasePath}"
            clang_use_chrome_plugins=false
        )
    endif()
    if(MACOS)
        get_darwin_sdk_version(macSdkVersion)
        get_filename_component(clangBasePath ${CMAKE_OBJCXX_COMPILER} DIRECTORY)
        get_filename_component(clangBasePath ${clangBasePath} DIRECTORY)
        list(APPEND gnArgArg
            use_system_xcode=true
            clang_base_path="${clangBasePath}"
            clang_use_chrome_plugins=false
            mac_deployment_target="${CMAKE_OSX_DEPLOYMENT_TARGET}"
            mac_sdk_min="${macSdkVersion}"
            fatal_linker_warnings=false
       )
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
    if(QT_FEATURE_sanitizer)
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
    endif()
    if(WIN32)
        list(APPEND gnArgArg target_cpu="x64")
    endif()
    if(MAC AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR
        CMAKE_OSX_ARCHITECTURES STREQUAL "arm64"))
            list(APPEND gnArgArg
                target_cpu="arm64"
            )
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

macro(execute_gn)
    get_target_property(gnCmd Gn::gn IMPORTED_LOCATION)
    set(gnArg gen ${buildDir}/${config})

    list(APPEND gnArg
        --script-executable=${Python2_EXECUTABLE}
        --root=${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium)
    list(JOIN gnArgArg " " gnArgArg)

    list(APPEND gnArg "--args=${gnArgArg}")

    list(JOIN gnArg " " printArg)
    message("-- Running ${config} Configuration for GN \n-- ${gnCmd} ${printArg}")
    execute_process(
        COMMAND ${gnCmd} ${gnArg}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE gnResult
        OUTPUT_VARIABLE gnOutput
        ERROR_VARIABLE gnError
    )

    if(NOT gnResult EQUAL 0)
        message(FATAL_ERROR "\n-- GN FAILED\n${gnOutput}\n${gnError}")
    else()
        string(REGEX REPLACE "\n$" "" gnOutput "${gnOutput}")
        message("-- GN ${gnOutput}")
    endif()
endmacro()

macro(execute_ninja ninjaTargets)
    string(REPLACE " " ";" NINJAFLAGS "$ENV{NINJAFLAGS}")
    string(REPLACE " " ";" NINJATARGETS "${ninjaTargets} ${ARGN}")
    add_custom_command(
        OUTPUT
            ${buildDir}/${config}/${ninjaTargets}.stamp
            ${sandboxOutput}
            ${buildDir}/${config}/runAlways # use generator expression in CMAKE 3.20
        WORKING_DIRECTORY ${buildDir}/${config}
        COMMAND ${CMAKE_COMMAND} -E echo "Ninja ${config} build"
        COMMAND Ninja::ninja
            ${NINJAFLAGS}
            -C ${buildDir}/${config}
            ${NINJATARGETS}
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
endmacro()
