{
  'variables': {
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/chrome',
  },
  'targets': [
    {
      'target_name': 'chrome_qt',
      'type': 'static_library',
      'include_dirs': [
        '<(chromium_src_dir)',
        '<(chromium_src_dir)/skia/config',
      ],
      'sources': [
        '<(chromium_src_dir)/chrome/browser/media/desktop_streams_registry.h',
        '<(chromium_src_dir)/chrome/browser/media/desktop_streams_registry.cc',
        '<(chromium_src_dir)/chrome/browser/media/desktop_media_list.h',
      ],
    },
    {
      'target_name': 'chrome_strings',
      'type': 'none',
      'actions': [
        {
          'action_name': 'generated_resources',
          'variables': {
            'grit_grd_file': '<(chromium_src_dir)/chrome/app/generated_resources.grd',
          },
          'includes': [ 'resources/grit_action.gypi' ],
#          'direct_dependent_settings': {
#            'include_dirs': [
#              '<(grit_out_dir)',
#            ],
#          },
        },
      ]
    },
  ],
}
