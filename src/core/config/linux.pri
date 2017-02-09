include(common.pri)
include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
QT_FOR_CONFIG += gui-private webengine-private

use?(gn) {
    gn_args += \
        use_cups=false \
        use_gconf=false \
        use_gio=false \
        use_gnome_keyring=false \
        use_kerberos=false \
        linux_use_bundled_binutils=false

    use?(nss) {
        gn_args += \
            use_nss_certs=true \
            use_openssl_certs=false
    } else {
        gn_args += \
            use_nss_certs=false \
            use_openssl_certs=true
    }
    gcc:!clang: greaterThan(QT_GCC_MAJOR_VERSION, 5): gn_args += no_delete_null_pointer_checks=true

    host_build {
        gn_args += custom_toolchain=\"$$QTWEBENGINE_OUT_ROOT/src/toolchain:host\"
        # Don't bother trying to use system libraries in this case
        gn_args += use_glib=false
        gn_args += use_system_libffi=false
    } else {
        gn_args += custom_toolchain=\"$$QTWEBENGINE_OUT_ROOT/src/toolchain:target\"
        cross_compile {
            # FIXME: Try enabling snapshot again
            gn_args += v8_use_snapshot=false
            gn_args += host_toolchain=\"$$QTWEBENGINE_OUT_ROOT/src/toolchain:host\"
            gn_args += v8_snapshot_toolchain=\"$$QTWEBENGINE_OUT_ROOT/src/toolchain:v8_snapshot\"
            GN_HOST_CPU = $$gnArch($$QMAKE_HOST.arch)
            GN_TARGET_CPU = $$gnArch($$QT_ARCH)
            gn_args += host_cpu=\"$$GN_HOST_CPU\" target_cpu=\"$$GN_TARGET_CPU\"
        }
        !contains(QT_CONFIG, no-pkg-config) {
            # Strip '>2 /dev/null' from $$pkgConfigExecutable()
            PKGCONFIG = $$first($$list($$pkgConfigExecutable()))
            gn_args += pkg_config=\"$$PKGCONFIG\"
        }

        qtConfig(system-zlib): use?(system_minizip): gn_args += use_system_zlib=true use_system_minizip=true
        qtConfig(system-png): gn_args += use_system_libpng=true
        qtConfig(system-jpeg): gn_args += use_system_libjpeg=true
        use?(system_harfbuzz): gn_args += use_system_harfbuzz=true
        !use?(glib): gn_args += use_glib=false
        qtConfig(pulseaudio) {
            gn_args += use_pulseaudio=true
        } else {
            gn_args += use_pulseaudio=false
        }
        qtConfig(alsa) {
            gn_args += use_alsa=true
        } else {
            gn_args += use_alsa=false
        }
        packagesExist(libffi): gn_args += use_system_libffi=true
        else: gn_args += use_system_libffi=false
        !packagesExist(libpci): gn_args += use_libpci=false

        use?(system_libevent): gn_args += use_system_libevent=true
        use?(system_libwebp):  gn_args += use_system_libwebp=true
        #use?(system_libsrtp):  gn_args += use_system_libsrtp=true
        use?(system_libxslt):  gn_args += use_system_libxml=true use_system_libxslt=true
        #use?(system_jsoncpp):  gn_args += use_system_jsoncpp=true
        #use?(system_opus):     gn_args += use_system_opus=true
        use?(system_snappy):   gn_args += use_system_snappy=true
        #use?(system_vpx):      gn_args += use_system_libvpx=true
        #use?(system_icu):      gn_args += use_system_icu=true icu_use_data_file=false
        #use?(system_ffmpeg):   gn_args += use_system_ffmpeg=true
        #use?(system_protobuf): gn_args += use_system_protobuf=true
    }
}

# linux_use_bundled_gold currently relies on a hardcoded relative path from chromium/src/out/(Release|Debug)
# Disable it along with the -Wl,--threads flag just in case gold isn't installed on the system.
GYP_CONFIG += \
    linux_use_bundled_gold=0 \
    linux_use_bundled_binutils=0 \
    linux_use_gold_flags=0 \

GYP_CONFIG += \
    toolkit_uses_gtk=0 \
    use_ash=0 \
    use_aura=1 \
    use_cairo=0 \
    use_clipboard_aurax11=0 \
    use_cups=0 \
    use_gconf=0 \
    use_gio=0 \
    use_gnome_keyring=0 \
    use_kerberos=0 \
    use_pango=0 \
    use_openssl=1 \
    use_allocator=none \
    use_experimental_allocator_shim=0

use?(nss) {
    GYP_CONFIG += \
        use_nss_certs=1 \
        use_nss_verifier=1 \
        use_openssl_certs=0
} else {
    GYP_CONFIG += \
        use_nss_certs=0 \
        use_nss_verifier=0 \
        use_openssl_certs=1
}

gcc:!clang: greaterThan(QT_GCC_MAJOR_VERSION, 5): GYP_CONFIG += no_delete_null_pointer_checks=1

qtConfig(system-zlib): use?(system_minizip): GYP_CONFIG += use_system_zlib=1
qtConfig(system-png): GYP_CONFIG += use_system_libpng=1
qtConfig(system-jpeg): GYP_CONFIG += use_system_libjpeg=1
qtConfig(system-harfbuzz): use?(system_harfbuzz): GYP_CONFIG += use_system_harfbuzz=1
!qtConfig(glib): GYP_CONFIG += use_glib=0
qtConfig(pulseaudio) {
    GYP_CONFIG += use_pulseaudio=1
} else {
    GYP_CONFIG += use_pulseaudio=0
}
qtConfig(alsa) {
    GYP_CONFIG += use_alsa=1
} else {
    GYP_CONFIG += use_alsa=0
}
use?(system_libevent): GYP_CONFIG += use_system_libevent=1
use?(system_libwebp):  GYP_CONFIG += use_system_libwebp=1
use?(system_libsrtp):  GYP_CONFIG += use_system_libsrtp=1
use?(system_libxslt):  GYP_CONFIG += use_system_libxml=1
use?(system_jsoncpp):  GYP_CONFIG += use_system_jsoncpp=1
use?(system_opus):     GYP_CONFIG += use_system_opus=1
use?(system_snappy):   GYP_CONFIG += use_system_snappy=1
use?(system_vpx):      GYP_CONFIG += use_system_libvpx=1
use?(system_icu):      GYP_CONFIG += use_system_icu=1 icu_use_data_file_flag=0
use?(system_ffmpeg):   GYP_CONFIG += use_system_ffmpeg=1
use?(system_protobuf): GYP_CONFIG += use_system_protobuf=1
