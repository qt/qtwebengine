TEMPLATE = aux
option(host_build)

!debug_and_release: CONFIG += release

include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
QT_FOR_CONFIG += webengine webengine-private

build_pass|!debug_and_release {
    !qtConfig(webengine-system-gn): CONFIG(release, debug|release) {
        buildgn.target = build_gn
        gn_args = $$gnArgs()
        out = $$gnPath()
        !qtConfig(webengine-system-ninja): ninja_path = "--path $$ninjaPath()"
        # check if it is not already build
        !exists($$out) {
            mkpath($$dirname(out))
            src_3rd_party_dir = $$absolute_path("$${getChromiumSrcDir()}/../", "$$QTWEBENGINE_ROOT")
            gn_bootstrap = $$system_path($$absolute_path(chromium/tools/gn/bootstrap/bootstrap.py, $$src_3rd_party_dir))

            gn_args = $$system_quote($$gn_args)
            win32:isDeveloperBuild() {
              # GN is always built in release mode, which conflicts with incremental linking when
              # doing a developer build of WebEngine.
              gn_args = $$replace(gn_args, "use_incremental_linking=true ", "")
            }

            gn_configure = $$system_quote($$gn_bootstrap) --shadow --gn-gen-args=$$gn_args $$ninja_path
            !system("cd $$system_quote($$system_path($$dirname(out))) && $$pythonPathForSystem() $$gn_configure") {
                error("GN build error!")
            }
        }
        QMAKE_DISTCLEAN += $$out
    }
}
