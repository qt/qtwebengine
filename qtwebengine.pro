TEMPLATE = subdirs

CONFIG += ordered

# The first three subdirs contain dummy .pro files that are used by qmake
# to generate a corresponding .gyp file
SUBDIRS = build/qmake_extras \ # Phony pro file that extracts things like compiler and linker from qmake
          resources \
          shared \
          lib \
          process \
          build \ # This is where we use the generated qt_generated.gypi and run gyp
          # Now build the API libraries
          lib/quick

qtHaveModule(widgets) {
    SUBDIRS += \
        lib/widgets \
        tests/widgets
}

SUBDIRS += examples

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
