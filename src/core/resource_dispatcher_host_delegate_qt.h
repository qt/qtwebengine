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

#ifndef RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H
#define RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H

#include "content/public/browser/resource_dispatcher_host_delegate.h"
#include "content/public/browser/resource_dispatcher_host_login_delegate.h"

#include "web_contents_adapter_client.h"

class ResourceDispatcherHostLoginDelegateQt : public content::ResourceDispatcherHostLoginDelegate {
public:
    ResourceDispatcherHostLoginDelegateQt(net::AuthChallengeInfo *authInfo, net::URLRequest *request);
    ~ResourceDispatcherHostLoginDelegateQt();

    // ResourceDispatcherHostLoginDelegate implementation
    virtual void OnRequestCancelled();

private:
    void triggerDialog();
    void sendAuthToRequester(bool success, const QString &user, const QString &password);

    QUrl m_url;
    QString m_realm;
    bool m_isProxy;
    QString m_host;

    int m_renderProcessId;
    int m_renderViewId;

    // The request that wants login data.
    // Must only be accessed on the IO thread.
    net::URLRequest *m_request;
};

class ResourceDispatcherHostDelegateQt : public content::ResourceDispatcherHostDelegate {
public:
    virtual bool AcceptAuthRequest(net::URLRequest *request, net::AuthChallengeInfo *authInfo) Q_DECL_OVERRIDE;
    virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(net::AuthChallengeInfo *authInfo, net::URLRequest *request) Q_DECL_OVERRIDE;
};

#endif // RESOURCE_DISPATCHER_HOST_DELEGATE_QT_H
