{
  'variables': {
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/chrome',
  },
  'targets': [
    {
      'target_name': 'chrome_qt',
      'type': 'static_library',
      'dependencies': [
          'chrome_resources',
      ],
      'include_dirs': [
        './',
        '<(chromium_src_dir)',
        '<(chromium_src_dir)/skia/config',
        '<(SHARED_INTERMEDIATE_DIR)/chrome', # Needed to include grit-generated files in localized_error.cc
      ],
      'sources': [
        '<(DEPTH)/chrome/browser/media/desktop_streams_registry.cc',
        '<(DEPTH)/chrome/browser/media/desktop_streams_registry.h',
        '<(DEPTH)/chrome/browser/media/desktop_media_list.h',
        '<(DEPTH)/chrome/common/chrome_switches.cc',
        '<(DEPTH)/chrome/common/chrome_switches.h',
        '<(DEPTH)/chrome/common/localized_error.cc',
        '<(DEPTH)/chrome/common/localized_error.h',
      ],
    },
    {
      'target_name': 'chrome_resources',
      'type': 'none',
      'actions': [
        {
          'action_name': 'generated_resources',
          'variables': {
            'grit_grd_file': '<(chromium_src_dir)/chrome/app/generated_resources.grd',
          },
          'includes': [ 'resources/grit_action.gypi' ],
        },
        {
          'action_name': 'chromium_strings.grd',
          'variables': {
            'grit_grd_file': '<(chromium_src_dir)/chrome/app/chromium_strings.grd',
          },
          'includes': [ 'resources/grit_action.gypi' ],
        },
        {
          'action_name': 'renderer_resources',
          'variables': {
            'grit_grd_file': '<(chromium_src_dir)/chrome/renderer/resources/renderer_resources.grd',
          },
          'includes': [ 'resources/grit_action.gypi' ],
        },
      ]
    },
  ],
}
