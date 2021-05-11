if (NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    set(WEBENGINE_ROOT_BUILD_DIR  ${CMAKE_CURRENT_BINARY_DIR}/../..)
endif()

set(CMAKE_PROGRAM_PATH ${WEBENGINE_ROOT_BUILD_DIR}/install/bin)

find_program(Gn_EXECUTABLE NAMES gn)

if(Gn_EXECUTABLE)
    execute_process(
        COMMAND ${Gn_EXECUTABLE} --version
        OUTPUT_VARIABLE Gn_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

string(REGEX MATCHALL "([1-9]\.[0-9]\.[0-9])\.qtwebengine\.qt\.io.*" Gn_QT_VERSION "${Gn_VERSION}")
if("${Gn_QT_VERSION}")
    set(Gn_VERSION "${Gn_QT_VERSION}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Gn
    FOUND_VAR
        Gn_FOUND
    VERSION_VAR
        Gn_VERSION
    REQUIRED_VARS
        Gn_EXECUTABLE
)

if(Gn_FOUND AND NOT TARGET Gn::gn)
    add_executable(Gn::gn IMPORTED)
    set_property(TARGET Gn::gn PROPERTY IMPORTED_LOCATION ${Gn_EXECUTABLE})
endif()

include(FeatureSummary)
set_package_properties(Gn PROPERTIES
    URL "https://gn.googlesource.com/gn/"
    DESCRIPTION "Meta-build system"
)
