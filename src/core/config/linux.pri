include(common.pri)

qtConfig(webengine-embedded-build) {
    gn_args += is_desktop_linux=false
}

!host_build{



    qtConfig(webengine-pulseaudio) {
        gn_args += use_pulseaudio=true
    } else {
        gn_args += use_pulseaudio=false
    }

    qtConfig(webengine-alsa) {
        gn_args += use_alsa=true
    } else {
        gn_args += use_alsa=false
    }

    !packagesExist(libpci): gn_args += use_libpci=false

    qtConfig(webengine-ozone-x11) {
        gn_args += ozone_platform_x11=true
        gn_args += use_xkbcommon=true
        packagesExist(xscrnsaver): gn_args += use_xscrnsaver=true
        qtConfig(webengine-webrtc): gn_args += rtc_use_x11=true
    }

    qtConfig(webengine-webrtc): qtConfig(webengine-webrtc-pipewire): gn_args += rtc_use_pipewire=true

    qtConfig(webengine-system-libevent): gn_args += use_system_libevent=true
    qtConfig(webengine-system-libwebp):  gn_args += use_system_libwebp=true
    qtConfig(webengine-system-libxml2):  gn_args += use_system_libxml=true use_system_libxslt=true
    qtConfig(webengine-system-opus):     gn_args += use_system_opus=true
    qtConfig(webengine-system-snappy):   gn_args += use_system_snappy=true
    qtConfig(webengine-system-libvpx):   gn_args += use_system_libvpx=true
    qtConfig(webengine-system-icu):      gn_args += use_system_icu=true icu_use_data_file=false
    qtConfig(webengine-system-ffmpeg):   gn_args += use_system_ffmpeg=true
    qtConfig(webengine-system-re2):      gn_args += use_system_re2=true
    qtConfig(webengine-system-lcms2):    gn_args += use_system_lcms2=true

    # FIXME:
    #qtConfig(webengine-system-protobuf): gn_args += use_system_protobuf=true
    #qtConfig(webengine-system-jsoncpp): gn_args += use_system_jsoncpp=true
    #qtConfig(webengine-system-libsrtp: gn_args += use_system_libsrtp=true
}
