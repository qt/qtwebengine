// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef LOGIN_DELEGATE_QT_H
#define LOGIN_DELEGATE_QT_H

#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/login_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "url/gurl.h"

#include "web_contents_adapter_client.h"

namespace net {
class AuthChallengeInfo;
class AuthCredentials;
}

namespace QtWebEngineCore {

class AuthenticationDialogController;

class LoginDelegateQt : public content::LoginDelegate,
                        public content::WebContentsObserver
{
public:
    LoginDelegateQt(const net::AuthChallengeInfo &authInfo,
                    content::WebContents *web_contents,
                    GURL url,
                    bool first_auth_attempt,
                    LoginAuthRequiredCallback auth_required_callback);

    QUrl url() const;
    QString realm() const;
    QString host() const;
    int port() const;
    bool isProxy() const;

    void sendAuthToRequester(bool success, const QString &user, const QString &password);

private:
    void triggerDialog();

    net::AuthChallengeInfo m_authInfo;

    GURL m_url;
    LoginAuthRequiredCallback m_auth_required_callback;
    base::WeakPtrFactory<LoginDelegateQt> m_weakFactory;

    // This member is used to keep authentication dialog controller alive until
    // authorization is sent or cancelled.
    QSharedPointer<AuthenticationDialogController> m_dialogController;
};

} // namespace QtWebEngineCore

#endif // LOGIN_DELEGATE_QT_H
