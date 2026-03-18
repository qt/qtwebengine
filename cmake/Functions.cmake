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

# Links Chromium object files and static libraries to a target Qt library (e.g. Pdf or
# WebEngineCore), using response files.
#
# The function is used on all platforms except for:
# - universal macOS
# - static single-arch macOS
# - iOS of any kind
# The function IS used for single-arch shared-library macOS builds.
# The function IS NOT used for static macOS builds and iOS, as the default macOS 'ar'
# static library archiver does not support response files.
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
    set(lflags_rsp "${buildDir}/${ninjaTarget}_lflags.rsp")
    set_target_properties(${cmakeTarget} PROPERTIES STATIC_LIBRARY_OPTIONS "@${objects_rsp}")
    if(LINUX OR ANDROID)
         get_gn_arch(cpu ${TEST_architecture_arch})

         #QTBUG-145054#
         if(CMAKE_CROSSCOMPILING AND cpu STREQUAL "arm" AND cmakeTarget STREQUAL "WebEngineCore")
             target_link_options(${cmakeTarget} PRIVATE "LINKER:--strip-debug")
         endif()

         if(CMAKE_CROSSCOMPILING AND cpu STREQUAL "arm" AND ${config} STREQUAL "Debug")
             target_link_options(${cmakeTarget} PRIVATE "LINKER:--long-plt")
         endif()
         target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${objects_rsp}>")
         # Chromium is meant for linking with gc-sections, which seems to not always get applied otherwise
         target_link_options(${cmakeTarget} PRIVATE "-Wl,--gc-sections")
         target_link_options(${cmakeTarget} PRIVATE $<$<CONFIG:${config}>:@${lflags_rsp}>)
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
        target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:${config}>:@${ldir_rsp}>" "$<$<CONFIG:${config}>:@${libs_rsp}>" "$<$<CONFIG:${config}>:@${lflags_rsp}>")
    endif()
    if(WIN32)
        get_copy_of_response_file(objects_rsp ${target} objects)
        if(NOT MINGW)
            target_link_options(${cmakeTarget}
                PRIVATE /DELAYLOAD:mf.dll /DELAYLOAD:mfplat.dll /DELAYLOAD:mfreadwrite.dll /DELAYLOAD:winmm.dll
            )
            # enable larger PDBs if webenginecore debug build
            if(cmakeTarget STREQUAL "WebEngineCore")
                target_link_options(${cmakeTarget} PRIVATE "$<$<CONFIG:Debug>:/pdbpagesize:16384>")
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
        set_property(TARGET ${cmakeTarget} PROPERTY ARCHIVES_RSP ${archives_rsp})
    endif()
endfunction()

