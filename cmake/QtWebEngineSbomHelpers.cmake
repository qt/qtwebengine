# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Helper functions for SBOM generation. We defer setting up targets until the end of build setup
# because we only want to generate e.g. one SBOM document for WebEngine. This in turn requires
# us to only call sbom.py once for WebEngine.

# Setup data structures used in other calls
function(qt_webengine_sbom_project_begin project_name cmake_target_name)
    if(NOT QT_GENERATE_SBOM)
        return()
    endif()

    set_property(GLOBAL PROPERTY QTWEBENGINE_SBOM_PROJECT_NAME ${project_name})
    set_property(GLOBAL PROPERTY QTWEBENGINE_SBOM_CMAKE_TARGET_NAME ${cmake_target_name})
    set_property(GLOBAL PROPERTY QTWEBENGINE_SBOM_GN_TARGETS "")
    set_property(GLOBAL PROPERTY QTWEBENGINE_SBOM_BUILD_DIRS "")
    set_property(GLOBAL PROPERTY QTWEBENGINE_SBOM_CMAKE_TO_GN_DEPENDENCY_PAIRS "")
endfunction()

# Add a GN target to the list of components contained in an SBOM document
function(qt_webengine_add_gn_target_to_sbom gn_target build_dir)
    if(NOT QT_GENERATE_SBOM)
        return()
    endif()

    get_property(project_name GLOBAL PROPERTY QTWEBENGINE_SBOM_PROJECT_NAME)
    if(NOT DEFINED project_name)
        message(FATAL_ERROR "Call qt_webengine_sbom_project_begin first")
        return()
    endif()

    set_property(GLOBAL APPEND PROPERTY QTWEBENGINE_SBOM_GN_TARGETS ":${gn_target}")
    set_property(GLOBAL APPEND PROPERTY QTWEBENGINE_SBOM_BUILD_DIRS "${build_dir}")
endfunction()

function(qt_webengine_add_gn_artifact_relationship_to_sbom ninja_target cmake_target)
    if(NOT QT_GENERATE_SBOM)
        return()
    endif()

    set_property(GLOBAL APPEND PROPERTY QTWEBENGINE_SBOM_CMAKE_TO_GN_DEPENDENCY_PAIRS
        "${cmake_target};${ninja_target}")
endfunction()

