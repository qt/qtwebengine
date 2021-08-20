if(NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    set(WEBENGINE_ROOT_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()
find_program(Gn_EXECUTABLE NAMES gn PATHS "${WEBENGINE_ROOT_BUILD_DIR}/install/bin" NO_DEFAULT_PATH)
if(NOT QT_HOST_PATH STREQUAL "")
   find_program(Gn_EXECUTABLE NAMES gn PATHS ${QT_HOST_PATH}/${INSTALL_LIBEXECDIR} NO_DEFAULT_PATH)
endif()
# script mode does not have QT_HOST_PATH or INSTALL_LIBEXECDIR instead it uses QT_HOST_GN_PATH
if(NOT QT_HOST_GN_PATH STREQUAL "")
   find_program(Gn_EXECUTABLE NAMES gn PATHS ${QT_HOST_GN_PATH} NO_DEFAULT_PATH)
endif()
find_program(Gn_EXECUTABLE NAMES gn)

if(Gn_EXECUTABLE)
    execute_process(
        COMMAND ${Gn_EXECUTABLE} --version
        OUTPUT_VARIABLE Gn_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

string(REGEX MATCHALL "([1-9]\\.[0-9]\\.[0-9])\\.qtwebengine\\.qt\\.io.*" Gn_QT_VERSION "${Gn_VERSION}")
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

if(Gn_FOUND AND NOT TARGET Gn::gn AND NOT CMAKE_SCRIPT_MODE_FILE)
    add_executable(Gn::gn IMPORTED)
    set_property(TARGET Gn::gn PROPERTY IMPORTED_LOCATION ${Gn_EXECUTABLE})
endif()

include(FeatureSummary)
set_package_properties(Gn PROPERTIES
    URL "https://gn.googlesource.com/gn/"
    DESCRIPTION "Meta-build system"
)
