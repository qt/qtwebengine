# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'action_name': 'repack_resources',
  'variables': {
    'repack_path': '<(chromium_src_dir)/tools/grit/grit/format/repack.py',
    'pak_inputs': [
      '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
      '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
# FIXME: we'll probably want those as well
#      '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/webui_resources.pak',
    ],
  },
  'inputs': [
    '<(repack_path)',
    '<@(pak_inputs)',
  ],
  'outputs': [
    '<(SHARED_INTERMEDIATE_DIR)/repack/qtwebengine_resources.pak',
  ],
  'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)'],
}
