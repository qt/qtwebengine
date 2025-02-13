# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_CONFIGURE_RUNNING)
   function(qt_webengine_set_version)
   endfunction()
endif()

qt_webengine_set_version(cmake ${QT_SUPPORTED_MIN_CMAKE_VERSION_FOR_BUILDING_WEBENGINE})
qt_webengine_set_version(ninja 1.7.2)
qt_webengine_set_version(python3 3.8)
qt_webengine_set_version(nodejs 14.9)
qt_webengine_set_version(nss 3.26)
qt_webengine_set_version(gcc 10.0)
qt_webengine_set_version(glib 2.32.0)
qt_webengine_set_version(glibc 2.16)
qt_webengine_set_version(harfbuzz 4.3.0)
qt_webengine_set_version(libpng 1.6.0)
qt_webengine_set_version(libtiff 4.2.0)
qt_webengine_set_version(re2 11.0.0)
qt_webengine_set_version(icu 70)
qt_webengine_set_version(opus 1.3.1)
qt_webengine_set_version(vpx 1.10.0)
qt_webengine_set_version(libavutil 58.29.100)
qt_webengine_set_version(libavcodec 60.31.102)
qt_webengine_set_version(libavformat 60.16.100)
qt_webengine_set_version(windows_sdk 26100) # we only care about minor number "10.0.26100.0"

if(QT_CONFIGURE_RUNNING)
    function(qt_webengine_configure_check)
    endfunction()
    function(qt_webengine_configure_check_for_ulimit)
    endfunction()
    function(qt_webengine_get_windows_sdk_version)
    endfunction()
else()
    find_package(Ninja ${QT_CONFIGURE_CHECK_ninja_version})
    find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
    set(Python3_ROOT_DIR $ENV{PYTHON3_PATH})
    find_package(Python3 ${QT_CONFIGURE_CHECK_python3_version})
    unset(Python3_ROOT_DIR)
    find_package(GPerf)
    find_package(BISON)
    find_package(FLEX)
    find_package(Perl)
    find_package(PkgConfig)
    find_package(Snappy)
    find_package(Nodejs ${QT_CONFIGURE_CHECK_nodejs_version})
    _qt_internal_sbom_verify_deps_for_generate_tag_value_spdx_document(
        OUT_VAR_DEPS_FOUND sbom_deps_found
        OUT_VAR_REASON_FAILURE_MESSAGE sbom_missing_deps_message
    )
endif()

if(PkgConfig_FOUND)
    pkg_check_modules(DBUS dbus-1)
    pkg_check_modules(FONTCONFIG fontconfig)
    pkg_check_modules(LIBDRM libdrm)
    pkg_check_modules(XCOMPOSITE xcomposite)
    pkg_check_modules(XCURSOR xcursor)
    pkg_check_modules(XI xi)
    pkg_check_modules(XRANDR xrandr)
    pkg_check_modules(XSHMFENCE xshmfence)
    pkg_check_modules(XTST xtst)
    pkg_check_modules(NSS nss>=${QT_CONFIGURE_CHECK_nss_version})
    pkg_check_modules(X11 x11)
    pkg_check_modules(XPROTO glproto)
    pkg_check_modules(GLIB glib-2.0>=${QT_CONFIGURE_CHECK_glib_version})
    pkg_check_modules(HARFBUZZ harfbuzz>=${QT_CONFIGURE_CHECK_harfbuzz_version} harfbuzz-subset>=${QT_CONFIGURE_CHECK_harfbuzz_version})
    pkg_check_modules(JPEG libjpeg IMPORTED_TARGET)
    pkg_check_modules(LIBEVENT libevent)
    pkg_check_modules(MINIZIP minizip)
    pkg_check_modules(PNG libpng>=${QT_CONFIGURE_CHECK_libpng_version})
    pkg_check_modules(TIFF libtiff-4>=${QT_CONFIGURE_CHECK_libtiff_version})
    pkg_check_modules(ZLIB zlib)
    # TODO: chromium may replace base::StringView with std::string_view. See: crbug.com/691162
    pkg_check_modules(RE2 re2>=${QT_CONFIGURE_CHECK_re2_version} IMPORTED_TARGET)
    pkg_check_modules(ICU icu-uc>=${QT_CONFIGURE_CHECK_icu_version} icu-i18n>=${QT_CONFIGURE_CHECK_icu_version})
    pkg_check_modules(WEBP libwebp libwebpmux libwebpdemux)
    pkg_check_modules(LCMS2 lcms2)
    pkg_check_modules(FREETYPE freetype2 IMPORTED_TARGET)
    pkg_check_modules(LIBXML2 libxml-2.0 libxslt IMPORTED_TARGET)
    pkg_check_modules(FFMPEG libavcodec>=${QT_CONFIGURE_CHECK_libavcodec_version}
                             libavformat>=${QT_CONFIGURE_CHECK_libavformat_version}
                             libavutil>=${QT_CONFIGURE_CHECK_libavutil_version}
                             IMPORTED_TARGET)
    pkg_check_modules(OPUS opus>=${QT_CONFIGURE_CHECK_opus_version})
    pkg_check_modules(VPX vpx>=${QT_CONFIGURE_CHECK_vpx_version} IMPORTED_TARGET)
    pkg_check_modules(LIBPCI libpci)
    pkg_check_modules(LIBOPENJP2 libopenjp2)
    pkg_check_modules(XKBCOMMON xkbcommon)
    pkg_check_modules(XKBFILE xkbfile)
    pkg_check_modules(XCBDRI3 xcb-dri3)
    pkg_check_modules(LIBUDEV libudev)
