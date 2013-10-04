{
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
    }
  ],
}

