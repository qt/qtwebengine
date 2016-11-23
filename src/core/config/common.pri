# Shared configuration for all our supported platforms

use?(gn) {

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
