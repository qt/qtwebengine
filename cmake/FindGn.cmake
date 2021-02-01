if (NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    set(WEBENGINE_ROOT_BUILD_DIR  ${CMAKE_CURRENT_BINARY_DIR}/../..)
endif()

set(CMAKE_PROGRAM_PATH ${WEBENGINE_ROOT_BUILD_DIR}/install/bin)
find_program(Gn_EXECUTABLE NAMES gn)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    Gn
    REQUIRED_VARS Gn_EXECUTABLE)

if(Gn_FOUND AND NOT TARGET Gn::gn)
    add_executable(Gn::gn IMPORTED)
    set_property(TARGET Gn::gn PROPERTY IMPORTED_LOCATION ${Gn_EXECUTABLE})
endif()

include(FeatureSummary)
set_package_properties(Gn PROPERTIES
    URL "https://gn.googlesource.com/gn/"
    DESCRIPTION "Meta-build system"
)
