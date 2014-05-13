{
  'variables': {
    'werror%': '',
    'qt_os%': '',
    'win_use_allocator_shim': 0,
    'win_release_RuntimeLibrary': 2,
    'win_debug_RuntimeLibrary': 3,
  },
  'target_defaults': {
    # patterns used to exclude chromium files from the build when we have a drop-in replacement
    'sources/': [
      ['exclude', 'base/clipboard/clipboard_android.cc$'],
      ['exclude', 'base/clipboard/clipboard_aura.cc$'],
      ['exclude', 'base/clipboard/clipboard_aurax11.cc$'],
      ['exclude', 'base/clipboard/clipboard_gtk.cc$'],
      ['exclude', 'base/clipboard/clipboard_mac.mm$'],
      ['exclude', 'base/clipboard/clipboard_win.cc$'],
      ['exclude', 'base/clipboard/clipboard_util_win\\.(cc|h)$'],
      ['exclude', 'base/dragdrop/os_exchange_data_provider_aurax11\\.(cc|h)$'],
      ['exclude', 'base/dragdrop/os_exchange_data_provider_win\\.(cc|h)$'],
      ['exclude', 'base/resource/resource_bundle_android.cc$'],
      ['exclude', 'base/resource/resource_bundle_auralinux.cc$'],
      ['exclude', 'base/resource/resource_bundle_gtk.cc$'],
      ['exclude', 'base/resource/resource_bundle_mac.mm$'],
      ['exclude', 'base/resource/resource_bundle_win.cc$'],
      ['exclude', 'browser/web_contents/web_contents_view_android\\.(cc|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_aura\\.(cc|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_mac\\.(mm|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_win\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/gtk_im_context_wrapper\\.cc$'],
      ['exclude', 'browser/renderer_host/pepper/pepper_truetype_font_list_pango\\.cc$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_android\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_aura\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_mac\\.(mm|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_win\\.(cc|h)$'],
      ['exclude', 'common/font_list_pango\\.cc$'],
    ],
    'defines': [
      'TOOLKIT_QT',
    ],
  },
  'conditions': [
    [ 'qt_os=="embedded_linux"', {
      'variables': {
        'external_ozone_platforms': [ 'eglfs', ],
      },
      'target_defaults': {
        'defines': [
            'GL_GLEXT_PROTOTYPES',
            'EGL_EGLEXT_PROTOTYPES',
            # At runtime the env variable SSL_CERT_DIR can be used to override this
            'OPENSSLDIR="/usr/lib/ssl"',
            'OPENSSL_LOAD_CONF',
        ],
        'defines!': [
            'OPENSSLDIR="/etc/ssl"',
        ],
        'target_conditions': [
          ['_type=="shared_library"', {
            'ldflags': [
              # Tell the linker to prefer symbols within the library before looking outside
              '-Wl,-shared,-Bsymbolic',
            ],
          }],
          ['_toolset=="target"', {
            'libraries': [
              '-licui18n',
              '-licuuc',
              '-licudata',
            ],
          }],
        ],
      },
    }],
  ],
}
