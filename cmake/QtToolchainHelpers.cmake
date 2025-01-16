# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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
    if(MSVC)
        if(CLANG)
            set(toolchain_in_file "BUILD.clang-cl.toolchain.gn.in")
        else()
            set(toolchain_in_file "BUILD.msvc.toolchain.gn.in")
        endif()
    else()
        set(toolchain_in_file "BUILD.toolchain.gn.in")
    endif()
    get_gn_arch(gn_arch ${TEST_architecture_arch})
    if(NOT CMAKE_CROSSCOMPILING) # delivered by hostBuild project
        configure_gn_toolchain(host ${gn_arch} ${gn_arch}
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/${toolchain_in_file}
            ${buildDir}/host_toolchain)
        configure_gn_toolchain(v8 ${gn_arch} ${gn_arch}
            ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/${toolchain_in_file}
            ${buildDir}/v8_toolchain)
    endif()
    configure_gn_toolchain(target ${gn_arch} ${gn_arch}
        ${WEBENGINE_ROOT_SOURCE_DIR}/src/host/${toolchain_in_file}
        ${buildDir}/target_toolchain)
    unset(gn_arch)
    unset(toolchain_in_file)
endmacro()

macro(append_build_type_setup)
    list(APPEND gnArgArg
        use_ml=false
        init_stack_vars=false
        is_component_build=false
        is_shared=true
        use_sysroot=false
        forbid_non_component_debug_builds=false
        treat_warnings_as_errors=false
        use_allocator_shim=false
        use_freelist_dispatcher=false
        use_partition_alloc=true
        use_partition_alloc_as_malloc=false
        use_custom_libcxx=false
        enable_rust=false # We do not yet support rust
        enable_chromium_prelude=false
        assert_cpp20=false
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

function(get_clang_version_from_runtime_path result)
if(CLANG AND CMAKE_CXX_COMPILER)
    if(NOT DEFINED CLANG_RUNTIME_PATH)
        set(CLANG_PRINT_RUNTIME_DIR_COMMAND -print-runtime-dir)
        if (MSVC)
            # clang-cl does not accept the argument unless it's piped via /clang:
            set(CLANG_PRINT_RUNTIME_DIR_COMMAND /clang:-print-runtime-dir)
        endif()
        execute_process(
           COMMAND ${CMAKE_CXX_COMPILER} ${CLANG_PRINT_RUNTIME_DIR_COMMAND}
           OUTPUT_VARIABLE clang_output
           ERROR_QUIET
           OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        cmake_path(CONVERT "${clang_output}" TO_CMAKE_PATH_LIST clang_output NORMALIZE)
        set(CLANG_RUNTIME_PATH "${clang_output}" CACHE INTERNAL "internal")
        mark_as_advanced(CLANG_RUNTIME_PATH)
     endif()
     string(REGEX MATCH "\\/([0-9.]+)\\/" clang_run_time_path_version "${CLANG_RUNTIME_PATH}")
     if(clang_run_time_path_version)
         string(REPLACE "/" "" clang_run_time_path_version ${clang_run_time_path_version})
     else()
         string(REGEX MATCH "[0-9]+" clang_run_time_path_version ${CMAKE_CXX_COMPILER_VERSION})
     endif()
     set(${result} ${clang_run_time_path_version} PARENT_SCOPE)
endif()
endfunction()

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
            get_clang_version_from_runtime_path(clang_version)
        if (NOT DEFINED clang_version)
            message(FATAL_ERROR "Clang version for runtime is missing."
                    "Please open bug report.Found clang runtime path: ${CLANG_RUNTIME_PATH}"
            )
        endif()
        list(APPEND gnArgArg
            clang_base_path="${clangBasePath}"
            clang_version="${clang_version}"
            clang_use_chrome_plugins=false
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
        get_filename_component(wdk_path $ENV{WINDOWSSDKDIR} ABSOLUTE)
        qt_webengine_get_windows_sdk_version(windows_sdk_version sdk_minor)
        list(APPEND gnArgArg
            win_linker_timing=true
            use_incremental_linking=false
            visual_studio_version=2022
            visual_studio_path=\"${visual_studio_path}\"
            windows_sdk_version=\"${windows_sdk_version}\"
            windows_sdk_path=\"${windows_sdk_path}\"
            wdk_path=\"${windows_sdk_path}\"
            setup_toolchain_script=\"//build/toolchain/win/qwe_setup_toolchain.py\"
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
        CONDITION QT_FEATURE_use_lld_linker OR (MSVC AND CLANG)
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
        if(CMAKE_CROSSCOMPILING)
           #TODO: fetch this from HOST QT or gn
           set(host_cpu "x64")
           get_gn_arch(target_cpu ${TEST_architecture_arch})
        else()
           get_gn_arch(host_cpu ${TEST_architecture_arch})
           set(target_cpu ${host_cpu})
        endif()
        list(APPEND gnArgArg target_cpu="${target_cpu}")
        if(MINGW)
            list(APPEND gnArgArg
                # note '/' prefix
                custom_toolchain="/${buildDir}/target_toolchain:target"
                host_toolchain="/${buildDir}/host_toolchain:host"
                host_cpu="${host_cpu}"
            )
        else()
            #TODO: no point genrete this in buildDir, it is a fixed set of toolchain afterall
            list(APPEND gnArgArg
                # note '/' prefix
                custom_toolchain="/${buildDir}/target_toolchain:${target_cpu}"
                host_toolchain="/${buildDir}/target_toolchain:${host_cpu}"
            )
        endif()
        unset(host_cpu)
        unset(target_cpu)
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