endif()

if(Python3_EXECUTABLE)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import html5lib"
        RESULT_VARIABLE html5lib_NOT_FOUND
        OUTPUT_QUIET
    )
endif()

#### Tests

if(LINUX)
   qt_webengine_configure_check_for_ulimit()
endif()

qt_config_compile_test(cxx20
    LABEL "C++20 support"
    CODE
"#if __cplusplus > 201703L
#else
#  error __cplusplus must be > 201703L
#endif
int main(void)
{
    return 0;
}
"
   CXX_STANDARD 20
)

qt_config_compile_test(re2
    LABEL "re2"
    LIBRARIES
        PkgConfig::RE2
    CODE
"
#include \"re2/filtered_re2.h\"
#include <vector>
int main() {
    std::string s;
    re2::FilteredRE2 fre2(1);
    int id = 0;
    fre2.Add(s, {}, &id);
    std::vector<std::string> pattern = {\"match\"};
    fre2.Compile(&pattern);
    const RE2 &re2 = fre2.GetRE2(id);
}"
)

qt_config_compile_test(vpx
    LABEL "vpx"
    LIBRARIES
        PkgConfig::VPX
    CODE
"
#include \"vpx/vpx_encoder.h\"
#include \"vpx/vp8cx.h\"
#include \"vpx/vpx_image.h\"
int main() {
    vpx_codec_cx_pkt pkt;
    pkt.data.frame.width[0] = 0u;
    pkt.data.frame.height[0] = 0u;
    auto a = CONSTRAINED_FROM_ABOVE_DROP;
    auto b = VPX_IMG_FMT_NV12;
    auto v9 = vpx_codec_vp9_cx();
}"
)

qt_config_compile_test(webengine_system_snappy
    LABEL "snappy"
    LIBRARIES
        Snappy::Snappy
    CODE
"
#include \"snappy.h\"
#include <string>
int main() {
    snappy::Source *src = 0;
    snappy::Sink *sink = 0;
    const char *str = \"string\";
    std::string compressed;
    snappy::Compress(str, 7, &compressed);
    return 0;
}"
)

qt_config_compile_test(jpeg
    LABEL "compatible libjpeg"
    LIBRARIES
        PkgConfig::JPEG
    CODE
"
#include <cstdio>
#include <cstring>
extern \"C\" {
    #include <jpeglib.h>
}
int main() {
    JDIMENSION dummy;
    jpeg_crop_scanline(nullptr, &dummy, &dummy);
    jpeg_skip_scanlines(nullptr, dummy);
}"
)

qt_config_compile_test(freetype
    LABEL "freetype >= 2.4.2"
    LIBRARIES
        PkgConfig::FREETYPE
    CODE
"
#include <ft2build.h>
#include FT_FREETYPE_H
#if ((FREETYPE_MAJOR*10000 + FREETYPE_MINOR*100 + FREETYPE_PATCH) < 20402)
#  error This version of freetype is too old.
#endif
int main() {
    FT_Face ft_face = 0;
    FT_Reference_Face(ft_face);
    return 0;
}"
)

qt_config_compile_test(glibc
    LABEL "glibc > 2.16"
    CODE
"
#include <features.h>
#if __GLIBC__ < 2 || __GLIBC_MINOR__ < 17
#error glibc versions below 2.17 are not supported
#endif
int main(void) {
    return 0;
}"
)

qt_config_compile_test(khr
    LABEL "khr"
    CODE
"
#include <KHR/khrplatform.h>
int main(void) {
    return 0;
}"
)

