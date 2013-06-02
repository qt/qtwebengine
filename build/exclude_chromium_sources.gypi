{
  'target_defaults': {
    # patterns used to exclude chromium files from the build when we have a drop-in replacement
    'sources/': [
      ['exclude', 'shell/shell_gtk\\.cc$'],
      ['exclude', 'shell/shell_mac\\.mm$'],
      ['exclude', 'shell/shell_win\\.cc$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_gtk\\.(cc|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_mac.*\\.(mm|h)$'],
      ['exclude', 'browser/renderer_host/render_widget_host_view_win.*\\.(cc|h)$'],
    ],
  }
}
