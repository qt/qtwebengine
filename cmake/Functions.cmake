# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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
        elseif("Debug" IN_LIST CMAKE_CONFIGURATION_TYPES)
            set(${result} "Debug" PARENT_SCOPE)
        else()
            # assume MinSizeRel ?
            set(${result} "${CMAKE_CONFIGURATION_TYPES}" PARENT_SCOPE)
        endif()
    endif()
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

function(get_ios_target_triple_and_sysroot result arch)
    get_ios_sysroot(sysroot ${arch})
    set(${result}
        -target ${arch}-apple-ios${CMAKE_OSX_DEPLOYMENT_TARGET}
        -isysroot ${sysroot} PARENT_SCOPE
    )
endfunction()

function(add_ninja_target)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "" "TARGET;CMAKE_TARGET;NINJA_TARGET;BUILDDIR;NINJA_STAMP;NINJA_DATA_STAMP;CONFIG;ARCH" ""
    )
    _qt_internal_validate_all_args_are_parsed(arg)
    set(stamps ${arg_NINJA_STAMP} ${arg_NINJA_DATA_STAMP})
    list(TRANSFORM stamps PREPEND "${arg_BUILDDIR}/${arg_CONFIG}/${arg_ARCH}/")
    add_custom_target(${arg_TARGET} DEPENDS ${stamps})
    set_target_properties(${arg_TARGET} PROPERTIES
        CONFIG ${arg_CONFIG}
        ARCH ${arg_ARCH}
        CMAKE_TARGET ${arg_CMAKE_TARGET}
        NINJA_TARGET ${arg_NINJA_TARGET}
        NINJA_STAMP ${arg_NINJA_STAMP}
    )
endfunction()

function(get_copy_of_response_file result target rsp)
    get_target_property(config ${target} CONFIG)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    set(rsp_dst "CMakeFiles_${ninjaTarget}_${config}_${rsp}.rsp")
    set(rsp_src "${${result}}")
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
    set(${result} ${rsp_dst} PARENT_SCOPE)
    add_custom_target(${cmakeTarget}_${rsp}_copy_${config}
        DEPENDS ${rsp_output}
    )
    add_dependencies(${cmakeTarget} ${cmakeTarget}_${rsp}_copy_${config})
endfunction()

function(add_archiver_options target buildDir completeStatic)
    get_target_property(config ${target} CONFIG)
    string(TOUPPER ${config} cfg)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    set(objects_out "${buildDir}/${cmakeTarget}_objects.o")
    add_library(GnObject_${cmakeTarget}_${config} OBJECT IMPORTED GLOBAL)
    target_link_libraries(${cmakeTarget} PRIVATE $<$<CONFIG:${config}>:GnObject_${cmakeTarget}_${config}>)
    set_property(TARGET GnObject_${cmakeTarget}_${config} PROPERTY IMPORTED_OBJECTS_${cfg} ${objects_out})
endfunction()

