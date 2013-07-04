TEMPLATE = lib
GYPI_CONTENTS =  "{" \
                "  'make_global_settings': [" \
                "    ['CC', '$$which($$QMAKE_CC)']," \
                "    ['CXX', '$$which($$QMAKE_CXX)']," \
                "    ['LD', '$$which($$QMAKE_LINK)'],"
option(host_build)
GYPI_CONTENTS += "    ['CC.host', '$$which($$QMAKE_CC)']," \
                 "    ['CXX.host', '$$which($$QMAKE_CXX)']," \
                 "    ['LD.host', '$$which($$QMAKE_LINK)']," \
                 "  ]," \
                 "}"

GYPI_FILE = $$absolute_path('build/qmake_extras.gypi', $$QTWEBENGINE_ROOT)
write_file($$GYPI_FILE, GYPI_CONTENTS)

# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux
