include(linux.pri)

gn_args += \
    use_sysroot=false \
    enable_session_service=false \
    toolkit_views=false

!use_gold_linker: gn_args += use_gold=false