# Creates a single config, arch-specific static library archive from multiple Chromium object
# files and static libraries.
#
# Steps:
# 1. Merges multiple object files into a single object file via partial linking, aka `clang -r`
# 2. If completeStatic is false, merges all static archives into a single object (not archive)
#    file with `clang -r`.
#    This is done for WebEngineCore, but not Pdf, as Pdf uses only object files.
# 3. Creates a single final archive with the above created object and archive object files with
#    `ar -crs`.
#
# The complicated setup is used due to missing response file support in the macOS `ar` archiver.
#
# This is only used for universal macOS builds of QtPdf and QtWebEngineCore.
#
# TODO: This currently loses debug information for static Qt builds, because `clang -r` does NOT
# transfer debug information, but only adds debug link info, see QTBUG-116619.
function(add_intermediate_archive target buildDir completeStatic)
    get_target_property(config ${target} CONFIG)
    get_target_property(arch ${target} ARCH)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    get_target_property(ninjaStamp ${target} NINJA_STAMP)
    string(TOUPPER ${config} cfg)
    set(objects_rsp "${buildDir}/${ninjaTarget}_objects.rsp")
    set(objects_out "${buildDir}/${cmakeTarget}_objects.o")
    if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
        set(deployment_target_arg -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    else()
        unset(deployment_target_arg)
    endif()

    if(NOT completeStatic)
        set(archives_rsp "${buildDir}/${ninjaTarget}_archives.rsp")
        set(archives_out "${buildDir}/${cmakeTarget}_archives.o")
        set(archives_command
            COMMAND clang++ -r -nostdlib -arch ${arch}
            ${deployment_target_arg}
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
            ${deployment_target_arg}
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

# Overrides the cmake archive creation rule with `libtool`, instead of `ar`, because it supports
# response files and that simplifies linking to the Chromium static libraries and object files a
# lot. libtool supports all the input kinds we need, including single or multi-arch object files
# and static libraries, in any combination, which means we don't need to manually merge them or
# lipo them.
macro(qt_webengine_set_apple_archiver_flags)
    if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        set(CMAKE_CXX_ARCHIVE_CREATE
            "libtool -o <TARGET> <LINK_FLAGS> -no_warning_for_no_symbols <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_APPEND
            "libtool -o <TARGET> <LINK_FLAGS> -no_warning_for_no_symbols <TARGET> <OBJECTS>")
    endif()
endmacro()

# Given the <module>_objects.rsp file, creates a new response file with symlinked object files
# that have unique names, to avoid issues with duplicate object names when archiving and running
# dsymutil.
# Uses a custom CMake script to create the symlinks in a separate dir.
function(symlink_renamed_objects_in_new_rsp_file target buildDir output_rsp output_stamp)
    get_target_property(config ${target} CONFIG)
    get_target_property(arch ${target} ARCH)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)
    get_target_property(ninjaStamp ${target} NINJA_STAMP)

    set(objects_rsp "${buildDir}/${ninjaTarget}_objects.rsp")
    set(symlinked_objects_rsp "${buildDir}/${ninjaTarget}_symlinked_objects.rsp")
    set(symlinker_stamp_file "${buildDir}/${ninjaTarget}_symlink_stamp.txt")

    # This is where the symlinks are created.
    set(symlinks_dir "${buildDir}/${ninjaTarget}_symlinked_objects")

    # This is used to make the symlink paths relative to the build dir.
    set(symlink_base_dir "${buildDir}")
    file(MAKE_DIRECTORY "${symlinks_dir}")

    set(symlinker_script
        "${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtWebEngineRspSymlinker.cmake")

    add_custom_command(
        OUTPUT "${symlinker_stamp_file}"
        COMMAND
            "${CMAKE_COMMAND}"
            "-DINPUT_RESPONSE_FILE_PATH=${objects_rsp}"
            "-DOUTPUT_RESPONSE_FILE_PATH=${symlinked_objects_rsp}"
            "-DOUTPUT_SYMLINKS_DIR=${symlinks_dir}"
            "-DSYMLINK_BASE_DIR=${symlink_base_dir}"
            -P
            "${symlinker_script}"
        COMMAND
            ${CMAKE_COMMAND} -E touch "${symlinker_stamp_file}"
        DEPENDS
            "${symlinker_script}"
            "${buildDir}/${ninjaStamp}"
        WORKING_DIRECTORY "${buildDir}/../../.."
        COMMENT "Creating symlink response file for ${cmakeTarget}/${config}/${arch}"
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    set(${output_rsp} "${symlinked_objects_rsp}" PARENT_SCOPE)
    set(${output_stamp} "${symlinker_stamp_file}" PARENT_SCOPE)
endfunction()

# Adds the arch-specific objects from <module>_objects.rsp into a single universal static
# library, using libtool.
# Appends the response file to the STATIC_LIBRARY_OPTIONS property of the module target.
# Used on static macOS and iOS builds for the QtPdf module.
function(add_objects_to_apple_static_library target buildDir)
    get_target_property(config ${target} CONFIG)
    get_target_property(cmakeTarget ${target} CMAKE_TARGET)
    get_target_property(ninjaTarget ${target} NINJA_TARGET)

    symlink_renamed_objects_in_new_rsp_file(
        "${target}" "${buildDir}" symlinked_objects_rsp symlinker_stamp_file)

    # We need a way to cause QtPdf to be recreated when any of the source object files change.
    # The dependency chain is:
    #   Chromium ninja stamp
    #   -> create new rsp with symlinked objects
    #   -> re-create QtPdf static archive
    # CMake doesn't allow to directly model the last step.
    # So there is no way to say that add_library(QtPdf) static should be recreated when the
    # rsp file in STATIC_LIBRARY_OPTIONS changes.
    #
    # To work around that, create an empty OBJECT library target that depends on the
    # symlink stamp file by adding it to OBJECT_DEPENDS, and then link that OBJECT library into
    # the module target. This creates the needed dependency, at the cost of one more build rule
    # to build the empty object file.
    set(sym_objects_target ${target}_sym_objects)
    set(sym_object_fake_cpp_path
        "${buildDir}/${ninjaTarget}_sym_objects_${config}_${arch}.cpp")
    file(GENERATE
        OUTPUT "${sym_object_fake_cpp_path}"
        CONTENT "// Empty source file to drive the ${sym_objects_target} target\n"
    )

    add_library("${sym_objects_target}" OBJECT "${sym_object_fake_cpp_path}")
    set_property(SOURCE "${sym_object_fake_cpp_path}" PROPERTY OBJECT_DEPENDS
        "${symlinker_stamp_file}"
    )

    # This genex construct is used to prevent leakage of the
    # ${sym_objects_target} target into the INTERFACE_LINK_LIBRARIES
    # property of the <module> target in its exported
    # <module>Targets.cmake file, and in the accompanying .prl file.
    set(config_genex "$<CONFIG:${config}>")

    # This prevents the .prl leakage, it always evaluates to true.
    set(skip_walk_genex "$<BOOL:QT_SKIP_WALK_LIBS_PROCESSING>")

    # This prevents the INTERFACE_LINK_LIBRARIES leakage.
    if(CMAKE_VERSION VERSION_LESS "3.26")
        set(build_genex "BUILD_INTERFACE")
    else()
        set(build_genex "BUILD_LOCAL_INTERFACE")
    endif()

    set(link_genex "$<${build_genex}:${sym_objects_target}>")
    set(condition "$<AND:${config_genex},${skip_walk_genex}>")
    set(final_genex "$<${condition}:${link_genex}>")
    target_link_libraries("${cmakeTarget}" PRIVATE "${final_genex}")

    set_property(TARGET ${cmakeTarget} APPEND PROPERTY STATIC_LIBRARY_OPTIONS
        "$<$<CONFIG:${config}>:@${symlinked_objects_rsp}>"
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

# Lipos-s two arch-specific static archives into a single universal static archive.
#
# The resulting lipo-ed static archive is added as an IMPORTED static library, and is linked
# to the module target (e.g. Pdf)
#
# Only used for universal macOS builds.
#
# TODO: Currently broken for universal static macOS builds, because target_link_libraries() will
# not try to merge the lipo-ed static archive with the module static archive. The Qt build will
# succeed, but user projects will fail with linker errors.
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

    # Linking Rust libraries adds an additional personality symbol, which exceeds
    # the compact-unwind limit. Disable it here as a workaround.
    if (QT_FEATURE_webengine_rust_build)
        target_link_options(${cmakeTarget} PRIVATE "-Wl,-no_compact_unwind")
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

function(add_ninja_command)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "" "TARGET;BUILDDIR;MODULE" "OUTPUT;BYPRODUCTS;DEPENDS"
    )
    _qt_internal_validate_all_args_are_parsed(arg)

    string(REPLACE " " ";" ninja_flags "$ENV{NINJAFLAGS}")
    if(CMAKE_VERBOSE_MAKEFILE)
        list(APPEND ninja_flags -v)
    endif()
    list(TRANSFORM arg_OUTPUT PREPEND "${arg_BUILDDIR}/")
    list(TRANSFORM arg_BYPRODUCTS PREPEND "${arg_BUILDDIR}/")
    add_custom_command(
        OUTPUT
            ${arg_OUTPUT}
            ${arg_BUILDDIR}/${arg_TARGET} # use generator expression in CMAKE 3.20
        BYPRODUCTS ${arg_BYPRODUCTS}
        COMMENT "Running ninja for ${arg_TARGET} in ${arg_BUILDDIR}"
        COMMAND ${CMAKE_COMMAND}
            -E env
            "NODEJS_EXECUTABLE=${Nodejs_EXECUTABLE}"
            $<TARGET_FILE:Ninja::ninja>
            ${ninja_flags}
            -C ${arg_BUILDDIR}
            ${arg_TARGET}
        USES_TERMINAL
        VERBATIM
        COMMAND_EXPAND_LISTS
        DEPENDS run_${arg_MODULE}_NinjaReady ${arg_DEPENDS}
    )
endfunction()

function(add_gn_build_artifacts_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "" "CMAKE_TARGET;NINJA_TARGET;BUILDDIR;MODULE;COMPLETE_STATIC;NINJA_STAMP;NINJA_DATA_STAMP;DEPENDS" ""
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
            set(data_stamp_option "")
            if(arg_NINJA_DATA_STAMP)
                set(data_stamp_option NINJA_DATA_STAMP ${arg_NINJA_DATA_STAMP})
            endif()
            add_ninja_target(
                TARGET ${target}
                NINJA_TARGET ${arg_NINJA_TARGET}
                CMAKE_TARGET ${arg_CMAKE_TARGET}
                NINJA_STAMP ${arg_NINJA_STAMP}
                ${data_stamp_option}
                CONFIG ${config}
                ARCH ${arch}
                BUILDDIR ${arg_BUILDDIR}
            )
            add_ninja_command(
                TARGET ${arg_NINJA_TARGET}
                OUTPUT ${stamps}
                BUILDDIR ${arg_BUILDDIR}/${config}/${arch}
                MODULE ${arg_MODULE}
                DEPENDS ${arg_DEPENDS}
            )
            qt_webengine_add_gn_target_to_sbom(${arg_NINJA_TARGET} ${arg_BUILDDIR}/${config}/${arch})
            qt_webengine_add_gn_artifact_relationship_to_sbom(${arg_NINJA_TARGET} ${arg_CMAKE_TARGET})
            add_dependencies(run_${arg_MODULE}_NinjaDone ${target})
            set_target_properties(${arg_CMAKE_TARGET} PROPERTIES
                LINK_DEPENDS ${arg_BUILDDIR}/${config}/${arch}/${arg_NINJA_STAMP}
            )
            if(APPLE)
                if(QT_IS_MACOS_UNIVERSAL AND QT_FEATURE_shared)
                    # For universal macOS builds we create (merge) per-arch intermediate archives
                    # from objects and static archives, to lipo them later, and then link them to
                    # the module target via target_link_libraries().
                    add_intermediate_archive(
                        ${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
                elseif(MACOS AND QT_FEATURE_shared)
                    # For single-arch macOS shared builds, we directly link the response files to
                    # the module target, like for all other platforms below..
                    add_linker_options(
                        ${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
                else()
                    # For macOS/iOS static builds that only support building QtPdf, but not
                    # QtWebEngine, directly add the objects response file to the module target via
                    # STATIC_LIBRARY_OPTIONS. libtool takes care of the lipo-ing. Works for
                    # both single-arch and universal builds.
                    add_objects_to_apple_static_library(
                        ${target} ${arg_BUILDDIR}/${config}/${arch})
                endif()
           else()
                # For all other platforms (Linux, Windows, Android), instead of merging objects,
                # we directly link the response files to the module target.
                add_linker_options(
                    ${target} ${arg_BUILDDIR}/${config}/${arch} ${arg_COMPLETE_STATIC})
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
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.0.0" AND
                    CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0.0")
                target_link_options(${arg_CMAKE_TARGET} PRIVATE -ld_classic)
                set_target_properties(${arg_CMAKE_TARGET} PROPERTIES
                    QT_NO_DISABLE_WARN_DUPLICATE_LIBRARIES TRUE)
            endif()
        endif()
        if(QT_IS_MACOS_UNIVERSAL AND QT_FEATURE_shared)
            add_lipo_command(${target} ${arg_BUILDDIR}/${config})
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
            "${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtBuildGnHelpers.cmake"
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
    # On macOS, force the architecture variable to be populated to avoid issues with ccache
    # picking the wrong object artifacts on CI
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        set(OSX_ARCH_STR ${CMAKE_HOST_SYSTEM_PROCESSOR})
    else()
        list(JOIN CMAKE_OSX_ARCHITECTURES "," OSX_ARCH_STR)
    endif()
    externalproject_add(${ARGN}
        PREFIX      gn
        USES_TERMINAL_BUILD TRUE
        LIST_SEPARATOR ","
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
                   -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                   -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
                   -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
                   -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
                   -DCMAKE_OSX_ARCHITECTURES=${OSX_ARCH_STR}
                   -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
                   -DQT_ALLOW_SYMLINK_IN_PATHS=${QT_ALLOW_SYMLINK_IN_PATHS}
                   -DPython3_EXECUTABLE=${Python3_EXECUTABLE}
                   -DGCC_LEGACY_SUPPORT=${QT_FEATURE_webengine_gcc_legacy_support}
    )
    unset(OSX_ARCH_STR)
endmacro()
