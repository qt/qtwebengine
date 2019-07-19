# Shared configuration for all our supported platforms
include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += webenginecore

gn_args += \
    use_qt=true \
    closure_compile=false \
    is_component_build=false \
    is_shared=true \
    enable_message_center=false \
    enable_mus=false \
    enable_nacl=false \
    enable_remoting=false \
    enable_reporting=false \
    enable_swiftshader=false \
    enable_web_auth=false \
    enable_web_speech=false \
    enable_widevine=true \
    has_native_accessibility=false \
    enable_debugallocation=false \
    use_allocator_shim=false \
    use_allocator=\"none\" \
    use_custom_libcxx=false \
    v8_use_external_startup_data=false \
    toolkit_views=false \
    treat_warnings_as_errors=false \
    safe_browsing_mode=0 \
    optimize_webui=false

!win32: gn_args += \
    use_jumbo_build=true \
    jumbo_file_merge_limit=8 \
    jumbo_build_excluded="[\"browser\"]"

qtConfig(webengine-printing-and-pdf) {
    gn_args += enable_basic_printing=true enable_print_preview=true
    gn_args += enable_pdf=true
} else {
    gn_args += enable_basic_printing=false enable_print_preview=false
    gn_args += enable_pdf=false
}

qtConfig(webengine-pepper-plugins) {
    gn_args += enable_plugins=true
} else {
    gn_args += enable_plugins=false
}

qtConfig(webengine-spellchecker) {
    gn_args += enable_spellcheck=true
} else {
    gn_args += enable_spellcheck=false
}

qtConfig(webengine-webrtc) {
    gn_args += enable_webrtc=true
} else {
    gn_args += enable_webrtc=false audio_processing_in_audio_service_supported=false
}

qtConfig(webengine-proprietary-codecs): gn_args += proprietary_codecs=true ffmpeg_branding=\"Chrome\"

qtConfig(webengine-extensions) {
    gn_args += enable_extensions=true
} else {
    gn_args += enable_extensions=false
}

precompile_header {
    gn_args += enable_precompiled_headers=true
} else {
    gn_args += enable_precompiled_headers=false
}

CONFIG(release, debug|release):!isDeveloperBuild() {
    gn_args += is_official_build=true
} else {
    gn_args += is_official_build=false
    !isDeveloperBuild(): gn_args += is_unsafe_developer_build=false
}

CONFIG(release, debug|release) {
    gn_args += is_debug=false
    force_debug_info {
        # Level 1 is not enough to generate all Chromium debug symbols on Windows
        msvc: gn_args += symbol_level=2
        else: gn_args += symbol_level=1
    } else {
        gn_args += symbol_level=0
    }
}

CONFIG(debug, debug|release) {
    gn_args += is_debug=true
    gn_args += use_debug_fission=false
    # MSVC requires iterator debug to always match and Qt has leaves it default on.
    msvc: gn_args += enable_iterator_debugging=true

    # We also can not have optimized V8 binaries for MSVC as iterator debugging
    # would mismatch.
    msvc|v8base_debug: gn_args += v8_optimized_debug=false
}

!webcore_debug: gn_args += remove_webcore_debug_symbols=true
!v8base_debug: gn_args += remove_v8base_debug_symbols=true

# Compiling with -Os makes a huge difference in binary size
optimize_size: gn_args += optimize_for_size=true

# We don't want to apply sanitizer options to the build tools (GN, dict convert, etc).
!host_build {
    sanitize_address: gn_args += is_asan=true
    sanitize_thread: gn_args += is_tsan=true
    sanitize_memory: gn_args += is_msan=true
    sanitize_undefined: gn_args += is_ubsan=true is_ubsan_vptr=true
}

qtConfig(webengine-v8-snapshot) {
    gn_args += v8_use_snapshot=true
} else {
    gn_args += v8_use_snapshot=false
}

qtConfig(webengine-kerberos) {
    gn_args += use_kerberos=true
} else {
    gn_args += use_kerberos=false
}

ccache {
    gn_args += cc_wrapper=\"ccache\"
}
