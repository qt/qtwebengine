/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "resource_dispatcher_host_delegate_qt.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/resource_request_info.h"
#include "net/url_request/url_request.h"

#include "authentication_dialog_controller.h"
#include "authentication_dialog_controller_p.h"
#include "type_conversion.h"
#include "web_contents_view_qt.h"

namespace QtWebEngineCore {

ResourceDispatcherHostLoginDelegateQt::ResourceDispatcherHostLoginDelegateQt(
        net::AuthChallengeInfo *authInfo,
        content::ResourceRequestInfo::WebContentsGetter web_contents_getter,
        GURL url,
        bool first_auth_attempt,
        const base::Callback<void(const base::Optional<net::AuthCredentials>&)> &auth_required_callback)
    : m_authInfo(authInfo)
    , m_url(url)
    , m_auth_required_callback(auth_required_callback)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    content::BrowserThread::PostTask(
            content::BrowserThread::UI, FROM_HERE,
            base::Bind(&ResourceDispatcherHostLoginDelegateQt::triggerDialog,
                       this,
                       web_contents_getter));
}

ResourceDispatcherHostLoginDelegateQt::~ResourceDispatcherHostLoginDelegateQt()
{
    Q_ASSERT(m_dialogController.isNull());
}

void ResourceDispatcherHostLoginDelegateQt::OnRequestCancelled()
{
    destroy();
}

QUrl ResourceDispatcherHostLoginDelegateQt::url() const
{
    return toQt(m_url);
}

QString ResourceDispatcherHostLoginDelegateQt::realm() const
{
    return QString::fromStdString(m_authInfo->realm);
}

QString ResourceDispatcherHostLoginDelegateQt::host() const
{
    return QString::fromStdString(m_authInfo->challenger.host());
}

bool ResourceDispatcherHostLoginDelegateQt::isProxy() const
{
    return m_authInfo->is_proxy;
}

void ResourceDispatcherHostLoginDelegateQt::triggerDialog(const content::ResourceRequestInfo::WebContentsGetter &webContentsGetter)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::WebContentsImpl *webContents =
            static_cast<content::WebContentsImpl *>(webContentsGetter.Run());
    if (!webContents)
        return;
    WebContentsAdapterClient *client = WebContentsViewQt::from(webContents->GetView())->client();

    AuthenticationDialogControllerPrivate *dialogControllerData = new AuthenticationDialogControllerPrivate(this);
    m_dialogController.reset(new AuthenticationDialogController(dialogControllerData));
    client->authenticationRequired(m_dialogController);
}

void ResourceDispatcherHostLoginDelegateQt::sendAuthToRequester(bool success, const QString &user, const QString &password)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (!m_auth_required_callback.is_null()) {
        if (success)
            std::move(m_auth_required_callback).Run(net::AuthCredentials(toString16(user), toString16(password)));
        else
            std::move(m_auth_required_callback).Run(base::nullopt);
    }

    destroy();
}

void ResourceDispatcherHostLoginDelegateQt::destroy()
{
    m_dialogController.reset();
    m_auth_required_callback.Reset();
}

static void LaunchURL(const GURL& url, int render_process_id,
                      const content::ResourceRequestInfo::WebContentsGetter& web_contents_getter,
                      ui::PageTransition page_transition, bool is_main_frame, bool has_user_gesture)
{
    Q_UNUSED(render_process_id);
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    content::WebContents* webContents = web_contents_getter.Run();
    if (!webContents)
        return;
    WebContentsDelegateQt *contentsDelegate = static_cast<WebContentsDelegateQt*>(webContents->GetDelegate());
    contentsDelegate->launchExternalURL(toQt(url), page_transition, is_main_frame, has_user_gesture);
}


bool ResourceDispatcherHostDelegateQt::HandleExternalProtocol(const GURL& url, content::ResourceRequestInfo* info)
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::Bind(&LaunchURL, url,
                   info->GetChildID(),
                   info->GetWebContentsGetterForRequest(),
                   info->GetPageTransition(),
                   info->IsMainFrame(),
                   info->HasUserGesture())
    );
    return true;
}

} // namespace QtWebEngineCore
