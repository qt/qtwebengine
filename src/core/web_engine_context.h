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

#ifndef WEB_ENGINE_CONTEXT_H
#define WEB_ENGINE_CONTEXT_H

#include "qtwebenginecoreglobal.h"

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include <QExplicitlySharedDataPointer>

namespace base {
class RunLoop;
}

namespace content {
class BrowserMainRunner;
class ContentMainRunner;
}

QT_FORWARD_DECLARE_CLASS(QObject)

namespace QtWebEngineCore {

class BrowserContextAdapter;
class ContentMainDelegateQt;
class DevToolsHttpHandlerDelegateQt;
class SurfaceFactoryQt;
} // namespace

class WebEngineContext : public base::RefCounted<WebEngineContext> {
public:
    static scoped_refptr<WebEngineContext> current();

    QtWebEngineCore::BrowserContextAdapter *defaultBrowserContext();
    QObject *globalQObject();

    void destroyBrowserContext();
    void destroy();

private:
    friend class base::RefCounted<WebEngineContext>;
    WebEngineContext();
    ~WebEngineContext();

    scoped_ptr<base::RunLoop> m_runLoop;
    scoped_ptr<QtWebEngineCore::ContentMainDelegateQt> m_mainDelegate;
    scoped_ptr<content::ContentMainRunner> m_contentRunner;
    scoped_ptr<content::BrowserMainRunner> m_browserRunner;
    QObject* m_globalQObject;
    QExplicitlySharedDataPointer<QtWebEngineCore::BrowserContextAdapter> m_defaultBrowserContext;
    scoped_ptr<QtWebEngineCore::DevToolsHttpHandlerDelegateQt> m_devtools;
};

#endif // WEB_ENGINE_CONTEXT_H
