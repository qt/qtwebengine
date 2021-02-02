#### Libraries

find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(ALSA alsa IMPORTED_TARGET)
    pkg_check_modules(PULSEAUDIO libpulse>=0.9.10 libpulse-mainloop-glib)
    pkg_check_modules(GIO gio-2.0)
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Positioning WebChannel PrintSupport)

#### Tests

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
    AUTODETECT tests.webengine-embedded-build OR FIXME
    CONDITION UNIX
)
qt_feature("webengine-system-alsa" PRIVATE
    LABEL "Use ALSA"
    CONDITION UNIX AND TEST_alsa
)
qt_feature("webengine-v8-snapshot-support" PRIVATE
    LABEL "Building v8 snapshot supported"
    CONDITION NOT UNIX OR NOT QT_FEATURE_cross_compile OR ( TEST_architecture_arch STREQUAL arm64 ) OR TEST_webengine_host_compiler
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
qt_feature("webengine-pepper-plugins" PRIVATE
    LABEL "Pepper Plugins"
    PURPOSE "Enables use of Pepper Flash plugins."
    AUTODETECT NOT QT_FEATURE_webengine_embedded_build
)
qt_feature("webengine-printing-and-pdf" PRIVATE
    LABEL "Printing and PDF"
    PURPOSE "Provides printing and output to PDF."
    AUTODETECT NOT QT_FEATURE_webengine_embedded_build
    CONDITION TARGET Qt::PrintSupport AND QT_FEATURE_printer
)
qt_feature("webengine-webchannel" PUBLIC
    SECTION "WebEngine"
    LABEL "WebChannel support"
    PURPOSE "Provides QtWebChannel integration."
    CONDITION TARGET Qt::WebChannel
)
qt_feature("webengine-proprietary-codecs" PRIVATE
    LABEL "Proprietary Codecs"
    PURPOSE "Enables the use of proprietary codecs such as h.264/h.265 and MP3."
    AUTODETECT OFF
)
qt_feature("webengine-kerberos" PRIVATE
    SECTION "WebEngine"
    LABEL "Kerberos Authentication"
    PURPOSE "Enables Kerberos Authentication Support"
    AUTODETECT WIN32
)
qt_feature("webengine-spellchecker" PUBLIC
    LABEL "Spellchecker"
    PURPOSE "Provides a spellchecker."
    AUTODETECT OFF
)
qt_feature("webengine-native-spellchecker" PUBLIC
    LABEL "Native Spellchecker"
    PURPOSE "Use the system's native spellchecking engine."
    AUTODETECT OFF
    CONDITION MACOS AND QT_FEATURE_webengine_spellchecker
)
qt_feature("webengine-extensions" PUBLIC
    SECTION "WebEngine"
    LABEL "Extensions"
    PURPOSE "Enables Chromium extensions within certain limits. Currently used for enabling the pdf viewer."
    AUTODETECT QT_FEATURE_webengine_printing_and_pdf
    CONDITION QT_FEATURE_webengine_printing_and_pdf
)
qt_feature("webengine-webrtc" PRIVATE
    LABEL "WebRTC"
    PURPOSE "Provides WebRTC support."
    AUTODETECT OFF
)
qt_feature("webengine-webrtc-pipewire" PRIVATE
    LABEL "PipeWire over GIO"
    PURPOSE "Provides PipeWire support in WebRTC using GIO."
    AUTODETECT false
    CONDITION QT_FEATURE_webengine_webrtc AND GIO_FOUND
)
qt_feature_config("webengine-full-debug-info" QMAKE_PRIVATE_CONFIG
    NAME "v8base_debug"
)
qt_feature_config("webengine-full-debug-info" QMAKE_PRIVATE_CONFIG
    NAME "webcore_debug"
)
qt_configure_add_summary_section(NAME "Qt WebEngineCore")
qt_configure_add_summary_entry(ARGS "webengine-embedded-build")
qt_configure_add_summary_entry(ARGS "webengine-full-debug-info")
qt_configure_add_summary_entry(ARGS "webengine-pepper-plugins")
qt_configure_add_summary_entry(ARGS "webengine-printing-and-pdf")
qt_configure_add_summary_entry(ARGS "webengine-proprietary-codecs")
qt_configure_add_summary_entry(ARGS "webengine-spellchecker")
qt_configure_add_summary_entry(ARGS "webengine-native-spellchecker")
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
    ARGS "webengine-v8-snapshot-support"
    CONDITION UNIX AND cross_compile
)
qt_configure_add_summary_entry(
    ARGS "webengine-system-alsa"
    CONDITION UNIX
)
qt_configure_add_summary_entry(
    ARGS "webengine-system-pulseaudio"
    CONDITION UNIX
)
qt_configure_end_summary_section() # end of "Qt WebEngineCore" section
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "Thumb instruction set is required to build ffmpeg for QtWebEngine."
    CONDITION LINUX AND QT_FEATURE_webengine_embedded_build AND NOT QT_FEATURE_webengine_system_ffmpeg AND ( TEST_architecture_arch STREQUAL arm ) AND NOT QT_FEATURE_webengine_arm_thumb
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "V8 snapshot cannot be built. Most likely, the 32-bit host compiler does not work. Please make sure you have 32-bit devel environment installed."
    CONDITION UNIX AND cross_compile AND NOT QT_FEATURE_webengine_v8_snapshot_support
)
