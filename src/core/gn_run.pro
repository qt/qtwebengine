isQtMinimum(5, 8) {
    include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
    QT_FOR_CONFIG += webengine-private
}

TEMPLATE = aux

build_pass|!debug_and_release {

    ninja_binary = ninja
    runninja.target = run_ninja

    !qtConfig(system-ninja) {
        ninja_binary = $$shell_quote($$shell_path($$ninjaPath()))
        buildninja.target = build_ninja
        buildninja.commands = $$buildNinja()
        QMAKE_EXTRA_TARGETS += buildninja
        runninja.depends = buildninja
    }

    !qtConfig(system-gn) {
        buildgn.target = build_gn
        buildgn.commands = $$buildGn()
        !qtConfig(system-ninja): buildgn.depends = buildninja
        QMAKE_EXTRA_TARGETS += buildgn
        runninja.depends = buildgn
    }

    runninja.commands = $$ninja_binary \$\(NINJAFLAGS\) -C $$shell_quote($$OUT_PWD/$$getConfigDir())
    QMAKE_EXTRA_TARGETS += runninja

    default_target.depends = buildgn

    QMAKE_EXTRA_TARGETS += default_target
} else {
    # Special GNU make target for the meta Makefile that ensures that our debug and release Makefiles won't both run ninja in parallel.
    notParallel.target = .NOTPARALLEL
    QMAKE_EXTRA_TARGETS += notParallel
}
