
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

qtConfig(webengine-proprietary-codecs) {
    gn_args += proprietary_codecs=true ffmpeg_branding=\"Chrome\"
    qtConfig(webengine-webrtc) {
        gn_args += rtc_use_h264=true
    }
} else {
    gn_args += proprietary_codecs=false
}

qtConfig(webengine-extensions) {
    gn_args += enable_extensions=true
} else {
    gn_args += enable_extensions=false
}

qtConfig(webengine-kerberos) {
    gn_args += use_kerberos=true
} else {
    gn_args += use_kerberos=false
}

qtConfig(webengine-nodejs) {
    gn_args += have_nodejs=true
} else {
    gn_args += have_nodejs=false
}
