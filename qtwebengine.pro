TEMPLATE = subdirs

CONFIG += ordered

# The first three subdirs contain dummy .pro files that are used by qmake
# to generate a corresponding .gyp file
SUBDIRS = shared \
          lib \
          process \
          build \ # This is where we use the generated qt_generated.gypi and run gyp
          example \

# Extra targets that invoke ninja on the desired configuration added for convenience
release.target = release
release.commands = $$CHROMIUM_SRC_DIR/../depot_tools/ninja -C $$getOutDir()/Release
release.depends: qmake

debug.target = debug
debug.commands = $$CHROMIUM_SRC_DIR/../depot_tools/ninja -C $$getOutDir()/Debug
debug.depends: qmake

QMAKE_EXTRA_TARGETS += release \
                       debug
