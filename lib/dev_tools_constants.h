/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

// FIXME: This is a hack until we figure out where to put this and/or where the embedder should tell us to find it

#ifndef DEV_TOOLS_CONSTANTS_H
#define DEV_TOOLS_CONSTANTS_H

const char* DISCOVERY_PAGE_HTML =
"<html>"
"<head>"
"<title>Content shell remote debugging</title>"
"<style>"
"</style>"
""
"<script>"
"function onLoad() {"
"  var tabs_list_request = new XMLHttpRequest();"
"  tabs_list_request.open('GET', '/json/list?t=' + new Date().getTime(), true);"
"  tabs_list_request.onreadystatechange = onReady;"
"  tabs_list_request.send();"
"}"
""
"function onReady() {"
"  if(this.readyState == 4 && this.status == 200) {"
"    if(this.response != null)"
"      var responseJSON = JSON.parse(this.response);"
"      for (var i = 0; i < responseJSON.length; ++i)"
"        appendItem(responseJSON[i]);"
"  }"
"}"
""
"function appendItem(item_object) {"
"  var frontend_ref;"
"  if (item_object.devtoolsFrontendUrl) {"
"    frontend_ref = document.createElement('a');"
"    frontend_ref.href = item_object.devtoolsFrontendUrl;"
"    frontend_ref.title = item_object.title;"
"  } else {"
"    frontend_ref = document.createElement('div');"
"    frontend_ref.title = 'The tab already has active debugging session';"
"  }"
""
"  var text = document.createElement('div');"
"  if (item_object.title)"
"    text.innerText = item_object.title;"
"  else"
"    text.innerText = '(untitled tab)';"
"  text.style.cssText = 'background-image:url(' + item_object.faviconUrl + ')';"
"  frontend_ref.appendChild(text);"
""
"  var item = document.createElement('p');"
"  item.appendChild(frontend_ref);"
""
"  document.getElementById('items').appendChild(item);"
"}"
"</script>"
"</head>"
"<body onload='onLoad()'>"
"  <div id='caption'>Inspectable WebContents</div>"
"  <div id='items'></div>"
"</body>"
"</html>"
"";

#endif // DEV_TOOLS_CONSTANTS_H
