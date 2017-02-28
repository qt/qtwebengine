TEMPLATE = aux
CONFIG += release

defineReplace(buildNinja) {
   out = $$ninjaPath()
   # check if it is not already build
   !exists($$out) {
       mkpath($$dirname(out))
       src_3rd_party_dir = $$absolute_path("$${getChromiumSrcDir()}/../", "$$QTWEBENGINE_ROOT")
       ninja_configure =  $$system_quote($$system_path($$absolute_path(ninja/configure.py, $$src_3rd_party_dir)))
       !system("cd $$system_quote($$system_path($$dirname(out))) && python $$ninja_configure --bootstrap") {
            error("NINJA build error!")
       }
   }
}

isQtMinimum(5, 8) {
    include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
    QT_FOR_CONFIG += webengine-private
}

!qtConfig(system-ninja) {
    buildninja.target = build_ninja
    buildninja.commands = $$buildNinja()
    QMAKE_EXTRA_TARGETS += buildninja

    default_target.target = first
    default_target.depends = buildninja
    QMAKE_EXTRA_TARGETS += default_target
}