# "Unmodified ffmpeg >= 5.0 is not supported."
qt_config_compile_test(libavformat
    LABEL "libavformat"
    LIBRARIES
        PkgConfig::FFMPEG
    CODE
"
#include \"libavformat/version.h\"
extern \"C\" {
#include \"libavformat/avformat.h\"
}
int main(void) {
#if LIBAVFORMAT_VERSION_MAJOR >= 59
    AVStream stream;
    auto first_dts = av_stream_get_first_dts(&stream);
#endif
    return 0;
}"
)

#### Support Checks

qt_webengine_configure_check("compiler-cxx20"
    MODULES QtWebEngine QtPdf
    CONDITION TEST_cxx20
    MESSAGE "Missing C++20 compiler support."
    DOCUMENTATION "C++20 compiler support"
)

qt_webengine_configure_check("cmake"
    MODULES QtWebEngine QtPdf
    CONDITION CMAKE_VERSION VERSION_GREATER_EQUAL ${QT_CONFIGURE_CHECK_cmake_version}
    MESSAGE
        "Build requires CMake ${QT_CONFIGURE_CHECK_cmake_version} or higher."
    DOCUMENTATION
        "CMake version at least ${QT_CONFIGURE_CHECK_cmake_version} or higher."
)

set(targets_to_check Gui Quick Qml)
foreach(target_to_check ${targets_to_check})
    qt_webengine_configure_check("required-target-${target_to_check}"
        MODULES QtWebEngine QtPdf
        CONDITION TARGET Qt::${target_to_check}
        MESSAGE "Missing required Qt::${target_to_check}."
    )
endforeach()
unset(targets_to_check)

if(WIN32 AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR
         CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR
         CMAKE_CROSSCOMPILING))
    set(WIN_ARM_64 ON)
else()
    set(WIN_ARM_64 OFF)
endif()

qt_webengine_configure_check("supported-platform"
    MODULES QtWebEngine
    CONDITION LINUX OR (WIN32 AND NOT (WIN_ARM_64 AND DEFINED ENV{COIN_PLATFORM_ID})) OR MACOS
    MESSAGE "Build can be done only on Linux, Windows or macOS."
)
qt_webengine_configure_check("supported-platform"
    MODULES QtPdf
    CONDITION LINUX OR WIN32 OR MACOS OR IOS OR ANDROID
    MESSAGE "Build can be done only on Linux, Windows, macO, iOS and Android."
)

if(LINUX AND CMAKE_CROSSCOMPILING)
    set(supported_targets "arm" "arm64" "armv7-a" "x86_64")
    qt_webengine_configure_check("supported-arch"
        MODULES QtWebEngine QtPdf
        CONDITION TEST_architecture_arch IN_LIST supported_targets
        MESSAGE "Cross compiling is not supported for ${TEST_architecture_arch}."
    )
    unset(supported_targets)
endif()

qt_webengine_configure_check("static-build"
    MODULES QtWebEngine
    CONDITION NOT QT_FEATURE_static
    MESSAGE "Static build is not supported."
)

qt_webengine_configure_check("nodejs"
    MODULES QtWebEngine
    CONDITION TARGET Nodejs::Nodejs AND
        NOT (Nodejs_ARCH STREQUAL "ia32") AND
        NOT (Nodejs_ARCH STREQUAL "x86") AND
        NOT (Nodejs_ARCH STREQUAL "arm")
    MESSAGE "64-bit Node.js ${QT_CONFIGURE_CHECK_nodejs_version} version or later is required."
    DOCUMENTATION "64-bit Nodejs ${QT_CONFIGURE_CHECK_nodejs_version} version or later."
)
qt_webengine_configure_check("python3"
    MODULES QtWebEngine QtPdf
    CONDITION Python3_FOUND
    MESSAGE "Python ${QT_CONFIGURE_CHECK_python3_version} or later is required. Please use -DPython3_EXECUTABLE for custom path to interpreter."
    DOCUMENTATION "Python ${QT_CONFIGURE_CHECK_python3_version} version or later."
)
if(QT_GENERATE_SBOM AND QT_SBOM_GENERATE_JSON AND QT_SBOM_REQUIRE_GENERATE_JSON)
    qt_webengine_configure_check("sbom-generate-json"
        MODULES QtWebEngine QtPdf
        CONDITION sbom_deps_found
        MESSAGE
            "SBOM JSON file generation requirements missing, but JSON files were explicitly required. ${sbom_missing_deps_message}"
    )
endif()
qt_webengine_configure_check("python3-html5lib"
    MODULES QtWebEngine
    CONDITION Python3_EXECUTABLE AND NOT html5lib_NOT_FOUND
    MESSAGE "Python3 html5lib is missing (${Python3_EXECUTABLE})."
    DOCUMENTATION "Python3 html5lib module.")
