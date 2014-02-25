load(qt_build_config)

# Examples will only be able to find libQt5WebEngineCore.so if the proper RPATH was set on libQt5WebEngineWidgets.so
# Arch Linux disables RPATH for security reasons.
!isEmpty($$QMAKE_LFLAGS_RPATH) {
    # As long as we are a module separate from the rest of Qt, we want to unconditionally build examples.
    # Once part of Qt 5, this should be removed and we should respect the Qt wide configuration.
    QTWEBENGINE_BUILD_PARTS = $$QT_BUILD_PARTS
    QTWEBENGINE_BUILD_PARTS *= examples
}

load(qt_parts)
