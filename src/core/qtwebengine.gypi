{
    # This asks gyp to generate a .pri file with linking
    # information so that qmake can take care of the deployment.
    'let_qmake_do_the_linking': 1,
    'dependencies': [
      '<(chromium_src_dir)/content/content.gyp:content',
      '<(chromium_src_dir)/content/content.gyp:content_app_browser',
      '<(chromium_src_dir)/content/content.gyp:content_browser',
      '<(chromium_src_dir)/content/content.gyp:content_common',
      '<(chromium_src_dir)/content/content.gyp:content_gpu',
      '<(chromium_src_dir)/content/content.gyp:content_renderer',
      '<(chromium_src_dir)/content/content.gyp:content_utility',
      '<(chromium_src_dir)/content/content.gyp:content_worker',
      '<(chromium_src_dir)/content/content_resources.gyp:content_resources',
      '<(chromium_src_dir)/base/base.gyp:base',
      '<(chromium_src_dir)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
      '<(chromium_src_dir)/ipc/ipc.gyp:ipc',
      '<(chromium_src_dir)/media/media.gyp:media',
      '<(chromium_src_dir)/net/net.gyp:net',
      '<(chromium_src_dir)/net/net.gyp:net_resources',
      '<(chromium_src_dir)/skia/skia.gyp:skia',
      '<(chromium_src_dir)/ui/gl/gl.gyp:gl',
      '<(chromium_src_dir)/ui/ui.gyp:ui',
      '<(chromium_src_dir)/url/url.gyp:url_lib',
      '<(chromium_src_dir)/v8/tools/gyp/v8.gyp:v8',
      '<(chromium_src_dir)/webkit/glue/webkit_glue.gyp:*',
      '<(chromium_src_dir)/third_party/WebKit/Source/web/web.gyp:webkit',
    ],
    'include_dirs': [
      '<(chromium_src_dir)',
      '<(SHARED_INTERMEDIATE_DIR)/net', # Needed to include grit/net_resources.h
    ],
    # Chromium code defines those in common.gypi, do the same for our code that include Chromium headers.
    'defines': [
      '__STDC_CONSTANT_MACROS',
      '__STDC_FORMAT_MACROS',
    ],
    'msvs_settings': {
      'VCLinkerTool': {
        'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
      },
    },
    'conditions': [
      ['OS=="win" and win_use_allocator_shim==1', {
        'dependencies': [
          '<(chromium_src_dir)/base/allocator/allocator.gyp:allocator',
        ],
      }],
      ['_toolset=="target" and qt_os=="android"', {
        'configurations': {
          'Debug_Base': {
            # Reduce the binary size.
            'variables': {
              'debug_optimize%': 's',
            },
            'ldflags': [
              # Only link with needed input sections.
              '-Wl,--gc-sections',
              # Warn in case of text relocations.
              '-Wl,--warn-shared-textrel',
              '-Wl,-O1',
              '-Wl,--as-needed',
            ],
            'cflags': [
              '-fomit-frame-pointer',
              '-fdata-sections',
              '-ffunction-sections',
            ],
          },
        },
        'dependencies': [
          '<(chromium_src_dir)/third_party/ashmem/ashmem.gyp:ashmem',
          '<(chromium_src_dir)/third_party/freetype/freetype.gyp:ft2',
          '<(chromium_src_dir)/third_party/android_tools/ndk/android_tools_ndk.gyp:cpu_features',
        ],
      }],
      ['OS=="win"', {
        'resource_include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/webkit',
        ],
        'configurations': {
          'Debug_Base': {
            'msvs_settings': {
              'VCLinkerTool': {
                'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
              },
            },
          },
        },
        # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
        'msvs_disabled_warnings': [ 4267, ],
      }],  # OS=="win"
      ['OS=="linux"', {
        'dependencies': [
          '<(chromium_src_dir)/build/linux/system.gyp:fontconfig',
        ],
      }],
    ],
}