qt_webengine_configure_check("gperf"
    MODULES QtWebEngine
    CONDITION GPerf_FOUND
    MESSAGE "Tool gperf is required."
    DOCUMENTATION "GNU gperf binary."
)
qt_webengine_configure_check("bison"
    MODULES QtWebEngine
    CONDITION BISON_FOUND
    MESSAGE "Tool bison is required."
    DOCUMENTATION "GNU bison binary."
)
qt_webengine_configure_check("flex"
    MODULES QtWebEngine
    CONDITION FLEX_FOUND
    MESSAGE "Tool flex is required."
    DOCUMENTATION "GNU flex binary."
)
qt_webengine_configure_check("pkg-config"
    MODULES QtWebEngine QtPdf
    CONDITION NOT LINUX OR PkgConfig_FOUND
    MESSAGE "A pkg-config support is required."
    DOCUMENTATION "A pkg-config binary on Linux."
    TAGS LINUX_PLATFORM
)
qt_webengine_configure_check("glibc"
    MODULES QtWebEngine
    CONDITION NOT LINUX OR TEST_glibc
    MESSAGE "A suitable version >= ${QT_CONFIGURE_CHECK_glibc_version} of glibc is required."
    DOCUMENTATION "glibc library at least ${QT_CONFIGURE_CHECK_glibc_version} version or later."
    TAGS LINUX_PLATFORM
)
qt_webengine_configure_check("glib"
    MODULES QtWebEngine
    CONDITION NOT UNIX OR GLIB_FOUND
    MESSAGE "No glib library at least ${QT_CONFIGURE_CHECK_glib_version} version or later. Using build-in one"
    DOCUMENTATION "glib library at least ${QT_CONFIGURE_CHECK_glib_version} version or later."
    TAGS PLATFROM_MACOS PLATFORM_LINUX
    OPTIONAL
)
qt_webengine_configure_check("harfbuzz"
    MODULES QtWebEngine QtPdf
    CONDITION NOT UNIX OR HARFBUZZ_FOUND
    MESSAGE "No harfbuzz library at least ${QT_CONFIGURE_CHECK_harfbuzz_version} version or later. Using build-in one"
    DOCUMENTATION "harfbuzz library at least ${QT_CONFIGURE_CHECK_harfbuzz_version} version or later."
    TAGS PLATFORM_MACOS PLATFORM_LINUX
    OPTIONAL
)
qt_webengine_configure_check("mesa-headers"
    MODULES QtWebEngine
    CONDITION NOT LINUX OR TEST_khr
    MESSAGE "Build requires Khronos development headers for build - see mesa/libegl1-mesa-dev"
    DOCUMENTATION "Mesa development headers."
    TAGS LINUX_PLATFORM
)
qt_webengine_configure_check("fontconfig"
    MODULES QtWebEngine
    CONDITION NOT LINUX OR FONTCONFIG_FOUND
    MESSAGE "Build requires fontconfig."
    DOCUMENTATION "Fontconfig"
    TAGS LINUX_PKG_CONFIG
)
qt_webengine_configure_check("nss"
    MODULES QtWebEngine
    CONDITION NOT LINUX OR NSS_FOUND
    MESSAGE "Build requires nss >= ${QT_CONFIGURE_CHECK_nss_version}."
    DOCUMENTATION "Nss library are least ${QT_CONFIGURE_CHECK_nss_version} version."
    TAGS LINUX_PLATFORM
)
qt_webengine_configure_check("dbus"
    MODULES QtWebEngine
    CONDITION NOT LINUX OR DBUS_FOUND
    MESSAGE "Build requires dbus."
    DOCUMENTATION "Dbus"
    TAGS LINUX_PKG_CONFIG
)
qt_webengine_configure_check("libudev"
    MODULES QtWebEngine
    CONDITION NOT UNIX OR LIBUDEV_FOUND
    MESSAGE "No libudev found."
    DOCUMENTATION "libudev library."
    TAGS PLATFROM_MACOS PLATFORM_LINUX
    OPTIONAL
)

# Only check for the 'xcb' feature if the Gui targets exists, aka Qt was not configured with
# -no-gui.
set(x_libs X11 LIBDRM XCOMPOSITE XCURSOR XRANDR XI XPROTO XSHMFENCE XTST XKBCOMMON XKBFILE XCBDRI3)
set(qpa_xcb_support_check TRUE)
foreach(x_lib ${x_libs})
    string(TOLOWER ${x_lib} x)
    qt_webengine_configure_check("${x}"
        MODULES QtWebEngine
        CONDITION NOT TARGET Qt6::Gui OR NOT LINUX OR NOT QT_FEATURE_xcb OR ${x_lib}_FOUND
        MESSAGE "Could not find ${x} library for qpa-xcb support."
        DOCUMENTATION "${x}"
        TAGS LINUX_XCB
        OPTIONAL
    )
    if(qpa_xcb_support_check AND NOT QT_CONFIGURE_CHECK_${x})
        set(qpa_xcb_support_check FALSE)
    endif()
    unset(x)
