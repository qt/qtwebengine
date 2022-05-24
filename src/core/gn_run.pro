include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += buildtools-private webenginecore-private core-private gui-private

TEMPLATE = aux

qtConfig(debug_and_release): CONFIG += debug_and_release
qtConfig(build_all): CONFIG += build_all

qtConfig(webengine-system-ninja) {
    QT_TOOL.ninja.binary = ninja
} else {
    QT_TOOL.ninja.binary = $$shell_quote($$shell_path($$ninjaPath()))
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
    QMAKE_EXTRA_TARGETS += runninja

    gn_args = $$gnWebEngineArgs()


    !qtConfig(webengine-system-gn) {
        gn_binary = $$system_quote($$system_path($$gnPath()))
    }

    gn_src_root = $$system_quote($$system_path($$QTWEBENGINE_ROOT/$$getChromiumSrcDir()))
    gn_python = "--script-executable=$$pythonPathForSystem()"

    ninjaflags = $$(NINJAFLAGS)
    enableThreads = $$(GN_MORE_THREADS)
    isEmpty(enableThreads):macos {
        gn_threads = "--threads=1"
    }
    isEmpty(ninjaflags):!silent: ninjaflags = "-v"
    build_pass:build_all: default_target.target = all
    else: default_target.target = first
    default_target.depends = runninja
    QMAKE_EXTRA_TARGETS += default_target

    isUniversal(){
        for(arch, QT_ARCHS) {
            gn_target = "qtwebengine_target=\"$$system_path($$OUT_PWD/$$arch/$$getConfigDir()):QtWebEngineCore\""
            gn_args_per_arch = $$system_quote($$gn_args $$gn_target target_cpu=\"$$gnArch($$arch)\")
            gn_build_root = $$system_quote($$system_path($$OUT_PWD/$$arch/$$getConfigDir()))
            gn_run = $$gn_binary gen $$gn_build_root $$gn_python $$gn_threads --args=$$gn_args_per_arch --root=$$gn_src_root
            message("Running for $$arch: $$gn_run")
            !system($$gn_run) {
                 error("GN run error for $$arch!")
            }
            runninja_$${arch}.target = run_ninja_$${arch}
            runninja_$${arch}.commands = $$NINJA $$ninjaflags \$\(NINJAJOBS\) -C $$gn_build_root QtWebEngineCore
            QMAKE_EXTRA_TARGETS += runninja_$${arch}
            runninja.depends += runninja_$${arch}
        }
    } else {
        gn_args+= "qtwebengine_target=\"$$system_path($$OUT_PWD/$$getConfigDir()):QtWebEngineCore\""
        gn_args = $$system_quote($$gn_args)
        gn_build_root = $$system_quote($$system_path($$OUT_PWD/$$getConfigDir()))
        gn_run = $$gn_binary gen $$gn_build_root $$gn_python $$gn_threads --args=$$gn_args --root=$$gn_src_root
        message("Running: $$gn_run")
        !system($$gn_run) {
            error("GN run error!")
        }
        runninja.commands = $$NINJA $$ninjaflags \$\(NINJAJOBS\) -C $$gn_build_root QtWebEngineCore
    }
}

!build_pass:debug_and_release {
    # Special GNU make target for the meta Makefile that ensures that our debug and release Makefiles won't both run ninja in parallel.
    notParallel.target = .NOTPARALLEL
    QMAKE_EXTRA_TARGETS += notParallel
}

build_pass:CONFIG(debug, debug|release) {
    TARGET = gn_run_debug
}

