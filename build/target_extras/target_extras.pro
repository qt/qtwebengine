# Prevent generating a makefile that attempts to create a lib
TEMPLATE = aux

!android_no_sdk: return

GYPI_CONTENTS =  "{" \
                 "  'target_defaults': ["

GYPI_CONTENTS += "    'target_conditions': [" \
                 "      ['_target_name==\"v8\"', {" \
                 "        'libraries!': [" \
                 "          '-lrt'," \
                 "          '-lpthread', '-lnss3', '-lnssutil3', '-lsmime3', '-lplds4', '-lplc4', '-lnspr4'," \
                 "        ]," \
                 "      }]," \
                 "      ['_target_name==\"libwebp_dsp\"', {" \
                 "        'include_dirs': [ '<(android_ndk_root)/sources/cpufeatures' ]," \
                 "        'sources': [" \
                 "          '<(android_ndk_root)/sources/android/cpufeatures/cpu-features.c'," \
                 "        ]," \
                 "      }]," \
                 "      ['_target_name==\"media\"', {" \
                 "        'include_dirs': [ '<(SHARED_INTERMEDIATE_DIR)/media', ]," \
                 "        'libraries': [" \
                 "          '-lOpenSLES'," \
                 "        ]," \
                 "      }]," \
                 "    }],"
GYPI_CONTENTS += "  ],"
GYPI_CONTENTS += "}"

GYPI_FILE = $$absolute_path('build/target_extras.gypi', $$QTWEBENGINE_ROOT)
write_file($$GYPI_FILE, GYPI_CONTENTS)
