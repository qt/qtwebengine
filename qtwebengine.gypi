{
  'variables': {
    'blink_process_product_name': 'Blink Process',
# Define used when building the user agent. Pass as recent enough chrome with an irrealistic minor version.
      'user_agent_version': '22.42.5.2',
      'conditions': [
        ['OS=="linux"', {
          'use_custom_freetype%': 1,
        }, {
          'use_custom_freetype%': 0,
        }],
        ],
  },
# Needed to get access to content::GetContentClient()
    'defines': ['CONTENT_IMPLEMENTATION'],
    'dependencies': [
      '<(chromium_src_dir)/content/content.gyp:content',
      '<(chromium_src_dir)/content/content.gyp:content_app',
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
      '<(chromium_src_dir)/webkit/support/webkit_support.gyp:webkit_resources',
      '<(chromium_src_dir)/webkit/support/webkit_support.gyp:webkit_support',
      '<(chromium_src_dir)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
      '<(chromium_src_dir)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit_test_support',
    ],
    'include_dirs': [
      '<(qtwebengine_src_dir)',
      '<(chromium_src_dir)',
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
    ['OS=="win"', {
      'resource_include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)/webkit',
      ],
      'dependencies': [
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_strings',
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
    ['os_posix==1 and linux_use_tcmalloc==1', {
      'dependencies': [
# This is needed by content/app/content_main_runner.cc
        '<(chromium_src_dir)/base/allocator/allocator.gyp:allocator',
      ],
    }],
    ['use_aura==1', {
      'dependencies': [
        '<(chromium_src_dir)/ui/aura/aura.gyp:aura',
        '<(chromium_src_dir)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(chromium_src_dir)/ui/views/controls/webview/webview.gyp:webview',
        '<(chromium_src_dir)/ui/views/views.gyp:views',
        '<(chromium_src_dir)/ui/views/views.gyp:views_test_support',
        '<(chromium_src_dir)/ui/ui.gyp:ui_resources',
      ],
    }],  # use_aura==1
  ['use_custom_freetype==1', {
    'dependencies': [
      '<(chromium_src_dir)/third_party/freetype2/freetype2.gyp:freetype2',
    ],
  }],
    ],
}
