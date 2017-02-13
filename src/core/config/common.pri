# Shared configuration for all our supported platforms

use?(gn) {
    gn_args += \
        use_qt=true \
        is_component_build=false \
        enable_remoting=false \
        enable_nacl=false \
        use_experimental_allocator_shim=false \
        use_allocator=\"none\" \
        v8_use_external_startup_data=false \
        treat_warnings_as_errors=false

    use?(printing) {
        gn_args += enable_basic_printing=true enable_print_preview=true
    } else {
        gn_args += enable_basic_printing=false enable_print_preview=false
    }

    use?(pdf) {
        gn_args += enable_pdf=true
    } else {
        gn_args += enable_pdf=false
    }

    use?(pepper_plugins) {
        gn_args += enable_plugins=true enable_widevine=true
    } else {
        gn_args += enable_plugins=false enable_widevine=false
    }

    use?(spellchecker) {
        gn_args += enable_spellcheck=true
    } else {
        gn_args += enable_spellcheck=false
    }

    use?(webrtc) {
        gn_args += enable_webrtc=true
    } else {
        gn_args += enable_webrtc=false
    }
    !webcore_debug: gn_args += remove_webcore_debug_symbols=true
    !v8base_debug: gn_args += remove_v8base_debug_symbols=true

    # Compiling with -Os makes a huge difference in binary size
    contains(WEBENGINE_CONFIG, reduce_binary_size): gn_args += optimize_for_size=true

} else {
    # Trigger Qt-specific build conditions.
    GYP_CONFIG += use_qt=1
    # We do not want to ship more external binary blobs, so let v8 embed its startup data.
    GYP_CONFIG += v8_use_external_startup_data=0
    # WebSpeech requires Google API keys and adds dependencies on speex and flac.
    GYP_CONFIG += enable_web_speech=0
    # We do not use or even include the extensions
    GYP_CONFIG += enable_extensions=0

    sanitize_address: GYP_CONFIG += asan=1
    sanitize_thread: GYP_CONFIG += tsan=1
    sanitize_memory: GYP_CONFIG += msan=1
    sanitize_undefined: GYP_CONFIG += ubsan=1

    use?(printing) {
        GYP_CONFIG += enable_basic_printing=1 enable_print_preview=1
    } else {
        GYP_CONFIG += enable_basic_printing=0 enable_print_preview=0
    }

    use?(pdf) {
        GYP_CONFIG += enable_pdf=1
    } else {
        GYP_CONFIG += enable_pdf=0
    }

    use?(pepper_plugins) {
        GYP_CONFIG += enable_plugins=1 enable_widevine=1
    } else {
        GYP_CONFIG += enable_plugins=0 enable_widevine=0
    }
}

use?(webrtc): GYP_CONFIG += enable_webrtc=1
else: GYP_CONFIG += enable_webrtc=0
