# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

TOOLCHAIN_INCLUDES = $${QMAKE_INCDIR_EGL} $${INCLUDEPATH} $${QMAKE_INCDIR}

GYPI_CONTENTS =  "{" \
                 "  'target_defaults': {" \
                 "    'target_conditions': [" \
                 "      ['_toolset==\"target\"', {" \
                 "        'include_dirs': ["
for(includes, TOOLCHAIN_INCLUDES) {
    GYPI_CONTENTS += "          '$$includes',"
}
GYPI_CONTENTS += "        ]," \
                 "        'cflags': ["
for(cflag, QT_CFLAGS_DBUS) {
    GYPI_CONTENTS += "          '$$cflag',"
}
GYPI_CONTENTS += "        ]," \
                 "      }]," \
                 "    ]," \
                 "  }," \
                 "}"

GYPI_FILE = $$absolute_path('build/target_extras.gypi', $$QTWEBENGINE_ROOT)
!build_pass: write_file($$GYPI_FILE, GYPI_CONTENTS)
