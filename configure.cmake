# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_CONFIGURE_RUNNING)
    function(assertTargets)
    endfunction()
    function(add_check_for_support)
    endfunction()
    function(check_for_ulimit)
    endfunction()
else()
    find_package(Ninja 1.7.2)
    find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
    find_program(Python3_EXECUTABLE NAMES python3 python HINTS $ENV{PYTHON3_PATH})
    if(NOT Python3_EXECUTABLE)
        find_package(Python3 3.6)
    endif()
    find_package(GPerf)
    find_package(BISON)
    find_package(FLEX)
    find_package(Perl)
    find_package(PkgConfig)
    find_package(Snappy)
    find_package(Nodejs 14.0)
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
    pkg_check_modules(NSS nss>=3.26)
    pkg_check_modules(X11 x11)
    pkg_check_modules(XPROTO glproto)
    pkg_check_modules(GLIB glib-2.0>=2.32.0)
    pkg_check_modules(HARFBUZZ harfbuzz>=4.3.0 harfbuzz-subset>=4.3.0)
    pkg_check_modules(JPEG libjpeg IMPORTED_TARGET)
    pkg_check_modules(LIBEVENT libevent)
    pkg_check_modules(MINIZIP minizip)
    pkg_check_modules(PNG libpng>=1.6.0)
    pkg_check_modules(TIFF libtiff-4>=4.2.0)
    pkg_check_modules(ZLIB zlib)
    pkg_check_modules(RE2 re2 IMPORTED_TARGET)
    pkg_check_modules(ICU icu-uc>=70 icu-i18n>=70)
    pkg_check_modules(WEBP libwebp libwebpmux libwebpdemux)
    pkg_check_modules(LCMS2 lcms2)
    pkg_check_modules(FREETYPE freetype2 IMPORTED_TARGET)
    pkg_check_modules(LIBXML2 libxml-2.0 libxslt IMPORTED_TARGET)
    pkg_check_modules(FFMPEG libavcodec libavformat libavutil IMPORTED_TARGET)
    pkg_check_modules(OPUS opus>=1.3.1)
    pkg_check_modules(VPX vpx>=1.10.0 IMPORTED_TARGET)
    pkg_check_modules(LIBPCI libpci)
    pkg_check_modules(LIBOPENJP2 libopenjp2)
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
   check_for_ulimit()
endif()

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

qt_config_compile_test(snappy
    LABEL "snappy"
    LIBRARIES
        Snappy::Snappy
    CODE
"
#include \"snappy.h\"
int main() {
    snappy::Source *src = 0;
    snappy::Sink *sink = 0;
    return 0;
}"
)

