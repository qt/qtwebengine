# Shared configuration for all our supported platforms
include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += buildtools-private webenginecore webenginecore-private

gn_args += \
    use_qt=true \
    init_stack_vars=false \
    is_component_build=false \
    is_shared=true \
    enable_debugallocation=false \
    enable_media_remoting=false \
    enable_message_center=false \
    enable_nacl=false \
    enable_remoting=false \
    enable_reporting=false \
    enable_resource_whitelist_generation=false \
    enable_swiftshader=false \
    angle_enable_swiftshader=false \
    enable_web_auth=true \
    enable_web_speech=false \
    enable_widevine=true \
    forbid_non_component_debug_builds=false \
    has_native_accessibility=false \
    safe_browsing_mode=0 \
    toolkit_views=false \
    treat_warnings_as_errors=false \
    use_allocator_shim=false \
    use_allocator=\"none\" \
    use_custom_libcxx=false

# No closure compile supported at this time
gn_args += \
    closure_compile=false \
    optimize_webui=false

# We always embed v8 startup data currently
gn_args += \
    v8_use_external_startup_data=false

# Uses special flags for clang not available on xcode, and messes up gdb debugging too.
gn_args += \
    strip_absolute_paths_from_debug_symbols=false

greaterThan(QMAKE_JUMBO_MERGE_LIMIT,0) {
    gn_args += \
        use_jumbo_build=true \
        jumbo_file_merge_limit=$$QMAKE_JUMBO_MERGE_LIMIT
}

!greaterThan(QMAKE_JUMBO_MERGE_LIMIT,8) {
    gn_args += jumbo_build_excluded="[\"browser\"]"
}

precompile_header {
    gn_args += enable_precompiled_headers=true
} else {
    gn_args += enable_precompiled_headers=false
}

CONFIG(release, debug|release):!qtConfig(webengine-developer-build) {
    gn_args += is_official_build=true
} else {
    gn_args += is_official_build=false
    !qtConfig(webengine-developer-build): gn_args += is_unsafe_developer_build=false
    # Just doesn't work in many configurations:
    gn_args += from_here_uses_location_builtins=false
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
    # MSVC requires iterator debug to always match and Qt leaves it default on.
    msvc: gn_args += enable_iterator_debugging=true

    # We also can not have optimized V8 binaries for MSVC as iterator debugging
    # would mismatch.
    msvc|v8base_debug: gn_args += v8_optimized_debug=false
}

!webcore_debug: gn_args += blink_symbol_level=0
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

ccache {
    gn_args += cc_wrapper=\"ccache\"
}

qtConfig(force_asserts): gn_args += dcheck_always_on=true