function(add_linker_options target buildDir completeStatic)
    get_target_property(config ${target} CONFIG)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    string(TOUPPER ${cmakeTarget} tg)
    string(TOUPPER ${config} cfg)
    set(objects_rsp "${buildDir}/${ninjaTarget}_objects.rsp")
    set(archives_rsp "${buildDir}/${ninjaTarget}_archives.rsp")
    set(libs_rsp "${buildDir}/${ninjaTarget}_libs.rsp")
    set(ldir_rsp "${buildDir}/${ninjaTarget}_ldir.rsp")
    set_target_properties(${cmakeTarget} PROPERTIES STATIC_LIBRARY_OPTIONS "@${objects_rsp}")
    if(LINUX OR ANDROID)
         get_gn_arch(cpu ${TEST_architecture_arch})
         if(CMAKE_CROSSCOMPILING AND cpu STREQUAL "arm" AND ${config} STREQUAL "Debug")
             target_link_options(${cmakeTarget} PRIVATE "LINKER:--long-plt")
         endif()
         target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${objects_rsp}>")
         # Chromium is meant for linking with gc-sections, which seems to not always get applied otherwise
         target_link_options(${cmakeTarget} PRIVATE "-Wl,--gc-sections")
         if(NOT completeStatic)
             target_link_libraries(${cmakeTarget} PRIVATE
                 "$<1:-Wl,--start-group $<$<CONFIG:${config}>:@${archives_rsp}> -Wl,--end-group>"
             )
         endif()

         # linker here options are just to prevent processing it by cmake
         target_link_libraries(${cmakeTarget} PRIVATE
             "$<1:-Wl,--no-fatal-warnings $<$<CONFIG:${config}>:@${ldir_rsp}> $<$<CONFIG:${config}>:@${libs_rsp}> -Wl,--no-fatal-warnings>"
         )
         unset(cpu)
    endif()
    if(MACOS)
        target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${objects_rsp}>")
        if(NOT completeStatic)
            target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${archives_rsp}>")
        endif()
        target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${ldir_rsp}>" "$<$<CONFIG:${config}>:@${libs_rsp}>")
    endif()
    if(WIN32)
        get_copy_of_response_file(objects_rsp ${target} objects)
        if(NOT MINGW)
            target_link_options(${cmakeTarget}
                PRIVATE /DELAYLOAD:mf.dll /DELAYLOAD:mfplat.dll /DELAYLOAD:mfreadwrite.dll /DELAYLOAD:winmm.dll
            )
            # enable larger PDBs if webenginecore debug build
            if(cmakeTarget STREQUAL "WebEngineCore")
                target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:Debug>:/pdbpagesize:8192>")
            endif()
        endif()
        target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${objects_rsp}>")
        if(NOT completeStatic)
            get_copy_of_response_file(archives_rsp ${target} archives)
            target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${archives_rsp}>")
        endif()
        get_copy_of_response_file(libs_rsp ${target} libs)
        target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${libs_rsp}>")
        # we need libs rsp also when linking process with sandbox lib
        set_property(TARGET ${cmakeTarget} PROPERTY LIBS_RSP ${libs_rsp})
    endif()
endfunction()

