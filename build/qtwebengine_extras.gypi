{
  'target_defaults': {
    # patterns used to exclude chromium files from the build when we have a drop-in replacement
    'sources/': [
      ['exclude', 'browser/web_contents/web_contents_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_mac\\.(mm|h)$'],
      ['exclude', 'browser/web_contents/web_contents_view_win\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/gtk_im_context_wrapper\\.cc$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_mac.*\\.(mm|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_win.*\\.(cc|h)$'],
    ],
  }
}
