# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

# Pick up the host toolchain
option(host_build)

GYPI_CONTENTS = "{" \
                "  'make_global_settings': [" \
                "    ['CC.host', '$$which($$QMAKE_CC)']," \
                "    ['CXX.host', '$$which($$QMAKE_CXX)']," \
                "    ['LD.host', '$$which($$QMAKE_LINK)'],"

GYPI_FILE = $$absolute_path('build/qmake_extras.gypi', $$QTWEBENGINE_ROOT)
write_file($$GYPI_FILE, GYPI_CONTENTS)
