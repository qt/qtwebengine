find_program(Nodejs_EXECUTABLE NAMES node nodejs)

if(Nodejs_EXECUTABLE)
    execute_process(
        COMMAND ${Nodejs_EXECUTABLE} --version
        OUTPUT_VARIABLE Nodejs_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

string (REGEX MATCHALL "([1-9][0-9])\..*" Nodejs_VERSION "${Nodejs_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nodejs
    FOUND_VAR
        Nodejs_FOUND
    VERSION_VAR
        Nodejs_VERSION
    REQUIRED_VARS
        Nodejs_EXECUTABLE
)

if(Nodejs_FOUND AND NOT TARGET Nodejs::Nodejs)
    add_executable(Nodejs::Nodejs IMPORTED)
    set_target_properties(Nodejs::Nodejs PROPERTIES IMPORTED_LOCATION "${Nodejs_EXECUTABLE}")
endif()

include(FeatureSummary)
set_package_properties(Nodejs PROPERTIES
    URL "https://nodejs.org/"
    DESCRIPTION "JavaScript runtime environment that runs on the V8 engine"
)
