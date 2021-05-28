#### Inputs

#### Libraries

find_package(Ninja 1.7.2)
find_package(Gn ${QT_REPO_MODULE_VERSION} EXACT)
find_package(Python2 2.7.5)
find_package(GPerf)
find_package(BISON)
find_package(FLEX)
find_package(Protobuf)
find_package(PkgConfig)
find_package(Snappy)
find_package(Nodejs)
find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Gui Widgets Network OpenGL OpenGLWidgets Quick Qml)

if(PkgConfig_FOUND)
    pkg_check_modules(DBUS dbus-1)
    pkg_check_modules(FONTCONFIG fontconfig)
    pkg_check_modules(LIBDRM libdrm)
    pkg_check_modules(XCOMPOSITE xcomposite)
    pkg_check_modules(XCURSOR xcursor)
    pkg_check_modules(XI xi)
    pkg_check_modules(XTST xtst)
    pkg_check_modules(NSS nss>=3.26)
    pkg_check_modules(X11 x11)
    pkg_check_modules(XPROTO glproto)
    pkg_check_modules(GLIB glib-2.0>=2.32.0)
    pkg_check_modules(HARFBUZZ harfbuzz>=2.4.0 harfbuzz-subset>=2.4.0)
    pkg_check_modules(JPEG libjpeg IMPORTED_TARGET)
    pkg_check_modules(LIBEVENT libevent)
    pkg_check_modules(MINIZIP minizip)
    pkg_check_modules(PNG libpng>=1.6.0)
    pkg_check_modules(ZLIB zlib)
    pkg_check_modules(RE2 re2 IMPORTED_TARGET)
    pkg_check_modules(ICU icu-uc>=65 icu-i18n>=65)
    pkg_check_modules(WEBP libwebp libwebpmux libwebpdemux)
    pkg_check_modules(LCMS2 lcms2)
    pkg_check_modules(FREETYPE freetype2 IMPORTED_TARGET)
    pkg_check_modules(LIBXML2 libxml-2.0 libxslt IMPORTED_TARGET)
    pkg_check_modules(FFMPEG libavcodec libavformat libavutil)
    pkg_check_modules(OPUS opus>=1.3.1)
    pkg_check_modules(VPX vpx>=1.10.0 IMPORTED_TARGET)
    pkg_check_modules(LIBPCI libpci)
endif()

#### Tests

