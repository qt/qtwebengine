{
  'dependencies': [
      '<(chromium_src_dir)/webkit/webkit_resources.gyp:webkit_resources',
      '<(chromium_src_dir)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
  ],
  'targets': [
  {
    'target_name': 'qtwebengine_resources',
    'type': 'none',
    'actions' : [
      {
        'action_name': 'repack_resources',
        'includes': [ 'repack_resources.gypi' ],
      },
    ]
  }
  ]
}
