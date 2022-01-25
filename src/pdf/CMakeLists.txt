cmake_minimum_required(VERSION 3.19)
find_package(Ninja 1.7.2 REQUIRED)
find_package(Python2 2.7.5 REQUIRED)
find_package(Nodejs 10.19 REQUIRED)
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
        qpdfdestination.cpp qpdfdestination.h qpdfdestination_p.h
        qpdfdocument.cpp qpdfdocument.h qpdfdocument_p.h
        qpdfdocumentrenderoptions.h
        qpdflinkmodel.cpp qpdflinkmodel_p.h qpdflinkmodel_p_p.h
        qpdfpagenavigation.cpp qpdfpagenavigation.h
        qpdfpagerenderer.cpp qpdfpagerenderer.h
        qpdfsearchmodel.cpp qpdfsearchmodel.h qpdfsearchmodel_p.h
        qpdfsearchresult.cpp qpdfsearchresult.h qpdfsearchresult_p.h
        qpdfselection.cpp qpdfselection.h qpdfselection_p.h
        qtpdfglobal.h
        qpdfnamespace.h
    INCLUDE_DIRECTORIES
        ../3rdparty/chromium
    DEFINES
        QT_BUILD_PDF_LIB
        NOMINMAX
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
)

add_subdirectory(plugins/imageformats/pdf)

##
#  PDF DOCS
##

qt_internal_add_docs(Pdf
    doc/qtpdf.qdocconf
)


##
#   TOOLCHAIN SETUP
##

if(LINUX)
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
            SOURCES DEFINES CXX_COMPILE_OPTIONS C_COMPILE_OPTIONS INCLUDES MOC_PATH
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
          enable_remoting=false
          enable_resource_allowlist_generation=false
          enable_web_speech=false
          chrome_pgo_phase=0
        )

        if(LINUX)
          list(APPEND gnArgArg
              use_x11=false
              is_cfi=false
              ozone_auto_platforms=false
              use_gnome_keyring=false)
        endif()
        if(MACOS)
          list(APPEND gnArgArg angle_enable_vulkan=false)
        endif()
        if(WIN32)
          list(APPEND gnArgArg
              ninja_use_custom_environment_files=false
              safe_browsing_mode=0
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

        add_gn_command(
            CMAKE_TARGET Pdf
            NINJA_TARGETS QtPdf
            GN_TARGET ${buildGn}
            GN_ARGS "${gnArgArg}"
            BUILDDIR ${buildDir}/${config}/${arch}
            MODULE pdf
        )

    endforeach()
    create_cxx_configs(Pdf ${arch})
endforeach()


##
#  PDF SETUP
##

set(arch ${CMAKE_SYSTEM_PROCESSOR})
target_include_directories(Pdf PRIVATE ${buildDir}/$<CONFIG>/${arch}/gen)
add_gn_build_aritfacts_to_target(Pdf QtPdf pdf ${buildDir} TRUE)
add_dependencies(Pdf run_pdf_NinjaDone)

