include(common.pri)

gn_args += \
    is_clang=false \
    use_sysroot=false \
    use_kerberos=true \
    enable_notifications=false \
    enable_session_service=false \
    ninja_use_custom_environment_files=false \
    is_multi_dll_chrome=false \
    win_linker_timing=true

isDeveloperBuild() {
    gn_args += \
        is_win_fastlink=true \
        use_incremental_linking=true
} else {
    gn_args += \
        use_incremental_linking=false
}

msvc {
    equals(MSVC_VER, 14.0) {
        MSVS_VERSION = 2015
    } else:equals(MSVC_VER, 15.0) {
        MSVS_VERSION = 2017
    } else {
        fatal("Visual Studio compiler version \"$$MSVC_VER\" is not supported by Qt WebEngine")
    }

    gn_args += visual_studio_version=$$MSVS_VERSION

    SDK_PATH = $$(WINDOWSSDKDIR)
    VS_PATH= $$(VSINSTALLDIR)
    gn_args += visual_studio_path=$$shell_quote($$VS_PATH)
    gn_args += windows_sdk_path=$$shell_quote($$SDK_PATH)

    contains(QT_ARCH, "i386"): gn_args += target_cpu=\"x86\"

} else {
    fatal("Qt WebEngine for Windows can only be built with the Microsoft Visual Studio C++ compiler")
}
