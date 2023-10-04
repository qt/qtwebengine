# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#### Libraries

if(NOT QT_CONFIGURE_RUNNING)
    find_package(GLIB2 COMPONENTS GIO)
    find_package(GSSAPI)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(ALSA alsa IMPORTED_TARGET)
        pkg_check_modules(PULSEAUDIO libpulse>=0.9.10 libpulse-mainloop-glib)
        pkg_check_modules(XDAMAGE xdamage)
        pkg_check_modules(POPPLER_CPP poppler-cpp IMPORTED_TARGET)
        pkg_check_modules(GBM gbm)
        pkg_check_modules(LIBVA libva)
        if(NOT GIO_FOUND)
            pkg_check_modules(GIO gio-2.0)
        endif()
    endif()
    find_package(Cups)

    find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET
        OPTIONAL_COMPONENTS Positioning WebChannel PrintSupport)
endif()

#### Tests

qt_config_compile_test(poppler
    LABEL "poppler"
    LIBRARIES
        PkgConfig::POPPLER_CPP
    CODE
"
#include <poppler-document.h>

int main() {
   auto *pdf = poppler::document::load_from_raw_data(\"file\",100,std::string(\"user\"));
}"
)

qt_config_compile_test(alsa
    LABEL "alsa"
    LIBRARIES
        PkgConfig::ALSA
    CODE
"
#include \"alsa/asoundlib.h\"
#if SND_LIB_VERSION < 0x1000a  // 1.0.10
#error Alsa version found too old, require >= 1.0.10
#endif
int main(){};
")

#### Features

qt_feature("webengine-embedded-build" PRIVATE
    LABEL "Embedded build"
    PURPOSE "Enables the embedded build configuration."
    AUTODETECT CMAKE_CROSSCOMPILING
    CONDITION UNIX
)
qt_feature("webengine-system-alsa" PRIVATE
    LABEL "Use ALSA"
    CONDITION UNIX AND TEST_alsa
)
qt_feature("webengine-v8-context-snapshot" PRIVATE
    LABEL "Use v8 context snapshot"
    CONDITION NOT CMAKE_CROSSCOMPILING
)
qt_feature("webengine-geolocation" PUBLIC
    LABEL "Geolocation"
    CONDITION TARGET Qt::Positioning
)
qt_feature("webengine-system-pulseaudio" PRIVATE
    LABEL "Use PulseAudio"
    AUTODETECT UNIX
    CONDITION PULSEAUDIO_FOUND
)
qt_feature("webengine-printing-and-pdf" PRIVATE
    LABEL "Printing and PDF"
    PURPOSE "Provides printing and output to PDF."
    AUTODETECT NOT QT_FEATURE_webengine_embedded_build
    CONDITION TARGET Qt::PrintSupport AND QT_FEATURE_printer AND
    (CUPS_FOUND OR WIN32)
)
qt_feature("webengine-pepper-plugins" PRIVATE
    LABEL "Pepper Plugins"
    PURPOSE "Enables use of Pepper plugins."
    AUTODETECT QT_FEATURE_webengine_printing_and_pdf
)
qt_feature("webengine-webchannel" PUBLIC
    SECTION "WebEngine"
    LABEL "WebChannel support"
    PURPOSE "Provides QtWebChannel integration."
    CONDITION TARGET Qt::WebChannel
)
qt_feature("webengine-proprietary-codecs" PRIVATE
    SECTION "WebEngine"
    LABEL "Proprietary Codecs"
    PURPOSE "Enables the use of proprietary codecs such as h.264/h.265 and MP3."
    AUTODETECT OFF
)
qt_feature("webengine-kerberos" PRIVATE
    SECTION "WebEngine"
    LABEL "Kerberos Authentication"
    PURPOSE "Enables Kerberos Authentication Support"
    AUTODETECT WIN32
    CONDITION NOT LINUX OR GSSAPI_FOUND
)
qt_feature("webengine-spellchecker" PUBLIC
    LABEL "Spellchecker"
    PURPOSE "Provides a spellchecker."
)
qt_feature("webengine-native-spellchecker" PUBLIC
    LABEL "Native Spellchecker"
    PURPOSE "Use the system's native spellchecking engine."
    AUTODETECT OFF
    CONDITION QT_FEATURE_webengine_spellchecker AND NOT LINUX
)
qt_feature("webengine-extensions" PUBLIC
    SECTION "WebEngine"
    LABEL "Extensions"
    PURPOSE "Enables Chromium extensions within certain limits. Currently used by the pdf viewer and hangout webrtc extension."
    AUTODETECT ON
    CONDITION QT_FEATURE_webengine_printing_and_pdf OR QT_FEATURE_webengine_webrtc
)
qt_feature("webengine-webrtc" PRIVATE
    LABEL "WebRTC"
    PURPOSE "Provides WebRTC support."
    AUTODETECT NOT QT_FEATURE_webengine_embedded_build
    CONDITION XDAMAGE_FOUND OR NOT QT_FEATURE_webengine_ozone_x11
)
qt_feature("webengine-webrtc-pipewire" PRIVATE
    LABEL "PipeWire over GIO"
    PURPOSE "Provides PipeWire support in WebRTC using GIO."
    AUTODETECT false
    CONDITION QT_FEATURE_webengine_webrtc AND GIO_FOUND
)
qt_feature("webengine-full-debug-info" PRIVATE
    SECTION "WebEngine"
    LABEL "Full debug information"
    PURPOSE "Enables debug information for Blink and V8."
    AUTODETECT OFF
    CONDITION CMAKE_BUILD_TYPE STREQUAL Debug OR Debug IN_LIST CMAKE_CONFIGURATION_TYPES OR
              CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo OR RelWithDebInfo IN_LIST CMAKE_CONFIGURATION_TYPES
)
qt_feature("webengine-sanitizer" PRIVATE
    SECTION "WebEngine"
    LABEL "Sanitizer support"
    PURPOSE "Enables support for build with sanitizers"
    AUTODETECT CLANG
    CONDITION CLANG AND ECM_ENABLE_SANITIZERS
)
qt_feature("webengine-vulkan" PRIVATE
    SECTION "WebEngine"
    LABEL "Vulkan support"
    PURPOSE "Enables support for Vulkan rendering"
    CONDITION QT_FEATURE_vulkan
)
qt_feature("webengine-vaapi" PRIVATE
    SECTION "WebEngine"
    LABEL "VA-API support"
    PURPOSE "Enables support for VA-API hardware acceleration"
    AUTODETECT GBM_FOUND AND LIBVA_FOUND AND QT_FEATURE_vulkan
    # hardware accelerated encoding requires bundled libvpx
    CONDITION LINUX AND NOT QT_FEATURE_webengine_system_libvpx
)
# internal testing feature
qt_feature("webengine-system-poppler" PRIVATE
    LABEL "popler"
    CONDITION UNIX AND TEST_poppler
)
qt_configure_add_summary_section(NAME "Qt WebEngineCore")
qt_configure_add_summary_entry(ARGS "webengine-embedded-build")
qt_configure_add_summary_entry(ARGS "webengine-full-debug-info")
qt_configure_add_summary_entry(ARGS "webengine-sanitizer")
qt_configure_add_summary_entry(ARGS "webengine-pepper-plugins")
qt_configure_add_summary_entry(ARGS "webengine-printing-and-pdf")
qt_configure_add_summary_entry(ARGS "webengine-proprietary-codecs")
qt_configure_add_summary_entry(ARGS "webengine-spellchecker")
qt_configure_add_summary_entry(
    ARGS "webengine-native-spellchecker"
    CONDITION NOT LINUX
)
qt_configure_add_summary_entry(ARGS "webengine-webrtc")
qt_configure_add_summary_entry(ARGS "webengine-webrtc-pipewire")
qt_configure_add_summary_entry(ARGS "webengine-geolocation")
qt_configure_add_summary_entry(ARGS "webengine-webchannel")
qt_configure_add_summary_entry(ARGS "webengine-kerberos")
qt_configure_add_summary_entry(ARGS "webengine-extensions")
qt_configure_add_summary_entry(
    ARGS "webengine-ozone-x11"
    CONDITION UNIX
)
qt_configure_add_summary_entry(
    ARGS "webengine-vulkan"
    CONDITION QT_FEATURE_vulkan
)
qt_configure_add_summary_entry(
    ARGS "webengine-vaapi"
    CONDITION LINUX
)
qt_configure_add_summary_entry(
    ARGS "webengine-system-alsa"
    CONDITION LINUX
)
qt_configure_add_summary_entry(
    ARGS "webengine-system-pulseaudio"
    CONDITION LINUX
)
qt_configure_add_summary_entry(ARGS "webengine-v8-context-snapshot")
qt_configure_end_summary_section() # end of "Qt WebEngineCore" section
if(CMAKE_CROSSCOMPILING)
    check_thumb(armThumb)
    qt_configure_add_report_entry(
        TYPE WARNING
        MESSAGE "Thumb instruction set is required to build ffmpeg for QtWebEngine."
        CONDITION LINUX
            AND NOT QT_FEATURE_webengine_system_ffmpeg
            AND TEST_architecture_arch MATCHES arm
            AND NOT armThumb
   )
endif()
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "WebRTC requires XDamage with qpa_xcb."
    CONDITION QT_FEATURE_webengine_ozone_x11 AND NOT XDAMAGE_FOUND
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "VA-API is incompatible with system libvpx."
    CONDITION QT_FEATURE_webengine_system_libvpx AND QT_FEATURE_webengine_vaapi
)
