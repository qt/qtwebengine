GYP_ARGS += "-D qt_os=\"desktop_linux\""

GYP_CONFIG += \
    desktop_linux=1 \
    toolkit_uses_gtk=0 \
    use_aura=1 \
    use_ash=0 \
    use_cairo=0 \
    use_clipboard_aurax11=0 \
    use_cups=0 \
    use_gconf=0 \
    use_gio=0 \
    use_gnome_keyring=0 \
    use_kerberos=0 \
    use_pango=0 \
    host_clang=0 \
    clang=0 \
    enable_plugins=1 \

config_system_libxslt: GYP_CONFIG += use_system_libxml=1
contains(QT_CONFIG, system-zlib): config_system_minizip: GYP_CONFIG += use_system_zlib=1
contains(QT_CONFIG, system-png): GYP_CONFIG += use_system_libpng=1
contains(QT_CONFIG, system-jpeg): GYP_CONFIG += use_system_libjpeg=1

!contains(QT_CONFIG, pulseaudio): GYP_CONFIG += use_pulseaudio=0
