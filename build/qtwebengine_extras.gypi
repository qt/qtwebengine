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
  }
}
