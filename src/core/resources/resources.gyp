{
  'variables': {
    # Used in repack_locales
    'locales': [
      'am', 'ar', 'bg', 'bn', 'ca', 'cs', 'da', 'de', 'el', 'en-GB',
      'en-US', 'es-419', 'es', 'et', 'fa', 'fi', 'fil', 'fr', 'gu', 'he',
      'hi', 'hr', 'hu', 'id', 'it', 'ja', 'kn', 'ko', 'lt', 'lv',
      'ml', 'mr', 'ms', 'nb', 'nl', 'pl', 'pt-BR', 'pt-PT', 'ro', 'ru',
      'sk', 'sl', 'sr', 'sv', 'sw', 'ta', 'te', 'th', 'tr', 'uk',
      'vi', 'zh-CN', 'zh-TW',
    ],
  },
  'dependencies': [
      '<(chromium_src_dir)/webkit/webkit_resources.gyp:webkit_strings',
      '<(chromium_src_dir)/webkit/webkit_resources.gyp:webkit_resources',
      '<(chromium_src_dir)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
      '<(chromium_src_dir)/chrome/chrome_resources.gyp:chrome_strings',
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
      {
        'action_name': 'repack_locales',
        'includes': [ 'repack_locales.gypi' ],
      },
    ]
  }
  ]
}
