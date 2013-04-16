{
  'targets': [
  {
    'target_name': 'blinq',
      'type': 'shared_library',
      'includes': [
        '../blinq.gypi',
      ],
      'sources': [
        'blinqpage.cpp',
        'blinqpage.h',
      ],
      'libraries': [
        '-lQt5Core',
        '-lQt5Gui',
      ],
      'ldflags': [
        '-L<(qt_libdir)',
        '-Wl,-rpath,<(qt_libdir)',
      ],
      'cflags': [
        '-DQT_NO_KEYWORDS',
      ],
      'include_dirs': [
        '<(qt_headers)',
        '<(qt_headers)/QtCore',
        '<(qt_headers)/QtGui',
        '<(qt_headers)/QtGui/5.2.0/QtGui',
      ],
  },
    ],
}
