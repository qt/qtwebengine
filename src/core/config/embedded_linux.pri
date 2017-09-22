include(linux.pri)

gn_args += \
    is_desktop_linux=false \
    use_gold=false \
    use_ozone=true \
    use_sysroot=false \
    enable_session_service=false \
    ozone_auto_platforms=false \
    ozone_platform_headless=true \
    ozone_platform_external=true \
    ozone_platform=\"qt\" \
    toolkit_views=false
