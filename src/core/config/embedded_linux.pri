GYP_ARGS += "-D qt_os=\"embedded_linux\" -I config/embedded_linux.gypi"

GYP_CONFIG += \
    build_ffmpegsumo=1 \
    clang=0 \
    desktop_linux=0 \
    disable_nacl=1 \
    embedded=1 \
    enable_autofill_dialog=0 \
    enable_automation=0 \
    enable_captive_portal_detection=0 \
    enable_extensions=0 \
    enable_google_now=0 \
    enable_language_detection=0 \
    enable_managed_users=0 \
    enable_plugin_installation=0 \
    enable_plugins=0 \
    enable_printing=0 \
    enable_session_service=0 \
    enable_spellcheck=0 \
    enable_task_manager=0 \
    enable_themes=0 \
    enable_webrtc=0 \
    gtest_target_type=none \
    host_clang=0 \
    notifications=0 \
    ozone_platform_dri=0 \
    ozone_platform_test=0 \
    p2p_apis=0 \
    safe_browsing=0 \
    toolkit_views=1 \
    use_ash=0 \
    use_aura=1 \
    use_cairo=0 \
    use_clipboard_aurax11=0 \
    use_cups=0 \
    use_custom_freetype=0 \
    use_gconf=0 \
    use_gio=0 \
    use_gnome_keyring=0 \
    use_kerberos=0 \
    use_libpci=0 \
    use_openssl=1 \
    use_ozone=1 \
    use_pango=0 \
    use_system_fontconfig=1 \
    use_system_icu=1 \
    icu_use_data_file_flag=0 \
    use_x11=0 \
    v8_use_snapshot=false \
    want_separate_host_toolset=1 \

contains(QT_CONFIG, system-jpeg): GYP_CONFIG += use_system_libjpeg=1
!contains(QT_CONFIG, pulseaudio): GYP_CONFIG += use_pulseaudio=0
