# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

GYPI_CONTENTS += "    ['CC', '$$which($$QMAKE_CC)']," \
                 "    ['CXX', '$$which($$QMAKE_CXX)']," \
                 "    ['LD', '$$which($$QMAKE_LINK)'],"
GYPI_CONTENTS += "  ],"
GYPI_CONTENTS += "}"

GYPI_FILE = $$absolute_path('build/qmake_extras.gypi', $$QTWEBENGINE_ROOT)

!exists($$GYPI_FILE): error("-- $$GYPI not found --")

# Append to the file already containing the host settings.
!build_pass {
    write_file($$GYPI_FILE, GYPI_CONTENTS, append)
}