qt_config_compile_test(libxml2
    LABEL "compatible libxml2 and libxslt"
    LIBRARIES
        PkgConfig::LIBXML2
    CODE
"
#include \"libxml/xmlversion.h\"
#if !defined(LIBXML_ICU_ENABLED)
#error libxml icu not enabled
#endif
int main() {
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

qt_config_compile_test(winversion
    LABEL "winversion"
    CODE
"
#if !defined(__clang__) && _MSC_VER < 1922
#error unsupported Visual Studio version
#endif
int main(void){
    return 0;
}"
)

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

#### Features

qt_feature("qtwebengine-build" PUBLIC
    LABEL "Build QtWebEngine Modules"
    PURPOSE "Enables building the Qt WebEngine modules."
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

function(qtwebengine_internal_is_file_inside_root_build_dir out_var file)
    set(result ON)
    if(NOT QT_CONFIGURE_RUNNING)
        file(RELATIVE_PATH relpath "${WEBENGINE_ROOT_BUILD_DIR}" "${file}")
        if(IS_ABSOLUTE "${relpath}" OR relpath MATCHES "^\\.\\./")
            set(result OFF)
        endif()
    endif()
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()

if(Ninja_FOUND)
    qtwebengine_internal_is_file_inside_root_build_dir(
        Ninja_INSIDE_WEBENGINE_ROOT_BUILD_DIR "${Ninja_EXECUTABLE}")
endif()
qt_feature("webengine-build-ninja" PRIVATE
    LABEL "Build Ninja"
    AUTODETECT NOT Ninja_FOUND OR Ninja_INSIDE_WEBENGINE_ROOT_BUILD_DIR
)

if(Gn_FOUND)
    qtwebengine_internal_is_file_inside_root_build_dir(
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
    CONDITION UNIX AND TEST_re2
)
qt_feature("webengine-system-icu" PRIVATE
    LABEL "icu"
    AUTODETECT FALSE
    CONDITION ICU_FOUND
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
    CONDITION FFMPEG_FOUND AND QT_FEATURE_webengine_system_opus AND QT_FEATURE_webengine_system_libwebp
)
qt_feature("webengine-system-libvpx" PRIVATE
    LABEL "libvpx"
    AUTODETECT FALSE
    CONDITION UNIX AND TEST_vpx
)
qt_feature("webengine-system-snappy" PRIVATE
    LABEL "snappy"
    CONDITION UNIX AND TEST_snappy
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
    CONDITION UNIX AND TEST_libxml2
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

qt_feature("webengine-ozone-x11" PRIVATE
    LABEL "Support GLX on qpa-xcb"
    CONDITION LINUX
        AND TARGET Qt::Gui
        AND QT_FEATURE_xcb
        AND X11_FOUND
        AND LIBDRM_FOUND
        AND XCOMPOSITE_FOUND
        AND XCURSOR_FOUND
        AND XI_FOUND
        AND XPROTO_FOUND
        AND XRANDR_FOUND
        AND XTST_FOUND
        AND XSHMFENCE_FOUND
)

#### Support Checks
if(WIN32 AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_CROSSCOMPILING))
   set(WIN_ARM_64 ON)
else()
   set(WIN_ARM_64 OFF)
endif()

add_check_for_support(
    MODULES QtWebEngine QtPdf
    CONDITION
        CMAKE_VERSION
        VERSION_GREATER_EQUAL
        ${QT_SUPPORTED_MIN_CMAKE_VERSION_FOR_BUILDING_WEBENGINE}
    MESSAGE
        "Build requires CMake ${QT_SUPPORTED_MIN_CMAKE_VERSION_FOR_BUILDING_WEBENGINE} or higher."
)

assertTargets(
   MODULES QtWebEngine QtPdf
   TARGETS Gui Quick Qml
)
add_check_for_support(
   MODULES QtWebEngine
   CONDITION LINUX OR (WIN32 AND NOT WIN_ARM_64) OR MACOS
   MESSAGE "Build can be done only on Linux, Windows or macOS."
)
add_check_for_support(
   MODULES QtPdf
   CONDITION LINUX OR (WIN32 AND NOT WIN_ARM_64) OR MACOS OR IOS OR (ANDROID AND NOT CMAKE_HOST_WIN32)
   MESSAGE "Build can be done only on Linux, Windows, macO, iOS and Android(on non-Windows hosts only)."
)
if(LINUX AND CMAKE_CROSSCOMPILING)
   get_gn_arch(testArch ${TEST_architecture_arch})
   add_check_for_support(
       MODULES QtWebEngine QtPdf
       CONDITION testArch
       MESSAGE "Cross compiling is not supported for ${TEST_architecture_arch}."
   )
endif()
add_check_for_support(
   MODULES QtWebEngine
   CONDITION NOT QT_FEATURE_static
   MESSAGE "Static build is not supported."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION TARGET Nodejs::Nodejs
   MESSAGE "node.js version 14 or later is required."
)
add_check_for_support(
    MODULES QtWebEngine
    CONDITION NOT (Nodejs_ARCH STREQUAL ia32) AND
              NOT (Nodejs_ARCH STREQUAL x86) AND
              NOT (Nodejs_ARCH STREQUAL arm)
    MESSAGE "32bit version of Nodejs is not supported."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION Python3_EXECUTABLE
   MESSAGE "Python version 3.6 or later is required."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION Python3_EXECUTABLE AND NOT html5lib_NOT_FOUND
   MESSAGE "Python3 html5lib is missing."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION GPerf_FOUND
   MESSAGE "Tool gperf is required."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION BISON_FOUND
   MESSAGE "Tool bison is required."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION FLEX_FOUND
   MESSAGE "Tool flex is required."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION NOT LINUX OR PkgConfig_FOUND
   MESSAGE "A pkg-config support is required."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION NOT LINUX OR TEST_glibc
   MESSAGE "A suitable version >= 2.17 of glibc is required."
)
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION NOT LINUX OR TEST_khr
   MESSAGE "Build requires Khronos development headers for build - see mesa/libegl1-mesa-dev"
)
add_check_for_support(
   MODULES QtWebEngine
   CONDITION NOT LINUX OR FONTCONFIG_FOUND
   MESSAGE "Build requires fontconfig."
)
add_check_for_support(
   MODULES QtWebEngine
   CONDITION NOT LINUX OR NSS_FOUND
   MESSAGE "Build requires nss >= 3.26."
)
add_check_for_support(
   MODULES QtWebEngine
   CONDITION NOT LINUX OR DBUS_FOUND
   MESSAGE "Build requires dbus."
)
add_check_for_support(
    MODULES QtWebEngine
    CONDITION NOT LINUX OR NOT QT_FEATURE_webengine_system_ffmpeg OR TEST_libavformat
    MESSAGE "Unmodified ffmpeg >= 5.0 is not supported."
)
# FIXME: This prevents non XCB Linux builds from building:
set(xcbSupport X11 LIBDRM XCOMPOSITE XCURSOR XRANDR XI XPROTO XSHMFENCE XTST)
foreach(xs ${xcbSupport})
    if(${xs}_FOUND)
       set(xcbErrorMessage "${xcbErrorMessage} ${xs}:YES")
    else()
       set(xcbErrorMessage "${xcbErrorMessage} ${xs}:NO")
    endif()
endforeach()
add_check_for_support(
   MODULES QtWebEngine
   CONDITION NOT LINUX OR NOT QT_FEATURE_xcb OR QT_FEATURE_webengine_ozone_x11
   MESSAGE "Could not find all necessary libraries for qpa-xcb support.\
${xcbErrorMessage}"
)
add_check_for_support(
   MODULES QtWebEngine
   CONDITION MSVC OR
       (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL GNU) OR
       (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL Clang) OR
       (MACOS AND CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
   MESSAGE
       "${CMAKE_CXX_COMPILER_ID} compiler is not supported."
)

add_check_for_support(
   MODULES QtPdf
   CONDITION MSVC OR
       (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL GNU) OR
       (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL Clang) OR
       (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL AppleClang) OR
       (ANDROID AND CMAKE_CXX_COMPILER_ID STREQUAL Clang) OR
       (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL GNU) OR
       (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL Clang)
   MESSAGE
       "${CMAKE_CXX_COMPILER_ID} compiler is not supported."
)
if(WIN32)
    if(MSVC)
        add_check_for_support(
            MODULES QtWebEngine QtPdf
            CONDITION NOT MSVC_VERSION LESS 1922
            MESSAGE "VS compiler version must be at least 14.22"
        )
    endif()
    set(windowsSdkVersion $ENV{WindowsSDKVersion})
    string(REGEX REPLACE "([0-9.]+).*" "\\1" windowsSdkVersion "${windowsSdkVersion}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" sdkMinor "${windowsSdkVersion}")
    message("-- Windows 10 SDK version: ${windowsSdkVersion}")
    add_check_for_support(
        MODULES QtWebEngine
        CONDITION sdkMinor GREATER_EQUAL 22621
        MESSAGE "Build requires Windows 11 SDK at least version 10.0.22621.0"
    )
endif()
add_check_for_support(
   MODULES QtWebEngine QtPdf
   CONDITION NOT MSVC OR TEST_winversion
   MESSAGE "Build requires Visual Studio 2019 or higher."
)

#### Summary

# > Qt WebEngine Build Features
qt_configure_add_summary_section(NAME "WebEngine Repository Build Options")
qt_configure_add_summary_entry(ARGS "webengine-build-ninja")
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

