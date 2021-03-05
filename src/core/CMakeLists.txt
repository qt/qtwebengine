cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED WEBENGINE_ROOT_SOURCE_DIR)
    get_filename_component(WEBENGINE_ROOT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()
if (NOT DEFINED WEBENGINE_ROOT_BUILD_DIR)
    get_filename_component(WEBENGINE_ROOT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/../.." ABSOLUTE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBENGINE_ROOT_SOURCE_DIR}/cmake")
set(buildDir "${CMAKE_CURRENT_BINARY_DIR}/debug")

include(${WEBENGINE_ROOT_SOURCE_DIR}/cmake/Functions.cmake)

assertRunAsTopLevelBuild(TRUE)

add_subdirectory(api)

get_qt_features(featureList webengine)
message(${featureList})

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Gui Widgets Network OpenGL OpenGLWidgets Quick Qml QuickTest Test)
find_package(Gn REQUIRED)
find_package(Ninja REQUIRED)
find_package(Python2 REQUIRED)

get_target_property(qtWebEngineProcessName Qt6::WebEngineCore QTWEBENGINEPROCESS_NAME)

include(generator.cmake)
configure_file(${WEBENGINE_ROOT_SOURCE_DIR}/src/core/configure/BUILD.toolchain.gn.in ${buildDir}/toolchain/BUILD.gn @ONLY)
configure_file(${WEBENGINE_ROOT_SOURCE_DIR}/src/core/configure/BUILD.root.gn.in ${buildDir}/BUILD.gn @ONLY)

get_target_property(gnCmd Gn::gn IMPORTED_LOCATION)

set(gnArg gen ${buildDir})

list(APPEND gnArg
     --script-executable=${Python2_EXECUTABLE}
     --root=${WEBENGINE_ROOT_SOURCE_DIR}/src/3rdparty/chromium)

list(APPEND gnArgArg
     use_qt=true
     init_stack_vars=false
     is_component_build=false
     is_shared=true
     enable_debugallocation=false
     enable_media_remoting=false
     enable_message_center=false
     enable_nacl=false
     enable_remoting=false
     enable_reporting=false
     enable_resource_allowlist_generation=false
     enable_swiftshader=false
     enable_swiftshader_vulkan=false
     angle_enable_swiftshader=false
     enable_web_speech=false
     enable_widevine=true
     forbid_non_component_debug_builds=false
     has_native_accessibility=false
     safe_browsing_mode=0
     skia_use_dawn=false
     toolkit_views=false
     treat_warnings_as_errors=false
     use_allocator_shim=false
     use_allocator="none"
     use_custom_libcxx=false
     chrome_pgo_phase=0
     enable_hangout_services_extension=false
     optimize_webui=false
     enable_js_type_check=false
     v8_use_external_startup_data=false
     strip_absolute_paths_from_debug_symbols=false
     use_jumbo_build=true
     jumbo_file_merge_limit=8
     jumbo_build_excluded=["browser"]
     enable_precompiled_headers=false
     is_official_build=false
     is_unsafe_developer_build=false
     from_here_uses_location_builtins=false
     is_debug=true use_debug_fission=false
     blink_symbol_level=0
     remove_v8base_debug_symbols=true
     use_cups=false
     use_gio=false
     use_gnome_keyring=false
     use_udev=true
     use_bundled_fontconfig=false
     use_sysroot=false
     enable_session_service=false
     is_cfi=false
     use_ozone=true
     use_x11=false
     ozone_auto_platforms=false
     ozone_platform_headless=false
     ozone_platform_external=true
     ozone_platform="qt"
     ozone_extra_path="${CMAKE_CURRENT_LIST_DIR}/ozone/ozone_extra.gni"
     use_gold=false
     use_lld=true
     is_clang=false
     custom_toolchain="${buildDir}/toolchain:target"
     host_toolchain="${buildDir}/toolchain:host"
     host_cpu="x64"
     pkg_config="pkg-config"
     host_pkg_config="/usr/bin/pkg-config"
     use_system_zlib=true
     use_system_minizip=true
     pdfium_use_system_zlib=true
     use_system_libpng=true
     pdfium_use_system_libpng=true
     use_system_libjpeg=true
     use_system_freetype=true
     use_system_harfbuzz=false
     use_glib=false
     enable_basic_printing=false
     enable_print_preview=false
     enable_pdf=false
     enable_plugins=false
     enable_spellcheck=false
     enable_webrtc=false
     proprietary_codecs=false
     enable_extensions=false
     use_kerberos=false
     use_pulseaudio=true
     use_alsa=true
     ozone_platform_x11=true
     use_xkbcommon=true
     use_xscrnsaver=true
     rtc_use_x11=true
     use_system_libevent=true
     use_system_libwebp=true
     use_system_opus=true
     use_system_snappy=true
     use_system_libvpx=true
     use_system_re2=true
     use_system_lcms2=true
     qtwebengine_target="${buildDir}:QtWebEngineCore")
