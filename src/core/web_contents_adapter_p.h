/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WEB_CONTENTS_ADAPTER_P_H
#define WEB_CONTENTS_ADAPTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "web_contents_adapter.h"

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include <QExplicitlySharedDataPointer>

QT_FORWARD_DECLARE_CLASS(QWebChannel)

class WebEngineContext;

namespace QtWebEngineCore {

class BrowserContextAdapter;
class QtRenderViewObserverHost;
class UserScriptControllerHost;
class WebChannelIPCTransportHost;
class WebContentsAdapterClient;
class WebContentsDelegateQt;

class WebContentsAdapterPrivate {
public:
    WebContentsAdapterPrivate();
    ~WebContentsAdapterPrivate();
    scoped_refptr<WebEngineContext> engineContext;
    QExplicitlySharedDataPointer<BrowserContextAdapter> browserContextAdapter;
    scoped_ptr<content::WebContents> webContents;
    scoped_ptr<WebContentsDelegateQt> webContentsDelegate;
    scoped_ptr<QtRenderViewObserverHost> renderViewObserverHost;
    scoped_ptr<WebChannelIPCTransportHost> webChannelTransport;
    QWebChannel *webChannel;
    WebContentsAdapterClient *adapterClient;
    quint64 nextRequestId;
    int lastFindRequestId;
};

} // namespace QtWebEngineCore

#endif // WEB_CONTENTS_ADAPTER_P_H
