# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.19)
find_package(Ninja 1.7.2 REQUIRED)
find_package(Nodejs 14.19 REQUIRED)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    create_pkg_config_host_wrapper(${CMAKE_CURRENT_BINARY_DIR})
endif()

set(buildDir "${CMAKE_CURRENT_BINARY_DIR}")

##
#  PDF MODULE
##

qt_internal_add_module(Pdf
    SOURCES
        qpdfbookmarkmodel.cpp qpdfbookmarkmodel.h
        qpdfdocument.cpp qpdfdocument.h qpdfdocument_p.h
        qpdfdocumentrenderoptions.h
        qpdffile.cpp qpdffile_p.h
        qpdflink.cpp qpdflink.h qpdflink_p.h
        qpdflinkmodel.cpp qpdflinkmodel.h qpdflinkmodel_p.h
        qpdfpagenavigator.cpp qpdfpagenavigator.h
        qpdfpagerenderer.cpp qpdfpagerenderer.h
        qpdfsearchmodel.cpp qpdfsearchmodel.h qpdfsearchmodel_p.h
        qpdfselection.cpp qpdfselection.h qpdfselection_p.h
        qtpdfglobal.h
    INCLUDE_DIRECTORIES
        ../3rdparty/chromium
    DEFINES
        QT_BUILD_PDF_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    GENERATE_CPP_EXPORTS
    GENERATE_PRIVATE_CPP_EXPORTS
)

add_subdirectory(plugins/imageformats/pdf)

get_install_config(config)
get_architectures(archs)
list(GET archs 0 arch)

##
#  PDF DOCS
##

qt_internal_add_docs(Pdf
    doc/qtpdf.qdocconf
)

add_code_attributions_target(
    TARGET generate_pdf_attributions
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pdf_attributions.qdoc
    GN_TARGET :QtPdf
    FILE_TEMPLATE doc/about_credits.tmpl
    ENTRY_TEMPLATE doc/about_credits_entry.tmpl
    BUILDDIR ${buildDir}/${config}/${arch}
)
add_dependencies(generate_pdf_attributions run_pdf_GnDone)
add_dependencies(docs generate_pdf_attributions)

##
#   TOOLCHAIN SETUP
##

if(LINUX OR MINGW)
    setup_toolchains()
endif()

##
#   GN BUILD SETUP
##

addSyncTargets(pdf)

