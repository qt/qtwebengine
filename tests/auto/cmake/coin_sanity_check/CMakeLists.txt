# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Unit tests for qt_webengine_configure_check_coin()

get_filename_component(helpers_file
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/QtConfigureHelpers.cmake" ABSOLUTE)
set(white_list_file "${CMAKE_CURRENT_SOURCE_DIR}/test.nodes")
set(run_check "${CMAKE_CURRENT_SOURCE_DIR}/run_check.cmake")

function(add_coin_sanity_test name)
    cmake_parse_arguments(arg "" "PLATFORM;BRANCH;WEBENGINE;PDF;PASS;FAIL" "" ${ARGN})
    set(env --unset=COIN_PLATFORM_ID --unset=TESTED_MODULE_BRANCH_COIN)
    if(arg_PLATFORM)
        list(APPEND env "COIN_PLATFORM_ID=${arg_PLATFORM}")
    endif()
    if(arg_BRANCH)
        list(APPEND env "TESTED_MODULE_BRANCH_COIN=${arg_BRANCH}")
    endif()
    set(test_name coin_sanity_check_${name})
    add_test(NAME ${test_name}
        COMMAND "${CMAKE_COMMAND}" -E env ${env}
            "${CMAKE_COMMAND}"
                "-DHELPERS_FILE=${helpers_file}"
                "-DWHITE_LIST_FILE=${white_list_file}"
                "-DWEBENGINE_CHECK=${arg_WEBENGINE}"
                "-DPDF_CHECK=${arg_PDF}"
                -P "${run_check}"
    )
    if(arg_PASS)
        set_tests_properties(${test_name} PROPERTIES PASS_REGULAR_EXPRESSION "${arg_PASS}")
    endif()
    if(arg_FAIL)
        set_tests_properties(${test_name} PROPERTIES FAIL_REGULAR_EXPRESSION "${arg_FAIL}")
    endif()
endfunction()

set(any_issue "CMake (Error|Warning)")
set(passed "Coin sanity check passed")
set(error "Coin sanity check failed")

add_coin_sanity_test(match_release PLATFORM node-both BRANCH 6.140 WEBENGINE ON PDF ON
    PASS "${passed}" FAIL "${any_issue}")
add_coin_sanity_test(match_dev PLATFORM node-both BRANCH dev WEBENGINE ON PDF ON
    PASS "${passed}" FAIL "${any_issue}")
add_coin_sanity_test(match_none PLATFORM node-none BRANCH 6.140 WEBENGINE OFF PDF OFF
    PASS "${passed}" FAIL "${any_issue}")
add_coin_sanity_test(match_more_than_expected PLATFORM node-pdf BRANCH 6.140 WEBENGINE ON PDF ON
    PASS "${passed}" FAIL "${any_issue}")
add_coin_sanity_test(mismatch_webengine_dev PLATFORM node-both BRANCH dev WEBENGINE OFF PDF ON
    PASS "${error}"
    FAIL "${passed}")
add_coin_sanity_test(mismatch_webengine_release PLATFORM node-both BRANCH 6.140 WEBENGINE OFF PDF ON
    PASS "${error}"
    FAIL "${passed}")
add_coin_sanity_test(unknown_dev PLATFORM node-new BRANCH dev WEBENGINE ON PDF ON
    PASS "Undefined coin sanity check"
    FAIL "${passed}")

