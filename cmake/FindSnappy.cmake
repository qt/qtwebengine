
if(TARGET Snappy::Snappy)
    set(Snappy_FOUND TRUE)
    return()
endif()
find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(Snappy snappy IMPORTED_TARGET)

    if (TARGET PkgConfig::Snappy)
        add_library(Snappy::Snappy INTERFACE IMPORTED)
        target_link_libraries(Snappy::Snappy INTERFACE PkgConfig::Snappy)
        set(Snappy_FOUND TRUE)
    endif()
endif()

if(NOT TARGET Snappy::Snappy)
    find_path(SNAPPY_INCLUDE_DIR NAMES snappy.h)
    find_library(SNAPPY_LIBRARY NAMES snappy)

    if(SNAPPY_LIBRARY AND SNAPPY_INCLUDE_DIR)
        add_library(Snappy::Snappy UNKNOWN IMPORTED)
        set_target_properties(Snappy::Snappy PROPERTIES
            IMPORTED_LOCATION ${SNAPPY_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${SNAPPY_INCLUDE_DIR}
        )
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Snappy REQUIRED_VARS
        SNAPPY_LIBRARY
        SNAPPY_INCLUDE_DIR
    )
endif()
