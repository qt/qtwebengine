isQtMinimum(5, 8) {
    include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
    QT_FOR_CONFIG += webengine-private
}

TEMPLATE = aux

defineReplace(runGn) {
    message("Running: $$1")
    !system($$1) {
        error("GN run error!")
    }
}

qtConfig(debug_and_release): CONFIG += debug_and_release build_all

qtConfig(system-ninja) {
    QT_TOOL.ninja.binary = ninja
} else {
    QT_TOOL.ninja.binary = $$ninjaPath()
}

win32 {
    # Add the gnuwin32/bin subdir of qt5.git to PATH. Needed for calling bison and friends.
    gnuwin32path.name = PATH
    gnuwin32path.value = $$shell_path($$clean_path($$QTWEBENGINE_ROOT/../gnuwin32/bin))
    gnuwin32path.CONFIG += prepend
    exists($$gnuwin32path.value): QT_TOOL_ENV = gnuwin32path
}

qtPrepareTool(NINJA, ninja)
QT_TOOL_ENV =

build_pass|!debug_and_release {
    gn_binary = gn

    runninja.target = run_ninja
    rungn.target = run_gn

    gn_args = $$gnArgs()

    CONFIG(release, debug|release) {
        gn_args += is_debug=false
    }

    gn_args += "qtwebengine_target=\"$$shell_path($$OUT_PWD/$$getConfigDir()):QtWebEngineCore\""

    !qtConfig(system-gn) {
        gn_binary = $$shell_quote($$shell_path($$gnPath()))
    }

    gn_args = $$shell_quote($$gn_args)
    gn_src_root = $$shell_quote($$shell_path($$QTWEBENGINE_ROOT/$$getChromiumSrcDir()))
    gn_build_root = $$shell_quote($$shell_path($$OUT_PWD/$$getConfigDir()))
    rungn.commands = $$runGn($$gn_binary gen $$gn_build_root --args=$$gn_args --root=$$gn_src_root)
    QMAKE_EXTRA_TARGETS += rungn

    runninja.commands = $$NINJA \$\(NINJAFLAGS\) -C $$shell_quote($$OUT_PWD/$$getConfigDir()) QtWebEngineCore
    runninja.depends += rungn
    QMAKE_EXTRA_TARGETS += runninja

    build_pass:build_all: default_target.target = all
    else: default_target.target = first
    default_target.depends = runninja
    QMAKE_EXTRA_TARGETS += default_target
}

!build_pass:debug_and_release {
    # Special GNU make target for the meta Makefile that ensures that our debug and release Makefiles won't both run ninja in parallel.
    notParallel.target = .NOTPARALLEL
    QMAKE_EXTRA_TARGETS += notParallel
}