# Join all the targets into (at most) two documents for Pdf / WebEngine
function(qt_webengine_sbom_project_end)
    if(NOT QT_GENERATE_SBOM)
        return()
    endif()

    # We have the situation that qtbase by default does not generate JSON files if the required
    # python dependency spdx-tools is not found.
    # But QtWebEngine requires the spdx-tools package to be available, otherwise we can't generate
    # a tag/value document from the Chromium-generated json file, and then link the Chromium
    # SBOM document to the qtwebengine one.
    # Skip the Chromium SBOM generation if the dependency is not found.
    # A warning or skip message should have already been shown at general configure check time.
    qt_internal_sbom_verify_deps_for_generate_tag_value_spdx_document(
        OUT_VAR_DEPS_FOUND sbom_deps_found
    )
    if(NOT sbom_deps_found)
        return()
    endif()

    get_property(project_name GLOBAL PROPERTY QTWEBENGINE_SBOM_PROJECT_NAME)
    get_property(gn_target_list GLOBAL PROPERTY QTWEBENGINE_SBOM_GN_TARGETS)
    get_property(build_dir_list GLOBAL PROPERTY QTWEBENGINE_SBOM_BUILD_DIRS)
    get_property(cmake_target_name GLOBAL PROPERTY QTWEBENGINE_SBOM_CMAKE_TARGET_NAME)
    get_property(cmake_to_gn_dep_pairs GLOBAL PROPERTY
        QTWEBENGINE_SBOM_CMAKE_TO_GN_DEPENDENCY_PAIRS)

    string(TOLOWER ${project_name} project_name_lower)
    set(chromium_project_name QtWebEngine-Chromium-${project_name})
    string(TOLOWER ${chromium_project_name} chromium_project_name_lower)
    qt_internal_sbom_get_project_supplier_url(supplier_url)
    qt_internal_sbom_compute_project_namespace(doc_namespace SUPPLIER_URL ${supplier_url}
        PROJECT_NAME ${chromium_project_name_lower})
    qt_internal_sbom_compute_project_file_name(output_file_name EXTENSION_JSON
        PROJECT_NAME ${chromium_project_name_lower})
    set(output_file_path ${WEBENGINE_ROOT_BUILD_DIR}/qt_sbom/${output_file_name})

    set(generate_sbom_script_path
        "${CMAKE_CURRENT_BINARY_DIR}/gen_qtwebengine_chromium_sbom_${project_name}-$<CONFIG>.cmake")
    set(chromium_sbom_script
        "${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium/tools/licenses/sbom.py")
    set(generate_sbom_script_contents "
message(STATUS \"Generating Chromium SBOM for ${project_name}...\")
execute_process(
    COMMAND \"${CMAKE_COMMAND}\"
        \"-DSCRIPT_PATH=${chromium_sbom_script}\"
        \"-DGN_TARGET_LIST=${gn_target_list}\"
        \"-DBUILD_DIR_LIST=${build_dir_list}\"
        \"-DQT_HOST_PATH=${QT_HOST_PATH}\"
        \"-DQT6_HOST_INFO_LIBEXECDIR=${QT6_HOST_INFO_LIBEXECDIR}\"
        \"-DQT6_HOST_INFO_BINDIR=${QT6_HOST_INFO_BINDIR}\"
        -DPACKAGE_ID=${project_name}
        -DDOC_NAMESPACE=${doc_namespace}
        \"-DOUTPUT=${output_file_path}\"
        \"-DPython3_EXECUTABLE=${Python3_EXECUTABLE}\"
        -P \"${WEBENGINE_ROOT_SOURCE_DIR}/cmake/QtGnSbom.cmake\"
    WORKING_DIRECTORY \"${WEBENGINE_ROOT_BUILD_DIR}\"
    COMMAND_ERROR_IS_FATAL ANY
)
file(INSTALL \"${output_file_path}\" DESTINATION \"${QT6_INSTALL_PREFIX}/${INSTALL_SBOMDIR}\")
message(STATUS \"Done generating Chromium SBOM for ${project_name}.\")
")
    file(GENERATE OUTPUT "${generate_sbom_script_path}" CONTENT "${generate_sbom_script_contents}")
    qt_internal_sbom_add_cmake_include_step(STEP BEGIN INCLUDE_PATH "${generate_sbom_script_path}")
    qt_internal_sbom_generate_tag_value_spdx_document(
        OPERATION_ID ${chromium_project_name_lower}
        INPUT_JSON_FILE_PATH "${output_file_path}"
        OUT_VAR_OUTPUT_FILE_NAME external_output_file_name
    )
    # Reference to external document.
    qt_internal_sbom_get_external_document_ref_spdx_id(${chromium_project_name_lower} document_ref_spdx_id)
    set(external_package_spdx_id "SPDXRef-${chromium_project_name}-Internal-Components")
    qt_internal_sbom_add_external_reference(
        EXTERNAL_DOCUMENT_FILE_PATH "sbom/${external_output_file_name}"
        EXTERNAL_DOCUMENT_SPDX_ID "${document_ref_spdx_id}"
        EXTERNAL_PACKAGE_SPDX_ID "${external_package_spdx_id}"
    )

    while(NOT "${cmake_to_gn_dep_pairs}" STREQUAL "")
        list(POP_FRONT cmake_to_gn_dep_pairs cmake_target gn_target)
        qt_internal_sbom_get_target_spdx_id("${cmake_target}" cmake_spdx_id)
        qt_internal_sbom_get_sanitized_spdx_id(gn_spdx_id
            "SPDXRef-${chromium_project_name}-${gn_target}")
        set(relationship "${cmake_spdx_id} CONTAINS ${document_ref_spdx_id}:${gn_spdx_id}")
        qt_internal_extend_target(${cmake_target}
            SBOM_RELATIONSHIPS
            "${relationship}"
        )
    endwhile()
endfunction()

# Shims for SBOM commands that may be missing < 6.9
if(NOT COMMAND qt_internal_sbom_begin_qt_repo_project)
    function(qt_internal_sbom_begin_qt_repo_project)
    endfunction()
    function(qt_internal_sbom_end_qt_repo_project)
    endfunction()
    function(qt_internal_sbom_add_files)
    endfunction()
endif()