function(add_intermediate_archive target buildDir completeStatic)
    get_target_property(config ${target} CONFIG)
    get_target_property(arch ${target} ARCH)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    get_target_property(ninjaStamp ${target} NINJA_STAMP)
    string(TOUPPER ${config} cfg)
    set(objects_rsp "${buildDir}/${ninjaTarget}_objects.rsp")
    set(objects_out "${buildDir}/${cmakeTarget}_objects.o")
    if(NOT completeStatic)
        set(archives_rsp "${buildDir}/${ninjaTarget}_archives.rsp")
        set(archives_out "${buildDir}/${cmakeTarget}_archives.o")
        set(archives_command
            COMMAND clang++ -r -nostdlib -arch ${arch}
            -o ${archives_out}
            -Wl,-keep_private_externs
            -Wl,-all_load
            @${archives_rsp}
        )
    endif()
    add_custom_command(
        OUTPUT ${buildDir}/${cmakeTarget}.a
        BYPRODUCTS ${objects_out} ${archives_out}
        COMMAND clang++ -r -nostdlib -arch ${arch}
            -o ${objects_out}
            -Wl,-keep_private_externs
            @${objects_rsp}
        ${archives_command}
        COMMAND ar -crs
            ${buildDir}/${cmakeTarget}.a
            ${objects_out}
            ${archives_out}
        DEPENDS
            ${buildDir}/${ninjaStamp}
        WORKING_DIRECTORY "${buildDir}/../../.."
        COMMENT "Creating intermediate archives for ${cmakeTarget}/${config}/${arch}"
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
endfunction()

function(add_intermediate_object target buildDir completeStatic)
    get_target_property(config ${target} CONFIG)
    get_target_property(arch ${target} ARCH)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    get_target_property(ninjaStamp ${target} NINJA_STAMP)
    string(TOUPPER ${config} cfg)
    if(IOS)
        get_ios_target_triple_and_sysroot(args ${arch})
    endif()
    set(objects_rsp "${buildDir}/${ninjaTarget}_objects.rsp")
    set(objects_out "${buildDir}/${cmakeTarget}_objects.o")
    add_custom_command(
        OUTPUT ${objects_out}
        COMMAND clang++ -r -nostdlib
            ${args}
            -o ${objects_out}
            -Wl,-keep_private_externs
            @${objects_rsp}
        DEPENDS
            ${buildDir}/${ninjaStamp}
        WORKING_DIRECTORY "${buildDir}/../../.."
        COMMENT "Creating intermediate object files for ${cmakeTarget}/${config}/${arch}"
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
endfunction()

# Lipo the object files together to a single fat archive
function(create_lipo_command target buildDir fileName)
    get_target_property(config ${target} CONFIG)
    get_architectures(archs)
    foreach(arch ${archs})
        list(APPEND lipo_objects ${buildDir}/${arch}/${fileName})
    endforeach()
    add_custom_command(
        OUTPUT ${buildDir}/${fileName}
        COMMAND lipo -create
            -output ${buildDir}/${fileName}
        ARGS ${lipo_objects}
        DEPENDS ${lipo_objects}
        USES_TERMINAL
        COMMENT "Running lipo for ${target}/${config}/${arch}"
        VERBATIM
    )
endfunction()

# this function only deals with objects as it is only
# used by qtpdf and we do not need anything more
function(add_ios_lipo_command target buildDir)
    get_target_property(config ${target} CONFIG)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    set(fileName ${cmakeTarget}_objects.o)
    create_lipo_command(${target} ${buildDir} ${fileName})
    add_custom_target(lipo_${cmakeTarget}_${config} DEPENDS
        ${buildDir}/${fileName}
    )
    add_dependencies(${cmakeTarget} lipo_${cmakeTarget}_${config})
    qt_internal_get_target_property(options ${cmakeTarget} STATIC_LIBRARY_OPTIONS)
    set_target_properties(${cmakeTarget} PROPERTIES STATIC_LIBRARY_OPTIONS
        "${options}$<$<CONFIG:${config}>:${buildDir}/${fileName}>"
    )
endfunction()

function(add_lipo_command target buildDir)
    get_target_property(config ${target} CONFIG)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    set(fileName ${cmakeTarget}.a)
    create_lipo_command(${target} ${buildDir} ${fileName})
    add_library(${cmakeTarget}_${config} STATIC IMPORTED GLOBAL)
    set_property(TARGET ${cmakeTarget}_${config}
        PROPERTY IMPORTED_LOCATION ${buildDir}/${fileName}
    )
    add_custom_target(lipo_${cmakeTarget}_${config} DEPENDS
        ${buildDir}/${fileName}
    )
    add_dependencies(${cmakeTarget}_${config} lipo_${cmakeTarget}_${config})
    target_link_libraries(${cmakeTarget} PRIVATE ${cmakeTarget}_${config})

    # Just link with dynamic libs once
    # TODO: this is evil hack, since cmake has no idea about libs
    set(libs_rsp "${buildDir}/x86_64/${ninjaTarget}_libs.rsp")
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

# Function maps TEST_architecture_arch or CMAKE_SYSTEM_PROCESSOR into gn architecture
function(get_gn_arch result arch)
    set(armList arm armv7-a)
    set(arm64List arm64 ARM64 aarch64)
    set(mips64List mips64 mipsel64)
    set(x86List i386 i686)
    set(x64List x86_64 AMD64 x86_64h)
    if(arch IN_LIST x86List)
        set(${result} "x86" PARENT_SCOPE)
    elseif(arch IN_LIST x64List)
        set(${result} "x64" PARENT_SCOPE)
    elseif(arch IN_LIST armList)
        set(${result} "arm" PARENT_SCOPE)
    elseif(arch IN_LIST arm64List)
        set(${result} "arm64" PARENT_SCOPE)
    elseif(arch STREQUAL "mipsel")
        set(${result} "mipsel" PARENT_SCOPE)
    elseif(arch IN_LIST mipsList)
        set(${result} "mips64el" PARENT_SCOPE)
    elseif(arch STREQUAL "riscv64")
        set(${result} "riscv64" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown architecture: ${arch}")
    endif()
endfunction()

# Function maps gn architecture for v8
function(get_v8_arch result targetArch hostArch)
    set(list32 x86 arm mipsel riscv32)
    if(hostArch STREQUAL targetArch)
        set(${result} "${targetArch}" PARENT_SCOPE)
    elseif(targetArch IN_LIST list32)
        # 32bit target which needs a 32bit compatible host
        if(hostArch STREQUAL "x64")
            set(${result} "x86" PARENT_SCOPE)
        elseif(hostArch STREQUAL "arm64")
            set(${result} "arm" PARENT_SCOPE)
        elseif(hostArch STREQUAL "mips64el")
            set(${result} "mipsel" PARENT_SCOPE)
        elseif(hostArch STREQUAL "riscv64")
            set(${result} "riscv32" PARENT_SCOPE)
        elseif(hostArch IN_LIST list32)
            set(${result} "${hostArch}" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Unknown architecture: ${hostArch}")
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


function(get_gn_is_mingw result)
    if(MINGW)
        set(${result} "true" PARENT_SCOPE)
    else()
        set(${result} "false" PARENT_SCOPE)
    endif()
endfunction()

function(get_ios_sysroot result arch)
    if(NOT CMAKE_APPLE_ARCH_SYSROOTS)
      message(FATAL_ERROR "CMAKE_APPLE_ARCH_SYSROOTS not set.")
    endif()
    get_architectures(archs)
    list(FIND archs ${arch} known_arch)
    if (known_arch EQUAL "-1")
        message(FATAL_ERROR "Unknown iOS architecture ${arch}.")
    endif()
    list(GET CMAKE_APPLE_ARCH_SYSROOTS ${known_arch} sysroot)
    set(${result} ${sysroot} PARENT_SCOPE)
endfunction()

function(configure_gn_toolchain name cpu v8Cpu toolchainIn toolchainOut)
    set(GN_TOOLCHAIN ${name})
    get_gn_os(GN_OS)
    get_gn_is_clang(GN_IS_CLANG)
    get_gn_is_mingw(GN_IS_MINGW)
    set(GN_CPU ${cpu})
    set(GN_V8_CPU ${v8Cpu})
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
    cmake_parse_arguments(PARSE_ARGV 1 GN "" "" "ARG;CFLAG")
    _qt_internal_validate_all_args_are_parsed(GN)

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
    get_gn_arch(gn_arch ${TEST_architecture_arch})
    if(NOT CMAKE_CROSSCOMPILING) # delivered by hostBuild project
        configure_gn_toolchain(host ${gn_arch} ${gn_arch}
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
            ${buildDir}/host_toolchain)
        configure_gn_toolchain(v8 ${gn_arch} ${gn_arch}
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
            ${buildDir}/v8_toolchain)
    endif()
    configure_gn_toolchain(target ${gn_arch} ${gn_arch}
        ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/BUILD.toolchain.gn.in
        ${buildDir}/target_toolchain)
    unset(gn_arch)
endmacro()

macro(append_build_type_setup)
    list(APPEND gnArgArg
        is_qtwebengine=true
        init_stack_vars=false
        is_component_build=false
        is_shared=true
        use_sysroot=false
        forbid_non_component_debug_builds=false
        treat_warnings_as_errors=false
        use_allocator_shim=false
        use_partition_alloc=true
        use_partition_alloc_as_malloc=false
        use_custom_libcxx=false
        enable_rust=false # We do not yet support rust
        enable_chromium_prelude=false
        build_tflite_with_xnnpack=false
    )
    if(${config} STREQUAL "Debug")
        list(APPEND gnArgArg is_debug=true symbol_level=2)
        if(WIN32)
            list(APPEND gnArgArg enable_iterator_debugging=true)
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
             use_viz_debugger=false
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

    #TODO: refactor to not check for IOS here
    if(NOT QT_FEATURE_webengine_full_debug_info AND NOT IOS)
        list(APPEND gnArgArg blink_symbol_level=0 v8_symbol_level=0)
    endif()

    extend_gn_list(gnArgArg ARGS use_jumbo_build CONDITION QT_FEATURE_webengine_jumbo_build)
    if(QT_FEATURE_webengine_jumbo_build)
        list(APPEND gnArgArg jumbo_file_merge_limit=${QT_FEATURE_webengine_jumbo_file_merge_limit})
        if(QT_FEATURE_webengine_jumbo_file_merge_limit LESS_EQUAL 8)
            list(APPEND gnArgArg "jumbo_build_excluded=[\"browser\"]")
        endif()
    endif()

    extend_gn_list(gnArgArg
        ARGS enable_precompiled_headers
        CONDITION BUILD_WITH_PCH AND NOT LINUX
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
    extend_gn_list(gnArgArg ARGS is_mingw CONDITION MINGW)
    extend_gn_list(gnArgArg ARGS is_msvc CONDITION MSVC)
    extend_gn_list(gnArgArg ARGS is_gcc CONDITION LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

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

        string(REGEX MATCH "[0-9]+" clangVersion ${CMAKE_CXX_COMPILER_VERSION})
        list(APPEND gnArgArg
            clang_base_path="${clangBasePath}"
            clang_use_chrome_plugins=false
            clang_version=${clangVersion}
            fatal_linker_warnings=false
        )

        if(MACOS)
            list(APPEND gnArgArg
                use_system_xcode=true
                mac_deployment_target="${CMAKE_OSX_DEPLOYMENT_TARGET}"
                mac_sdk_min="${macSdkVersion}"
                use_libcxx=true
            )
            _qt_internal_get_apple_sdk_version(apple_sdk_version)
            if (apple_sdk_version LESS 13.2)
                list(APPEND gnArgArg
                    use_sck=false
                )
            endif()
        endif()
        if(IOS)
            list(APPEND gnArgArg
                use_system_xcode=true
                enable_ios_bitcode=true
                ios_deployment_target="${CMAKE_OSX_DEPLOYMENT_TARGET}"
                ios_enable_code_signing=false
                use_libcxx=true
            )
        endif()
        if(DEFINED QT_FEATURE_stdlib_libcpp AND LINUX)
            extend_gn_list(gnArgArg ARGS use_libcxx
                CONDITION QT_FEATURE_stdlib_libcpp
            )
        endif()
        if(ANDROID)
            list(APPEND gnArgArg
                android_ndk_root="${CMAKE_ANDROID_NDK}"
                android_ndk_version="${CMAKE_ANDROID_NDK_VERSION}"
                clang_use_default_sample_profile=false
                #android_ndk_major_version=22
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

    if(MSVC)
        get_filename_component(windows_sdk_path $ENV{WINDOWSSDKDIR} ABSOLUTE)
        get_filename_component(visual_studio_path $ENV{VSINSTALLDIR} ABSOLUTE)
        qt_webengine_get_windows_sdk_version(windows_sdk_version sdk_minor)
        list(APPEND gnArgArg
            win_linker_timing=true
            use_incremental_linking=false
            visual_studio_version=2022
            visual_studio_path=\"${visual_studio_path}\"
            windows_sdk_version=\"${windows_sdk_version}\"
            windows_sdk_path=\"${windows_sdk_path}\"
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
        extract_cflag(march "march")
        get_arm_version(arm_version ${march})
        if(arm_version EQUAL 7)
            list(APPEND gnArgArg use_arm_crc32=false)
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
    unset(cpu)
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
        if(APPLE)
            list(APPEND gnArgArg
                clang_version=\"${QT_COMPILER_VERSION_MAJOR}.${QT_COMPILER_VERSION_MINOR}.${QT_COMPILER_VERSION_PATCH}\"
            )
        endif()
    endif()
endmacro()

macro(append_toolchain_setup)
    if(WIN32)
        get_gn_arch(cpu ${arch})
        list(APPEND gnArgArg target_cpu="${cpu}")
        if(MINGW)
            get_gn_arch(cpu ${TEST_architecture_arch})
            list(APPEND gnArgArg
                # note '/' prefix
                custom_toolchain="/${buildDir}/target_toolchain:target"
                host_toolchain="/${buildDir}/host_toolchain:host"
                host_cpu="${cpu}"
            )
        endif()
    elseif(LINUX)
        get_gn_arch(cpu ${TEST_architecture_arch})
        list(APPEND gnArgArg
            custom_toolchain="${buildDir}/target_toolchain:target"
            host_toolchain="${buildDir}/host_toolchain:host"
        )
        if(CMAKE_CROSSCOMPILING)
            list(APPEND gnArgArg
                v8_snapshot_toolchain="${buildDir}/v8_toolchain:v8"
                target_cpu="${cpu}"
            )
        else()
            list(APPEND gnArgArg host_cpu="${cpu}")
        endif()
        if(CMAKE_SYSROOT)
            list(APPEND gnArgArg target_sysroot="${CMAKE_SYSROOT}")
        endif()
    elseif(MACOS)
        get_gn_arch(cpu ${arch})
        list(APPEND gnArgArg target_cpu="${cpu}")
    elseif(IOS)
        get_gn_arch(cpu ${arch})
        get_ios_sysroot(sysroot ${arch})
        list(APPEND gnArgArg target_cpu="${cpu}" target_sysroot="${sysroot}" target_os="ios")
    elseif(ANDROID)
        get_gn_arch(cpu ${TEST_architecture_arch})
        list(APPEND gnArgArg target_os="android" target_cpu="${cpu}")
        if(CMAKE_HOST_WIN32)
            list(APPEND gnArgArg
                host_toolchain="/${buildDir}/host_toolchain:host"
                host_cpu="x64"
                v8_snapshot_toolchain="/${buildDir}/v8_toolchain:v8"
            )
        endif()
    endif()
    unset(cpu)
endmacro()


macro(append_pkg_config_setup)
    if(PkgConfig_FOUND)
        list(APPEND gnArgArg
            pkg_config="${PKG_CONFIG_EXECUTABLE}"
            host_pkg_config="${PKG_CONFIG_HOST_EXECUTABLE}"
        )
        if(NOT "$ENV{PKG_CONFIG_LIBDIR}" STREQUAL "")
            list(APPEND gnArgArg
                system_libdir="$ENV{PKG_CONFIG_LIBDIR}"
            )
        endif()
    endif()
endmacro()

function(add_ninja_command)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "" "TARGET;BUILDDIR;MODULE" "OUTPUT;BYPRODUCTS"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    string(REPLACE " " ";" NINJAFLAGS "$ENV{NINJAFLAGS}")
    list(TRANSFORM arg_OUTPUT PREPEND "${arg_BUILDDIR}/")
    list(TRANSFORM arg_BYPRODUCTS PREPEND "${arg_BUILDDIR}/")
    add_custom_command(
        OUTPUT
            ${arg_OUTPUT}
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
        set(${result} ${CMAKE_CONFIGURATION_TYPES})
    else()
        set(${result} ${CMAKE_BUILD_TYPE})
    endif()
    if(NOT ${result})
        message(FATAL_ERROR "No valid configurations found !")
    endif()
    set(${result} ${${result}} PARENT_SCOPE)
endfunction()

function(get_architectures result)
    if(CMAKE_OSX_ARCHITECTURES)
        set(${result} ${CMAKE_OSX_ARCHITECTURES})
    else()
        set(${result} ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    if(NOT ${result})
    message(FATAL_ERROR "No valid architectures found. In case of cross-compiling make sure you have CMAKE_SYSTEM_PROCESSOR in your toolchain file.")
    endif()
    set(${result} ${${result}} PARENT_SCOPE)
endfunction()

function(add_gn_build_artifacts_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "" "CMAKE_TARGET;NINJA_TARGET;BUILDDIR;MODULE;COMPLETE_STATIC;NINJA_STAMP;NINJA_DATA_STAMP" ""
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    # config loop is a workaround to be able to add_custom_command per config
    # note this is fixed in CMAKE.3.20 and should be cleaned up when 3.20 is
    # the minimum cmake we support
    get_configs(configs)
    get_architectures(archs)
    foreach(config ${configs})
        foreach(arch ${archs})
            set(target ${arg_NINJA_TARGET}_${config}_${arch})
            set(stamps ${arg_NINJA_STAMP} ${arg_NINJA_DATA_STAMP})
            add_ninja_target(
                TARGET ${target}
                NINJA_TARGET ${arg_NINJA_TARGET}
                CMAKE_TARGET ${arg_CMAKE_TARGET}
                NINJA_STAMP ${arg_NINJA_STAMP}
                NINJA_DATA_STAMP ${arg_NINJA_DATA_STAMP}
                CONFIG ${config}
                ARCH ${arch}
                BUILDDIR ${arg_BUILDDIR}
            )
            add_ninja_command(
                TARGET ${arg_NINJA_TARGET}
                OUTPUT ${stamps}
                BUILDDIR ${arg_BUILDDIR}/${config}/${arch}
                MODULE ${arg_MODULE}
            )
            add_dependencies(run_${arg_MODULE}_NinjaDone ${target})
            set_target_properties(${arg_CMAKE_TARGET} PROPERTIES
                LINK_DEPENDS ${arg_BUILDDIR}/${config}/${arch}/${arg_NINJA_STAMP}
            )
            if(QT_IS_MACOS_UNIVERSAL)
                add_intermediate_archive(${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
            elseif(IOS)
                add_intermediate_object(${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
            else()
                if(MACOS AND QT_FEATURE_static)
                    # mac archiver does not support @file notation, do intermediate object istead
                    add_intermediate_object(${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
                    add_archiver_options(${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
                else()
                    add_linker_options(${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
                endif()
            endif()
            unset(target)
        endforeach()
        list(GET archs 0 arch)
        set(target ${arg_NINJA_TARGET}_${config}_${arch})
        # Work around for broken builds with new Apple linker ld_prime. Force
        # use of the classic linker until this has been fixed.
        # TODO: remove once this has been fixed by Apple. See issue FB13667242
        # or QTBUG-122655 for details.
        if(APPLECLANG)
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0.0")
                target_link_options(${arg_CMAKE_TARGET} PRIVATE -ld_classic)
                set_target_properties(${arg_CMAKE_TARGET} PROPERTIES
                    QT_NO_DISABLE_WARN_DUPLICATE_LIBRARIES TRUE)
            endif()
        endif()
        if(QT_IS_MACOS_UNIVERSAL)
            add_lipo_command(${target} ${arg_BUILDDIR}/${config})
        endif()
        if(IOS)
            add_ios_lipo_command(${target} ${arg_BUILDDIR}/${config})
        endif()
    endforeach()
endfunction()

function(add_gn_command)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "" "CMAKE_TARGET;GN_TARGET;MODULE;BUILDDIR" "NINJA_TARGETS;GN_ARGS"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    get_config_filenames(cConfigFileName cxxConfigFileName staticConfigFileName targetConfigFileName)
    set(gnArgArgFile ${arg_BUILDDIR}/args.gn)

    list(JOIN arg_GN_ARGS "\n" arg_GN_ARGS)
    file(WRITE ${gnArgArgFile} ${arg_GN_ARGS})

    foreach(ninjaTarget ${arg_NINJA_TARGETS})
        list(APPEND output ${ninjaTarget}_objects.rsp ${ninjaTarget}_archives.rsp ${ninjaTarget}_libs.rsp ${ninjaTarget}_ldir.rsp)
    endforeach()
    list(TRANSFORM output PREPEND "${arg_BUILDDIR}/")

    add_custom_command(
        OUTPUT ${output}
        COMMAND ${CMAKE_COMMAND}
             -DBUILD_DIR=${arg_BUILDDIR}
             -DSOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
             -DMODULE=${arg_MODULE}
             -DQT_HOST_PATH=${QT_HOST_PATH}
             -DQT6_HOST_INFO_LIBEXECDIR=${QT6_HOST_INFO_LIBEXECDIR}
             -DQT6_HOST_INFO_BINDIR=${QT6_HOST_INFO_BINDIR}
             -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
             -DGN_THREADS=$ENV{QTWEBENGINE_GN_THREADS}
             -DQT_ALLOW_SYMLINK_IN_PATHS=${QT_ALLOW_SYMLINK_IN_PATHS}
             -P ${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtGnGen.cmake
        WORKING_DIRECTORY ${WEBENGINE_ROOT_BUILD_DIR}
        COMMENT "Run gn for target ${arg_CMAKE_TARGET} in ${arg_BUILDDIR}"
        DEPENDS
            ${gnArgArgFile}
            run_${arg_MODULE}_GnReady
            "${WEBENGINE_ROOT_SOURCE_DIR}/src/${arg_MODULE}/configure/BUILD.root.gn.in"
            "${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtGnGen.cmake"
    )
    add_custom_target(runGn_${arg_GN_TARGET}
        DEPENDS #TODO this is fixed in cmake 3.20 so we could simply use GN_TARGET and not create new one
            ${output}
            ${arg_BUILDDIR}/${cxxConfigFileName}
            ${arg_BUILDDIR}/${cConfigFileName}
            ${arg_BUILDDIR}/${targetConfigFileName}
    )
    add_dependencies(run_${arg_MODULE}_GnDone runGn_${arg_GN_TARGET})
    if(TARGET thirdparty_sync_headers)
        add_dependencies(runGn_${arg_GN_TARGET} thirdparty_sync_headers)
    endif()
    create_gn_target_config(${arg_GN_TARGET} ${arg_BUILDDIR}/${targetConfigFileName})
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

function(add_code_attributions_target)
    cmake_parse_arguments(PARSE_ARGV 0 arg ""
        "TARGET;OUTPUT;GN_TARGET;FILE_TEMPLATE;ENTRY_TEMPLATE;BUILDDIR"
        "EXTRA_THIRD_PARTY_DIRS"
    )
    _qt_internal_validate_all_args_are_parsed(arg)
    get_filename_component(fileTemplate ${arg_FILE_TEMPLATE} ABSOLUTE)
    get_filename_component(entryTemplate ${arg_ENTRY_TEMPLATE} ABSOLUTE)
    add_custom_command(
        OUTPUT ${arg_OUTPUT}
        COMMAND ${CMAKE_COMMAND}
            -DLICENSE_SCRIPT=${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium/tools/licenses/licenses.py
            -DFILE_TEMPLATE=${fileTemplate}
            -DENTRY_TEMPLATE=${entryTemplate}
            -DGN_TARGET=${arg_GN_TARGET}
            -DEXTRA_THIRD_PARTY_DIRS="${arg_EXTRA_THIRD_PARTY_DIRS}"
            -DBUILDDIR=${arg_BUILDDIR}
            -DOUTPUT=${arg_OUTPUT}
            -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
            -P ${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtGnCredits.cmake
        WORKING_DIRECTORY ${WEBENGINE_ROOT_BUILD_DIR}
        DEPENDS
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium/tools/licenses/licenses.py
            ${arg_FILE_TEMPLATE}
            ${arg_ENTRY_TEMPLATE}
            ${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtGnCredits.cmake
        USES_TERMINAL
     )
     add_custom_target(${arg_TARGET} DEPENDS ${arg_OUTPUT})
endfunction()

macro(qt_webengine_build_and_install_gn)
    set(suppress_warning "${BUILD_ONLY_GN} ${QT_INTERNAL_CALLED_FROM_CONFIGURE}")
    qt_internal_project_setup()
    qt_webengine_externalproject_add(gn
        SOURCE_DIR  ${CMAKE_CURRENT_LIST_DIR}/src/gn
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/src/gn
        INSTALL_DIR ${PROJECT_BINARY_DIR}/install
    )
    qt_internal_set_cmake_build_type()
    get_install_config(install_config)
    qt_install(
        PROGRAMS "${PROJECT_BINARY_DIR}/install/bin/gn${CMAKE_EXECUTABLE_SUFFIX}"
        CONFIGURATIONS ${install_config}
        RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
    )
    unset(suppress_warning)
    unset(install_config)
endmacro()

macro(qt_webengine_externalproject_add)
    list(JOIN CMAKE_OSX_ARCHITECTURES "," OSX_ARCH_STR)
    externalproject_add(${ARGN}
        PREFIX      gn
        USES_TERMINAL_BUILD TRUE
        LIST_SEPARATOR ","
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                   -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                   -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
                   -DCMAKE_OSX_ARCHITECTURES=${OSX_ARCH_STR}
                   -DWEBENGINE_ROOT_BUILD_DIR=${PROJECT_BINARY_DIR}
                   -DQT_ALLOW_SYMLINK_IN_PATHS=${QT_ALLOW_SYMLINK_IN_PATHS}
    )
    unset(OSX_ARCH_STR)
endmacro()
