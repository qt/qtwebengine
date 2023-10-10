// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

marked.setOptions({
  renderer: new marked.Renderer(),
  gfm: true,
  tables: true,
  breaks: false,
  pedantic: false,
  sanitize: false,
  smartLists: true,
  smartypants: false
});

// Poor man document.ready();
(function() {
  var placeholder = document.getElementById('placeholder');
  var content = document.getElementById('content');
  placeholder.innerHTML = marked(content.innerHTML);
})();
