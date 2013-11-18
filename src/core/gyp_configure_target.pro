# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

GYPI_CONTENTS += "    ['CC', '$$which($$QMAKE_CC)']," \
                 "    ['CXX', '$$which($$QMAKE_CXX)']," \
                 "    ['LD', '$$which($$QMAKE_LINK)'],"
GYPI_CONTENTS += "  ],"
GYPI_CONTENTS += "}"

GYPI_FILE = $$absolute_path('qmake_extras.gypi')

!exists($$GYPI_FILE): error("-- $$GYPI_FILE not found --")

# Append to the file already containing the host settings.
!build_pass {
    write_file($$GYPI_FILE, GYPI_CONTENTS, append)
}
