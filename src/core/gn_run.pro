isQtMinimum(5, 8) {
    include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
    QT_FOR_CONFIG += webengine-private
}

TEMPLATE = aux

build_pass|!debug_and_release {

    macos: include(config/mac_osx.pri)
    linux: include(config/desktop_linux.pri)
    isEmpty(gn_args): error(No gn_args found please make sure you have valid configuration.)

    ninja_binary = ninja
    gn_binary = gn

    runninja.target = run_ninja
    rungn.target = run_gn

    !qtConfig(system-ninja) {
        ninja_binary = $$shell_quote($$shell_path($$ninjaPath()))
        buildninja.target = build_ninja
        buildninja.commands = $$buildNinja()
        QMAKE_EXTRA_TARGETS += buildninja
        runninja.depends = buildninja
    }

    CONFIG(release, debug|release):
        gn_args += is_debug=false

    gn_args += "qtwebengine_target=\"$$shell_path($$OUT_PWD/$$getConfigDir()):QtWebEngineCore\""

    !qtConfig(system-gn) {
        gn_binary = $$shell_quote($$shell_path($$gnPath()))
        buildgn.target = build_gn
        buildgn.commands = $$buildGn($$gn_args)
        !qtConfig(system-ninja) {
            buildgn.depends = buildninja
            rungn.depends = buildninja
        }
        QMAKE_EXTRA_TARGETS += buildgn
    }

    gn_args = $$shell_quote($$gn_args)
    gn_src_root = $$shell_quote($$shell_path($$QTWEBENGINE_ROOT/$$getChromiumSrcDir()))
    gn_build_root = $$shell_quote($$shell_path($$OUT_PWD/$$getConfigDir()))
    rungn.commands = $$gn_binary gen $$gn_build_root --args=$$gn_args --root=$$gn_src_root
    QMAKE_EXTRA_TARGETS += rungn

    runninja.commands = $$ninja_binary \$\(NINJAFLAGS\) -v -C $$shell_quote($$OUT_PWD/$$getConfigDir()) QtWebEngineCore
    runninja.depends += rungn
    QMAKE_EXTRA_TARGETS += runninja

    build_pass:build_all: default_target.target = all
    else: default_target.target = first
    default_target.depends = runninja

    QMAKE_EXTRA_TARGETS += default_target
} else {
    # Special GNU make target for the meta Makefile that ensures that our debug and release Makefiles won't both run ninja in parallel.
    notParallel.target = .NOTPARALLEL
    QMAKE_EXTRA_TARGETS += notParallel
}
