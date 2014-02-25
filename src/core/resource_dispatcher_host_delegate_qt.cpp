/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "resource_dispatcher_host_delegate_qt.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_request_info.h"
#include "net/url_request/url_request.h"

#include "type_conversion.h"
#include "web_contents_view_qt.h"

ResourceDispatcherHostLoginDelegateQt::ResourceDispatcherHostLoginDelegateQt(net::AuthChallengeInfo *authInfo, net::URLRequest *request)
    : m_request(request)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    content::ResourceRequestInfo::ForRequest(request)->GetAssociatedRenderView(&m_renderProcessId,  &m_renderViewId);

    m_url = toQt(request->url());
    m_realm = QString::fromStdString(authInfo->realm);
    m_isProxy = authInfo->is_proxy;
    m_host = QString::fromStdString(authInfo->challenger.ToString());

    content::BrowserThread::PostTask(
        content::BrowserThread::UI, FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegateQt::triggerDialog, this));
}

ResourceDispatcherHostLoginDelegateQt::~ResourceDispatcherHostLoginDelegateQt()
{
    // We must have called ClearLoginDelegateForRequest if we didn't receive an OnRequestCancelled.
    Q_ASSERT(!m_request);
}

void ResourceDispatcherHostLoginDelegateQt::OnRequestCancelled()
{
    m_request = 0;
}

void ResourceDispatcherHostLoginDelegateQt::triggerDialog()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::RenderViewHost* renderViewHost = content::RenderViewHost::FromID(m_renderProcessId, m_renderViewId);
    content::WebContents *webContents = content::WebContents::FromRenderViewHost(renderViewHost);
    WebContentsAdapterClient *client = WebContentsViewQt::from(webContents->GetView())->client();

    // The widgets API will ask for credentials synchronouly, keep it simple for now.
    // We'll have to figure out a mechanism to keep a ref to the ResourceDispatcherHostLoginDelegateQt
    // to avoid crashing in the OnRequestCancelled case if we want to allow the credentials to
    // come back asynchronously in the QtQuick API.
    QString user, password;
    client->authenticationRequired(m_url , m_realm , m_isProxy , m_host, &user, &password);

    bool success = !user.isEmpty() || !password.isEmpty();
    content::BrowserThread::PostTask(
        content::BrowserThread::IO, FROM_HERE,
        base::Bind(&ResourceDispatcherHostLoginDelegateQt::sendAuthToRequester, this, success, user, password));
}

void ResourceDispatcherHostLoginDelegateQt::sendAuthToRequester(bool success, const QString &user, const QString &password)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    if (!m_request)
        return;

    if (success)
        m_request->SetAuth(net::AuthCredentials(toString16(user), toString16(password)));
    else
        m_request->CancelAuth();
    content::ResourceDispatcherHost::Get()->ClearLoginDelegateForRequest(m_request);

    m_request = 0;
}

bool ResourceDispatcherHostDelegateQt::AcceptAuthRequest(net::URLRequest *, net::AuthChallengeInfo *)
{
    return true;
}

content::ResourceDispatcherHostLoginDelegate *ResourceDispatcherHostDelegateQt::CreateLoginDelegate(net::AuthChallengeInfo *authInfo, net::URLRequest *request)
{
    // ResourceDispatcherHostLoginDelegateQt is ref-counted and will be released after we called ClearLoginDelegateForRequest.
    return new ResourceDispatcherHostLoginDelegateQt(authInfo, request);
}