get_configs(configs)
get_architectures(archs)
foreach(arch ${archs})
    foreach(config ${configs})

        ##
        #   BULID.gn SETUP
        ##

        set(buildGn pdf_${config}_${arch})
        add_gn_target(${buildGn} ${config} ${arch}
            SOURCES DEFINES CXX_COMPILE_OPTIONS C_COMPILE_OPTIONS
            INCLUDES MOC_PATH PNG_INCLUDES JPEG_INCLUDES HARFBUZZ_INCLUDES
            FREETYPE_INCLUDES ZLIB_INCLUDES
        )
        resolve_target_includes(gnIncludes Pdf)
        get_forward_declaration_macro(forwardDeclarationMacro)

        extend_gn_target(${buildGn}
            INCLUDES
                ${gnIncludes}
        )

        ##
        #  GN PARAMETERS
        ##

        unset(gnArgArg)
        append_build_type_setup(gnArgArg)
        append_compiler_linker_sdk_setup(gnArgArg)
        append_sanitizer_setup(gnArgArg)
        append_toolchain_setup(gnArgArg)
        append_pkg_config_setup(gnArgArg)

        list(APPEND gnArgArg
          qtwebengine_target="${buildDir}/${config}/${arch}:QtPdf"
          qt_libpng_config="${buildDir}/${config}/${arch}:qt_libpng_config"
          qt_libjpeg_config="${buildDir}/${config}/${arch}:qt_libjpeg_config"
          qt_harfbuzz_config="${buildDir}/${config}/${arch}:qt_harfbuzz_config"
          qt_freetype_config="${buildDir}/${config}/${arch}:qt_freetype_config"
          enable_swiftshader=false
          enable_swiftshader_vulkan=false
          angle_enable_swiftshader=false
          dawn_use_swiftshader=false
          use_dawn=false
          enable_ipc_fuzzer=false
          enable_remoting=false
          enable_resource_allowlist_generation=false
          enable_web_speech=false
          chrome_pgo_phase=0
          strip_absolute_paths_from_debug_symbols=false
        )

        if(LINUX OR ANDROID)
            list(APPEND gnArgArg
                is_cfi=false
                ozone_auto_platforms=false
                use_gnome_keyring=false
                enable_vr=false
                use_ml_inliner=false
            )
            extend_gn_list(gnArgArg
                ARGS use_system_icu
                CONDITION QT_FEATURE_webengine_system_icu
            )
            extend_gn_list(gnArgArg
                ARGS use_system_libopenjpeg2
                CONDITION QT_FEATURE_webengine_system_libopenjpeg2
            )
        endif()
        if(MACOS)
            list(APPEND gnArgArg angle_enable_vulkan=false)
        endif()
        if(IOS)
            extend_gn_list(gnArgArg
                ARGS enable_ios_bitcode
                CONDITION QT_FEATURE_pdf_bitcode
            )
        endif()
        if(WIN32)
          list(APPEND gnArgArg
              ninja_use_custom_environment_files=false
              safe_browsing_mode=0
          )
          extend_gn_list(gnArgArg
              ARGS qt_uses_static_runtime
              CONDITION QT_FEATURE_pdf_static_runtime
          )
        endif()

        extend_gn_list(gnArgArg
            ARGS pdf_enable_v8
            CONDITION QT_FEATURE_pdf_v8
        )
        extend_gn_list(gnArgArg
            ARGS pdf_enable_xfa
            CONDITION QT_FEATURE_pdf_xfa
        )
        extend_gn_list(gnArgArg
            ARGS pdf_enable_xfa_bmp
            CONDITION QT_FEATURE_pdf_xfa_bmp
        )
        extend_gn_list(gnArgArg
            ARGS pdf_enable_xfa_gif
            CONDITION QT_FEATURE_pdf_xfa_gif
        )
        extend_gn_list(gnArgArg
            ARGS pdf_enable_xfa_png
            CONDITION QT_FEATURE_pdf_xfa_png
        )
        extend_gn_list(gnArgArg
            ARGS pdf_enable_xfa_tiff
            CONDITION QT_FEATURE_pdf_xfa_tiff
        )
        extend_gn_list(gnArgArg
            ARGS pdfium_use_system_zlib
            CONDITION QT_FEATURE_webengine_system_zlib
        )
        extend_gn_list(gnArgArg
            ARGS pdfium_use_system_libpng use_system_libpng
            CONDITION QT_FEATURE_webengine_system_libpng
        )
        extend_gn_list(gnArgArg
            ARGS pdfium_use_qt_libpng
            CONDITION QT_FEATURE_webengine_qt_libpng
        )
        extend_gn_list(gnArgArg
            ARGS pdfium_use_system_libtiff
            CONDITION QT_FEATURE_webengine_system_libtiff
        )
        extend_gn_list(gnArgArg
            ARGS use_qt_libjpeg
            CONDITION QT_FEATURE_webengine_qt_libjpeg
        )
        extend_gn_list(gnArgArg
            ARGS use_qt_harfbuzz
            CONDITION QT_FEATURE_webengine_qt_harfbuzz
        )
        extend_gn_list(gnArgArg
            ARGS use_qt_freetype
            CONDITION QT_FEATURE_webengine_qt_freetype
        )

        add_gn_command(
            CMAKE_TARGET Pdf
            NINJA_TARGETS QtPdf
            GN_TARGET ${buildGn}
            GN_ARGS ${gnArgArg}
            BUILDDIR ${buildDir}/${config}/${arch}
            MODULE pdf
        )

    endforeach()
    create_cxx_configs(Pdf ${arch})
endforeach()


##
#  PDF SETUP
##

get_architectures(archs)
list(GET archs 0 arch)
target_include_directories(Pdf PRIVATE ${buildDir}/$<CONFIG>/${arch}/gen)
add_gn_build_aritfacts_to_target(
    CMAKE_TARGET Pdf
    NINJA_TARGET QtPdf
    MODULE pdf
    BUILDDIR ${buildDir}
    COMPLETE_STATIC TRUE
    NINJA_STAMP QtPdf.stamp
)
add_dependencies(Pdf run_pdf_NinjaDone)

