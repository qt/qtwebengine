/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginebrowsercontext_p.h"

#include "browser_context_adapter.h"
#include <qtwebenginecoreglobal.h>

QT_BEGIN_NAMESPACE

QWebEngineBrowserContext::QWebEngineBrowserContext(QSharedPointer<QtWebEngineCore::BrowserContextAdapter> browserContext,
                                                   QtWebEngineCore::BrowserContextAdapterClient *profile)
        : QObject(QtWebEngineCore::BrowserContextAdapter::globalQObjectRoot())
        , browserContextRef(browserContext)
        , m_profile(profile)
{
    browserContextRef->addClient(m_profile);
}

QWebEngineBrowserContext::~QWebEngineBrowserContext()
{
    if (m_profile)
        shutdown();
}

void QWebEngineBrowserContext::shutdown()
{
    Q_ASSERT(m_profile);
    // In the case the user sets this profile as the parent of the interceptor
    // it can be deleted before the browser-context still referencing it is.
    browserContextRef->setRequestInterceptor(nullptr);
    browserContextRef->removeClient(m_profile);
    m_profile = 0;
    deleteLater();
}

QT_END_NAMESPACE