endforeach()
unset(x_libs)

qt_webengine_configure_check("compiler"
    MODULES QtWebEngine
    CONDITION MSVC OR
        (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
        (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR
        (MACOS AND CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    MESSAGE
        "${CMAKE_CXX_COMPILER_ID} compiler is not supported."
)
qt_webengine_configure_check("compiler"
    MODULES QtPdf
    CONDITION MSVC OR
        (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
        (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR
        (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR
        (ANDROID AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR
        (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
        (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    MESSAGE
        "${CMAKE_CXX_COMPILER_ID} compiler is not supported."
)
qt_webengine_configure_check("visual-studio"
    MODULES QtWebEngine QtPdf
    CONDITION NOT WIN32 OR NOT MSVC OR MSVC_TOOLSET_VERSION EQUAL 142 OR MSVC_TOOLSET_VERSION EQUAL 143
    MESSAGE "Build requires Visual Studio 2019 or higher."
    DOCUMENTATION "Visual Studio 2019 or higher."
    TAGS WINDOWS_PLATFORM
)
qt_webengine_configure_check("msvc-2019"
    MODULES QtWebEngine QtPdf
    CONDITION NOT WIN32 OR NOT MSVC OR NOT MSVC_TOOLSET_VERSION EQUAL 142 OR NOT MSVC_VERSION LESS 1929
    MESSAGE "VS compiler version must be at least 14.29"
    DOCUMENTATION "Visual Studio compiler version at least 14.29 if compiled with Visual Studio 2019"
    TAGS WINDOWS_PLATFORM
)
qt_webengine_configure_check("msvc-2022"
    MODULES QtWebEngine QtPdf
    CONDITION NOT WIN32 OR NOT MSVC OR NOT MSVC_TOOLSET_VERSION EQUAL 143 OR NOT MSVC_VERSION LESS 1936
    MESSAGE "VS compiler version must be at least 14.36"
    DOCUMENTATION "Visual Studio compiler version at least 14.36 if compiled with Visual Studio 2022"
    TAGS WINDOWS_PLATFORM
)

qt_webengine_configure_check("gcc"
    MODULES QtWebEngine QtPdf
    CONDITION NOT (LINUX OR MINGW) OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
              NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${QT_CONFIGURE_CHECK_gcc_version}
    MESSAGE "GCC version must be at least ${QT_CONFIGURE_CHECK_gcc_version}"
    DOCUMENTATION "GCC version must be at least ${QT_CONFIGURE_CHECK_gcc_version}"
    TAGS LINUX_PLATFORM
)

if(WIN32)
    qt_webengine_get_windows_sdk_version(windows_sdk_version sdk_minor)
    message("-- Windows 10 SDK version: ${windows_sdk_version}")
    unset(windows_sdk_version)
endif()

qt_webengine_configure_check("windows-sdk"
    MODULES QtWebEngine
    CONDITION NOT WIN32 OR sdk_minor GREATER_EQUAL ${QT_CONFIGURE_CHECK_windows_sdk_version}
    MESSAGE "Build requires Windows 11 SDK at least version 10.0.${QT_CONFIGURE_CHECK_windows_sdk_version}.0"
    DOCUMENTATION "Windows 11 SDK at least version 10.0.${QT_CONFIGURE_CHECK_windows_sdk_version}.0"
    TAGS WINDOWS_PLATFORM
)
unset(sdk_minor)

#### Features

qt_feature("qtwebengine-build" PUBLIC
    LABEL "Build QtWebEngine Modules"
    PURPOSE "Enables building the Qt WebEngine modules."
    CONDITION QT_CONFIGURE_CHECK_qtwebengine_build
)
qt_feature("qtwebengine-core-build" PRIVATE
    LABEL "Build QtWebEngineCore"
    PURPOSE "Enables building the Qt WebEngineCore module."
    CONDITION QT_FEATURE_qtwebengine_build
)
qt_feature("qtwebengine-widgets-build" PRIVATE
    LABEL "Build QtWebEngineWidgets"
    PURPOSE "Enables building the Qt WebEngineWidgets module."
    CONDITION TARGET Qt::Widgets AND QT_FEATURE_qtwebengine_build
)
qt_feature("qtwebengine-quick-build" PRIVATE
    LABEL "Build QtWebEngineQuick"
    PURPOSE "Enables building the Qt WebEngineQuick module."
    CONDITION TARGET Qt::Quick AND TARGET Qt::Qml AND QT_FEATURE_qtwebengine_build
)
qt_feature("qtpdf-build" PUBLIC
    LABEL "Build Qt PDF"
    PURPOSE "Enables building the Qt Pdf modules."
    CONDITION QT_CONFIGURE_CHECK_qtpdf_build
)
qt_feature("qtpdf-widgets-build" PRIVATE
    LABEL "Build QtPdfWidgets"
    PURPOSE "Enables building the Qt Pdf module."
    CONDITION TARGET Qt::Widgets AND QT_FEATURE_qtpdf_build
)
qt_feature("qtpdf-quick-build" PRIVATE
    LABEL "Build QtPdfQuick"
    PURPOSE "Enables building the QtPdfQuick module."
    CONDITION TARGET Qt::Quick AND TARGET Qt::Qml AND QT_FEATURE_qtpdf_build AND
        Qt6Quick_VERSION VERSION_GREATER_EQUAL "6.4.0"
)

if(Gn_FOUND)
    qt_webengine_is_file_inside_root_build_dir(
        Gn_INSIDE_WEBENGINE_ROOT_BUILD_DIR "${Gn_EXECUTABLE}")
endif()
qt_feature("webengine-build-gn" PRIVATE
    LABEL "Build Gn"
    AUTODETECT NOT Gn_FOUND OR Gn_INSIDE_WEBENGINE_ROOT_BUILD_DIR
)
# default assumed merge limit (should match the one in qt_cmdline.cmake)
set(jumbo_merge_limit 8)
# check value provided through configure script with -webengine-jumbo-build=(on|off|32)
if(DEFINED INPUT_webengine_jumbo_file_merge_limit)
    set(jumbo_merge_limit ${INPUT_webengine_jumbo_file_merge_limit})
# then also verify if set directly with cmake call and -DFEATURE_webengine_jumbo_build=(ON|OFF|32)
elseif(DEFINED FEATURE_webengine_jumbo_build)
    if(FEATURE_webengine_jumbo_build GREATER 0)
        set(jumbo_merge_limit ${FEATURE_webengine_jumbo_build})
    elseif (NOT FEATURE_webengine_jumbo_build)
        set(jumbo_merge_limit 0)
    endif()
endif()
set(QT_FEATURE_webengine_jumbo_file_merge_limit ${jumbo_merge_limit}
    CACHE STRING "Jumbo merge limit for WebEngineCore" FORCE)
qt_feature("webengine-jumbo-build" PUBLIC
    LABEL "Jumbo Build"
    PURPOSE "Enables support for jumbo build of core library"
    AUTODETECT FALSE
    ENABLE jumbo_merge_limit GREATER 0
)
qt_feature("webengine-developer-build" PRIVATE
    LABEL "Developer build"
    PURPOSE "Enables the developer build configuration."
    AUTODETECT QT_FEATURE_private_tests
)
qt_feature("webengine-system-re2" PRIVATE
    LABEL "re2"
    AUTODETECT FALSE
    CONDITION UNIX AND TEST_re2
)
qt_feature("webengine-system-icu" PRIVATE
    LABEL "icu"
    AUTODETECT FALSE
    CONDITION UNIX AND NOT APPLE AND ICU_FOUND
)
qt_feature("webengine-system-libwebp" PRIVATE
    LABEL "libwebp, libwebpmux and libwebpdemux"
    CONDITION UNIX AND WEBP_FOUND
)
qt_feature("webengine-system-libopenjpeg2" PRIVATE
    LABEL "libopenjpeg2"
    CONDITION UNIX AND LIBOPENJP2_FOUND
)
qt_feature("webengine-system-opus" PRIVATE
    LABEL "opus"
    CONDITION UNIX AND OPUS_FOUND
)
qt_feature("webengine-system-ffmpeg" PRIVATE
    LABEL "ffmpeg"
    AUTODETECT FALSE
    CONDITION FFMPEG_FOUND AND QT_FEATURE_webengine_system_opus AND QT_FEATURE_webengine_system_libwebp AND TEST_libavformat
)
qt_feature("webengine-system-libvpx" PRIVATE
    LABEL "libvpx"
    AUTODETECT FALSE
    CONDITION UNIX AND TEST_vpx
)
qt_feature("webengine-system-snappy" PRIVATE
    LABEL "snappy"
    CONDITION UNIX AND TEST_webengine_system_snappy
)
qt_feature("webengine-system-glib" PRIVATE
    LABEL "glib"
    CONDITION UNIX AND GLIB_FOUND
)
qt_feature("webengine-system-zlib" PRIVATE
    LABEL "zlib"
    CONDITION UNIX AND QT_FEATURE_system_zlib AND ZLIB_FOUND
)
qt_feature("webengine-qt-zlib" PRIVATE
    LABEL "qtzlib"
    CONDITION QT_FEATURE_static
        AND TARGET Qt::Gui
        AND NOT QT_FEATURE_system_zlib
)
qt_feature("webengine-system-minizip" PRIVATE
    LABEL "minizip"
    CONDITION UNIX AND MINIZIP_FOUND
)
qt_feature("webengine-system-libevent" PRIVATE
    LABEL "libevent"
    CONDITION UNIX AND LIBEVENT_FOUND
)
qt_feature("webengine-system-libxml" PRIVATE
    LABEL "libxml2 and libxslt"
    CONDITION UNIX AND LIBXML2_FOUND
)
qt_feature("webengine-system-lcms2" PRIVATE
    LABEL "lcms2"
    CONDITION UNIX AND LCMS2_FOUND
)
qt_feature("webengine-system-libpng" PRIVATE
    LABEL "png"
    CONDITION UNIX AND TARGET Qt::Gui AND PNG_FOUND AND QT_FEATURE_system_png
)
qt_feature("webengine-system-libtiff" PRIVATE
    LABEL "tiff"
    CONDITION UNIX AND TARGET Qt::Gui AND TIFF_FOUND
)
qt_feature("webengine-qt-libpng" PRIVATE
    LABEL "qtpng"
    CONDITION QT_FEATURE_static
        AND TARGET Qt::Gui
        AND QT_FEATURE_png
        AND NOT QT_FEATURE_system_png
)
qt_feature("webengine-system-libjpeg" PRIVATE
    LABEL "jpeg"
    CONDITION UNIX AND TARGET Qt::Gui AND TEST_jpeg AND QT_FEATURE_system_jpeg
)
qt_feature("webengine-qt-libjpeg" PRIVATE
    LABEL "qtjpeg"
    CONDITION QT_FEATURE_static
        AND TARGET Qt::Gui
        AND QT_FEATURE_jpeg
        AND NOT QT_FEATURE_system_jpeg
)
qt_feature("webengine-system-harfbuzz" PRIVATE
    LABEL "harfbuzz"
    CONDITION UNIX AND TARGET Qt::Gui AND HARFBUZZ_FOUND AND QT_FEATURE_system_harfbuzz
)
qt_feature("webengine-qt-harfbuzz" PRIVATE
    LABEL "qtharfbuzz"
    CONDITION QT_FEATURE_static
        AND TARGET Qt::Gui
        AND QT_FEATURE_harfbuzz
        AND NOT QT_FEATURE_system_harfbuzz
)
qt_feature("webengine-system-freetype" PRIVATE
    LABEL "freetype"
    CONDITION UNIX AND TARGET Qt::Gui AND TEST_freetype AND QT_FEATURE_system_freetype
)
qt_feature("webengine-qt-freetype" PRIVATE
    LABEL "qtfreetype"
    CONDITION QT_FEATURE_static
        AND TARGET Qt::Gui
        AND QT_FEATURE_freetype
        AND NOT QT_FEATURE_system_freetype
)
qt_feature("webengine-system-libpci" PRIVATE
    LABEL "libpci"
    CONDITION UNIX AND LIBPCI_FOUND
)

qt_feature("webengine-system-libudev" PRIVATE
    LABEL "libudev"
    CONDITION UNIX AND LIBUDEV_FOUND
)

qt_feature("webengine-ozone-x11" PRIVATE
    LABEL "Support X11 on qpa-xcb"
    CONDITION LINUX
        AND TARGET Qt::Gui
        AND QT_FEATURE_xcb
        AND qpa_xcb_support_check
)


#### Summary

# > Qt WebEngine Build Features
qt_configure_add_summary_section(NAME "WebEngine Repository Build Options")
qt_configure_add_summary_entry(ARGS "webengine-build-gn")
qt_configure_add_summary_entry(ARGS "webengine-jumbo-build")
qt_configure_add_summary_entry(ARGS "webengine-developer-build")
qt_configure_add_summary_section(NAME "Build QtWebEngine Modules")
qt_configure_add_summary_entry(ARGS "qtwebengine-core-build")
qt_configure_add_summary_entry(ARGS "qtwebengine-widgets-build")
qt_configure_add_summary_entry(ARGS "qtwebengine-quick-build")
qt_configure_end_summary_section()
qt_configure_add_summary_section(NAME "Build QtPdf Modules")
qt_configure_add_summary_entry(ARGS "qtpdf-widgets-build")
qt_configure_add_summary_entry(ARGS "qtpdf-quick-build")
qt_configure_end_summary_section()
# >> Optional system libraries
if(UNIX)
    qt_configure_add_summary_section(NAME "Optional system libraries")
    qt_configure_add_summary_entry(ARGS "webengine-system-re2")
    qt_configure_add_summary_entry(ARGS "webengine-system-icu")
    qt_configure_add_summary_entry(ARGS "webengine-system-libwebp")
    qt_configure_add_summary_entry(ARGS "webengine-system-opus")
    qt_configure_add_summary_entry(ARGS "webengine-system-ffmpeg")
    qt_configure_add_summary_entry(ARGS "webengine-system-libvpx")
    qt_configure_add_summary_entry(ARGS "webengine-system-snappy")
    qt_configure_add_summary_entry(ARGS "webengine-system-glib")
    qt_configure_add_summary_entry(ARGS "webengine-system-zlib")
    qt_configure_add_summary_entry(ARGS "webengine-system-minizip")
    qt_configure_add_summary_entry(ARGS "webengine-system-libevent")
    qt_configure_add_summary_entry(ARGS "webengine-system-libxml")
    qt_configure_add_summary_entry(ARGS "webengine-system-lcms2")
    qt_configure_add_summary_entry(ARGS "webengine-system-libpng")
    qt_configure_add_summary_entry(ARGS "webengine-system-libtiff")
    qt_configure_add_summary_entry(ARGS "webengine-system-libjpeg")
    qt_configure_add_summary_entry(ARGS "webengine-system-libopenjpeg2")
    qt_configure_add_summary_entry(ARGS "webengine-system-harfbuzz")
    qt_configure_add_summary_entry(ARGS "webengine-system-freetype")
    qt_configure_add_summary_entry(ARGS "webengine-system-libpci")
    qt_configure_add_summary_entry(ARGS "webengine-system-libudev")
    qt_configure_end_summary_section()
endif()

if(QT_FEATURE_static)
    qt_configure_add_summary_section(NAME "Qt 3rdparty libs")
    qt_configure_add_summary_entry(ARGS "webengine-qt-freetype")
    qt_configure_add_summary_entry(ARGS "webengine-qt-harfbuzz")
    qt_configure_add_summary_entry(ARGS "webengine-qt-libpng")
    qt_configure_add_summary_entry(ARGS "webengine-qt-libjpeg")
    qt_configure_add_summary_entry(ARGS "webengine-qt-zlib")
endif()

# << Optional system libraries
qt_configure_end_summary_section()
# < Qt WebEngine Build Features

qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "QtWebEngine build is disabled by user."
    CONDITION QtWebEngine_SUPPORT AND NOT QT_FEATURE_qtwebengine_build
)

qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "QtPdf build is disabled by user."
    CONDITION QtPdf_SUPPORT AND NOT QT_FEATURE_qtpdf_build
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "Building fat libray with device and simulator architectures will disable NEON."
    CONDITION IOS AND simulator AND device AND QT_FEATURE_qtpdf_build
)
if(PRINT_BFD_LINKER_WARNING)
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "Using bfd linker requires at least 4096 open files limit"
    )
endif()
if(NOT FEATURE_webengine_opus_system AND NOT Perl_FOUND)
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "No perl found, compiling opus without some optimizations."
    )
endif()
if(NOT QT_SUPERBUILD)
    qt_configure_add_report_entry(
        TYPE NOTE
        MESSAGE "\nTo build only QtPdf configure with:\n 'qt-configure-module path/to/src -- -DFEATURE_qtwebengine_build=OFF'\n"
        CONDITION QT_FEATURE_qtwebengine_build
    )

    # Note this should be last message as scaning can take a while...
    qt_configure_add_report_entry(
        TYPE NOTE
        MESSAGE "\nScanning for ide sources...\nPlease note configure can execute faster if called with:\n 'qt-configure-module path/to/src -- -DQT_SHOW_EXTRA_IDE_SOURCES=OFF'"
        CONDITION QT_SHOW_EXTRA_IDE_SOURCES OR (NOT DEFINED QT_SHOW_EXTRA_IDE_SOURCES AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.20)
    )
endif()
# Only show the warning if JSON generation is not required. For the case when it is required,
# there's an extra configure check.
if(QT_GENERATE_SBOM AND NOT QT_SBOM_REQUIRE_GENERATE_JSON)
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "Qt WebEngine And Qt Pdf SBOM generation will be skipped due to missing dependencies. ${sbom_missing_deps_message}"
        CONDITION NOT sbom_deps_found
    )
endif()
