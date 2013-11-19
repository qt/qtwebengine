{
  'target_defaults': {
    # patterns used to exclude chromium files from the build when we have a drop-in replacement
    'sources/': [
      ['exclude', 'base/resource/resource_bundle_gtk.cc$'],
      ['exclude', 'base/resource/resource_bundle_mac.mm$'],
      ['exclude', 'base/resource/resource_bundle_win.cc$'],
      ['exclude', 'browser/web_contents/web_contents_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_mac\\.(mm|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_win\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/gtk_im_context_wrapper\\.cc$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_mac\\.(mm|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_win\\.(cc|h)$'],

      # QNX-specific excludes
      ['exclude', 'base/resource/resource_bundle_qnx.cc$'],
      ['exclude', 'browser/qnx/'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_qnx\\.(cc|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_qnx\\.(cc|h)$'],
    ],
  },
  'conditions': [
    [ 'OS=="linux" and qt_cross_compile==1', {
      'target_defaults': {
        'defines': [
          'TOOLKIT_QT',
        ],
        'target_conditions': [
          ['_toolset=="target"', {
            'ldflags!': ['-L/usr/lib'], # garbage added by icu-config
            'conditions': [
              [ '_target_name=="gl"', {
                'defines': [
                  'GL_GLEXT_PROTOTYPES',
                  'EGL_EGLEXT_PROTOTYPES',
                ],
              }],
              ['_type=="shared_library"', {
                'ldflags': [
                  # Tell the linker to prefer symbols within the library before looking outside
                  '-Wl,-shared,-Bsymbolic',
                ],
              }],
            ],
          }],
        ],
      },
    }],
  ],
}
