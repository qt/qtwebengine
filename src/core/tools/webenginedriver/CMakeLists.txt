# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WIN32)
    set(WEBENGINEDRIVER_EXECUTABLE webenginedriver.exe)
else()
    set(WEBENGINEDRIVER_EXECUTABLE webenginedriver)
endif()
set(WEBENGINEDRIVER_EXECUTABLE ${WEBENGINEDRIVER_EXECUTABLE} PARENT_SCOPE)

if(QT_FEATURE_webenginedriver)
    get_install_config(config)
    get_architectures(archs)
    list(GET archs 0 arch)

    ##
    #  DOCS
    ##
    add_code_attributions_target(
        TARGET generate_webenginedriver_attributions
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/webenginedriver_attributions.qdoc
        GN_TARGET //chrome/test/chromedriver:chromedriver_server
        EXTRA_THIRD_PARTY_DIRS
            third_party/selenium-atoms/sizzle
            third_party/selenium-atoms/wgxpath
            third_party/selenium-atoms/closure-lib
        FILE_TEMPLATE ../../doc/about_credits.tmpl
        ENTRY_TEMPLATE ../../doc/about_credits_entry.tmpl
        BUILDDIR ${buildDir}/${config}/${arch}
    )
    add_dependencies(generate_webenginedriver_attributions run_core_GnDone)
    add_dependencies(prepare_docs_WebEngineCore generate_webenginedriver_attributions)

    ##
    #  INSTALL
    ##
    install(
        PROGRAMS ${buildDir}/${config}/${arch}/${WEBENGINEDRIVER_EXECUTABLE}
        CONFIGURATIONS ${config}
        RUNTIME DESTINATION "${INSTALL_LIBEXECDIR}"
    )
    if(NOT QT_WILL_INSTALL)
        add_custom_target(copy-webenginedriver
            ALL
            DEPENDS ${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/${WEBENGINEDRIVER_EXECUTABLE}
        )
        add_custom_command(
            OUTPUT ${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}/${WEBENGINEDRIVER_EXECUTABLE}
            COMMAND ${CMAKE_COMMAND} -E copy ${buildDir}/${config}/${arch}/${WEBENGINEDRIVER_EXECUTABLE}
                    ${QT_BUILD_DIR}/${INSTALL_LIBEXECDIR}
            DEPENDS
                WebEngineCore
                ${buildDir}/${config}/${arch}/${WEBENGINEDRIVER_EXECUTABLE}
            USES_TERMINAL
        )
    endif()
endif()
