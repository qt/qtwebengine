// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "login_delegate_qt.h"

#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "extensions/buildflags/buildflags.h"
#include "services/network/public/cpp/features.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/browser/info_map.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/mime_types_handler.h"
#endif // BUILDFLAG(ENABLE_EXTENSIONS)

#include "net/url_request/url_request.h"

#include "authentication_dialog_controller.h"
#include "authentication_dialog_controller_p.h"
#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "extensions/extension_system_qt.h"
#endif // BUILDFLAG(ENABLE_EXTENSIONS)
#include "type_conversion.h"
#include "web_contents_view_qt.h"
#include "web_engine_context.h"

namespace QtWebEngineCore {

LoginDelegateQt::LoginDelegateQt(const net::AuthChallengeInfo &authInfo,
                                 content::WebContents *web_contents,
                                 GURL url,
                                 bool /*first_auth_attempt*/,
                                 LoginAuthRequiredCallback auth_required_callback)
    : content::WebContentsObserver(web_contents)
    , m_authInfo(authInfo)
    , m_url(url)
    , m_auth_required_callback(std::move(auth_required_callback))
    , m_weakFactory(this)
{
    content::GetUIThreadTaskRunner({})->PostTask(FROM_HERE,
            base::BindOnce(&LoginDelegateQt::triggerDialog, m_weakFactory.GetWeakPtr()));
}

QUrl LoginDelegateQt::url() const
{
    return toQt(m_url);
}

QString LoginDelegateQt::realm() const
{
    return QString::fromStdString(m_authInfo.realm);
}

QString LoginDelegateQt::host() const
{
    return QString::fromStdString(m_authInfo.challenger.host());
}

int LoginDelegateQt::port() const
{
    return m_authInfo.challenger.port();
}

bool LoginDelegateQt::isProxy() const
{
    return m_authInfo.is_proxy;
}

void LoginDelegateQt::triggerDialog()
{
    Q_ASSERT(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    if (!web_contents())
        return sendAuthToRequester(false, QString(), QString());

    if (isProxy()) {
        // workaround for 'ws' redefined symbols when including QNetworkProxy
        auto authentication = WebEngineContext::qProxyNetworkAuthentication(host(), port());
        if (std::get<0>(authentication))
            return sendAuthToRequester(true, std::get<1>(authentication), std::get<2>(authentication));
    }
    content::WebContentsImpl *webContents = static_cast<content::WebContentsImpl *>(web_contents());
    if (!webContents)
        return;
    WebContentsAdapterClient *client = WebContentsViewQt::from(webContents->GetView())->client();

    AuthenticationDialogControllerPrivate *dialogControllerData = new AuthenticationDialogControllerPrivate(m_weakFactory.GetWeakPtr());
    dialogControllerData->url = url();
    dialogControllerData->host = host();
    dialogControllerData->realm = realm();
    dialogControllerData->isProxy = isProxy();
    m_dialogController.reset(new AuthenticationDialogController(dialogControllerData));
    client->authenticationRequired(m_dialogController);
}

void LoginDelegateQt::sendAuthToRequester(bool success, const QString &user, const QString &password)
{
    if (!m_auth_required_callback.is_null()) {
        if (success && web_contents())
            std::move(m_auth_required_callback).Run(net::AuthCredentials(toString16(user), toString16(password)));
        else
            std::move(m_auth_required_callback).Run(absl::nullopt);
    }
}

} // namespace QtWebEngineCore
