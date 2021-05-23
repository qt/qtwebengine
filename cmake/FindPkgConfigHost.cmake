# this is just simply pkg config wrapper to pass executable path to gn

if(CMAKE_CROSSCOMPILING)
   # find pkg-config, use PKG_CONFIG_HOST if set
   if((NOT PKG_CONFIG_HOST_EXECUTABLE) AND (NOT "$ENV{PKG_CONFIG_HOST}" STREQUAL ""))
       set(PKG_CONFIG_HOST_EXECUTABLE "$ENV{PKG_CONFIG_HOST}" CACHE FILEPATH "pkg-config host executable")
   endif()

   find_program(PKG_CONFIG_HOST_EXECUTABLE NAMES "pkg-config" DOC "pkg-config executable"
       NO_SYSTEM_ENVIRONMENT_PATH
       NO_CMAKE_FIND_ROOT_PATH
   )

   if(PKG_CONFIG_HOST_EXECUTABLE)
       mark_as_advanced(PKG_CONFIG_HOST_EXECUTABLE)
       execute_process(COMMAND ${PKG_CONFIG_HOST_EXECUTABLE} --version
           OUTPUT_VARIABLE PKG_CONFIG_HOST_VERSION_STRING
           OUTPUT_STRIP_TRAILING_WHITESPACE
           ERROR_QUIET)
   endif ()

   include(FindPackageHandleStandardArgs)
   find_package_handle_standard_args(PkgConfigHost
       FOUND_VAR PkgConfigHost_FOUND
       VERSION_VAR PKG_CONFIG_HOST_VERSION_STRING
       REQUIRED_VARS PKG_CONFIG_HOST_EXECUTABLE
   )
else() # if not corss build simply wrap FindPkgConfig
   find_package(PkgConfig)
   if(PKG_CONFIG_FOUND)
       include(FindPackageHandleStandardArgs)
       set(PKG_CONFIG_HOST_VERSION ${PKG_CONFIG_VERSION})
       set(PKG_CONFIG_HOST_EXECUTABLE ${PKG_CONFIG_EXECUTABLE})
       find_package_handle_standard_args(PkgConfigHost
           FOUND_VAR PkgConfigHost_FOUND
           VERSION_VAR PKG_CONFIG_HOST_VERSION_STRING
           REQUIRED_VARS PKG_CONFIG_HOST_EXECUTABLE
       )
  endif()
endif()
