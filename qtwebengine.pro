TEMPLATE = subdirs

# The first three subdirs contain dummy .pro files that are used by qmake
# to generate a corresponding .gyp file

# Phony pro file that extracts things like compiler and linker from qmake
qmake_extras.subdir = build/qmake_extras

# Phony pro files that generate gyp files. Will be built by ninja.
shared.depends = qmake_extras
lib.depends = qmake_extras

# API libraries
webengine_lib.subdir = lib/quick
webengine_lib.target = sub-webengine-lib
webengine_lib.depends = build
quick_plugin_lib.subdir = lib/quick/plugin
quick_plugin_lib.target = sub-quick-plugin-lib
quick_plugin_lib.depends = webengine_lib
quick_experimental_plugin_lib.subdir = lib/quick/plugin/experimental
quick_experimental_plugin_lib.target = sub-quick-experimental-plugin-lib
quick_experimental_plugin_lib.depends = webengine_lib
widgets_lib.subdir = lib/widgets
widgets_lib.target = sub-widgets-lib
widgets_lib.depends = build

process.depends = build
sub_examples.depends = quick_plugin_lib quick_experimental_plugin_lib
sub_tests.depends = quick_plugin_lib quick_experimental_plugin_lib

# This is where we use the generated gypi files and run gyp_qtwebengine
build.depends = resources shared lib

SUBDIRS += qmake_extras \
          resources \
          shared \
          lib \
          process \
          build \
          webengine_lib \
          quick_plugin_lib \
          quick_experimental_plugin_lib

qtHaveModule(widgets) {
    SUBDIRS += widgets_lib
    sub_examples.depends += widgets_lib
    sub_tests.depends += widgets_lib
}

# Ninja executable location needs to be determined early for extra targets. Should be fetched from cache most of the time anyway.
NINJA_EXECUTABLE = $$findOrBuildNinja()

# Extra targets that invoke ninja on the desired configuration added for convenience
release.target = release
release.commands = $$NINJA_EXECUTABLE -C $$getOutDir()/Release
release.depends: qmake

debug.target = debug
debug.commands = $$NINJA_EXECUTABLE -C $$getOutDir()/Debug
debug.depends: qmake

QMAKE_EXTRA_TARGETS += release \
                       debug

# Move this to the beginning of the project file as soon as we moved to the src layout
load(qt_parts)
