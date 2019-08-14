TEMPLATE = aux
option(host_build)

!debug_and_release: CONFIG += release

include($$QTWEBENGINE_OUT_ROOT/src/buildtools/qtbuildtools-config.pri)
QT_FOR_CONFIG += buildtools-private

build_pass|!debug_and_release {
    !qtConfig(webengine-system-gn): CONFIG(release, debug|release) {
        buildgn.target = build_gn
        out = $$gnPath()
        out_path = $$dirname(out)
        !qtConfig(webengine-system-ninja): ninja_path = $$ninjaPath()
        else: ninja_path="ninja"
        # check if it is not already build
        !exists($$out) {
            src_3rd_party_dir = $$absolute_path("$${getChromiumSrcDir()}/../", "$$QTWEBENGINE_ROOT")
            gn_bootstrap = $$system_path($$absolute_path(gn/build/gen.py, $$src_3rd_party_dir))

            gn_gen_args = --no-last-commit-position --out-path $$out_path \
                          --cc \"$$which($$QMAKE_CC)\" --cxx \"$$which($$QMAKE_CXX)\" \
                          --ld \"$$which($$QMAKE_LINK)\"

            msvc:!clang_cl: gn_gen_args += --use-lto

            gn_configure = $$system_quote($$gn_bootstrap) $$gn_gen_args
            macos {
                gn_configure += --isysroot \"$$QMAKE_MAC_SDK_PATH\"
            }
            message($$gn_configure)
            !system("$$pythonPathForSystem() $$gn_configure") {
                error("GN generation error!")
            }
            !system("cd $$system_quote($$system_path($$out_path)) && $$ninja_path $$basename(out)" ) {
                error("GN build error!")
            }
        }
        QMAKE_DISTCLEAN += $$out
    }
}
