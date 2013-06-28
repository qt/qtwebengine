{
  'target_defaults': {
    # patterns used to exclude chromium files from the build when we have a drop-in replacement
    'sources/': [
      ['exclude', 'browser/renderer_host/gtk_im_context_wrapper\\.cc$'],
    ],
  }
}