qt_config_compile_test(re2
    LABEL "re2"
    LIBRARIES
        PkgConfig::RE2
    CODE
"
#include \"re2/filtered_re2.h\"
int main() {
    std::string s;
    re2::FilteredRE2 fre2(1);
    int id = 0;
    fre2.Add(s, {}, &id);
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
#if !defined(__clang__) && _MSC_FULL_VER < 191426428
#error unsupported Visual Studio version
#endif
int main(void){
    return 0;
}"
)

#### Features

qt_feature("qtwebengine-build" PRIVATE
    LABEL "Build Qt WebEngine"
    PURPOSE "Enables building the Qt WebEngine modules."
)
qt_feature("qtwebengine-widgets-build" PRIVATE
    LABEL "Build Qt WebEngineWidgets"
    PURPOSE "Enables building the Qt WebEngineWidgets module."
    CONDITION TARGET Qt::Widgets
)
qt_feature("qtwebengine-quick-build" PRIVATE
    LABEL "Build Qt WebEngineQuick"
    PURPOSE "Enables building the Qt WebEngineQuick module."
    CONDITION TARGET Qt::Quick AND TARGET Qt::Qml
)
qt_feature("qtpdf-build" PRIVATE
    LABEL "Build Qt PDF"
    PURPOSE "Enables building the Qt PDF rendering module."
)
qt_feature("webengine-system-ninja" PRIVATE
    LABEL "Use system ninja"
    CONDITION Ninja_FOUND
)
qt_feature("webengine-system-gn" PRIVATE
    LABEL "Use system gn"
    AUTODETECT FALSE
    CONDITION GN_Found
)
qt_feature("webengine-developer-build" PRIVATE
    LABEL "Developer build"
    PURPOSE "Enables the developer build configuration."
    AUTODETECT QT_FEATURE_private_tests
)
qt_feature("webengine-system-re2" PRIVATE
    LABEL "re2"
    AUTODETECT UNIX AND TEST_re2
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
qt_feature("webengine-system-minizip" PRIVATE
    LABEL "minizip"
    CONDITION UNIX AND MINIZIP_FOUND
)
qt_feature("webengine-system-libevent" PRIVATE
    LABEL "libevent"
    AUTODETECT FALSE # coin bug 711
    CONDITION UNIX AND LIBEVENT_FOUND
)
qt_feature("webengine-system-protobuf" PRIVATE
    LABEL "protobuf"
    CONDITION UNIX AND Protobuf_FOUND
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
qt_feature("webengine-system-libjpeg" PRIVATE
    LABEL "jpeg"
    CONDITION UNIX AND TARGET Qt::Gui AND TEST_jpeg AND QT_FEATURE_system_jpeg
)
qt_feature("webengine-system-harfbuzz" PRIVATE
    LABEL "harfbuzz"
    CONDITION UNIX AND TARGET Qt::Gui AND HARFBUZZ_FOUND AND QT_FEATURE_system_harfbuzz
)
qt_feature("webengine-system-freetype" PRIVATE
    LABEL "freetype"
    CONDITION UNIX AND TARGET Qt::Gui AND TEST_freetype AND QT_FEATURE_system_freetype
)
qt_feature("webengine-system-libpci" PRIVATE
    LABEL "libpci"
    CONDITION UNIX AND LIBPCI_FOUND
)

qt_feature("webengine-ozone-x11" PRIVATE
    LABEL "Support qpa-xcb"
    CONDITION LINUX
        AND TARGET Qt::Gui
        AND QT_FEATURE_xcb
        AND X11_FOUND
        AND LIBDRM_FOUND
        AND XCOMPOSITE_FOUND
        AND XCURSOR_FOUND
        AND XI_FOUND
        AND XPROTO_FOUND
        AND XTST_FOUND
)

#### Support Checks
if(APPLE AND NOT IOS AND
   (
    (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" AND CMAKE_OSX_ARCHITECTURES STREQUAL "")
    OR
    ("arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
   )
)
   set(MAC_ARM_64 ON)
else()
   set(MAC_ARM_64 OFF)
endif()

if(WIN32 AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_CROSSCOMPILING))
   set(WIN_ARM_64 ON)
else()
   set(WIN_ARM_64 OFF)
endif()

assertTargets(webEngineError webEngineSupport Gui Widgets OpenGL OpenGLWidgets Quick Qml)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION LINUX OR (WIN32 AND NOT WIN_ARM_64) OR (MACOS AND NOT MAC_ARM_64)
   MESSAGE "Build can be done only on Linux, Windows or macOS."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION TARGET Nodejs::Nodejs
   MESSAGE "Nodejs is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION Python2_FOUND
   MESSAGE "Python version 2 (2.7.5 or later) is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION GPerf_FOUND
   MESSAGE "Tool gperf is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION BISON_FOUND
   MESSAGE "Tool bison is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION FLEX_FOUND
   MESSAGE "Tool flex is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR PkgConfig_FOUND
   MESSAGE "A pkg-config support is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR TEST_glibc
   MESSAGE "A suitable version >= 2.17 of glibc is required."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR TEST_khr
   MESSAGE "Build requires Khronos development headers for build(see mesa/libegl1-mesa-dev)."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR FONTCONFIG_FOUND
   MESSAGE "Build requires fontconfig."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR NSS_FOUND
   MESSAGE "Build requires nss >= 3.26."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR DBUS_FOUND
   MESSAGE "Build requires dbus."
)
set(xcbSupport X11 LIBDRM XCOMPOSITE XCURSOR XI XPROTO XTST)
foreach(xs ${xcbSupport})
    if(${xs}_FOUND)
       set(xcbErrorMessage "${xcbErrorMessage} ${xs}:YES")
    else()
       set(xcbErrorMessage "${xcbErrorMessage} ${xs}:NO")
    endif()
endforeach()
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT LINUX OR NOT QT_FEATURE_xcb OR QT_FEATURE_webengine_ozone_x11
   MESSAGE "Could not find all necessary libraries for qpa-xcb support.\
${xcbErrorMessage}"
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION NOT WIN32 OR TEST_winversion
   MESSAGE "Build requires Visual Studio 2019 or higher."
)
add_check_for_support(webEngineError webEngineSupport
   MODULE QtWebEngine
   CONDITION
       (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL GNU) OR
       (LINUX AND CMAKE_CXX_COMPILER_ID STREQUAL Clang) OR
       (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL MSVC) OR
       (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL Clang AND
          CMAKE_CXX_SIMULATE_ID STREQUAL MSVC) OR
       (MACOS AND CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
   MESSAGE "${CMAKE_CXX_COMPILER_ID} compiler is not supported."
)
add_check_for_support(pdfError pdfSupport
   MODULE QtPdf
   CONDITION OFF
   MESSAGE "QtPdf is missing cmake port."
)

#### Summary

# > Qt WebEngine Build Features
qt_configure_add_summary_section(NAME "Build Features")
qt_configure_add_summary_entry(ARGS "webengine-system-ninja")
qt_configure_add_summary_entry(ARGS "webengine-system-gn")
qt_configure_add_summary_entry(ARGS "webengine-developer-build")
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
    qt_configure_add_summary_entry(ARGS "webengine-system-protobuf")
    qt_configure_add_summary_entry(ARGS "webengine-system-libxml")
    qt_configure_add_summary_entry(ARGS "webengine-system-lcms2")
    qt_configure_add_summary_entry(ARGS "webengine-system-libpng")
    qt_configure_add_summary_entry(ARGS "webengine-system-libjpeg")
    qt_configure_add_summary_entry(ARGS "webengine-system-harfbuzz")
    qt_configure_add_summary_entry(ARGS "webengine-system-freetype")
    qt_configure_add_summary_entry(ARGS "webengine-system-libpci")
    qt_configure_end_summary_section()
endif()
# << Optional system libraries
qt_configure_end_summary_section()
# < Qt WebEngine Build Features

qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "QtWebEngine build is disabled by user."
    CONDITION ${webEngineSupport} AND NOT QT_FEATURE_qtwebengine_build
)

qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "QtPdf build is disabled by user."
    CONDITION ${pdfSupport} AND NOT QT_FEATURE_qtpdf_build
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "Building fat libray with device and simulator architectures will disable NEON."
    CONDITION IOS AND simulator AND device AND QT_FEATURE_qtpdf_build
)
