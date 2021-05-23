if (NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    set(WEBENGINE_ROOT_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_program(Ninja_EXECUTABLE NAMES ninja ninja-build PATHS "${WEBENGINE_ROOT_BUILD_DIR}/install/bin" NO_DEFAULT_PATH)
find_program(Ninja_EXECUTABLE NAMES ninja ninja-build)

if(Ninja_EXECUTABLE)
    execute_process(
        COMMAND ${Ninja_EXECUTABLE} --version
        OUTPUT_VARIABLE Ninja_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Ninja
    REQUIRED_VARS Ninja_EXECUTABLE
    VERSION_VAR Ninja_VERSION)

if(Ninja_FOUND AND NOT TARGET Ninja::ninja)
    add_executable(Ninja::ninja IMPORTED)
    set_property(TARGET Ninja::ninja PROPERTY IMPORTED_LOCATION ${Ninja_EXECUTABLE})
endif()

include(FeatureSummary)
set_package_properties(Ninja PROPERTIES
    URL "https://ninja-build.org/"
    DESCRIPTION "Build tool"
)