list(JOIN gnArgArg " " gnArgArg)
list(APPEND gnArg "--args=${gnArgArg}")

message("configure with gn")
execute_process(
    COMMAND ${gnCmd} ${gnArg}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE gnResult
    OUTPUT_VARIABLE gnOutput
    ERROR_VARIABLE gnError
    )

if(NOT gnResult EQUAL 0)
    message(FATAL_ERROR "Calling gn - FAILED ! \n${gnOutput}\n${gnError}")
endif()

message("CMD: ${gnCmd}\nARGS: ${gnArg}\nOUTPUT: ${gnOutput}")

include(${buildDir}/QtWebEngineCore.cmake)

add_custom_command(
    OUTPUT QtWebEngineCore.stamp
    WORKING_DIRECTORY ${buildDir}
    COMMAND Ninja::ninja
            $ENV{NINJAFLAGS}
            -C ${buildDir}
            QtWebEngineCore
    USES_TERMINAL
    VERBATIM
)
add_custom_target(ninja ALL DEPENDS QtWebEngineCore.stamp)

add_dependencies(WebEngineCore ninja)
target_include_directories(WebEngineCore PRIVATE ${buildDir}/gen)

# there is no syncqt on this module so use includes provided for gn, which should be identical
target_include_directories(WebEngineCore PRIVATE ${GN_INCLUDES_IN})
add_library(chromiumObjects OBJECT IMPORTED GLOBAL)
set_property(TARGET chromiumObjects PROPERTY IMPORTED_OBJECTS ${NINJA_OBJECTS})
target_link_libraries(WebEngineCore PRIVATE chromiumObjects)
target_link_libraries(WebEngineCore PRIVATE ${NINJA_ARCHIVES})
target_link_libraries(WebEngineCore PUBLIC ${NINJA_LIBS})

#
# RESOURCES
#

#TODO: use simply filter / globbing-expressions
set(localeList am ar bg bn ca cs da de el en-GB en-US es-419 es et fa fi fil fr
    gu he hi hr hu id it ja kn ko lt lv ml mr ms nb nl pl pt-BR pt-PT ro ru sk
    sl sr sv sw ta te th tr uk vi zh-CN zh-TW)

foreach(loc ${localeList})
    get_filename_component(locSourcePath ${buildDir}/qtwebengine_locales/${loc}.pak ABSOLUTE)
    LIST(APPEND localeFiles ${locSourcePath})
endforeach()

set(resourceList qtwebengine_resources.pak
                  qtwebengine_resources_100p.pak
                  qtwebengine_resources_200p.pak
                  qtwebengine_devtools_resources.pak)

foreach(res ${resourceList})
    get_filename_component(resSourcePath ${buildDir}/${res} ABSOLUTE)
    LIST(APPEND resourceFiles ${resSourcePath})
endforeach()


get_filename_component(icuFiles ${buildDir}/icudtl.dat ABSOLUTE)

install(FILES ${localeFiles} DESTINATION ${INSTALL_DATADIR}/translations/qtwebengine_locales)
install(FILES ${resourceFiles} DESTINATION ${INSTALL_DATADIR}/resources)
install(FILES ${icuFiles} DESTINATION ${INSTALL_DATADIR}/resources)

