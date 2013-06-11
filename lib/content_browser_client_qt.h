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

#ifndef CONTENT_BROWSER_CLIENT_QT
#define CONTENT_BROWSER_CLIENT_QT

#include "content/public/browser/content_browser_client.h"

namespace net {
class URLRequestContextGetter;
}

namespace content {
class BrowserContext;
class BrowserMainParts;
class RenderProcessHost;
class RenderViewHostDelegateView;
class WebContentsViewPort;
class WebContents;
struct MainFunctionParams;
}

class BrowserContextQt;
class BrowserMainPartsQt;

class ContentBrowserClientQt : public content::ContentBrowserClient {

public:
    virtual content::WebContentsViewPort* OverrideCreateWebContentsView(content::WebContents* , content::RenderViewHostDelegateView**) /*Q_DECL_OVERRIDE*/;
    virtual content::BrowserMainParts* CreateBrowserMainParts(const content::MainFunctionParams& parameters) /*Q_DECL_OVERRIDE*/;


    BrowserContextQt* browser_context();

    net::URLRequestContextGetter *CreateRequestContext(content::BrowserContext *content_browser_context, content::ProtocolHandlerMap *protocol_handlers);

private:
    BrowserMainPartsQt* m_browserMainParts;

};

#endif // CONTENT_BROWSER_CLIENT_QT
